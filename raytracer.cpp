// Template for GP1, version 1
// IGAD/NHTV - Jacco Bikker - 2006-2013

#include "template.h"
#include "raytracer.h"
#include "surface.h"
#include "mtrand.h"
#include "BVH.h"
#include <vector>
#include "OBJLoader.h"
#include <assert.h>
#include <gl/glew.h>
#include "SceneImporter.h"

using namespace Tmpl8;

//Used for converting int mask to __m128 mask
const __m128 masktable[16] = {
	*(__m128*)&_mm_set_epi32(0,			0,			0,			0),
	*(__m128*)&_mm_set_epi32(0,			0,			0,			0xffffffff),
	*(__m128*)&_mm_set_epi32(0,			0,			0xffffffff,	0),
	*(__m128*)&_mm_set_epi32(0,			0,			0xffffffff,	0xffffffff),
	*(__m128*)&_mm_set_epi32(0,			0xffffffff,	0,			0),
	*(__m128*)&_mm_set_epi32(0,			0xffffffff,	0,			0xffffffff),
	*(__m128*)&_mm_set_epi32(0,			0xffffffff,	0xffffffff,	0),
	*(__m128*)&_mm_set_epi32(0,			0xffffffff,	0xffffffff,	0xffffffff),
	*(__m128*)&_mm_set_epi32(0xffffffff,0,			0,			0),
	*(__m128*)&_mm_set_epi32(0xffffffff,0,			0,			0xffffffff),
	*(__m128*)&_mm_set_epi32(0xffffffff,0,			0xffffffff,	0),
	*(__m128*)&_mm_set_epi32(0xffffffff,0,			0xffffffff,	0xffffffff),
	*(__m128*)&_mm_set_epi32(0xffffffff,0xffffffff,	0,			0),
	*(__m128*)&_mm_set_epi32(0xffffffff,0xffffffff,	0,			0xffffffff),
	*(__m128*)&_mm_set_epi32(0xffffffff,0xffffffff,	0xffffffff,	0),
	*(__m128*)&_mm_set_epi32(0xffffffff,0xffffffff,	0xffffffff,	0xffffffff)
};

