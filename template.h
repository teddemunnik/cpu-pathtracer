// Template for GP1, version 1
// IGAD/NHTV - Jacco Bikker - 2006-2013

#pragma once

#define SCRWIDTH	400
#define SCRHEIGHT	400

#include "math.h"
#include "stdlib.h"
#include "emmintrin.h"
#include "stdio.h"
#include "windows.h"

inline float Rand( float a_Range ) { return ((float)rand() / RAND_MAX) * a_Range; }
int filesize( FILE* f );
#define MALLOC64(x) _aligned_malloc(x,64)
#define FREE64(x) _aligned_free(x)

namespace Tmpl8 {

#ifndef __INTEL_COMPILER
#define restrict
#endif

#define MIN(a,b) (((a)>(b))?(b):(a))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define _fabs	fabsf
#define _cos	cosf
#define _sin	sinf
#define _acos	acosf
#define _floor	floorf
#define _ceil	ceilf
#define _sqrt	sqrtf
#define _pow	powf
#define _exp	expf

#define CROSS(A,B)		vector3(A.y*B.z-A.z*B.y,A.z*B.x-A.x*B.z,A.x*B.y-A.y*B.x)
#define DOT(A,B)		(A.x*B.x+A.y*B.y+A.z*B.z)
#define NORMALIZE(A)	{float l=1/_sqrt(A.x*A.x+A.y*A.y+A.z*A.z);A.x*=l;A.y*=l;A.z*=l;}
#define CNORMALIZE(A)	{float l=1/_sqrt(A.r*A.r+A.g*A.g+A.b*A.b);A.r*=l;A.g*=l;A.b*=l;}
#define LENGTH(A)		(_sqrt(A.x*A.x+A.y*A.y+A.z*A.z))
#define SQRLENGTH(A)	(A.x*A.x+A.y*A.y+A.z*A.z)
#define SQRDISTANCE(A,B) ((A.x-B.x)*(A.x-B.x)+(A.y-B.y)*(A.y-B.y)+(A.z-B.z)*(A.z-B.z))

#define PI				3.14159265358979323846264338327950288419716939937510582097494459072381640628620899862803482534211706798f
#define EPSILON			0.0001f

#define PREFETCH(x) _mm_prefetch((const char*)(x),_MM_HINT_T0)
#define PREFETCH_ONCE(x) _mm_prefetch((const char*)(x),_MM_HINT_NTA)
#define PREFETCH_WRITE(x) _m_prefetchw((const char*)(x))

#define loadss(mem)			_mm_load_ss((const float*const)(mem))
#define broadcastps(ps)		_mm_shuffle_ps((ps),(ps), 0)
#define broadcastss(ss)		broadcastps(loadss((ss)))

inline unsigned __int64 RDTSC()
{
   _asm _emit 0x0F
   _asm _emit 0x31
}

class TimerRDTSC
{
   unsigned __int64  start_cycle;
   unsigned __int64  end_cycle;
public:
   inline void Start() { start_cycle = RDTSC(); }
   inline void Stop() { end_cycle = RDTSC(); }
   unsigned __int64 Interval() { return end_cycle - start_cycle; }
};

template <class T> class GList
{
public:
	// constructor / destructor
	GList() : size( 0 ), allocated( 256 ) { data = MALLOC16( allocated, T ); }
	GList( int _allocated ) : size( 0 ), allocated( _allocated ) { data = MALLOC16( allocated, T ); }
	~GList() { FREE16( data ); data = 0; }
	void Add( T _Item ) 
	{ 
		if ((size + 1) == allocated) allocated += (allocated >> 1), data = (T*)_aligned_realloc( data, sizeof( T ) * allocated, 16 );
		data[size++] = _Item;
	}
	T& Add() 
	{ 
		if ((size + 1) == allocated) allocated += (allocated >> 1), data = (T*)_aligned_realloc( data, sizeof( T ) * allocated, 16 );
		return data[size++];
	}
	// operators
	inline T& operator[] ( int _Pos ) { return data[_Pos]; }
	// data members
	T* data;
	int size, allocated;
};

struct timer 
{ 
	typedef long long value_type; 
	static double inv_freq; 
	value_type start; 
	timer() : start( get() ) { init(); } 
	float elapsed() const { return (float)((get() - start) * 0.001); } 
	static value_type get() 
	{ 
		LARGE_INTEGER c; 
		QueryPerformanceCounter( &c ); 
		return c.QuadPart; 
	} 
	static double to_time(const value_type vt) { return double(vt) * inv_freq; } 
	void reset() { start = get(); }
	static void init() 
	{ 
		LARGE_INTEGER f; 
		QueryPerformanceFrequency( &f ); 
		inv_freq = 1000./double(f.QuadPart); 
	} 
}; 

#ifndef MIN
#	define MIN(a,b)(((a)<(b))?(a):(b))
#endif
#ifndef MAX
#	define MAX(a,b)(((a)<(b))?(b):(a))
#endif
#ifndef MIN4
#	define MIN4(a,b,c,d)(MIN(MIN(a,b),MIN(c,d)))
#endif
#ifndef MAX4
#	define MAX4(a,b,c,d)(MAX(MAX(a,b),MAX(c,d)))
#endif

typedef unsigned int uint;

#define BADFLOAT(x) ((*(uint*)&x & 0x7f000000) == 0x7f000000)
void*mymalloc(int s);
//not the right place for this kind of stuff
#define MALLOC(x,t) ((t*)mymalloc((x)*sizeof(t)))
#define FREE(x)(_aligned_free((x)))
#define CAST(x,t)(reinterpret_cast<t>(x))
#ifdef INLINE
#undef INLINE
#endif
#define INLINE __forceinline


typedef unsigned char byte;
class byte4;
class int2;
class int3;
class float2;
class float3;

class byte4
{
public:
	union
	{
		struct { byte x, y, z, w; };
		byte cell[4];
	};
	byte4(){}
	byte4( const byte _X, const byte _Y, const byte _Z, const byte _W) : x( _X ), y( _Y ), z( _Z ), w( _W ) {}

