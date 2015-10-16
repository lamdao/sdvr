//--------------------------------------------------------------------------
// core.cpp - Core algorithm of DVR, Bounding-Box draw, Task scheduling
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
#include <windows.h>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <math.h>
#include <vector>
//---------------------------------------------------------------------------
#include "volume.h"
#include "VolumeRender.h"
//---------------------------------------------------------------------------
typedef struct {
	bool active;
	HANDLE handle, report, permit;
	void create(int id, LPTHREAD_START_ROUTINE fx) {
		active = true;
		handle = CreateThread(NULL, 0, fx, (LPVOID)id, CREATE_SUSPENDED, NULL);
		report = CreateEventEx(NULL, 0, 0, EVENT_ALL_ACCESS | SYNCHRONIZE);
		permit = CreateEvent(NULL, FALSE, FALSE, 0);
		//SetThreadPriority(rts[n].handle, THREAD_PRIORITY_HIGHEST);
		SetThreadAffinityMask(handle, 1 << id);
		ResumeThread(handle);
	}
	void wait() {
		active = false;
		SignalObjectAndWait(report, permit, INFINITE, TRUE);
		active = true;
	}
	void activate() {
		SetEvent(permit);
	}
	void terminate() {
		TerminateThread(handle, 0);
		CloseHandle(report);
		CloseHandle(permit);
		CloseHandle(handle);
	}
	bool busy() {
		return active;
	}
} RenderThread;
//---------------------------------------------------------------------------
static inline int GetNumberOfProcessors()
{
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	DWORD np = sysinfo.dwNumberOfProcessors;
	return np > 0 ? np : 1;
}
//---------------------------------------------------------------------------
bool shiny = true;
bool shading = true;
bool fixedlight = false;
bool perspective = true;
bool showbox = true;
//---------------------------------------------------------------------------
bool working = true;
bool cancel = false;
bool busy = false;
bool dirty = true;
//---------------------------------------------------------------------------
volatile LONG rtready = 0;
//---------------------------------------------------------------------------
RenderVolume vol;
//---------------------------------------------------------------------------
int rtcount = GetNumberOfProcessors();
std::vector<RenderThread> rts(rtcount);
std::vector<HANDLE> rte(rtcount);
//---------------------------------------------------------------------------
float w2 = 256.0f, h2 = 256.0f;
int width = 512, height = 512;
//---------------------------------------------------------------------------
std::vector<uint> pixels, savepx;
//---------------------------------------------------------------------------
#define CalculateLight()	(4 * vtn + vtv).normalize()
//---------------------------------------------------------------------------
float CRANGE = 512.0f, DRANGE = 512.0f;
Vector3 vtu(1.0f, 0.0f, 0.0f),
		vtv(0.0f, 1.0f, 0.0f),
		vtn(0.0f, 0.0f, 1.0f),
		cam(0.0f, 0.0f, -CRANGE),
		light = CalculateLight();
