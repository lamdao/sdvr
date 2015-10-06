#ifndef __VECTOR3_H_
#define __VECTOR3_H_
//------------------------------------------------------------------------
#include <smmintrin.h>
#include <math.h>
//------------------------------------------------------------------------
class Vector3i
{
public:
	Vector3i() : v(_mm_setzero_si128()) { }
	Vector3i(__m128i d) : v(d) { }
	Vector3i(int x, int y, int z) : v(_mm_set_epi32(0, z, y, x)) { }
	bool inside(int W, int H, int D) {
		if (x < 0 || x >= W || y < 0 || y >= H || z < 0 || z >= D)
			return false;
		return true;
	}
#ifndef __GNUC__
	_MM_ALIGN16
#endif
	union
	{
		struct { int x, y, z, w; };
		int p[4];
		__m128i v;
	}
#ifdef __GNUC__
	__attribute__((aligned(16)))
#endif
	;
	operator __m128i () const { return v; }
	operator int * () const { return (int *)p; }
};
//------------------------------------------------------------------------
class Vector3
{
public:
	Vector3(): v(_mm_setzero_ps()) { };
	Vector3(__m128 d) : v(d) {}
	Vector3(__m128i d) : v(_mm_cvtepi32_ps(d)) {}
	Vector3(float x, float y, float z=0) : v(_mm_set_ps(0, z, y, x)) {}
	Vector3(int x, int y, int z) {
		unsigned int c[4] = { (unsigned)x, (unsigned)y, (unsigned)z, 0 };
		v = _mm_cvtepi32_ps(*(__m128i*)c);
	}
	Vector3(unsigned char x, unsigned char y, unsigned char z=0) {
		unsigned int c[4] = { x, y, z, 0 };
		v = _mm_cvtepi32_ps(*(__m128i*)c);
	}


	float length() const {
		return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(v,v,0x71)));
	}
	Vector3 normalize() const {
		return _mm_mul_ps(v, _mm_rsqrt_ps(_mm_dp_ps(v,v,0x7F)));
	}

	Vector3 operator - () const {
		return _mm_sub_ps(_mm_setzero_ps(), v);
	}
	Vector3 operator + (const Vector3 &b) const {
		return _mm_add_ps(v, b);
	}
	Vector3 operator - (const Vector3 &b) const {
		return _mm_sub_ps(v, b);
	}
	Vector3 operator * (float s) const {
		return _mm_mul_ps(v, _mm_set1_ps(s));
	}
	Vector3 operator / (float s) const {
		return _mm_div_ps(v, _mm_set1_ps(s));
	}
	Vector3 operator + (float s) const {
		return _mm_add_ps(v, _mm_set_ps(0, s, s, s));
	}
	Vector3 operator - (float s) const {
		return _mm_sub_ps(v, _mm_set_ps(0, s, s, s));
	}
	void operator *= (float s) {
		v = _mm_mul_ps(v, _mm_set1_ps(s));
	}
	void operator += (float s) {
		v = _mm_add_ps(v, _mm_set_ps(0, s, s, s));
	}
	void operator -= (float s) {
		v = _mm_sub_ps(v, _mm_set_ps(0, s, s, s));
	}
	// dot product
	float operator * (const Vector3 &b) const {
		return _mm_cvtss_f32(_mm_dp_ps(v, b, 0x71));
	}
	// cross product
	Vector3 operator ^ (const Vector3 &b) const {
		return _mm_sub_ps(
				_mm_mul_ps(
					_mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 0, 2, 1)),
					_mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 1, 0, 2))),
				_mm_mul_ps(
					_mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 1, 0, 2)),
					_mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1)))
			);
	}

	void operator += (const Vector3 &b) {
		v = _mm_add_ps(v, b);
	}
	void operator -= (const Vector3 &b) {
		v = _mm_sub_ps(v, b);
	}

	Vector3 inv() const {
		return _mm_rcp_ps(v);
	}
#ifndef __GNUC__
	_MM_ALIGN16
#endif
	union 
	{
		struct { float x, y, z, w; };
		float p[4];
		__m128 v;
	}
#ifdef __GNUC__
	__attribute__((aligned(16)))
#endif
	;
	operator __m128 () const { return v; }
	operator __m128i () const { return _mm_cvtps_epi32(v); }
};
//------------------------------------------------------------------------
static inline Vector3 operator * (float a, const Vector3 &b)
{
	return b * a;
}
//------------------------------------------------------------------------
static inline Vector3 operator / (float a, const Vector3 &b)
{
	return _mm_mul_ps(_mm_set_ps(0, a, a, a), b);
}
//------------------------------------------------------------------------
static inline Vector3 operator + (float a, const Vector3 &b)
{
	return _mm_add_ps(_mm_set_ps(0, a, a, a), b);
}
//------------------------------------------------------------------------
static inline Vector3 operator - (float a, const Vector3 &b)
{
	return _mm_sub_ps(_mm_set_ps(0, a, a, a), b);
}
//------------------------------------------------------------------------
class Vector3d
{
public:
	Vector3d() : x(0), y(0), z(0), w(0) { };
	Vector3d(double x, double y, double z) : x(x), y(y), z(z), w(0) {}
	Vector3d(const Vector3 &v) : x(v.x), y(v.y), z(v.z), w(0) {}

	double length() const {
		return sqrt(x*x+y*y+z*z);
	}
	Vector3d normalize() const {
		double l = length();
		return Vector3d(x/l,y/l,z/l);
	}

	Vector3d operator - () const {
		return Vector3d(-x,-y,-z);
	}
	Vector3d operator + (const Vector3d &b) const {
		return Vector3d(x+b.x,y+b.y,z+b.z);
	}
	Vector3d operator - (const Vector3d &b) const {
		return Vector3d(x-b.x, y-b.y, z-b.z);
	}
	Vector3d operator * (double s) const {
		return Vector3d(x*s, y*s, z*s);
	}
	Vector3d operator / (double s) const {
		return Vector3d(x/s, y/s, z/s);
	}
	void operator *= (double s) {
		x *= s;
		y *= s;
		z *= s;
	}
	// dot product
	double operator * (const Vector3d &b) const {
		return x*b.x+y*b.y+z*b.z;
	}
	// cross product
	Vector3d operator ^ (const Vector3d &b) const {
		return Vector3d(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x);
	}

	void operator += (const Vector3d &b) {
		x += b.x;
		y += b.y;
		z += b.z;
	}

	Vector3d inv() const {
		return Vector3d(1/x,1/y,1/z);
	}
	union
	{
		struct { double x, y, z, w; };
		double p[4];
	};
	operator __m128 () const { return _mm_set_ps(0, (float)z, (float)y, (float)x); }
	operator __m128i () const { return _mm_set_epi32(0, (int)z, (int)y, (int)x); }
};
//------------------------------------------------------------------------
static inline Vector3d operator * (double a, const Vector3d &b)
{
	return b * a;
}
//------------------------------------------------------------------------
static inline Vector3d operator / (double a, const Vector3d &b)
{
	return Vector3d(a/b.x, a/b.y, a/b.z);
}
//------------------------------------------------------------------------
static inline Vector3d operator + (double a, const Vector3d &b)
{
	return Vector3d(a + b.x, a + b.y, a + b.z);
}
//------------------------------------------------------------------------
static inline Vector3d operator - (double a, const Vector3d &b)
{
	return Vector3d(a - b.x, a - b.y, a - b.z);
}
//------------------------------------------------------------------------
#endif