	INLINE byte& operator[] ( unsigned int i ) { return cell[i]; }
	// vector operators
	INLINE void operator += ( byte4 a ){ x += a.x; y += a.y; z += a.z; w += a.w; }
	INLINE void operator -= ( byte4 a ){ x -= a.x; y -= a.y; z -= a.z; w -= a.w; }
	INLINE void operator *= ( byte4 a ){ x *= a.x; y *= a.y; z *= a.z; w *= a.w; }
	INLINE void operator /= ( byte4 a ){ x /= a.x; y /= a.y; z /= a.z; w /= a.w; }
	// int operators
	INLINE void operator *= ( byte a ) { x *= a; y *= a; z *= a; w *= a; }
	INLINE void operator /= ( byte a ) { x /= a; y /= a; z /= a; w /= a; }
	// vector operators
	INLINE byte4 operator + ( const byte4& a ) { return byte4( x + a.x, y + a.y, z + a.z, w + a.w ); }
	INLINE byte4 operator - ( const byte4& a ) { return byte4( x - a.x, y - a.y, z - a.z, w - a.w ); }
	INLINE byte4 operator * ( const byte4& a ) { return byte4( x * a.x, y * a.y, z * a.z, w * a.w ); }
	INLINE byte4 operator / ( const byte4& a ) { return byte4( x / a.x, y / a.y, z / a.z, w / a.w ); }
	// int operators
	INLINE byte4 operator * ( const int& a ) { return byte4( x * a, y * a, z / a, w / a ); }
	INLINE byte4 operator / ( const int& a ) { return byte4( x / a, y / a, z / a, w / a ); }

	bool operator == ( const byte4& a ) { return a.x == x && a.y == y && a.z == z && a.w == w; }
	bool operator != ( const byte4& a ) { return a.x != x || a.y != y || a.z != z || a.z != z; }
};

class int2
{
public:
	union
	{
		struct { int x, y; };
		int cell[2];
	};
	int2() {}
	int2( const int _X, const int _Y) : x( _X ), y( _Y ) {}

	INLINE int& operator[] ( unsigned int i ) { return cell[i]; }
	// vector operators
	INLINE void operator += ( int2 a ) { x +=  a.x; y +=  a.y; }
	INLINE void operator -= ( int2 a ) { x -= a.x; y -= a.y; }
	INLINE void operator *= ( int2 a ) { x *= a.x; y *= a.y; }
	INLINE void operator /= ( int2 a ) { x /= a.x; y /= a.y; }
	// int operators
	INLINE void operator *= ( int a ) { x *= a; y *= a; }
	INLINE void operator /= ( int a ) { x /= a; y /= a; }
	// vector operators
	INLINE int2 operator + ( const int2& a ) { return int2( x + a.x, y + a.y ); }
	INLINE int2 operator - ( const int2& a ) { return int2( x - a.x, y - a.y ); }
	INLINE int2 operator * ( const int2& a ) { return int2( x * a.x, y * a.y ); }
	INLINE int2 operator / ( const int2& a ) { return int2( x / a.x, y / a.y ); }
	// int operators
	INLINE int2 operator * ( const int& a ) { return int2( x * a, y * a ); }
	INLINE int2 operator / ( const int& a ) { return int2( x / a, y / a ); }

