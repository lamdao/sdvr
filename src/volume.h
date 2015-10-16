//--------------------------------------------------------------------------
// volume.h - Raw volume management with trilinear sampling algorithm
//--------------------------------------------------------------------------
// Author: Lam H. Dao <daohailam(at)yahoo(dot)com>
//--------------------------------------------------------------------------
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//--------------------------------------------------------------------------
#ifndef __RENDER_VOLUME_H
#define __RENDER_VOLUME_H
//------------------------------------------------------------------------
#include "box.h"
#include <string>
#include <vector>
#include <fstream>
//------------------------------------------------------------------------
typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned short ushort;
//------------------------------------------------------------------------
inline int get_dimension(char *str = 0)
{
	char *p;
	if ((p = strtok(str, "x")))
		return atoi(p);
	return 0;
}
//------------------------------------------------------------------------
class RenderVolume
{
private:
	int W, H, D, WxH, W1, H1, D1;
	size_t size;
	std::vector<uchar> data;
	size_t get_volume_dimension(char *s) {
		if (!s || !*s)
			return 0;
		std::string fn(s);
		char *p = strrchr(s = (char *)fn.c_str(), '.');
		if (!p || strcasecmp(p+1, "raw") != 0) return 0;
		*p = 0; p = strrchr(s, '.');
 		if (!p) return 0;
		W1 = (W = get_dimension(p+1)) - 1;
		H1 = (H = get_dimension()) - 1;
		D1 = (D = get_dimension()) - 1;
		return (WxH = W * H) * D;
	}
public:
	RenderVolume() : vdx(1.0f,0.0f,0.0f), vdy(0.0f,1.0f,0.0f), vdz(0.0f,0.0f,1.0f)
	{
		size = 0;
		W = H = D = WxH = 0;
	}
	~RenderVolume() {
	}
	bool Ready() { return data.size() > 0; }
	void Load(char *filename)
	{
		size = get_volume_dimension(filename);
		if (size <= 0) {
			MessageBox(0, "Cannot detect volume dimensions!", "Error", MB_OK);
			return;
		}
		std::ifstream fp(filename, std::ios::binary);
		if (!fp) {
			MessageBox(0, "Error opening file", "Error", MB_OK);
			return;
		}
		std::vector<uchar> dp(size);
		if (dp.size() != size) {
			MessageBox(0, "Error allocating memory for volume data", "Error", MB_OK);
			return;
		}
		fp.read((char *)&dp[0], size);

		data.swap(dp);
	}

	float GetDiagonalLength()
	{
		return Vector3(W, H, D).length();
	}
	Vector3 GetDimensions() const
	{
		return Vector3(W, H, D);
	}
	Vector3 GetBorderLine(size_t idx, Vector3& sp, Vector3& ep)
	{
		switch (idx) {
		case 1:
			sp = Vector3(1, 1, 1);
			ep = Vector3(W1, 1, 1);
			break;
		case 2:
			sp = Vector3(1, 1, 1);
			ep = Vector3(1, H1, 1);
			break;
		case 3:
			sp = Vector3(W1, 1, 1);
			ep = Vector3(W1, H1, 1);
			break;
		case 4:
			sp = Vector3(1, H1, 1);
			ep = Vector3(W1, H1, 1);
			break;
		case 5:
			sp = Vector3(1, 1, 1);
			ep = Vector3(1, 1, D1);
			break;
		case 6:
			sp = Vector3(W1, 1, 1);
			ep = Vector3(W1, 1, D1);
			break;
		case 7:
			sp = Vector3(1, H1, 1);
			ep = Vector3(1, H1, D1);
			break;
		case 8:
			sp = Vector3(W1, H1, 1);
			ep = Vector3(W1, H1, D1);
			break;
		case 9:
			sp = Vector3(1, 1, D1);
			ep = Vector3(W1, 1, D1);
			break;
		case 10:
			sp = Vector3(1, 1, D1);
			ep = Vector3(1, H1, D1);
			break;
		case 11:
			sp = Vector3(W1, 1, D1);
			ep = Vector3(W1, H1, D1);
			break;
		case 12:
			sp = Vector3(1, H1, D1);
			ep = Vector3(W1, H1, D1);
			break;
		}
		return ep - sp;
	}
	Vector3 center() { return Vector3(W, H, D) / 2.0f; }
	Box bounds() {
		Vector3 vx = Vector3(W, H, D) / 2.0f;
		Vector3 vn = -vx;
		return Box(vn, vx);
	}
	inline size_t offset(int x,int y,int z) { return (size_t)z * WxH + (size_t)y * W + x; }
	inline size_t offset(const Vector3i &v) { return offset(v.x, v.y, v.z); }

	inline float nearest(const Vector3 &v) {
		Vector3i m = (__m128i)v;
		if (!m.inside(W, H, D))
			return 0;
		return data[offset(m)];
	}

	Vector3 vdx, vdy, vdz;
	inline float trilinear(const Vector3 &v) {
		Vector3i m = (__m128i)v;
		if (!m.inside(W1, H1, D1)) {
			return 0;
		}
		uchar *dp = &data[offset(m)];
		Vector3 vd = v - Vector3(m);
		Vector3 sz(1.0f - vd.z, vd.z);
		float i1 = sz * Vector3(dp[0], dp[WxH]); dp += W;
		float i2 = sz * Vector3(dp[0], dp[WxH]); dp += 1;
		float j2 = sz * Vector3(dp[0], dp[WxH]); dp -= W;
		float j1 = sz * Vector3(dp[0], dp[WxH]);
		Vector3 sy(1.0f - vd.y, vd.y);
		return (Vector3(i1, i2) * sy) * (1.0f - vd.x) + (Vector3(j1, j2) * sy) * vd.x;
	}

	inline float sample(const Vector3 &v) { return trilinear(v); }

	inline Vector3 GetSurfaceNorm(const Vector3& v) {
		return Vector3(
					sample(v + vdx) - sample(v - vdx),
					sample(v + vdy) - sample(v - vdy),
					sample(v + vdz) - sample(v - vdz)
				).normalize();
	}

	inline void SetSamplingStep(float step)
	{
		vdx = Vector3(step, 0.0f, 0.0f);
		vdy = Vector3(0.0f, step, 0.0f);
		vdz = Vector3(0.0f, 0.0f, step);
	}

	uchar *GetSlice(int n)
	{
		return &data[n * WxH];
	}
};
//------------------------------------------------------------------------
#endif
