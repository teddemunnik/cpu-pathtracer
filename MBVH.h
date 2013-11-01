#pragma once
#include <emmintrin.h>

namespace Tmpl8{
	struct Triangle;
};
using namespace Tmpl8;

class MBVHNode{
public:
	bool isLeaf;
	union{
		struct{
			__m128 minx4;
			__m128 miny4;
			__m128 minz4;
			__m128 maxx4;
			__m128 maxy4;
			__m128 maxz4;
			MBVHNode* children[4];
		};
		struct{
			Triangle* primList;
			unsigned int primCount;
		};
	};

	MBVHNode();
	~MBVHNode();
	void split(int depth);
};