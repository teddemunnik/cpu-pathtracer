// Template for GP1, version 1
// IGAD/NHTV - Jacco Bikker - 2006-2013

#pragma once
#include "BVH.h"
#include "MBVH.h"
#include "template.h"
#include "surface.h"
#include <tt/jobsystem/jobsystem.h>

#define FEATURE_DOF_ENABLED 1
#define FEATURE_AA_ENABLED 1
#define FEATURE_SKYBOX_ENABLED 1
#define FEATURE_BILINEAR_ENABLED 1
//#define NUM_THREADS 8
#define SCENE_PATH "assets/scene2.txt"
#define ENVIRONMENT_MAP_PATH "assets/sky.hdr"

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

public:
		//HDR Skybox
	class HDRSurface* skytexture;
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
private:
	float4x4 m_Transform;
	float m_ApertureSize;
	float m_FocalDistance;

	void _invalidate();
public:
	Camera();
	~Camera();
	float4x4 transform() const;
	void setTransform(const float4x4& transform);
	void set( float3 _Pos, float3 _Direction );
	//Depth of field settings; aperture of 0 is no depth of field
	void setAperture(float apertureSize, float focalDistance);
	void GenerateRays( PrimaryRayBundle* _Rays, int _X, int _Y);


	float3 p1, p2, p3, p4;			// corners of screen plane
};
class Material{
public:
	Material() : texture(nullptr), refl(0.0f), refr(0.0f), refrIndex(0.0f), light(0.0f){}
	float3 color;
	Surface* texture;
	float refl;
	float refr;
	float refrIndex;
	float light;
	float absorption;
};

// == class Tracer ==========================================================
//    --------------
class TraceJob : public IJob{
public:
	class Tracer* tracer;
	void run();
};

class Tracer
{
public:
	enum ShadeMode{
		kShadeSimple,
		kShadeComplex
	};
private:
	//Accumulation buffer
	float3a* m_FpBuffer;
	int m_FpCount;
	Scene m_Scene;
	Camera m_Camera;

	//Multithreading
	JobManager m_JobManager;
	JobGroup m_Jobs;
	TraceJob* m_JobPtrs;
	__declspec(align(32)) LONG m_CurrentTileId;

	bool m_Simple;

public:
	// constructor
	Tracer();
	~Tracer();

	void init();
	void clear();
	void render();

	bool traceNext();
	void tracePrimary(PrimaryRayBundle* _Rays);
	float3 traceSecondary(Ray* _Ray, int bounce=0);
	float3 trace(Ray* _Ray, float power, int bounce=0);

	Scene& scene();
	Camera& camera();

	void setShadingMode(ShadeMode mode);

	const float3a* getAccumulationBuffer() const;
	int getAccumulationBufferCount() const;

	
};

};

// EOF