#include "MBVH.h"
#include "BVH.h"
#include <new>


//Custom allocation for friggin alignment
void* MBVHNode::operator new(size_t size){
	return _aligned_malloc(size, 16);
}
void MBVHNode::operator delete(void* memory){
	_aligned_free(memory);
}

MBVHNode::MBVHNode(){
	for(int i=0; i<4; ++i) children[i] = nullptr;
}
MBVHNode::~MBVHNode(){

}
void MBVHNode::fromBvh(const class BVHNode& root){
	_fill(root);
}
void MBVHNode::_fill(const BVHNode& node){
	const BVHNode* nodes[4] = {
		node.m_Left->m_Left ? node.m_Left->m_Left : node.m_Left,
		node.m_Left->m_Left ? node.m_Left->m_Right : nullptr,
		node.m_Right->m_Left ? node.m_Right->m_Left : node.m_Right,
		node.m_Right->m_Left ? node.m_Right->m_Right : nullptr
	};
	for(int i=0; i<4; ++i){
		children[i] = new MBVHNode;
		if(nodes[i]){
			children[i]->primList = nodes[i]->m_PrimiveList;
			children[i]->primCount = nodes[i]->m_PrimitiveCount;
			reinterpret_cast<float*>(&minx4)[i] = nodes[i]->m_Bounds.min.x;
			reinterpret_cast<float*>(&miny4)[i] = nodes[i]->m_Bounds.min.y;
			reinterpret_cast<float*>(&minz4)[i] = nodes[i]->m_Bounds.min.z;
			reinterpret_cast<float*>(&maxx4)[i] = nodes[i]->m_Bounds.max.x;
			reinterpret_cast<float*>(&maxy4)[i] = nodes[i]->m_Bounds.max.y;
			reinterpret_cast<float*>(&maxz4)[i] = nodes[i]->m_Bounds.max.z;

			primList = nodes[i]->m_PrimiveList;
			primCount = nodes[i]->m_PrimitiveCount;

			if(nodes[i]->m_Left){
				children[i] = new MBVHNode;
				children[i]->_fill(*nodes[i]);
			}
		}
	}
	
}