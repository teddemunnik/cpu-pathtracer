#pragma once
#include "template.h"

//friggin macros...
#pragma push_macro("min")
#pragma push_macro("max")
#undef min
#undef max

using namespace Tmpl8;

struct AABB{
	AABB(){}
	AABB(float3 _min, float3 _max) : min(_min), max(_max){}
	float3 min, max;

	void expand(const AABB& rhs){
		//TODO: This seems easy with SSE min, max instructions
		min.x = MIN(min.x, rhs.min.x);
		min.y = MIN(min.y, rhs.min.y);
		min.z = MIN(min.z, rhs.min.z);
		max.x = MAX(max.x, rhs.max.x);
		max.y = MAX(max.y, rhs.max.y);
		max.z = MAX(max.z, rhs.max.z);
	}
};
#pragma pop_macro("min")
#pragma pop_macro("max")