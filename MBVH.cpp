#include "MBVH.h"
#include "BVH.h"

MBVHNode::MBVHNode(){
}
MBVHNode::~MBVHNode(){
	if(isLeaf){
		if(children[0]) delete children[0];
		if(children[1]) delete children[1];
		if(children[2]) delete children[2];
		if(children[3]) delete children[3];
	}
}
void MBVHNode::split(int depth){
	BVHNode tmp;
	tmp.setTriangles(primList, primCount);
	
}