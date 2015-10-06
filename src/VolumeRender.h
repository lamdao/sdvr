/////////////////////////////////////////
// VolumeRender.h

#ifndef VOLUMERENDER_H
#define VOLUMERENDER_H

#include "View.h"


// Declaration of the CVolumeRender class
class CVolumeRender : public CWinApp
{
public:
    CVolumeRender();
    virtual ~CVolumeRender() {}
	virtual BOOL InitInstance();
	CView& GetView() { return m_View; }

private:
    CView m_View;
};


// returns a reference to the CVolumeRender object
inline CVolumeRender& Application() { return *((CVolumeRender*)GetApp()); }


#endif
