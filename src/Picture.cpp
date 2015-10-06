#include "Picture.h"


CPicture::CPicture()
{
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = 0;
	bmi.bmiHeader.biPlanes = 1;

	W = H = 0;
	data = 0;
}


CPicture::~CPicture()
{
}

void CPicture::Show(CDC dc)
{
	SetDIBitsToDevice(dc.GetHDC(), 0, 0, W, H, 0, 0, 0, H, data, &bmi, DIB_RGB_COLORS);
}

void CPicture::SetDimension(int width, int height)
{
	bmi.bmiHeader.biWidth = W = width;
	bmi.bmiHeader.biHeight = H = height;
	bmi.bmiHeader.biSizeImage = (DWORD)width * height * sizeof(int);
}
