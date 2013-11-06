#pragma warning (disable : 4530) // complaint about exception handler

#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <array>
#include <assert.h>
#include <cstdlib>
#include <sstream>
#include "SceneImporter.h"



enum VariableType{
	kTypeUndefined,
	kTypeModel,
	kTypeModelGroup,
	kTypeString,
	kTypeObject,
	kTypeNumber,
	kTypeVec3,
	kTypeCount
};
std::string g_TypeNames[] = {
	"Undefined",
	"Model",
	"ModelGroup",
	"string",
	"object",
	"number",
	"vec3"
};



struct Vec3{
	Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z){}
	float x, y, z;
};
struct Variable;
struct Object{
	std::map<std::string, Variable*> keys;
};
const unsigned int kStackData = 1024*1024;
std::array<char, kStackData> dataStack;
int dataStackPtr;
void* stackMalloc(int size){
	assert(dataStackPtr + size < (int)dataStack.max_size());
	void* ret =  &dataStack[dataStackPtr];
	dataStackPtr += size;
	return ret;
}
std::vector<Model*> models;
struct Variable* pushVariable();
struct Variable{
	VariableType type;
	void* data;

	Variable() : type(kTypeUndefined), data(nullptr){}
	bool convert(const Variable* in){
		switch(type){
		case kTypeModel:
			if(in->type != kTypeString) return false;
			if(data == nullptr){
				void* pos = stackMalloc(sizeof(Model));
				data = new(pos) Model;
				models.push_back((Model*)data);
			}
			reinterpret_cast<Model*>(data)->name = std::string(reinterpret_cast<char*>(in->data));
			return true;
		case kTypeModelGroup: {
			if(in->type != kTypeObject) return false;
			const Object* obj = reinterpret_cast<const Object*>(in->data);
			if(data == nullptr){
				void* pos = stackMalloc(sizeof(Group));
				data = new(pos) Group;
			}
			Group* g = reinterpret_cast<Group*>(data);
			
			auto lightIt = obj->keys.find("light");
			if(lightIt != obj->keys.end() && lightIt->second->type == kTypeNumber && lightIt->second->data != nullptr){
				float& l = *reinterpret_cast<float*>(lightIt->second->data);
				g->light = l;
			}
			auto refrIt = obj->keys.find("refr");
			if(refrIt != obj->keys.end() && refrIt->second->type == kTypeNumber && refrIt->second->data != nullptr){
				float& r = *reinterpret_cast<float*>(refrIt->second->data);
				g->refr = r;
			}
			auto refrIdxIt = obj->keys.find("refrIndex");
			if(refrIdxIt != obj->keys.end() && refrIdxIt->second->type == kTypeNumber && refrIdxIt->second->data != nullptr){
				float& r = *reinterpret_cast<float*>(refrIdxIt->second->data);
				g->refrIndex = r;
			}
			auto reflIt = obj->keys.find("refl");
			if(reflIt != obj->keys.end() && reflIt->second->type == kTypeNumber && reflIt->second->data != nullptr){
				float& r = *reinterpret_cast<float*>(reflIt->second->data);
				g->refl = r;
			}

			auto intensityIt = obj->keys.find("intensity");
			if(intensityIt != obj->keys.end() && intensityIt->second->type == kTypeNumber && intensityIt->second->data != nullptr){
				float& r = *reinterpret_cast<float*>(intensityIt->second->data);
				g->intensity = r;
			}

			auto absorptionIt = obj->keys.find("absorption");
			if(absorptionIt != obj->keys.end() && absorptionIt->second->type == kTypeNumber && absorptionIt->second->data != nullptr){
				float& r = *reinterpret_cast<float*>(absorptionIt->second->data);
				g->absorption = r;
			}
			return true;
		}

		default:
			return false;
		}
	}
	std::string debugValue() const{
		if(data == nullptr) return "<undefined>";
		switch(type){
		case kTypeModel: {
			Model* m = reinterpret_cast<Model*>(data);
			std::stringstream strstr;
			strstr << m->name << ":\n";
			for(auto it=m->groups.begin(); it!=m->groups.end(); ++it){
				Group* g = it->second;
				strstr << "\t" << it->first << "\n";
			}
			return strstr.str();
		}
		case kTypeString:
			return std::string(reinterpret_cast<char*>(data));
		case kTypeNumber: {
			std::stringstream strstr;
			strstr << *reinterpret_cast<float*>(data);
			return strstr.str();
		}
		case kTypeVec3: {
			std::stringstream strstr;
			Vec3& v = *reinterpret_cast<Vec3*>(data);
			strstr << "(" << v.x << ", " << v.y << ", " << v.z << ")";
			return strstr.str();
		}
		case kTypeObject: {
			std::stringstream strstr;
			Object& o = *reinterpret_cast<Object*>(data);
			strstr << "{";
			for(auto it=o.keys.begin(); it!=o.keys.end(); ++it){
				strstr << "\t" << it->first << ": " << it->second->debugValue() << "\n";
			}
			strstr << "}";
			return strstr.str();
		}
		default:
			return "<undefined>";
		}
	}
	Variable* arrayOperator(Variable* var){
		switch(type){
		case kTypeModel: {
			if(var->type != kTypeString) return nullptr;
			Model* m = reinterpret_cast<Model*>(data);
			Variable* out = pushVariable();

			auto it = m->groups.find(reinterpret_cast<char*>(var->data));
			if(it == m->groups.end()){
				void* pos = stackMalloc(sizeof(Group));
				Group* g = new(pos) Group;
				out->data = g;
				m->groups[reinterpret_cast<char*>(var->data)] = g;
			}else{
				out->data = it->second;
			}
			out->type = kTypeModelGroup;
			return out;
		};
		default:
			return nullptr;
		}
	}
	~Variable(){
		if(!data) return;
		switch(type){
			case kTypeModel: 
				break; //No delete; Need model after parsing
			case kTypeModelGroup:
				break; //No delete; Need model after parsing
			case kTypeString:
				break;	//No delete; will get auto deleted with stack
			case kTypeObject:
				reinterpret_cast<Object*>(data)->~Object();
				break;
			case kTypeNumber:
				break; //Will get automatically deleted with the stack
			case kTypeVec3:
				break;	//will get automatically deleted with the stack
		}
	}
};
const unsigned int kStackVariables = 1024;
std::array<Variable, kStackVariables> variableStack;
int variableStackPtr = 0;



