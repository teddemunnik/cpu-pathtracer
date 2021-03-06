#pragma once
#include "aabb.h"

namespace Tmpl8{
	struct Triangle;
};
using namespace Tmpl8;

class BVHNode{
public:
	AABB m_Bounds;
	BVHNode *m_Left, *m_Right;
	Triangle* m_PrimiveList;
	int m_PrimitiveCount;

public:
	BVHNode();
	~BVHNode();
	bool doSplit();
	bool split(int depth=0, int multipleOf=1);
	void setTriangles(Triangle* _PrimList, int _PrimCount);

	void schrink();
	void schrink_r();
};