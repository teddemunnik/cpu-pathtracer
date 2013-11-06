#pragma once
#include <vector>
#include "template.h"

using namespace Tmpl8;
extern "C"{
struct OBJSubMesh{
	std::string material;
	int offset;
};
struct OBJMaterial{
	OBJMaterial() : refl(0.0f){}
	std::string name;
	float3 diffuse;
	std::string diffuseMap;
	float refl;
};
bool LoadMtlLib(const char* filePath, std::vector<OBJMaterial>* out){
	FILE* file = fopen(filePath, "r");
	if(file == nullptr) return false;

	OBJMaterial* current=nullptr;
	while(true){
		char lineHeader[128];
		memset(lineHeader, 0, 128);
		int res = fscanf(file, "%s", lineHeader);
		if(res == EOF) break;

		if(strcmp(lineHeader, "newmtl") == 0){
			char name[256];
			int res = fscanf(file, "%s", name);
			if(res){
				OBJMaterial mat;
				mat.name = name;
				out->push_back(mat);
				current = &(*out)[out->size()-1];
			}
		}else if(strcmp(lineHeader, "Kd") == 0){
			if(!current) return false;
			float r, g, b;
			if(fscanf(file, "%f %f %f", &r, &g, &b) == 3){
				current->diffuse = float3(r,g,b);
			}
		}else if(strcmp(lineHeader, "map_Kd") == 0){
			if(!current) return false;
			char pathBuffer[256];
			int res = fscanf(file, "%s", pathBuffer);
			if(res){
				current->diffuseMap = pathBuffer;
			}
		}else if(strcmp(lineHeader, "Ns") == 0){
			float refl;
			int res = fscanf(file, "%f", &refl);
			if(res){
				refl = refl/100.0f;
				current->refl = refl;
			}
		}
	}

	return true;
}
bool LoadOBJ(const char* filePath, std::vector<float3>* out_verts, std::vector<float3>* out_normals, std::vector<float2>* out_uv, std::vector<OBJSubMesh>* out_submeshes, std::vector<OBJMaterial>* out_materials){
	FILE* file = fopen(filePath, "r");
	if(file == nullptr) return false;

	std::vector<float3> tmp_Verts;
	std::vector<unsigned int> tmp_iVerts;
	std::vector<float3> tmp_Normals;
	std::vector<unsigned int> tmp_iNormals;
	std::vector<float2> tmp_Uv;
	std::vector<unsigned int> tmp_iUv;

	while(true){
		char lineHeader[128];
		memset(lineHeader, 0, 128);
		int res = fscanf(file, "%s", lineHeader);
		if(res == EOF) break;

		if(strcmp(lineHeader, "v") == 0){
			float3 vertex;
			if (fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z) == 3){
				tmp_Verts.push_back(vertex);
			}
		}else if(strcmp(lineHeader, "vt") == 0){
			float2 uv;
			if (fscanf(file, "%f %f\n", &uv.x, &uv.y) == 2){
				tmp_Uv.push_back(uv);
			}
		}else if(strcmp(lineHeader, "vn") == 0){
			float3 normal;
			if (fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z) == 3){
				tmp_Normals.push_back(normal);
			}
		}else if(strcmp(lineHeader, "f") == 0){
			unsigned int vertexIndex[4], uvIndex[4], normalIndex[4];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n", 
				&vertexIndex[0], &uvIndex[0], &normalIndex[0], 
				&vertexIndex[1], &uvIndex[1], &normalIndex[1], 
				&vertexIndex[2], &uvIndex[2], &normalIndex[2], 
				&vertexIndex[3], &uvIndex[3], &normalIndex[3]);
			if(matches == 9){
				tmp_iVerts.push_back(vertexIndex[0]-1);
				tmp_iVerts.push_back(vertexIndex[1]-1);
				tmp_iVerts.push_back(vertexIndex[2]-1);
				tmp_iUv.push_back(uvIndex[0]-1);
				tmp_iUv.push_back(uvIndex[1]-1);
				tmp_iUv.push_back(uvIndex[2]-1);
				tmp_iNormals.push_back(normalIndex[0]-1);
				tmp_iNormals.push_back(normalIndex[1]-1);
				tmp_iNormals.push_back(normalIndex[2]-1);
			}else if(matches == 12){
				tmp_iVerts.push_back(vertexIndex[0]-1);
				tmp_iVerts.push_back(vertexIndex[1]-1);
				tmp_iVerts.push_back(vertexIndex[2]-1);
				tmp_iVerts.push_back(vertexIndex[0]-1);
				tmp_iVerts.push_back(vertexIndex[2]-1);
				tmp_iVerts.push_back(vertexIndex[3]-1);
				tmp_iUv.push_back(uvIndex[0]-1);
				tmp_iUv.push_back(uvIndex[1]-1);
				tmp_iUv.push_back(uvIndex[2]-1);
				tmp_iUv.push_back(uvIndex[0]-1);
				tmp_iUv.push_back(uvIndex[2]-1);
				tmp_iUv.push_back(uvIndex[3]-1);
				tmp_iNormals.push_back(normalIndex[0]-1);
				tmp_iNormals.push_back(normalIndex[1]-1);
				tmp_iNormals.push_back(normalIndex[2]-1);
				tmp_iNormals.push_back(normalIndex[0]-1);
				tmp_iNormals.push_back(normalIndex[2]-1);
				tmp_iNormals.push_back(normalIndex[3]-1);
			}
		}else if(strcmp(lineHeader, "mtllib") == 0){
			char pathBuffer[256];
			int res = fscanf(file, "%s", pathBuffer);
			if(res){
				char path[256];
				const char* last = strrchr(filePath, '/');
				if(!last){
					strcpy(pathBuffer, path);
				}else{
					strncpy(path, filePath, last-filePath+1);
					path[last-filePath+1] = 0;
					strcat(path, pathBuffer);
				}
				LoadMtlLib(path, out_materials);
			}
		}else if(strcmp(lineHeader, "usemtl") == 0){
			char nameBuffer[256];
			int res = fscanf(file, "%s", nameBuffer);
			if(res){
				OBJSubMesh sub;
				sub.material = nameBuffer;
				sub.offset = tmp_iVerts.size();
				out_submeshes->push_back(sub);
			}
		}
	}

	for(size_t i=0; i<tmp_iVerts.size(); ++i){
		out_verts->push_back(tmp_Verts[tmp_iVerts[i]]);
		out_normals->push_back(tmp_Normals[tmp_iNormals[i]]);
		out_uv->push_back(tmp_Uv[tmp_iUv[i]]);
	}

	return true;
}
}