	bool operator == ( const int2& a ) { return a.x == x && a.y == y; }
	bool operator != ( const int2& a ) { return a.x != x || a.y != y; }
};

class int3
{
public:
	union
	{
		struct { int x, y, z; };
		int cell[3];
	};
	int3() {}
	int3( const int _X, const int _Y, const int _Z) : x( _X ), y( _Y ), z( _Z ) {}

	INLINE int&operator[] ( int i ) { return cell[i]; }
	// vector operators
	INLINE void operator += ( int3 a ) { x += a.x; y += a.y; z += a.z; }
	INLINE void operator -= ( int3 a ) { x -= a.x; y -= a.y; z -= a.z; }
	INLINE void operator *= ( int3 a ) { x *= a.x; y *= a.y; z *= a.z; }
	INLINE void operator /= ( int3 a ) { x /= a.x; y /= a.y; z /= a.z; }
	// int operators
	INLINE void operator *= ( int a ) { x *= a; y *= a; z *= a; }
	INLINE void operator /= ( int a ) { x /= a; y /= a; z /= a; }
	// vector operators
	INLINE int3 operator + ( const int3& a ) { return int3( x + a.x, y + a.y, z + a.z ); }
	INLINE int3 operator - ( const int3& a ) { return int3( x - a.x, y - a.y, z - a.z ); }
	INLINE int3 operator * ( const int3& a ) { return int3( x * a.x, y * a.y, z * a.y ); }
	INLINE int3 operator / ( const int3& a ) { return int3( x / a.x, y / a.y, z / a.z ); }
	// int operators
	INLINE int3 operator * ( const int& a ) { return int3( x * a, y * a, z * a ); }
	INLINE int3 operator / ( const int& a ) { return int3( x / a, y / a, z / a ); }

	bool operator == ( const int3& a ) { return a.x == x && a.y == y && a.z == z; }
	bool operator != ( const int3& a ) { return a.x != x || a.y != y || a.z != z; }
};

class float2
{
public:
	union
	{
		struct { float x, y; };
		float cell[2];
	};
	float2() {}
	float2( const float _X, const float _Y ) : x( _X ), y( _Y ) {}
	float2(int2 a) : x((float)a.x), y((float)a.y){}

	INLINE float  operator[] ( int i ) const { return cell[i]; }
	INLINE float& operator[] ( int i ) 		 { return cell[i]; }
	INLINE float2 operator - (		 ) 		 { return float2( -x, -y ); }

	friend INLINE float2 operator + ( const float2&a, const float2&b ){ return float2(a.x + b.x, a.y + b.y); }
	friend INLINE float2 operator - ( const float2&a, const float2&b ){ return float2(a.x - b.x, a.y - b.y); }
	friend INLINE float2 operator * ( const float2&a, const float2&b ){ return float2(a.x * b.x, a.y * b.y); }
	friend INLINE float2 operator / ( const float2&a, const float2&b ){ return float2(a.x / b.x, a.y / b.y); }
	friend INLINE float2 operator + ( const float2&a, const float &b ){ return float2(a.x + b  , a.y + b  ); }
	friend INLINE float2 operator - ( const float2&a, const float &b ){ return float2(a.x - b  , a.y - b  ); }
	friend INLINE float2 operator * ( const float2&a, const float &b ){ return float2(a.x * b  , a.y * b  ); }
	friend INLINE float2 operator / ( const float2&a, const float &b ){ return float2(a.x / b  , a.y / b  ); }
	friend INLINE float2 operator * ( const float &a, const float2&b ){ return float2(a   * b.x, a   * b.y); }
	friend INLINE float2 operator / ( const float &a, const float2&b ){ return float2(a   / b.x, a   / b.y); }

	// vector operators
	INLINE void operator += ( const float2 a ) { *this = *this + a; }
	INLINE void operator -= ( const float2 a ) { *this = *this - a; }
	INLINE void operator *= ( const float2 a ) { *this = *this * a; }
	INLINE void operator /= ( const float2 a ) { *this = *this / a; }
	INLINE void operator *= ( const float  a ) { *this = *this * a; }
	INLINE void operator /= ( const float  a ) { *this = *this / a; }