// == Triangle class ==========================================================
//    --------------
float3 Triangle::getCenter() const{
	return (v0+v1+v2)/3.0f;
}
AABB Triangle::getBounds() const{
	return AABB(
		float3(MIN(v0.x, MIN(v1.x, v2.x)), MIN(v0.y, MIN(v1.y, v2.y)), MIN(v0.z, MIN(v1.z, v2.z)))-float3(EPSILON, EPSILON, EPSILON),
		float3(MAX(v0.x, MAX(v1.x, v2.x)), MAX(v0.y, MAX(v1.y, v2.y)), MAX(v0.z, MAX(v1.z, v2.z)))+float3(EPSILON, EPSILON, EPSILON)
	);
}
void Triangle::intersectPrimary(PrimaryRayBundle* _Rays, int first) const{
	const __m128 v0x4 = _mm_set1_ps(v0.x), v0y4 = _mm_set1_ps(v0.y), v0z4 = _mm_set1_ps(v0.z);
	const __m128 v1x4 = _mm_set1_ps(v1.x), v1y4 = _mm_set1_ps(v1.y), v1z4 = _mm_set1_ps(v1.z);
	const __m128 v2x4 = _mm_set1_ps(v2.x), v2y4 = _mm_set1_ps(v2.y), v2z4 = _mm_set1_ps(v2.z);

	//Frustum early out
	const __m128 reta4 = DOT128(_Rays->fx4, _Rays->fy4, _Rays->fz4, v0x4, v0y4, v0z4);
	const __m128 retb4 = DOT128(_Rays->fx4, _Rays->fy4, _Rays->fz4, v1x4, v1y4, v1z4);
	const __m128 retc4 = DOT128(_Rays->fx4, _Rays->fy4, _Rays->fz4, v2x4, v2y4, v2z4);
	if(_mm_movemask_ps(_mm_and_ps(_mm_and_ps(_mm_cmpgt_ps(reta4, _Rays->fd4), _mm_cmpgt_ps(retb4, _Rays->fd4)), _mm_cmpgt_ps(retc4, _Rays->fd4)))){
		return;
	}
	
	for(int i=first; i<PrimaryRayBundle::kPackedCount; ++i){
		const __m128 aox4 = _mm_sub_ps(v0x4, _Rays->Ox4[i]),
					 aoy4 = _mm_sub_ps(v0y4, _Rays->Oy4[i]),
					 aoz4 = _mm_sub_ps(v0z4, _Rays->Oz4[i]);
		const __m128 box4 = _mm_sub_ps(v1x4, _Rays->Ox4[i]),
					 boy4 = _mm_sub_ps(v1y4, _Rays->Oy4[i]),
					 boz4 = _mm_sub_ps(v1z4, _Rays->Oz4[i]);
		const __m128 cox4 = _mm_sub_ps(v2x4, _Rays->Ox4[i]),
					 coy4 = _mm_sub_ps(v2y4, _Rays->Oy4[i]),
					 coz4 = _mm_sub_ps(v2z4, _Rays->Oz4[i]);

		//Cross(CO, BO)
		const __m128 v0cx4 = _mm_sub_ps( _mm_mul_ps( coy4, boz4 ), _mm_mul_ps( coz4, boy4 ) ), 
					 v0cy4 = _mm_sub_ps( _mm_mul_ps( coz4, box4 ), _mm_mul_ps( cox4, boz4 ) ), 
					 v0cz4 = _mm_sub_ps( _mm_mul_ps( cox4, boy4 ), _mm_mul_ps( coy4, box4 ) ); 
		//Cross(BO, AO)
		const __m128 v1cx4 = _mm_sub_ps( _mm_mul_ps( boy4, aoz4 ), _mm_mul_ps( boz4, aoy4 ) ), 
					 v1cy4 = _mm_sub_ps( _mm_mul_ps( boz4, aox4 ), _mm_mul_ps( box4, aoz4 ) ), 
					 v1cz4 = _mm_sub_ps( _mm_mul_ps( box4, aoy4 ), _mm_mul_ps( boy4, aox4 ) );
		//Cross(AO, CO)
		const __m128 v2cx4 = _mm_sub_ps( _mm_mul_ps( aoy4, coz4 ), _mm_mul_ps( aoz4, coy4 ) ), 
					 v2cy4 = _mm_sub_ps( _mm_mul_ps( aoz4, cox4 ), _mm_mul_ps( aox4, coz4 ) ), 
					 v2cz4 = _mm_sub_ps( _mm_mul_ps( aox4, coy4 ), _mm_mul_ps( aoy4, cox4 ) ); 

		const __m128 v0d = DOT128(v0cx4, v0cy4, v0cz4, _Rays->Dx4[i], _Rays->Dy4[i], _Rays->Dz4[i]);
		const unsigned int v0s = _mm_movemask_ps(v0d);//check < 0
		const __m128 v1d = DOT128(v1cx4, v1cy4, v1cz4, _Rays->Dx4[i], _Rays->Dy4[i], _Rays->Dz4[i]);
		const unsigned int v1s = _mm_movemask_ps(v1d);//Check < 0
		const __m128 v2d = DOT128(v2cx4, v2cy4, v2cz4, _Rays->Dx4[i], _Rays->Dy4[i], _Rays->Dz4[i]);
		const unsigned int v2s = _mm_movemask_ps(v2d);//Check < 0
		const unsigned int mask = (v0s & v1s & v2s) | ((v0s^0xf) & (v1s^0xf) & (v2s^0xf));
		if( mask ){
			const __m128 nx4 = _mm_set1_ps(N.x), ny4 = _mm_set1_ps(N.y), nz4 = _mm_set1_ps(N.z);
			const __m128 t4 = _mm_div_ps(DOT128(nx4, ny4, nz4, aox4, aoy4, aoz4), DOT128(nx4, ny4, nz4, _Rays->Dx4[i], _Rays->Dy4[i], _Rays->Dz4[i]));
			const __m128 mask4 = _mm_and_ps(_mm_and_ps(masktable[mask], _mm_cmplt_ps(t4, _Rays->t4[i])), _mm_cmpgt_ps(t4, _mm_setzero_ps()));
			if(_mm_movemask_ps(mask4)){
				_Rays->t4[i] = SELECTMASK_PS(t4, _Rays->t4[i], mask4);
				const __m128 v0l = _mm_rcp_ps(_mm_add_ps(_mm_add_ps(v0d, v1d), v2d));
				_Rays->u4[i] = SELECTMASK_PS(_mm_mul_ps(v2d, v0l), _Rays->u4[i], mask4);
				_Rays->v4[i] = SELECTMASK_PS(_mm_mul_ps(v1d, v0l), _Rays->v4[i], mask4);
				_Rays->prim4[i] = SELECTMASK_SI128(_mm_set1_epi32((int)this), _Rays->prim4[i],*(__m128i*)&mask4);
			}
		}
	}
}
void Triangle::intersectSecondary(Ray* _Ray) const{
	const float3 e1 = v1-v0;
	const float3 e2 = v2-v0;

	const float3 pvec = Cross(_Ray->D, e2);

	float det = Dot(e1, pvec);
	//if(det > -EPSILON && det < EPSILON) return;
	const float invdet = 1.0f/det;

	const float3 tvec = _Ray->O - v0;
	const float u = Dot(tvec, pvec) * invdet;
	if(u < 0 || u > 1) return;
	const float3 qvec = Cross(tvec, e1);
	const float v = Dot(_Ray->D, qvec) * invdet;
	if(v < 0 || u + v > 1) return;
	const float t = Dot(e2, qvec) * invdet;
	if(t > 0 && t < _Ray->t){
		_Ray->t = t;
		_Ray->u = u;
		_Ray->v = v;
		_Ray->prim = const_cast<Triangle*>(this);
	}

}

