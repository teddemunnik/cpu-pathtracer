#pragma once
#include <string>
#include <vector>
#include <map>
#include "template.h"

namespace Tmpl8{
	class Material;
};

using namespace Tmpl8;

struct Group{
	Group() : intensity(1.0f), refl(0.0f), refr(0.0f), refrIndex(1.0f){}
	float intensity;
	float refl;
	float refr;
	float refrIndex;
	float light;
	float absorption;
	
};

struct Model{
	std::string name;
	std::map<std::string, Group*> groups;
};


void ImportScene( const char* path, std::vector<Model*>* out_models);