	bool operator == ( const float2& a ) { return a.x == x && a.y == y; }
	bool operator != ( const float2& a ) { return a.x != x || a.y != y; }
};

class float3
{
public:
	union
	{
		struct { float2 xy; };
		struct { float x, y, z; };
		float cell[3];
	};
	float3() {}
	float3( const float2 _V, const float _Z ) : xy( _V ), z( _Z ) {}
	float3( const float _X, const float _Y, const float _Z ) : x( _X ), y( _Y ), z( _Z ) {}

	INLINE float  operator[] ( int i ) const { return cell[i]; }
	INLINE float& operator[] ( int i ) 		 { return cell[i]; }
	INLINE float3 operator - (		 ) 		 { return float3( -x, -y, -z ); }

	friend INLINE float3 operator + ( const float3&a, const float3&b ){ return float3(a.x + b.x, a.y + b.y, a.z + b.z); }
	friend INLINE float3 operator - ( const float3&a, const float3&b ){ return float3(a.x - b.x, a.y - b.y, a.z - b.z); }
	friend INLINE float3 operator * ( const float3&a, const float3&b ){ return float3(a.x * b.x, a.y * b.y, a.z * b.z); }
	friend INLINE float3 operator / ( const float3&a, const float3&b ){ return float3(a.x / b.x, a.y / b.y, a.z / b.z); }
	friend INLINE float3 operator * ( const float3&a, const float &b ){ return float3(a.x * b  , a.y * b  , a.z * b  ); }
	friend INLINE float3 operator / ( const float3&a, const float &b ){ return float3(a.x / b  , a.y / b  , a.z / b  ); }
	friend INLINE float3 operator * ( const float &a, const float3&b ){ return float3(a   * b.x, a   * b.y, a   * b.z); }
	friend INLINE float3 operator / ( const float &a, const float3&b ){ return float3(a   / b.x, a   / b.y, a   / b.z); }

	// vector operators
	INLINE void operator += ( const float3 a ) { *this = *this + a; }
	INLINE void operator -= ( const float3 a ) { *this = *this - a; }
	INLINE void operator *= ( const float3 a ) { *this = *this * a; }
	INLINE void operator /= ( const float3 a ) { *this = *this / a; }
	INLINE void operator *= ( const float  a ) { *this = *this * a; }
	INLINE void operator /= ( const float  a ) { *this = *this / a; }

	bool operator == ( const float3& a ) { return a.x == x && a.y == y && a.z == z; }
	bool operator != ( const float3& a ) { return a.x != x || a.y != y || a.z != z; }

	
	static float3 Lerp(const float3&a, const float3&b, float t) { return a * (1.0f - t) + b * t; }

};

//aligned float3 ( for SIMD )
__declspec(align(16))
class float3a
{
public:
	union
	{
		__m128 quad;
		struct { float2 xy; };
		struct { float x, y, z, w; };
		float cell[4];
	};
	float3a() {}
	float3a( const float2 _V, const float _Z ) : xy( _V ), z( _Z ) {}
	float3a( const float _X, const float _Y, const float _Z ) : x( _X ), y( _Y ), z( _Z ) {}
	float3a( const float3 _V ) : x( _V.x ), y( _V.y ), z( _V.z ) {}

	INLINE float   operator[] ( int i ) const { return cell[i]; }
	INLINE float&  operator[] ( int i ) 	  { return cell[i]; }
	INLINE float3a operator - (		 ) 		  { return float3a( -x, -y, -z ); }

	friend INLINE float3a operator + ( const float3a&a, const float3a&b ){ return float3a(a.x + b.x, a.y + b.y, a.z + b.z); }
	friend INLINE float3a operator - ( const float3a&a, const float3a&b ){ return float3a(a.x - b.x, a.y - b.y, a.z - b.z); }
	friend INLINE float3a operator * ( const float3a&a, const float3a&b ){ return float3a(a.x * b.x, a.y * b.y, a.z * b.z); }
	friend INLINE float3a operator / ( const float3a&a, const float3a&b ){ return float3a(a.x / b.x, a.y / b.y, a.z / b.z); }
	friend INLINE float3a operator * ( const float3a&a, const float  &b ){ return float3a(a.x * b  , a.y * b  , a.z * b  ); }
	friend INLINE float3a operator / ( const float3a&a, const float  &b ){ return float3a(a.x / b  , a.y / b  , a.z / b  ); }
	friend INLINE float3a operator * ( const float  &a, const float3a&b ){ return float3a(a   * b.x, a   * b.y, a   * b.z); }
	friend INLINE float3a operator / ( const float  &a, const float3a&b ){ return float3a(a   / b.x, a   / b.y, a   / b.z); }