// == Camera class ============================================================
//    ------------
Camera::Camera() :
	m_Transform(float4x4::Identity()),
	m_ApertureSize(0.0f),
	m_FocalDistance(1.0f)
{}
Camera::~Camera(){}
float4x4 Camera::transform() const{
	return m_Transform;
}
void Camera::setTransform(const float4x4& transform){
	m_Transform = transform;
	_invalidate();
}
void Camera::_invalidate(){
	const float aspect = SCRWIDTH/(float)SCRHEIGHT;
	p1 = *(float3*)&(m_Transform * float4(m_FocalDistance * float3(-0.5f*aspect, 0.5f, 1),  1.0f));
	p2 = *(float3*)&(m_Transform * float4(m_FocalDistance * float3( 0.5f*aspect, 0.5f, 1),  1.0f));
	p3 = *(float3*)&(m_Transform * float4(m_FocalDistance * float3( 0.5f*aspect, -0.5f, 1), 1.0f));
	p4 = *(float3*)&(m_Transform * float4(m_FocalDistance * float3(-0.5f*aspect, -0.5f, 1), 1.0f));
}
void Camera::set( float3 _Pos, float3 _Rotation )
{
	// set position and view direction, then calculate screen corners
	float4x4 tmp = float4x4::Rotate(_Rotation) * float4x4::Translate(_Pos);
	setTransform(tmp);
}
void Camera::setAperture(float apertureSize, float focalDist){
	if(focalDist <= EPSILON) focalDist = EPSILON;
	m_ApertureSize = MAX(0.0f, apertureSize);
	m_FocalDistance = focalDist;
	_invalidate();
}
void Camera::GenerateRays( PrimaryRayBundle* _Rays, int _X, int _Y){
	//Constants defining the layout of the simd packet, [ a, b ]
	//													[ c, d ]
	const static __m128i kSIMDXOffsets4 = _mm_set_epi32(1, 0, 1, 0);
	const static __m128i kSIMDYOffsets4 = _mm_set_epi32(1, 1, 0, 0);
	const float3 pos(m_Transform.w.x, m_Transform.w.y, m_Transform.w.z);
	for(int i=0; i<PrimaryRayBundle::kPackedCount; ++i){
		//Ray origin
		_Rays->Ox4[i] = _mm_set1_ps(pos.x);
		_Rays->Oy4[i] = _mm_set1_ps(pos.y);
		_Rays->Oz4[i] = _mm_set1_ps(pos.z);
		
		#if FEATURE_DOF_ENABLED
			//Create random offset based on camera lens size for depth of field
			const float3 dofRand = float3(
				randf_oo()*2-1,
				randf_oo()*2-1,
				randf_oo()*2-1
			)*m_ApertureSize;
			_Rays->Ox4[i] = _mm_add_ps(_Rays->Ox4[i], _mm_set1_ps(dofRand.x));
			_Rays->Oy4[i] = _mm_add_ps(_Rays->Oy4[i], _mm_set1_ps(dofRand.y));
			_Rays->Oz4[i] = _mm_add_ps(_Rays->Oz4[i], _mm_set1_ps(dofRand.z));
		#endif
		
		//Normalized screen coordinates
		const __m128i scrx4 = _mm_add_epi32(_mm_set1_epi32(_X*PrimaryRayBundle::kWidth + (i % (PrimaryRayBundle::kWidth/2))*2), kSIMDXOffsets4);
		const __m128i scry4 = _mm_add_epi32(_mm_set1_epi32(_Y*PrimaryRayBundle::kHeight + (i / (PrimaryRayBundle::kWidth/2))*2), kSIMDYOffsets4);
		__m128 px4 = _mm_cvtepi32_ps(scrx4);
		__m128 py4 = _mm_cvtepi32_ps(scry4);
		#if FEATURE_AA_ENABLED
			px4 = _mm_add_ps(px4, _mm_set1_ps(randf_oo()));
			py4 = _mm_add_ps(py4, _mm_set1_ps(randf_oo()));
		#endif
		px4 = _mm_div_ps(px4, _mm_set1_ps(SCRWIDTH));
		py4 = _mm_div_ps(py4, _mm_set1_ps(SCRHEIGHT));


		//Calculate ray directions based on 'virtual screen plane' position and ray origins
		const __m128 p1x4 = _mm_set1_ps(p1.x), p1y4 = _mm_set1_ps(p1.y), p1z4 = _mm_set1_ps(p1.z);
		const __m128 hoffsx4 = _mm_set1_ps(p2.x-p1.x), hoffsy4 = _mm_set1_ps(p2.y-p1.y), hoffsz4 = _mm_set1_ps(p2.z-p1.z);
		const __m128 voffsx4 = _mm_set1_ps(p4.x-p1.x), voffsy4 = _mm_set1_ps(p4.y-p1.y), voffsz4 = _mm_set1_ps(p4.z-p1.z);
		const __m128 x4 = _mm_add_ps(_mm_add_ps(p1x4, _mm_mul_ps(px4, hoffsx4)), _mm_mul_ps(py4, voffsx4));
		const __m128 y4 = _mm_add_ps(_mm_add_ps(p1y4, _mm_mul_ps(px4, hoffsy4)), _mm_mul_ps(py4, voffsy4));
		const __m128 z4 = _mm_add_ps(_mm_add_ps(p1z4, _mm_mul_ps(px4, hoffsz4)), _mm_mul_ps(py4, voffsz4));
		_Rays->Dx4[i] = _mm_sub_ps(x4, _Rays->Ox4[i]);
		_Rays->Dy4[i] = _mm_sub_ps(y4, _Rays->Oy4[i]);
		_Rays->Dz4[i] = _mm_sub_ps(z4, _Rays->Oz4[i]);
		const __m128 invlen4 = _mm_div_ps(_mm_set1_ps(1.0f), _mm_sqrt_ps(DOT128(_Rays->Dx4[i], _Rays->Dy4[i], _Rays->Dz4[i], _Rays->Dx4[i], _Rays->Dy4[i], _Rays->Dz4[i])));
		_Rays->Dx4[i] = _mm_mul_ps(_Rays->Dx4[i], invlen4);
		_Rays->Dy4[i] = _mm_mul_ps(_Rays->Dy4[i], invlen4);
		_Rays->Dz4[i] = _mm_mul_ps(_Rays->Dz4[i], invlen4);


		_Rays->t4[i] = _mm_set1_ps(1e34f);
		_Rays->prim4[i] = _mm_setzero_si128();

		for(int j=0; j<4; ++j){
			_Rays->addr[i*4+j] = reinterpret_cast<const unsigned int*>(&scrx4)[j] + reinterpret_cast<const unsigned int*>(&scry4)[j]*SCRWIDTH;
		}
	}

	//Frustum
	//TODO: Simd & support DOF
	const float leftOffs = ((_X*PrimaryRayBundle::kWidth)/(float)SCRWIDTH);
	const float rightOffs = (((_X+1)*PrimaryRayBundle::kWidth)/(float)SCRWIDTH);
	const float upOffs = ((_Y*PrimaryRayBundle::kHeight)/(float)SCRHEIGHT);
	const float downOffs = (((_Y+1)*PrimaryRayBundle::kHeight)/(float)SCRHEIGHT);
	const float3 hdir = (p2-p1);
	const float3 vdir = (p4-p1);


	//Extract rotation
	float4x4 rotation = transform();
	rotation.x.w=0;
	rotation.y.w=0;
	rotation.z.w=0;
	rotation.w = float4(0,0,0,1);
	float3 poss[4] = {
		(rotation * float4(-m_ApertureSize, m_ApertureSize, 0.0f, 0.0f)).xyz,
		(rotation * float4(m_ApertureSize, m_ApertureSize, 0.0f, 0.0f)).xyz,
		(rotation * float4(m_ApertureSize, -m_ApertureSize, 0.0f, 0.0f)).xyz,
		(rotation * float4(-m_ApertureSize, -m_ApertureSize, 0.0f, 0.0f)).xyz
	};

	const float3 c1 = Normalize(p1 + leftOffs*hdir + upOffs*vdir    - (pos - poss[0]));
	const float3 c2 = Normalize(p1 + rightOffs*hdir + upOffs*vdir   - (pos - poss[1]));
	const float3 c3 = Normalize(p1 + rightOffs*hdir + downOffs*vdir - (pos - poss[2]));
	const float3 c4 = Normalize(p1 + leftOffs*hdir + downOffs*vdir  - (pos - poss[3]));
	const float3 p0 = Normalize(Cross(c1, c2));
	const float3 p1 = Normalize(Cross(c2, c3));
	const float3 p2 = Normalize(Cross(c3, c4));
	const float3 p3 = Normalize(Cross(c4, c1));
	_Rays->fx4 = _mm_set_ps(p3.x, p2.x, p1.x, p0.x);
	_Rays->fy4 = _mm_set_ps(p3.y, p2.y, p1.y, p0.y);
	_Rays->fz4 = _mm_set_ps(p3.z, p2.z, p1.z, p0.z);
	_Rays->fd4 = _mm_set_ps(
					Dot(pos + poss[3], p3), 
					Dot(pos + poss[2], p2), 
					Dot(pos + poss[1], p1), 
					Dot(pos + poss[0], p0));

	//Calc offsets to nearest vector on AABB for each plane
	const unsigned int* signPtr = (const unsigned int*)&_Rays->fx4;
	for(int i=0; i<4; ++i){
		_Rays->sign[i + 0] = (signPtr[  i] >> 31) * 3 + 0;
		_Rays->sign[i + 4] = (signPtr[4+i] >> 31) * 3 + 1;
		_Rays->sign[i + 8] = (signPtr[8+i] >> 31) * 3 + 2;
	}

}
// == Scene class =============================================================
//    -----------
Scene::Scene()
{
	//FFS STATIC INITIALISATION ORDER I HATE YOU :(
}
Scene::~Scene(){

}
void Scene::intersectPrimary(PrimaryRayBundle* _Rays){
	//for(int i=0; i<PrimaryRayBundle::kPackedCount; ++i) _Rays->t4[i] = _mm_setzero_ps();
	intersectPrimary_r(_Rays, root, 0);
}
void Scene::intersectPrimary_r(PrimaryRayBundle* _Rays, const BVHNode& node, int first){
	const int firstActive = findFirstPrimary(_Rays, node, first);
	
	if(firstActive < PrimaryRayBundle::kPackedCount){
		//for(int i=firstActive; i<PrimaryRayBundle::kPackedCount; ++i) _Rays->t4[i] = _mm_add_ps(_Rays->t4[i], _mm_set1_ps(0.01f));
		if(node.m_Left){
			intersectPrimary_r(_Rays, *node.m_Left, firstActive);
			intersectPrimary_r(_Rays, *node.m_Right, firstActive);
		}else{
			for(int i=0; i<node.m_PrimitiveCount; ++i){
				node.m_PrimiveList[i].intersectPrimary(_Rays, firstActive);
			}
		}
	}
}
int Scene::findFirstPrimary(PrimaryRayBundle* _Rays, const BVHNode& node, int first){
	//--Frustum fast out
	//First find closest vertex on AABB
	const float* boundsRaw = reinterpret_cast<const float*>(&node.m_Bounds);
	const __m128 bx4 = _mm_set_ps(boundsRaw[_Rays->sign[3 ]], boundsRaw[_Rays->sign[2 ]], boundsRaw[_Rays->sign[1]], boundsRaw[_Rays->sign[0]]),
				 by4 = _mm_set_ps(boundsRaw[_Rays->sign[7 ]], boundsRaw[_Rays->sign[6 ]], boundsRaw[_Rays->sign[5]], boundsRaw[_Rays->sign[4]]),
				 bz4 = _mm_set_ps(boundsRaw[_Rays->sign[11]], boundsRaw[_Rays->sign[10]], boundsRaw[_Rays->sign[9]], boundsRaw[_Rays->sign[8]]);

	//Check against all frustum planeswant
	const __m128 frustdst4 = DOT128(_Rays->fx4, _Rays->fy4, _Rays->fz4, bx4, by4, bz4);
	if(_mm_movemask_ps(_mm_cmpgt_ps(frustdst4, _Rays->fd4))){
		return PrimaryRayBundle::kPackedCount;
	}

	//--Find first ray that hits
	const __m128 minx4 = _mm_set1_ps(node.m_Bounds.min.x), maxx4 = _mm_set1_ps(node.m_Bounds.max.x);
	const __m128 miny4 = _mm_set1_ps(node.m_Bounds.min.y), maxy4 = _mm_set1_ps(node.m_Bounds.max.y);
	const __m128 minz4 = _mm_set1_ps(node.m_Bounds.min.z), maxz4 = _mm_set1_ps(node.m_Bounds.max.z);
	for(int i=first; i<PrimaryRayBundle::kPackedCount; ++i){
		__m128 tmin4, tmax4;
		const __m128 t0x4 = _mm_div_ps(_mm_sub_ps(minx4, _Rays->Ox4[i]), _Rays->Dx4[i]);
		const __m128 t1x4 = _mm_div_ps(_mm_sub_ps(maxx4, _Rays->Ox4[i]), _Rays->Dx4[i]);
		tmin4 = _mm_min_ps(t0x4, t1x4);
		tmax4 = _mm_max_ps(t0x4, t1x4);
		const __m128 t0y4 = _mm_div_ps(_mm_sub_ps(miny4, _Rays->Oy4[i]), _Rays->Dy4[i]);
		const __m128 t1y4 = _mm_div_ps(_mm_sub_ps(maxy4, _Rays->Oy4[i]), _Rays->Dy4[i]);
		tmin4 = _mm_max_ps(tmin4, _mm_min_ps(t0y4, t1y4));
		tmax4 = _mm_min_ps(tmax4, _mm_max_ps(t0y4, t1y4));
		const __m128 t0z4 = _mm_div_ps(_mm_sub_ps(minz4, _Rays->Oz4[i]), _Rays->Dz4[i] );
		const __m128 t1z4 = _mm_div_ps(_mm_sub_ps(maxz4, _Rays->Oz4[i]), _Rays->Dz4[i]);
		tmin4 = _mm_max_ps(tmin4, _mm_min_ps(t0z4, t1z4));
		tmax4 = _mm_min_ps(tmax4, _mm_max_ps(t0z4, t1z4));

		const __m128 hitmask4 = _mm_and_ps(_mm_cmple_ps(tmin4, tmax4), _mm_cmpgt_ps(tmax4, _mm_setzero_ps()));
		if(_mm_movemask_ps(hitmask4)){
			return i;
		}
	}
	return PrimaryRayBundle::kPackedCount;
}
void Scene::intersectSecondary_r(Ray* _Ray, const MBVHNode& node){
	if(node.children[0]){
		const __m128 Ox4 = _mm_set1_ps(_Ray->O.x), Oy4 = _mm_set1_ps(_Ray->O.y), Oz4 = _mm_set1_ps(_Ray->O.z);
		const __m128 Dx4 = _mm_set1_ps(_Ray->D.x), Dy4 = _mm_set1_ps(_Ray->D.y), Dz4 = _mm_set1_ps(_Ray->D.z);

		__m128 tmin4, tmax4;
		const __m128 t0x4 = _mm_div_ps(_mm_sub_ps(node.minx4, Ox4), Dx4);
		const __m128 t1x4 = _mm_div_ps(_mm_sub_ps(node.maxx4, Ox4), Dx4);
		tmin4 = _mm_min_ps(t0x4, t1x4);
		tmax4 = _mm_max_ps(t0x4, t1x4);
		const __m128 t0y4 = _mm_div_ps(_mm_sub_ps(node.miny4, Oy4), Dy4);
		const __m128 t1y4 = _mm_div_ps(_mm_sub_ps(node.maxy4, Oy4), Dy4);
		tmin4 = _mm_max_ps(tmin4, _mm_min_ps(t0y4, t1y4));
		tmax4 = _mm_min_ps(tmax4, _mm_max_ps(t0y4, t1y4));
		const __m128 t0z4 = _mm_div_ps(_mm_sub_ps(node.minz4, Oz4), Dz4);
		const __m128 t1z4 = _mm_div_ps(_mm_sub_ps(node.maxz4, Oz4), Dz4);
		tmin4 = _mm_max_ps(tmin4, _mm_min_ps(t0z4, t1z4));
		tmax4 = _mm_min_ps(tmax4, _mm_max_ps(t0z4, t1z4));

		const unsigned int mask = _mm_movemask_ps(_mm_and_ps(_mm_cmple_ps(tmin4, tmax4), _mm_cmpgt_ps(tmax4, _mm_setzero_ps())));
		if(mask & 1 ) intersectSecondary_r(_Ray, *node.children[0]);
		if(mask & 2 ) intersectSecondary_r(_Ray, *node.children[1]);
		if(mask & 4 ) intersectSecondary_r(_Ray, *node.children[2]);
		if(mask & 8 ) intersectSecondary_r(_Ray, *node.children[3]);
	}else{
		for(int i=0; i<node.primCount; ++i){
			node.primList[i].intersectSecondary(_Ray);
		}
	}
}
void Scene::intersectSecondary(Ray* _Ray){
	intersectSecondary_r(_Ray, mbvhRoot);
}
void Scene::load(const char* path){
	primList = new Triangle[150000];

	std::vector<Model*> models;
	ImportScene(path, &models);

	for(int i=0; i<models.size(); ++i){
		LoadOBJ(models[i]);
	}

	//Boot tha BVH
	root.setTriangles(primList, primCount);
	root.split();

	mbvhRoot.fromBvh(root);

	
}
void Scene::LoadOBJ(Model* model){

	Material* def = new Material;
	def->color = float3(1.0f, 0.0f, 0.0f);



	std::vector<float3> vertices;
	std::vector<float3> normals;
	std::vector<float2> uvs;
	std::vector<OBJSubMesh> submeshes;
	std::vector<OBJMaterial> materials;
	std::vector<Material*> finalMaterials;
	if(::LoadOBJ(model->name.c_str(), &vertices, &normals, &uvs, &submeshes, &materials)){
		//Convert to triangles
		int start = primCount;
		for(int i=0; i<vertices.size(); i+=3){
			primList[primCount] = Triangle(vertices[i+0], vertices[i+1], vertices[i+2]);
			primList[primCount].n0 = normals[i];
			primList[primCount].n1 = normals[i+1];
			primList[primCount].n2 = normals[i+2];
			primList[primCount].uv0 = uvs[i];
			primList[primCount].uv1 = uvs[i+1];
			primList[primCount].uv2 = uvs[i+2];
			primList[primCount].material = def;
			primCount++;
		}

		//Convert OBJ materials to raytracer materials
		for(int i=0; i<materials.size(); ++i){
			Material* mat = new Material();
			mat->color = materials[i].diffuse;
			mat->refl = materials[i].refl;
			if(!materials[i].diffuseMap.empty()){
				auto lastSlash = model->name.find_last_of('/');
				std::string texturePath;
				if(lastSlash != std::string::npos){
					texturePath = model->name.substr(0, lastSlash+1) + materials[i].diffuseMap;
				}else{
					texturePath = materials[i].diffuseMap;
				}
				mat->texture = new Surface(const_cast<char*>(texturePath.c_str()));
			}
			finalMaterials.push_back(mat);
		}

		//Load extra material information from scene file
		for(auto it = model->groups.begin(); it!=model->groups.end(); ++it){
			Group* g = nullptr;
			int index = -1;
			for(int i=0; i<finalMaterials.size(); ++i){
				if(materials[i].name == it->first){
					g = it->second;
					index = i;
					break;
				}
			}
			if(g){
				finalMaterials[index]->color *= g->intensity;
				finalMaterials[index]->refr = g->refr;
				finalMaterials[index]->refl = g->refl;
				finalMaterials[index]->refrIndex = g->refrIndex;
				finalMaterials[index]->light = g->light;
				finalMaterials[index]->absorption = g->absorption;
			}
		}


		//Set materials
		for(int i=0; i<submeshes.size(); ++i){
			//find the material
			int index = -1;
			for(int j=0; j<materials.size(); ++j){
				if(materials[j].name == submeshes[i].material){
					index = j;
					break;
				}
			}
			if(index < 0) continue;

			int end;
			if( i<submeshes.size()-1) end = submeshes[i+1].offset;
			else end = vertices.size();
			end/=3;

			int astart = submeshes[i].offset / 3;
			for(int j=astart; j<end; ++j){
				primList[start+j].material = finalMaterials[index];
			}
		}
	} 

	int test = 0;
}

