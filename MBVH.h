#pragma once
#include <emmintrin.h>

namespace Tmpl8{
	struct Triangle;
};
using namespace Tmpl8;

class MBVHNode{
public:
	__m128 minx4;
	__m128 miny4;
	__m128 minz4;
	__m128 maxx4;
	__m128 maxy4;
	__m128 maxz4;
		
	Triangle* primList;
	int primCount;

	MBVHNode* children[4];

	MBVHNode();
	~MBVHNode();

	void fromBvh(const class BVHNode& root);
	void _fill(const class BVHNode& node);
	
	void* operator new(size_t size);
	void operator delete(void*);
};