std::map<std::string, Variable*> variables;


Variable* pushVariable(){
	Variable* var = &variableStack[variableStackPtr++];
	var->type = kTypeUndefined;
	var->data = nullptr;
	return var;
}

std::string::size_type findIdentifierEnd(const std::string& string, std::string::size_type offset){
	for(auto i = offset; i<string.size(); ++i){
		if(!isalnum(string[i]) && string[i] != '_')
			return i;
	}
	return std::string::npos;
}
Variable* findVariable(const std::string& str, int* out_offs){
	//String literal?
	const int start = str.find_first_not_of(" \n\t");
	if(str[start] == '"'){ 
		const int end = str.find('"', start+1);
		const std::string literalData = str.substr(start+1, end-start-1);

		Variable* var = pushVariable();
		var->type = kTypeString;

		char* data = (char*)stackMalloc(literalData.size()+1);
		strcpy(data, literalData.c_str());
		var->data = data;
		if(out_offs) *out_offs = end;
		return var;
	}
	//Object literal?
	{
		auto objStart = str.find('{', start);
		if(objStart != std::string::npos){
			auto objEnd = str.find('}', objStart+1);
			auto offs = objStart+1;

			Variable* objectVar = pushVariable();
			void* objPos = stackMalloc(sizeof(Object)); 
			Object* obj = new(objPos) Object;
			objectVar->data = obj;
			objectVar->type = kTypeObject;
			while(true){
				auto keyBegin = str.find_first_not_of(" \n\t", offs);
				auto keyEnd = findIdentifierEnd(str, keyBegin);
				if(keyEnd-keyBegin >= 0){
					char* key = (char*)stackMalloc(sizeof(char)*(keyEnd-keyBegin+1));
					strncpy(key, &str.c_str()[keyBegin], keyEnd-keyBegin);
					key[keyEnd-keyBegin] = '\0';

					auto valuePos = str.find(':', offs);
					if(valuePos != std::string::npos){
						int end;
						Variable* var = findVariable(str.substr(valuePos+1), &end);
						obj->keys[key] = var;
						offs = end+valuePos;
					}
				}

				offs = str.find(',', offs);
				if(offs == std::string::npos) break;
				offs++;
			}
			if(out_offs) *out_offs = objEnd;
			return objectVar;
		}
	}
	//Vec3 literal?
	auto vec3Begin = str.find('(', start);
	if(vec3Begin != std::string::npos){
		auto vec3End = str.find(')', vec3Begin+1);
		if(vec3End == std::string::npos){
			return nullptr;
		}
		float val[3];
		int num = 0;
		auto offs = vec3Begin+1;
		while(true){
			val[num] = (float)strtod(&str.c_str()[offs], nullptr);
			offs = str.find(',', offs);
			if(offs == std::string::npos) break;
			offs ++;
			num++;
			if( num >= 3) break;
				
		}

		Variable* var = pushVariable();
		void* pos = stackMalloc(sizeof(Vec3));
		var->type = kTypeVec3;
		var->data = new(pos) Vec3(val[0], val[1], val[2]);
		if(out_offs) *out_offs = vec3End;
		return var;

	}

	//Number literal?
	auto number = str.find_first_of("0123456789.", start);
	if(number != std::string::npos){
		Variable* var = pushVariable();
		var->type = kTypeNumber;
		void* pos = stackMalloc(sizeof(float)); 
		char* end;
		var->data = new(pos) float((float)strtod(&str.c_str()[number], &end));
		if(out_offs) *out_offs = end-str.c_str();
		return var;
	}

	//Is it a variable ID?
	const int end = findIdentifierEnd(str, start);
	auto it = variables.find(str.substr(start, end));
	if(it != variables.end()){
		if(out_offs) *out_offs = end;
		return it->second;
	}
		
	return nullptr;
}
void ImportScene(const char* path, std::vector<Model*>* out_models){
	std::ifstream file(path);
	if(!file.good()) return;

	std::string command;
	while(true){
		std::string::size_type offset = 0;
		std::getline(file, command, ';');
		if(file.eof()) break;

		const int keywordStart = command.find_first_not_of(" \n\t");
		const int keywordEnd = command.find_first_of(" \n\t", keywordStart);
		const std::string keyword = command.substr(keywordStart, keywordEnd-keywordStart);
		Variable* current;

		//Check if type declaration
		VariableType declType = kTypeUndefined;
		for(int i=0; i<kTypeCount; ++i){
			if(keyword == g_TypeNames[i]){
				declType = (VariableType)i;
				break;
			}
		}
		if(declType != kTypeUndefined){
			//Creating a new object
			const int identifierStart = command.find_first_not_of(" \n\t", keywordEnd);
			const int identifierEnd = findIdentifierEnd(command, identifierStart);
			const std::string identifier = command.substr(identifierStart, identifierEnd-identifierStart);

			Variable* var = pushVariable();
			var->type = declType;
			var->data = nullptr;
			variables[identifier] = var;
			current = var;
			offset = identifierEnd;
		}else{
			//operating on an existing object?
			const int identifierStart = keywordStart;
			const int identifierEnd = findIdentifierEnd(command, identifierStart);
			const std::string identifier = command.substr(identifierStart, identifierEnd-identifierStart);

			auto it = variables.find(identifier);
			if(it != variables.end()){
				current = it->second;
			}else{
				printf("Undefined variable %s", identifier);
				return;
			}
			offset = identifierEnd;
		}

		//[] operator
		{
			const int operatorStart = command.find('[', offset);
			if(operatorStart != std::string::npos){
				const int operatorEnd = command.find(']', operatorStart+1);
				Variable* var = findVariable(command.substr(operatorStart+1, operatorEnd-operatorStart-1), nullptr);
				current = current->arrayOperator(var);
				offset = operatorEnd+1;

			}
		}
		//= operator
		{
			const int operatorStart = command.find_first_not_of(" \n\t", offset);
			offset = operatorStart+1;
			if(command[operatorStart] == '='){ //Assignment operator
				const int valueStart = command.find_first_not_of(" \n\t", offset);
				Variable* var = findVariable(command.substr(valueStart), nullptr);
				if(var->type == current->type){
					current->data = var->data; //TODO: Real copy
				}else{
					bool convertStatus = current->convert(var);
					if(!convertStatus){
						printf("Error: Could not convert from %s to %s.\n", g_TypeNames[var->type].c_str(), g_TypeNames[current->type].c_str());
					}
				}
			}
		}
	}

	*out_models = models;
	//Delete variable stack
	//for(int i=0; i<variableStackPtr; ++i){
	//	variableStack[i].~Variable();
	//}

	
}