// == Renderer class ==========================================================
//    -----------
Tracer::Tracer() :
	m_FpCount(0)
{
	seed(50);
	m_Camera.set( float3( 0, 0, -3 ), float3( 0,180,0 ) );

	m_FpBuffer = new float3a[SCRWIDTH*SCRHEIGHT];
	clear();
}

void Tracer::init(){
	m_Scene.load("assets/scene.txt");
	
	m_JobManager.initialize(8);
	m_JobPtrs = new TraceJob[m_JobManager.maxConcurrency()];

	const int tileCount = (SCRWIDTH/PrimaryRayBundle::kWidth)*(SCRHEIGHT/PrimaryRayBundle::kHeight);
	const int perJob = tileCount / m_JobManager.maxConcurrency();
	for(int i=0; i<m_JobManager.maxConcurrency(); ++i){
		m_JobPtrs[i].tracer = this;
		m_JobPtrs[i].start = i * perJob;
		m_JobPtrs[i].end = (i+1) * perJob;
		m_Jobs.addRaw(&m_JobPtrs[i]);
	}
	m_JobPtrs[m_JobManager.maxConcurrency()-1].end += tileCount % m_JobManager.maxConcurrency();


	seed(0);
}
void Tracer::clear(){
	memset(m_FpBuffer, 0, sizeof(float3a)*SCRWIDTH*SCRHEIGHT);
	m_FpCount = 0;
}
Tracer::~Tracer(){
	delete [] m_FpBuffer;
}
void Tracer::render()
{
	m_JobManager.spawn(&m_Jobs);
	m_Jobs.waitUntilDone();

	m_FpCount++;
}
void TraceJob::run(){
	for(int i=start; i<end; ++i){
		tracer->trace(i);
	}
}
void Tracer::trace(int tileIdx){
	const int xcoord = tileIdx % (SCRWIDTH/PrimaryRayBundle::kWidth);
	const int ycoord = tileIdx / (SCRWIDTH/PrimaryRayBundle::kWidth);

	PrimaryRayBundle rays;
	m_Camera.GenerateRays(&rays, xcoord, ycoord);
	tracePrimary(&rays);
}
void Tracer::tracePrimary(PrimaryRayBundle* _Rays){
	m_Scene.intersectPrimary(_Rays);

	for(int i=0; i<PrimaryRayBundle::kPackedCount; ++i){
		for(int j=0; j<4; ++j){
			Ray ray;
			ray.O = float3(_Rays->Ox4[i].m128_f32[j], _Rays->Oy4[i].m128_f32[j], _Rays->Oz4[i].m128_f32[j]);
			ray.D = float3(_Rays->Dx4[i].m128_f32[j], _Rays->Dy4[i].m128_f32[j], _Rays->Dz4[i].m128_f32[j]);
			ray.t  = _Rays->t4[i].m128_f32[j];
			ray.u = _Rays->u4[i].m128_f32[j];
			ray.v = _Rays->v4[i].m128_f32[j];
			ray.prim = (Triangle*)_Rays->prim4[i].m128i_i32[j];
			m_FpBuffer[_Rays->addr4[i].m128i_i32[j]] += trace(&ray, 1.0f,  0);
		}
	}
}
float3 Tracer::traceSecondary(Ray* _Ray, int bounce){
	if(bounce >= 4) return float3(0,0,0);

	//Russian roulette
	if(randf_oo() > 0.5f) return float3(0,0,0);

	m_Scene.intersectSecondary(_Ray);
	return trace(_Ray, 2.0f, bounce);
}
float3 DiffuseDirection(const float3& normal){
	const float3 absNormal(fabsf(normal.x), fabsf(normal.y), fabsf(normal.z));
	float3 axis(0,0,1);
	if(absNormal.x <= absNormal.y && absNormal.y <= absNormal.z){
		axis = float3(1,0,0);
	}else if(absNormal.y <= absNormal.x && absNormal.y <= absNormal.z){
		axis = float3(0,1,0);
	}
	const float3 u1 = Cross(axis, normal);
	const float3 u2 = Cross(u1, normal);
	const float rand1 = randf_oo();
	const float tmp1 = sqrtf(1.0f - rand1);
	const float tmp2 = 2.0f * PI * randf_oo();

	const float3 dir = u1 * (cosf(tmp2) * tmp1) +
						u2 * (sinf(tmp2) * tmp1) +
						normal * sqrtf(rand1);
	return Dot(dir, normal) > 0 ? dir : dir * -1.0f;
}
float3 getColorAtIP(Ray& _Ray){
	if(_Ray.prim->material->texture){
		Surface& tex = *_Ray.prim->material->texture;
		const float2 uv = _Ray.prim->uv0 + _Ray.u * (_Ray.prim->uv1-_Ray.prim->uv0) + _Ray.v * (_Ray.prim->uv2-_Ray.prim->uv0);
		const float2 nuv(fmodf(uv.x+1000.0f, 1.0f), fmodf(uv.y+1000.0f, 1.0f));
		const int x = (int)(nuv.x * (float)(tex.GetWidth()-1));
		const int y =  tex.GetHeight() - 1 - (int)(nuv.y * (float)(tex.GetHeight()-1));
		const Pixel color =tex.GetBuffer()[y*tex.GetPitch()+x];
		return float3(
			(float)((color >> 16)&0xff),
			(float)((color >> 8)&0xff),
			(float)((color)&0xff)
		)/255.0f;
	}else{
		return _Ray.prim->material->color;
	}
}
float3 Tracer::trace(Ray* _Ray, float power, int bounce){
	if(!_Ray->prim || !_Ray->prim->material) return float3(0,0,0);
	const Material& mat = *_Ray->prim->material;

	//Light
	float3 color = getColorAtIP(*_Ray);
	if(mat.light>EPSILON){
		return color;
	}

	//Fresnel
	float3 normal = _Ray->prim->n0 + _Ray->u * (_Ray->prim->n1-_Ray->prim->n0) + _Ray->v  * (_Ray->prim->n2-_Ray->prim->n0);
	float nt, nnt, ddn, cosT2;
	float refl = mat.refl;
	float refr = mat.refr;
	if(mat.refr > EPSILON){
		ddn = Dot(normal, _Ray->D);
		if(ddn > 0){
			nnt = mat.refrIndex;
			nt = 1.0f/nnt;		

			//Exiting ray (color is beers law)
			const float c1 = mat.absorption * -_Ray->t;
			color = float3(expf(c1*(1.0f-color.x)), expf(c1*(1.0f-color.y)), expf(c1*(1.0f-color.z)));
		}else{
			normal = -normal;
			ddn = -ddn;
			nt = mat.refrIndex;
			nnt = 1.0f/nt;

			//Ingoing ray (no energy loss, will be handled when the ray goes out)
			color = float3(1,1,1);
		}

		cosT2 = 1.0f - nnt*nnt * (1.0f - ddn*ddn);
		if(cosT2 <= 0){
			refl += refr;
			refr = 0.0f;
		}else{
			const float v = ((nt-1)*(nt-1)) / ((nt+1)*(nt+1));
			const float c1 = (1-ddn);
			const float Wr =  v  + (1-v)*c1*c1*c1*c1*c1;
			const float Wt = 1 - Wr;
			refl = refl + Wr * refr;
			refr = Wt * refr;
		}


	}
	const float path = randf_oo();
	//Reflection
	if(path < refl){
		Ray r;
		r.O = _Ray->O + _Ray->D * _Ray->t;
		r.D = Reflect(_Ray->D, normal);
		r.O += r.D*EPSILON;
		r.t = 1e34f;
		r.prim = nullptr;
		return color * power * traceSecondary(&r, bounce+1);
	}

	//Refraction
	if(path < refl+refr){
		const float3 D = Normalize(nnt * _Ray->D - normal * (nnt*ddn - sqrtf(cosT2)));
		Ray r;
		r.O = _Ray->O + _Ray->D * _Ray->t + D * EPSILON;
		r.D = D;
		r.t = 1e34f;
		r.prim = nullptr;
		return color * power *  traceSecondary(&r, bounce+1);
	}

	{
		//Diffuse
		const float3 dir = DiffuseDirection(normal);

		Ray r;
		r.O = _Ray->O + _Ray->D * _Ray->t;
		r.D = dir;
		r.O += r.D*EPSILON;
		r.t = 1e34f;
		r.prim = nullptr;
		return color * power * traceSecondary(&r, bounce+1);
	}
}
Camera& Tracer::camera(){
	return m_Camera;
}
Scene& Tracer::scene(){
	return m_Scene;
}
const float3a* Tracer::getAccumulationBuffer() const{
	return m_FpBuffer;
}
int Tracer::getAccumulationBufferCount() const{
	return m_FpCount;
}
// EOF