	// vector operators
	INLINE void operator += ( const float3a&a ) { *this = *this + a; }
	INLINE void operator -= ( const float3a&a ) { *this = *this - a; }
	INLINE void operator *= ( const float3a&a ) { *this = *this * a; }
	INLINE void operator /= ( const float3a&a ) { *this = *this / a; }
	INLINE void operator *= ( const float   a ) { *this = *this * a; }
	INLINE void operator /= ( const float   a ) { *this = *this / a; }
};


class float4
{
public:
	union
	{
		struct{ float2 xy; float2 zw; };
		struct{ float3 xyz; };
		struct{ float x, y, z, w; };
		float cell[4];
	};
	float4() {}
	float4( const float2 _XY, const float2 _ZW ) : xy( _XY ), zw( _ZW ) {}
	float4( const float3 _XYZ, const float _W ) : xyz( _XYZ ), w( _W ) {}
	float4( const float _X, const float _Y, const float _Z, const float _W) : x( _X ), y( _Y ), z( _Z ), w( _W ) {}

	INLINE float  operator[] ( int i ) const { return cell[i]; }
	INLINE float& operator[] ( int i ) 		 { return cell[i]; }
	INLINE float4 operator - (		 ) 		  { return float4( -x, -y, -z, -w ); }

	friend INLINE float4 operator + ( const float4&a, const float4&b ){ return float4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
	friend INLINE float4 operator - ( const float4&a, const float4&b ){ return float4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
	friend INLINE float4 operator * ( const float4&a, const float4&b ){ return float4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w); }
	friend INLINE float4 operator / ( const float4&a, const float4&b ){ return float4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w); }
	friend INLINE float4 operator * ( const float4&a, const float &b ){ return float4(a.x * b  , a.y * b  , a.z * b  , a.w * b  ); }
	friend INLINE float4 operator / ( const float4&a, const float &b ){ return float4(a.x / b  , a.y / b  , a.z / b  , a.w / b  ); }
	friend INLINE float4 operator * ( const float &a, const float4&b ){ return float4(a   * b.x, a   * b.y, a   * b.z, a   * b.w); }
	friend INLINE float4 operator / ( const float &a, const float4&b ){ return float4(a   / b.x, a   / b.y, a   / b.z, a   / b.w); }

	// vector operators
	INLINE void operator += ( const float4 a ) { *this = *this + a; }
	INLINE void operator -= ( const float4 a ) { *this = *this - a; }
	INLINE void operator *= ( const float4 a ) { *this = *this * a; }
	INLINE void operator /= ( const float4 a ) { *this = *this / a; }
	INLINE void operator *= ( const float  a ) { *this = *this * a; }
	INLINE void operator /= ( const float  a ) { *this = *this / a; }
	
	bool operator == ( const float4& a ) const { return a.x == x && a.y == y && a.z == z && a.w == w; }
	bool operator != ( const float4& a ) const { return a.x != x || a.y != y || a.z != z || a.z != z; }
};

static INLINE float3 Min( const float3& a, const float3& b ) { return float3( MIN( a.x, b.x ), MIN( a.y, b.y ), MIN( a.z, b.z ) ); }
static INLINE float3a Min( const float3a& a, const float3a& b )
{
	float3a r;
	r.quad = _mm_min_ps(a.quad,b.quad);
	return r;
}
static INLINE int2 Min( const int2& a, const int2& b) { return int2( MIN( a.x, b.x ), MIN( a.y, b.y ) ); }

static INLINE float3 Max( const float3& a, const float3& b) { return float3( MAX( a.x, b.x ), MAX( a.y, b.y ), MAX( a.z, b.z ) ); }
static INLINE float3a Max( const float3a& a, const float3a& b )
{
	float3a r;
	r.quad = _mm_max_ps(a.quad,b.quad);
	return r;
}
static INLINE int2 Max( const int2& a, const int2& b) { return int2( MAX( a.x, b.x ), MAX( a.y, b.y ) ); }

// reciprocal
static INLINE float2 Rcp( const float2& a ) { return float2( 1 / a.x, 1 / a.y ); }
static INLINE float3 Rcp( const float3& a ) { return float3( 1 / a.x, 1 / a.y, 1 / a.z ); }
static INLINE float4 Rcp( const float4& a ) { return float4( 1 / a.x, 1 / a.y, 1 / a.z, 1 / a.w ); }
// dot product
static INLINE float Dot( const float2& a, const float2& b ) { return a.x * b.x + a.y * b.y; }
static INLINE float Dot( const float3& a, const float3& b ) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static INLINE float Dot( const float4& a, const float4& b ) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }
// cross product
static INLINE float  Cross( const float2& a, const float2& b ) { return a.x * b.y - a.y * b.x; }
static INLINE float3 Cross( const float3& a, const float3& b ) { return float3( a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x ); }
// vector length
static INLINE float Length( const float2& _Vec ) { return sqrtf( Dot( _Vec, _Vec ) ); }
static INLINE float Length( const float3& _Vec ) { return sqrtf( Dot( _Vec, _Vec ) ); }
static INLINE float Length( const float4& _Vec ) { return sqrtf( Dot( _Vec, _Vec ) ); }
static INLINE float LengthSquare( const float2& _Vec ) { return Dot( _Vec, _Vec ); }
static INLINE float LengthSquare( const float3& _Vec ) { return Dot( _Vec, _Vec ); }
static INLINE float LengthSquare( const float4& _Vec ) { return Dot( _Vec, _Vec ); }
// vector normalize
static INLINE float2 Normalize( float2& _Vec ) { return _Vec / Length( _Vec ); }
static INLINE float3 Normalize( float3& _Vec ) { return _Vec / Length( _Vec ); }
static INLINE float4 Normalize( float4& _Vec ) { return _Vec / Length( _Vec ); }
// vector reflect
static INLINE float3 Reflect(const  float3& i, const float3& n ) { return i - 2.0f * n * Dot( n,i ); }

