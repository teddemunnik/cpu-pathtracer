// Template for GP1, version 1
// IGAD/NHTV - Jacco Bikker - 2006-2013

#pragma once
#include "BVH.h"
#include "MBVH.h"
#include "template.h"
#include "surface.h"
#include <tt/jobsystem/jobsystem.h>

#define FEATURE_DOF_ENABLED 0
#define FEATURE_AA_ENABLED 1
#define FEATURE_SKYBOX_ENABLED 1


#define DOT128(ax,ay,az,bx,by,bz) _mm_add_ps(_mm_add_ps(_mm_mul_ps(ax, bx), _mm_mul_ps(ay, by)), _mm_mul_ps(az, bz))
#define SELECTMASK_PS(TRUE_VALUE, FALSE_VALUE, MASK) _mm_or_ps(_mm_and_ps(MASK, TRUE_VALUE), _mm_andnot_ps(MASK, FALSE_VALUE))
#define SELECTMASK_SI128(TRUE_VALUE, FALSE_VALUE, MASK) _mm_or_si128(_mm_and_si128(MASK, TRUE_VALUE), _mm_andnot_si128(MASK, FALSE_VALUE))

using namespace tt;

struct Model;
namespace Tmpl8 {
struct PrimaryRayBundle{
	const static unsigned int kWidth = 16;
	const static unsigned int kHeight = 16;
	const static unsigned int kRayCount = kWidth*kHeight;
	const static unsigned int kPackedCount = kWidth*kHeight/4;

	__m128 Ox4[kPackedCount];
	__m128 Oy4[kPackedCount];
	__m128 Oz4[kPackedCount];
	__m128 Dx4[kPackedCount];
	__m128 Dy4[kPackedCount];
	__m128 Dz4[kPackedCount];

	union {
		__m128 t4[kPackedCount];
		float t[kRayCount];
	};

	//Ray frustum
	__m128 fx4, fy4, fz4, fd4;
	union{
		__m128i sign4[3];
		unsigned int sign[12];
	};

	//Writeback address
	union{
		__m128i addr4[kPackedCount];
		unsigned int addr[kRayCount];
	};

	//U and V for interpolation
	union{ __m128 u4[kPackedCount]; float u[kRayCount]; };
	union{ __m128 v4[kPackedCount]; float v[kRayCount]; };
	//Hit primitive to extract info for shading
	union{ __m128i prim4[kPackedCount]; Triangle* prim[kRayCount]; };

};
struct Ray{
	float3 O;
	float3 D;
	float t;
	float u;
	float v;
	struct Triangle* prim;
};
struct Triangle{
	Triangle(){}
	Triangle(const float3& _0, const float3& _1, const float3& _2) :
		v0(_0), v1(_1), v2(_2), N(Normalize(Cross(v1-v0, v2-v0))){}
	float3 v0, v1, v2;
	float3 n0, n1, n2;
	float2 uv0, uv1, uv2;
	float3 N;
	class Material* material;

	float3 getCenter() const;
	AABB getBounds() const;
	void intersectPrimary(PrimaryRayBundle* _Rays, int first) const;
	void intersectSecondary(Ray* _Ray) const;

};
class Scene{
private:
	BVHNode root;
	MBVHNode mbvhRoot;

	//BVH primitives
	Triangle* primList;
	int primCount;

	//MBVH primitives
	Triangle* primList2;
public:
	Scene();
	~Scene();
	void load(const char* path);
	void LoadOBJ(Model* model);

	void intersectPrimary(PrimaryRayBundle* _Rays);
	void intersectPrimary_r(PrimaryRayBundle* _Rays, const BVHNode& node, int first=0);
	int  findFirstPrimary(PrimaryRayBundle* _Rays, const BVHNode& node, int first);

	void intersectSecondary(Ray* _Ray);
	void intersectSecondary_r(Ray* _Ray, const MBVHNode& node);

};
class Camera
{
public:

	// methods
	void Set( float3 _Pos, float3 _Direction );
	void GenerateRays( PrimaryRayBundle* _Rays, int _X, int _Y);
	// data members
	float3 pos;						// camera position
	float3 V;						// normalized view direction
	float3 p1, p2, p3, p4;			// corners of screen plane
};
class Material{
public:
	float3 color;
	Surface* texture;
	float refl;
	float refr;
	float refrIndex;
	float light;
};

// == class Tracer ==========================================================
//    --------------
class TraceJob : public IJob{
public:
	class Tracer* tracer;
	int start, end;
	void run();
};
class Tracer
{
private:
	//Accumulation buffer
	float3a* m_FpBuffer;
	int m_FpCount;
	Scene m_Scene;
	Camera m_Camera;

	JobManager m_JobManager;
	JobGroup m_Jobs;
	TraceJob* m_JobPtrs;

public:
	// constructor
	Tracer();
	~Tracer();

	void init();
	void clear();
	void render();

	void trace(int tileIdx);
	void tracePrimary(PrimaryRayBundle* _Rays);
	float3 traceSecondary(Ray* _Ray);
	void trace(Ray* _Ray);

	Scene& scene();
	Camera& camera();

	const float3a* getAccumulationBuffer() const;
	int getAccumulationBufferCount() const;

	
};

};

// EOF