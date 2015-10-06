////////////////////////////////////////
// VolumeRender.cpp

#include "VolumeRender.h"


// Definitions for the CVolumeRender class
CVolumeRender::CVolumeRender()
{
}

BOOL CVolumeRender::InitInstance()
{
    m_View.Create();
	return TRUE;
}