class Quaternion // not checked!
{
public:
	float x,y,z,w;
	Quaternion(){}
	Quaternion(float _x,float _y,float _z,float _w):x(_x),y(_y),z(_z),w(_w)
	{
		
	}
	static Quaternion AxisAngle(float3 _Axis, float _Angle)
	{
		Quaternion r;
		float result = sin(_Angle/2.0f);
		r.w = cosf(_Angle/2.0f);
		r.x = _Axis.x * result;
		r.y = _Axis.y * result;
		r.z = _Axis.z * result;
		return r;
	}
	Quaternion Quaternion::operator *(const Quaternion &q)
	{
		Quaternion r;
		r.w = w * q.w - x * q.x - y * q.y - z * q.z;
		r.x = w * q.x + x * q.w + y * q.z - z * q.y;
		r.y = w * q.y + y * q.w + z * q.x - x * q.z;
		r.z = w * q.z + z * q.w + x * q.y - y * q.x;
		return r;
	}

	void operator *= ( const Quaternion &q ) { *this = *this * q; }
};

class float4x4
{
public:
	union
	{
		struct { float4 row[4]; };
		struct { float4 x, y, z, w; };
	};
	float4x4()
	{
		x = float4( 1, 0, 0, 0 );
		y = float4( 0, 1, 0, 0 );
		z = float4( 0, 0, 1, 0 );
		w = float4( 0, 0, 0, 1 );
	}
	float4x4( float4 x, float4 y, float4 z, float4 w ) { x = x; y = y; z = z; w = w; }
	float4x4( float xx, float xy, float xz, float xw,
		float yx, float yy, float yz, float yw,
		float zx, float zy, float zz, float zw,
		float wx, float wy, float wz, float ww )
	{
		x = float4( xx, xy, xz, xw );
		y = float4( yx, yy, yz, yw );
		z = float4( zx, zy, zz, zw );
		w = float4( wx, wy, wz, ww );
	}
	float4x4( const Quaternion&_Q)
	{
		x = float4(1.0f - 2.0f * ( _Q.y * _Q.y + _Q.z * _Q.z ), 2.0f * ( _Q.x * _Q.y - _Q.w * _Q.z ), 2.0f * ( _Q.x * _Q.z + _Q.w * _Q.y ), 0.0f);
		y = float4(2.0f * ( _Q.x * _Q.y + _Q.w * _Q.z ), 1.0f - 2.0f * ( _Q.x * _Q.x + _Q.z * _Q.z ), 2.0f * ( _Q.y * _Q.z - _Q.w * _Q.x ), 0.0f);
		z = float4(2.0f * ( _Q.x * _Q.z - _Q.w * _Q.y ), 2.0f * ( _Q.y * _Q.z + _Q.w * _Q.x ), 1.0f - 2.0f * ( _Q.x * _Q.x + _Q.y * _Q.y ), 0.0f);
		w = float4(0, 0, 0, 1);
	}

	bool operator == ( const float4x4& a ) { return a.x == x && a.y == y && a.z == z && a.w == w; }
	bool operator != ( const float4x4& a ) { return a.x != x || a.y != y || a.z != z || a.w != w; }

