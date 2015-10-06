#ifndef __BOX_H_
#define __BOX_H_
//------------------------------------------------------------------------
#include "vector3.h"
//------------------------------------------------------------------------
class Box
{
public:
    Box() { }
    Box(const Vector3 &min, const Vector3 &max)
	{
		bmin = min;
		bmax = max;
    }

	bool intersect(const Vector3 &o, const Vector3 &d, float &tmin, float &tmax) const;
	bool intersect(const Vector3d &o, const Vector3d &d, double &tmin, double &tmax) const;
	Vector3 bmin, bmax;
};
//------------------------------------------------------------------------
#endif
