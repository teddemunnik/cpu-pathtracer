#include "MBVH.h"
#include "BVH.h"

MBVHNode::MBVHNode() :
	children(nullptr)
{
}

MBVHNode::~MBVHNode(){
	if(children) delete[] children;
}
void MBVHNode::split(int depth){
	if(primCount < 5 || depth >2) return;

	BVHNode tmp;
	tmp.setTriangles(primList, primCount);
	tmp.doSplit();
	tmp.m_Left->doSplit();
	tmp.m_Right->doSplit();
	tmp.schrink_r();

	children = new MBVHNode[4];
	const BVHNode* tmpChilds[4] = {
		tmp.m_Left->m_Left,
		tmp.m_Left->m_Right,
		tmp.m_Right->m_Left,
		tmp.m_Right->m_Right
	};
	for(int i=0; i<4; ++i){
		reinterpret_cast<float*>(&minx4)[i] = tmpChilds[i]->m_Bounds.min.x;
		reinterpret_cast<float*>(&miny4)[i] = tmpChilds[i]->m_Bounds.min.y;
		reinterpret_cast<float*>(&minz4)[i] = tmpChilds[i]->m_Bounds.min.z;
		reinterpret_cast<float*>(&maxx4)[i] = tmpChilds[i]->m_Bounds.max.x;
		reinterpret_cast<float*>(&maxy4)[i] = tmpChilds[i]->m_Bounds.max.y;
		reinterpret_cast<float*>(&maxz4)[i] = tmpChilds[i]->m_Bounds.max.z;
		children[i].primList = tmpChilds[i]->m_PrimiveList;
		children[i].primCount = tmpChilds[i]->m_PrimitiveCount;
		children[i].split(depth+1);
	}

}