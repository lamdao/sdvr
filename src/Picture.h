#ifndef __PICTURE_H
#define __PICTURE_H

#include "wincore.h"

class CPicture
{
private:
	unsigned int *data;
	BITMAPINFO bmi;
	int W, H;
public:
	CPicture();
	~CPicture();

	void Show(CDC dc);
	void SetDimension(int width, int height);
	void SetData(unsigned int *pixels) { data = pixels; }
};
#endif