	INLINE float4  operator[] ( int i ) const	{ return row[i]; }
	INLINE float4& operator[] ( int i ) 		{ return row[i]; }

	void operator *= ( const float4x4 b )
	{
		float4x4 a = *this;
		*this = a * b;
	}

	INLINE float4x4 operator * ( const float4x4& a ) const
	{
		float4x4 b = *this;
		return float4x4(
			Dot( b.x, float4( a.row[0][0], a.row[1][0], a.row[2][0], a.row[3][0] ) ),
			Dot( b.x, float4( a.row[0][1], a.row[1][1], a.row[2][1], a.row[3][1] ) ),
			Dot( b.x, float4( a.row[0][2], a.row[1][2], a.row[2][2], a.row[3][2] ) ),
			Dot( b.x, float4( a.row[0][3], a.row[1][3], a.row[2][3], a.row[3][3] ) ),

			Dot( b.y, float4( a.row[0][0], a.row[1][0], a.row[2][0], a.row[3][0] ) ),
			Dot( b.y, float4( a.row[0][1], a.row[1][1], a.row[2][1], a.row[3][1] ) ),
			Dot( b.y, float4( a.row[0][2], a.row[1][2], a.row[2][2], a.row[3][2] ) ),
			Dot( b.y, float4( a.row[0][3], a.row[1][3], a.row[2][3], a.row[3][3] ) ),

			Dot( b.z, float4( a.row[0][0], a.row[1][0], a.row[2][0], a.row[3][0] ) ),
			Dot( b.z, float4( a.row[0][1], a.row[1][1], a.row[2][1], a.row[3][1] ) ),
			Dot( b.z, float4( a.row[0][2], a.row[1][2], a.row[2][2], a.row[3][2] ) ),
			Dot( b.z, float4( a.row[0][3], a.row[1][3], a.row[2][3], a.row[3][3] ) ),

			Dot( b.w, float4( a.row[0][0], a.row[1][0], a.row[2][0], a.row[3][0] ) ),
			Dot( b.w, float4( a.row[0][1], a.row[1][1], a.row[2][1], a.row[3][1] ) ),
			Dot( b.w, float4( a.row[0][2], a.row[1][2], a.row[2][2], a.row[3][2] ) ),
			Dot( b.w, float4( a.row[0][3], a.row[1][3], a.row[2][3], a.row[3][3] ) )
			);
	}
	INLINE float4 operator * ( const float4& a ) const
	{
		return float4(
			Dot( a, float4( row[0][0], row[1][0], row[2][0], row[3][0] ) ),
			Dot( a, float4( row[0][1], row[1][1], row[2][1], row[3][1] ) ),
			Dot( a, float4( row[0][2], row[1][2], row[2][2], row[3][2] ) ),
			Dot( a, float4( row[0][3], row[1][3], row[2][3], row[3][3] ) )
			);
	}
	INLINE float4x4 operator * ( const float a ) const
	{
		float4x4 b;
		b = *this;
		b.x *= a;
		b.y *= a;
		b.z *= a;
		b.w *= a;
		return b;
	}

	static float4x4 LookAt(const float3 _From, const float3 _To, const float3 _Up)
	{
		float3 forward = Normalize(_To-_From);
		float3 side = Normalize(Cross(forward,_Up));
		float3 up = Normalize(Cross(side,forward));
		float4x4 mat=float4x4::Identity();
		mat.x.xyz = -side;		//not sure why i need a negation
		mat.y.xyz = up;
		mat.z.xyz = forward;
		mat.w.xyz = _From;
		return mat;
	}

	static float4x4 Lerp(const float4x4&a, const float4x4&b, float t)
	{
		float t2 = 1.0f - t;
		float4x4 r;
		for(int i = 0; i < 4; i++)
			r.row[i] = a.row[i] * t2 + b.row[i] * t;
		return r;
	}

	static float4x4 RotateX( float a )
	{
		float x = cosf( a );
		float y = sinf( a );
		return float4x4(1, 0, 0, 0,	0, x, y, 0,	0, -y, x, 0, 0, 0, 0, 1 );
	}
	static float4x4 RotateY( float a )
	{
		float x = cosf( a );
		float y = sinf( a );
		return float4x4( x, 0, -y, 0, 0, 1, 0, 0, y, 0, x, 0, 0, 0, 0, 1 );
	}
	static float4x4 RotateZ( float a )
	{
		float x = cosf( a );
		float y = sinf( a );
		return float4x4( x, y, 0, 0, -y, x, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 );
	}
	static float4x4 Rotate( float3 _XYZ )
	{
		return RotateX(_XYZ.x)*RotateY(_XYZ.y)*RotateZ(_XYZ.z);
	}

