//---------------------------------------------------------------------------
// View.cpp
//---------------------------------------------------------------------------
#undef _WIN32_WINNT
#define _WIN32_WINNT	0x0601
#include "View.h"
#include "resource.h"
#include "core.cpp"
//---------------------------------------------------------------------------
// Definitions for the CView class
//---------------------------------------------------------------------------
void CView::OnCreate()
{
	tracker = { 0, 0, 0 };
	picture.SetDimension(width, height);

	SetIconSmall(IDW_MAIN);
	SetIconLarge(IDW_MAIN);
}
//---------------------------------------------------------------------------
void CView::PreCreate(CREATESTRUCT& cs)
{
	cs.dwExStyle = WS_EX_CLIENTEDGE | WS_EX_ACCEPTFILES;		// Extended style
	cs.lpszClass = _T("MainWindow");		// Window Class
	cs.lpszName = LoadString(IDW_MAIN);		// Window title
	cs.x = CW_USEDEFAULT;					// top x
	cs.y = CW_USEDEFAULT;					// top y
	cs.cx = 512;							// width
	cs.cy = 512;							// height
}
//---------------------------------------------------------------------------
void CView::OnDestroy()
{
	cancel = true;
	working = false;
	for (int n = 0; n < rtcount; n++) {
		rts[n].terminate();
	}

	::PostQuitMessage(0);
}
//---------------------------------------------------------------------------
void CView::OnInitialUpdate()
{
}
//---------------------------------------------------------------------------
void CView::OnDraw(CDC *dc)
{
	if (!vol.Ready()) {
		CRect rc = GetClientRect();
		dc->DrawText(_T("Press F3 to load volume data"), -1, rc,
					DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		return;
	}

	if (!ViewReady()) {
		return;
	}

	if (render_time > 0) {
		char title[128] = { 0 };
		sprintf(title, "Volume Rendering [ %.1ffps ]", 1000.0f / render_time);
		SetWindowText(title);
	}
	else {
		SetWindowText("Volume Rendering");
	}
	picture.Show(*dc);
}
//---------------------------------------------------------------------------
void CView::OnSize()
{
	CRect r = GetClientRect();
	SetViewDimensions(r.Width(), r.Height());

	picture.SetDimension(width, height);
	picture.SetData(&pixels[0]);

	Invalidate(FALSE);
}
//---------------------------------------------------------------------------
void CView::OnKeypressed(WPARAM key)
{
	if (key != '>' && key != '<')
		SetRenderOptions((char)key);
	else {
		float u = 0.075f, v = 0.0f;
		if (key == '<') u = -u;
		vtn = (vtn + vtu * u).normalize();
		vtu = (vtv ^ vtn).normalize();
		vtv = (vtn ^ vtu).normalize();

		cam = -CRANGE * vtn;
		if (!fixedlight)
			light = CalculateLight();
		dirty = true;
	}
	Invalidate(FALSE);
}
//---------------------------------------------------------------------------
void CView::OnFileOpen()
{
	char szFilename[_MAX_PATH] = { 0 };

	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetHwnd();
	ofn.lpstrFilter = "Volume file (*.raw)\0*.raw\0\0";
	ofn.lpstrFile = szFilename;
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrTitle = _T("Open File");
	ofn.Flags = OFN_FILEMUSTEXIST;

	// Bring up the dialog, and open the file
	if (!::GetOpenFileName(&ofn))
		return;

	if (LoadVolume(szFilename))
		Invalidate(FALSE);
}
//---------------------------------------------------------------------------
void CView::OnMouseMove(int x, int y)
{
	if (!tracker.button)
		return;

	float u, v;
	u = 2.0f * (x - tracker.x) / width;
	v = 2.0f * (y - tracker.y) / height;
	if (fabs(u) < 0.05f && fabs(v) < 0.05f)
		return;

	if (tracker.button & 4) {
		fscale += v > 0 ? 0.05f : -0.05f;
		dirty = true;
	}
	else if (tracker.button & 1) {
		vtn = (vtn + (vtu * u + vtv * v)).normalize();
		vtu = (vtv ^ vtn).normalize();
		vtv = (vtn ^ vtu).normalize();

		cam = -CRANGE * vtn;
		if (!fixedlight)
			light = CalculateLight();
		dirty = true;
	}

	tracker.x = x;
	tracker.y = y;

	Invalidate(FALSE);
}
//---------------------------------------------------------------------------
void CView::OnDropFile(HDROP hd)
{
	char buf[_MAX_PATH];
	int n = DragQueryFile(hd, 0xFFFFFFFF, buf, _MAX_PATH);
	if (DragQueryFile(hd, 0, buf, _MAX_PATH)) {
		if (LoadVolume(buf))
			Invalidate(FALSE);
	}
	DragFinish(hd);
}
//---------------------------------------------------------------------------
LRESULT CView::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_ERASEBKGND:
		if (vol.Ready())
			return 0;
		break;
	case WM_DESTROY:
		OnDestroy();
		return 0;
	case WM_SIZE:
		OnSize();
		break;
	case WM_CHAR:
		OnKeypressed(wParam);
		break;
	case WM_LBUTTONDBLCLK:
		OnFileOpen();
		break;
	case WM_KEYUP:
		if (wParam == VK_F3) {
			OnFileOpen();
		}
		return 0;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		tracker.button = uMsg & 0x0F;
		tracker.x = GET_X_LPARAM(lParam);
		tracker.y = GET_Y_LPARAM(lParam);
		break;
	case WM_MOUSEMOVE:
		OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_MOUSELEAVE:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		tracker.button = 0;
		//dirty = true;
		Invalidate(FALSE);
		break;
	case WM_MOUSEWHEEL:
		//printf("mw=%d\n", (int)(wParam >> 8));
		SetRenderOptions((int)wParam > 0 ? '+' : '-');
		Invalidate(FALSE);
		break;
	case WM_DROPFILES:
		OnDropFile((HDROP)wParam);
	}
	return WndProcDefault(uMsg, wParam, lParam);
}
//---------------------------------------------------------------------------