//---------------------------------------------------------------------------
float fscale = 1.0f;
float brightness = 1.0f;
float bglevel = 6.0f;
float step = 1.0f;
//---------------------------------------------------------------------------
void ResetView()
{
	fscale = 1.0f;
	CRANGE = DRANGE;
	vtu = Vector3(1.0f, 0.0f, 0.0f);
	vtv = Vector3(0.0f, 1.0f, 0.0f);
	vtn = Vector3(0.0f, 0.0f, 1.0f);
	cam = Vector3(0.0f, 0.0f, -CRANGE);
	light = CalculateLight();
}
//---------------------------------------------------------------------------
#define GetCurrentVoxelPosition()	(Sox + dir * tmin)
//---------------------------------------------------------------------------
int get_voxel_color(const Box& vbx, const Vector3 &eye, const Vector3 &dir,
					const Vector3 &O, const Vector3 &L)
{
	float cv = 0;
	float tmin, tmax;
	if (vbx.intersect(eye, dir, tmin, tmax)) {
		if (tmin < 0) tmin = 0;
		Vector3 HL = (L + dir).normalize();
		Vector3 LL = 0.6f * L;
		Vector3 Sox = eye + O;
		Vector3 vox = GetCurrentVoxelPosition();
		while (vol.nearest(vox) <= bglevel && tmin <= tmax) {
			vox = GetCurrentVoxelPosition();
			tmin += step;
		}
		for (float op = 0; tmin <= tmax && op < 0.95f; tmin += step) {
			if (cancel) return -1;

			vox = GetCurrentVoxelPosition();
			float sv = vol.sample(vox);
			if (sv <= bglevel) continue;
			float cop = sv / 255.0f * (1.0f - op);
			if (!shading) {
				cv += cop * sv * brightness;
			}
			else {
				Vector3 N = vol.GetSurfaceNorm(vox);
				float sd = 0.35f + (LL * N);
				if (shiny) sd += pow(HL * N, 10.0f);
				cv += cop * sv * sd * brightness;
			}
			op += cop;
		}
		if (cv < 0)
			return 1;
		if (cv > 255)
			return 255;
	}
	return (uchar)cv;
}
//---------------------------------------------------------------------------
#define CalcEyePoint()		(2.0f * fscale * cam)
#define CalcImgPlane(E)		(E - cam)
//---------------------------------------------------------------------------
void perspective_render(size_t idx, const Box& vbx, const Vector3 &center)
{

	size_t total = width * height;
	size_t load = (total / rtcount) + ((total % rtcount) > 0);
	uint *px = &pixels[idx = idx * load];
	size_t endp = idx + load;
	int s = (int)(idx / width);
	Vector3 E = CalcEyePoint();
	Vector3 B = CalcImgPlane(E) - vtu * w2 + vtv * (h2 - s);

	if (endp > total) endp = total;
	while (idx < endp) {
		int x = (int)(idx % width);
		int y = (int)(idx / width);
		if (y > s) {
			B -= vtv;
			s  = y;
		}
		Vector3 p = B + vtu * (float)x;
		Vector3 d = (p - E).normalize();
		int cv = get_voxel_color(vbx, E, d, center, light);
		if (cv < 0)
			return;
		*px++ = 0x010101 * cv;
		idx++;
	}
}
//---------------------------------------------------------------------------
void parallel_render(size_t idx, const Box& vbx, const Vector3 &center)
{
	size_t total = width * height;
	size_t load = (total / rtcount) + ((total % rtcount) > 0);
	uint *px = &pixels[idx = idx * load];
	size_t endp = idx + load;
	int s = (int)(idx / width);

	Vector3 U = vtu * fscale * 2.0f;
	Vector3 V = vtv * fscale * 2.0f;
	Vector3 B = cam - U * w2 + V * (h2 - s);

	while (idx < endp) {
		int x = (int)(idx % width);
		int y = (int)(idx / width);
		if (y > s) {
			B -= V;
			s  = y;
		}
		Vector3 p = B + U * (float)x;
		int cv = get_voxel_color(vbx, p, vtn, center, light);
		if (cv < 0)
			return;
		*px++ = 0x010101 * cv;
		idx++;
	}
}
//---------------------------------------------------------------------------
Vector3 find_project_point(const Box& vbx, const Vector3 &E, const Vector3& B,
							const Vector3 &ldir, const Vector3 &pt,
							bool &over)
{
	float n = 0;
	float ti, to;
	Vector3 vdir;
	Vector3 ep = E;
	Vector3 cp = pt + n * ldir;
	if (perspective) {
		vdir = (cp - ep).normalize();
		while (!vbx.intersect(ep, vdir, ti, to)) {
			n += 1.0f;
			cp = pt + n * ldir;
			vdir = (cp - ep).normalize();
		}
	}
	else {
		vdir = vtn;
		ep = cp + vtn * ((B - cp) * vtn);
		while (!vbx.intersect(ep, vdir, ti, to)) {
			n += 1.0f;
			cp = pt + n * ldir;
			ep = cp + vtn * ((B - cp) * vtn);
		}
	}

	if (over) {
		Vector3 pi = ep + vdir * ti;
		Vector3 po = ep + vdir * to;
		float d0 = (pi - cp).length();
		float d1 = (po - cp).length();
		over = d0 < d1 || abs(d0 - d1) < 10;
	}

	float d = vtn * (B - ep) / (vtn * vdir);
	return ep + d * vdir - B;
}
//---------------------------------------------------------------------------
void draw_border_line(size_t idx, const Box& vbx, const Vector3 &O)
{
	bool over = true;
	Vector3 E = CalcEyePoint();
	Vector3 B = CalcImgPlane(E);
	Vector3 U, V, sp, ep;
	Vector3 ldir = vol.GetBorderLine(idx + 1, sp, ep).normalize();
	sp = find_project_point(vbx, E, B,  ldir, sp - O, over);
	ep = find_project_point(vbx, E, B, -ldir, ep - O, over);
	if (perspective) {
		U = vtu;
		V = vtv;
	}
	else {
		U = vtu / fscale / 2.0f;
		V = vtv / fscale / 2.0f;
	}
	// Calculate projection with center correction of
	float x0 = (sp * U + w2);	// sp on U
	float y0 = (h2 - sp * V);	// sp on V
	float x1 = (ep * U + w2);	// ep on U
	float y1 = (h2 - ep * V);	// ep on V
	// draw line from [x0,y0] to [x1, y1]
	sp = Vector3(x0, y0);
	ep = Vector3(x1, y1);
	float n = 0, l = (ldir = ep - sp).length();
	ldir = ldir.normalize();
	while (n < l) {
		if (cancel) break;

		Vector3 cp = sp + n * ldir;
		int x = (int)cp.x, y = (int)cp.y;
		if (x >= 0 && x < width && y >= 0 && y < height) {
			uint *px = &pixels[y * width + x];
			if (over || (*px & 0x00FFFFFF) == 0)
				*px = 0x00FF0000;
		}
		n += 1.0f;
	}
}
//---------------------------------------------------------------------------
inline void draw_border(size_t idx, const Box& vbx, const Vector3 &center)
{
	if (!showbox)
		return;

	InterlockedIncrement(&rtready);
	// Wait until all render thread finished
	while (rtready < rtcount) {
		Sleep(10);
	}

	// 12 is number of lines of bounding box
	while (!cancel && idx < 12) {
		draw_border_line(idx, vbx, center);
		idx += rtcount;
	}
}
//---------------------------------------------------------------------------
DWORD WINAPI render(LPVOID lpParam)
{
	size_t idx = (size_t)lpParam;
	RenderThread &rti = rts[idx];

	while (working) {
		rti.wait();

		Box vbx = vol.bounds();
		Vector3 center = vol.center();
		if (perspective)
			perspective_render(idx, vbx, center);
		else
			parallel_render(idx, vbx, center);

		draw_border(idx, vbx, center);
	}
	return 0;
}
//---------------------------------------------------------------------------
DWORD WINAPI monitor(LPVOID)
{
	while (working) {
		WaitForMultipleObjects(rtcount, &rte[0], TRUE, INFINITE);
		if (busy && !cancel) {
			Application().GetView().StopTimer();
			busy = false;
		}
	}
	return 0;
}
//---------------------------------------------------------------------------
bool ViewReady()
{
	if (dirty) {
		cancel = true;

		int n = 0;
		while (n < rtcount) {
			for (n = 0; n < rtcount; n++)
				if (rts[n].busy()) break;
			Sleep(5);
		}
		savepx.clear();

		busy = true;
		cancel = false;
		Application().GetView().StartTimer();
		rtready = 0;
		for (n = 0; n < rtcount; n++) {
			rts[n].activate();
		}
		dirty = false;
	}
	return !busy;
}
//---------------------------------------------------------------------------
void RenderThreadsInit()
{
	static bool ready = false;
	if (ready)
		return;
	for (int n = 0; n < rtcount; n++) {
		rts[n].create(n, render);
		rte[n] = rts[n].report;
	}
	CreateThread(NULL, 0, monitor, 0, 0, NULL);
	ready = true;
}
//---------------------------------------------------------------------------
void RenderInit()
{
	DRANGE = CRANGE = 2.0f * vol.GetDiagonalLength();
	cam = -CRANGE * vtn;

	RenderThreadsInit();
	dirty = true;
}
//---------------------------------------------------------------------------
bool LoadVolume(char *file)
{
	vol.Load(file);
	if (!vol.Ready()) {
		return false;
	}
	RenderInit();
	return true;
}
//---------------------------------------------------------------------------
void SetViewDimensions(int w, int h)
{
	savepx.swap(pixels);
	w2 = (width = w) / 2.0f;
	h2 = (height = h) / 2.0f;
	size_t psize = width * height;
	std::vector<uint> empty(psize);
	pixels.swap(empty);
	dirty = true;
}
//---------------------------------------------------------------------------
void SetRenderOptions(char key)
{
	switch (key) {
	case 27:
		exit(0);
		break;
	case '0':
	case '1':
		fscale = 1.0;
		break;
	case '2':
		fscale /= 2.0f;
		break;
	case '3':
		fscale /= 3.0f;
		break;
	case '4':
		fscale /= 4.0f;
		break;
	case '@':
		fscale *= 2.0f;
		break;
	case '#':
		fscale *= 3.0f;
		break;
	case '$':
		fscale *= 4.0f;
		break;
	case '-':
		fscale += 0.05f;
		break;
	case '+':
		fscale -= 0.05f;
		break;
	case 'b':
		brightness -= 0.1f;
		break;
	case 'B':
		brightness += 0.1f;
		break;
	case 's':
		shading = !shading;
		break;
	case 'S':
		shiny = !shiny;
		break;
	case 'C':
		if (CRANGE >= DRANGE)
			return;
		CRANGE += 50.0f;
		cam = -CRANGE * vtn;
		break;
	case 'c':
		if (CRANGE * fscale <= DRANGE / 10)
			return;
		CRANGE -= 50.0f;
		cam = -CRANGE * vtn;
		break;
	case 'z':
		CRANGE = DRANGE;
		cam = -CRANGE * vtn;
		break;
	case 'f':
		fixedlight = !fixedlight;
		if (!fixedlight)
			light = CalculateLight();
		break;
	case 'p':
		perspective = !perspective;
		break;
	case 'x':
		showbox = !showbox;
		break;
	case '^':
		bglevel += 1.0f;
		break;
	case 'v':
		bglevel -= 1.0f;
		break;
	case 'r':
		ResetView();
		break;
	case 'q':
		if (step >= 4.0f)
			return;
		step += 0.5f;
		vol.SetSamplingStep(step);
		break;
	case 'Q':
		if (step <= 0.5f)
			return;
		step -= 0.5f;
		vol.SetSamplingStep(step);
		break;
	default:
		return;
	}
	dirty = true;
}
//---------------------------------------------------------------------------
