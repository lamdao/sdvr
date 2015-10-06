#include "box.h"
//------------------------------------------------------------------------
#ifndef min
#define min(a,b)	((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)	((a) > (b) ? (a) : (b))
#endif
//------------------------------------------------------------------------
static inline Vector3 vmul(const Vector3& v1, const Vector3& v2)
{
	return _mm_mul_ps(v1, v2);
}
//------------------------------------------------------------------------
static inline Vector3d vmul(const Vector3d& v1, const Vector3d& v2)
{
	return Vector3d(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
}
//------------------------------------------------------------------------
inline Vector3 vmin(const Vector3 &a, const Vector3 &b)
{
	return _mm_min_ps(a, b);
}
//------------------------------------------------------------------------
inline Vector3d dvmin(const Vector3d &a, const Vector3d &b)
{
	return Vector3d(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z));
}
//------------------------------------------------------------------------
inline Vector3 vmax(const Vector3 &a, const Vector3 &b)
{
	return _mm_max_ps(a, b);
}
//------------------------------------------------------------------------
inline Vector3d dvmax(const Vector3d &a, const Vector3d &b)
{
	return Vector3d(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));
}
//------------------------------------------------------------------------
bool Box::intersect(const Vector3 &o, const Vector3 &d, float &tnear, float &tfar) const
{
	static __m128 one = _mm_set_ps(1, 1, 1, 1);
//	Vector3 invR(1 / d.x, 1 / d.y, 1 / d.z);
//	Vector3 invR(_mm_div_ps(one, d)); invR.w = 0;
	Vector3 invR = d.inv(); invR.w = 0;
	Vector3 tbot = vmul(invR, bmin - o);
    Vector3 ttop = vmul(invR, bmax - o);
    // re-order intersections to find smallest and largest on each axis
	Vector3 tmin = vmin(ttop, tbot);
	Vector3 tmax = vmax(ttop, tbot);
    // find the largest tmin and the smallest tmax
    tnear = max(max(tmin.x, tmin.y), max(tmin.x, tmin.z));
    tfar = min(min(tmax.x, tmax.y), min(tmax.x, tmax.z));
	return tfar > tnear;
}
//------------------------------------------------------------------------
bool Box::intersect(const Vector3d &o, const Vector3d &d, double &tnear, double &tfar) const
{
	Vector3d invR = d.inv(); invR.w = 0;
	Vector3d tbot = vmul(invR, Vector3d(bmin) - o);
	Vector3d ttop = vmul(invR, Vector3d(bmax) - o);
	// re-order intersections to find smallest and largest on each axis
	Vector3d tmin = dvmin(ttop, tbot);
	Vector3d tmax = dvmax(ttop, tbot);
	// find the largest tmin and the smallest tmax
	tnear = max(max(tmin.x, tmin.y), max(tmin.x, tmin.z));
	tfar = min(min(tmax.x, tmax.y), min(tmax.x, tmax.z));
	return tfar > tnear;
}
//------------------------------------------------------------------------
