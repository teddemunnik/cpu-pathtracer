#include "BVH.h"
#include <assert.h>
#include "raytracer.h"

BVHNode::BVHNode() : m_Left(nullptr), m_Right(nullptr), m_PrimitiveCount(0){}
BVHNode::~BVHNode(){
	if(m_Left){ //if m_Left assume m_Right is also set
		delete m_Left;
		delete m_Right;
	}
}
bool BVHNode::doSplit(){
	bool success=false;
	//Find the biggest axis to split on
	const float3 size = m_Bounds.max-m_Bounds.min;
	int dominantaxis = 0;
	if(size.y >= size.x && size.y >= size.z)		dominantaxis = 1;
	else if(size.z > size.x && size.z > size.y) dominantaxis = 2;

	//Try to split on this axis
	const float half = size[dominantaxis]*0.5f + m_Bounds.min[dominantaxis];
	int leftOffset=0, rightOffset=m_PrimitiveCount-1;
	while(true){
		//Find next incorrect pair
		while(leftOffset < rightOffset && m_PrimiveList[leftOffset].getCenter()[dominantaxis] < half) ++leftOffset;
		while(rightOffset > leftOffset && m_PrimiveList[rightOffset].getCenter()[dominantaxis] > half) --rightOffset;

		if(leftOffset < rightOffset){
			//Swap the pair
			Triangle tmp = m_PrimiveList[leftOffset];
			m_PrimiveList[leftOffset] = m_PrimiveList[rightOffset];
			m_PrimiveList[rightOffset] = tmp;
			++leftOffset;
			--rightOffset;
		}else break;
	}
	success = leftOffset > 0 && rightOffset < m_PrimitiveCount-1;
	m_Left = new BVHNode;
	m_Left->m_PrimiveList = m_PrimiveList;
	m_Left->m_PrimitiveCount = leftOffset;
	m_Left->m_Bounds = m_Bounds;
	m_Left->m_Bounds.max[dominantaxis] = half;
	m_Right = new BVHNode;
	m_Right->m_PrimiveList = &m_PrimiveList[leftOffset];
	m_Right->m_PrimitiveCount = m_PrimitiveCount - leftOffset;
	m_Right->m_Bounds = m_Bounds;
	m_Right->m_Bounds.min[dominantaxis] = half;
	return success;
}
bool BVHNode::split( int depth, int multipleOf){
	assert(m_Left==nullptr&&m_Right==nullptr);
	bool success = false;
	if(((depth) % multipleOf) != 0 || m_PrimitiveCount >= 5){
		success = doSplit();

		//Attempt to split the new nodes
		if(depth < 100){
			bool s1 = m_Left->split(depth+1);
			bool s2 = m_Right->split(depth+1);
			if(!s1 && !s2 && (depth) % multipleOf != 0){
				delete m_Left, m_Right;
				m_Left=nullptr, m_Right=nullptr;
			}else{
				success = true;
			}
		}
	}

	//Shrink to fit
	schrink();
	return success;
}
void BVHNode::schrink_r(){
	if(m_Left){
		m_Left->schrink_r();
		m_Right->schrink_r();
	}
	schrink();
}
void BVHNode::schrink(){
	if(m_Left){
		m_Bounds = m_Left->m_Bounds;
		m_Bounds.expand(m_Right->m_Bounds);
	}else{
		if(m_PrimitiveCount > 0){
			m_Bounds = m_PrimiveList[0].getBounds();
			for(int i=1; i<m_PrimitiveCount; ++i){
				m_Bounds.expand(m_PrimiveList[i].getBounds());
			}
		}
	}
}
void BVHNode::setTriangles(Triangle* _PrimList, int _PrimCount){
	m_PrimiveList = _PrimList;
	m_PrimitiveCount = _PrimCount;

	//Calc bounds
	if(m_PrimitiveCount > 0){
		m_Bounds = m_PrimiveList[0].getBounds();
		for(int i=1; i<m_PrimitiveCount; ++i){
			m_Bounds.expand(m_PrimiveList[i].getBounds());
		}
	}
}