	float4x4 Transpose()
	{
		return float4x4( x.x, y.x, z.x, w.x, x.y, y.y, z.y, w.y, x.z, y.z, z.z, w.z, x.w, y.w, z.w, w.w );
	}

	static const float4x4 Identity()
	{
		return float4x4( 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
	}

	static float4x4 Translate( float3 t )
	{
		return float4x4( 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, t.x, t.y, t.z, 1 );
	}

	void OrthoNormalize()
	{
		float4x4 n;
		n.x.xyz = Normalize( Cross( y.xyz, z.xyz ) );
		n.y.xyz = Normalize( Cross( z.xyz, x.xyz ) );
		n.z.xyz = Normalize( Cross( x.xyz, y.xyz ) );
		n.w = w;
		*this = n;
	}

	static float4x4 Scale( float3 s )
	{
		return float4x4( s.x, 0, 0, 0, 0, s.y, 0, 0, 0, 0, s.z, 0, 0, 0, 0, 1 );
	}
	static float4x4 Scale( float s )
	{
		return float4x4( s, 0, 0, 0, 0, s, 0, 0, 0, 0, s, 0, 0, 0, 0, 1 );
	}

	float4x4 Inverse()
	{
		float m00 = row[0][0], m01 = row[0][1], m02 = row[0][2], m03 = row[0][3];
		float m10 = row[1][0], m11 = row[1][1], m12 = row[1][2], m13 = row[1][3];
		float m20 = row[2][0], m21 = row[2][1], m22 = row[2][2], m23 = row[2][3];
		float m30 = row[3][0], m31 = row[3][1], m32 = row[3][2], m33 = row[3][3];

		float v0 = m20 * m31 - m21 * m30;
		float v1 = m20 * m32 - m22 * m30;
		float v2 = m20 * m33 - m23 * m30;
		float v3 = m21 * m32 - m22 * m31;
		float v4 = m21 * m33 - m23 * m31;
		float v5 = m22 * m33 - m23 * m32;

		float t00 = + (v5 * m11 - v4 * m12 + v3 * m13);
		float t10 = - (v5 * m10 - v2 * m12 + v1 * m13);
		float t20 = + (v4 * m10 - v2 * m11 + v0 * m13);
		float t30 = - (v3 * m10 - v1 * m11 + v0 * m12);

		float invDet = 1 / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

		float d00 = t00 * invDet;
		float d10 = t10 * invDet;
		float d20 = t20 * invDet;
		float d30 = t30 * invDet;

		float d01 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
		float d11 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
		float d21 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
		float d31 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

		v0 = m10 * m31 - m11 * m30;
		v1 = m10 * m32 - m12 * m30;
		v2 = m10 * m33 - m13 * m30;
		v3 = m11 * m32 - m12 * m31;
		v4 = m11 * m33 - m13 * m31;
		v5 = m12 * m33 - m13 * m32;

		float d02 = + (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
		float d12 = - (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
		float d22 = + (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
		float d32 = - (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

		v0 = m21 * m10 - m20 * m11;
		v1 = m22 * m10 - m20 * m12;
		v2 = m23 * m10 - m20 * m13;
		v3 = m22 * m11 - m21 * m12;
		v4 = m23 * m11 - m21 * m13;
		v5 = m23 * m12 - m22 * m13;

		float d03 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
		float d13 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
		float d23 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
		float d33 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

		return float4x4(
			d00, d01, d02, d03,
			d10, d11, d12, d13,
			d20, d21, d22, d23,
			d30, d31, d32, d33);
	}
};

class Thread 
{
private:
	unsigned long* m_hThread;
public:
	Thread() { m_hThread = NULL; }
	~Thread() { if (m_hThread != NULL) stop(); }
	unsigned long* handle() { return m_hThread; }
	void start();
	virtual void run() {};
	void sleep(long ms);
	void suspend();
	void resume();
	void kill();
	void stop();
	void setPriority(int p);
	static const int P_ABOVE_NORMAL;
	static const int P_BELOW_NORMAL;
	static const int P_HIGHEST;
	static const int P_IDLE;
	static const int P_LOWEST;
	static const int P_NORMAL;
	static const int P_CRITICAL;
};

extern "C" { unsigned int sthread_proc(void* param); }

}; // namespace Tmpl8
