/////////////////////////////////
// View.h

#ifndef VIEW_H
#define VIEW_H


#include "wincore.h"
#include "Picture.h"

class CView : public CWnd
{
private:
	DWORD render_time;
	struct {
		int x, y;
		int button;
	} tracker;

	CPicture picture;
public:
	CView() {}
	virtual ~CView() {}

	void StartTimer() {
		render_time = GetTickCount();
	}
	void StopTimer() {
		render_time = GetTickCount() - render_time;
		Invalidate(FALSE);
	}
protected:
	virtual void OnCreate();
	virtual void OnDestroy();
	virtual void OnInitialUpdate();
	virtual void OnDraw(CDC *dc);
	virtual void OnSize();
	virtual void PreCreate(CREATESTRUCT& cs);
	virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual void OnFileOpen();
	virtual void OnKeypressed(WPARAM key);
	virtual void OnMouseMove(int x, int y);
	virtual void OnDropFile(HDROP d);
};

#endif
