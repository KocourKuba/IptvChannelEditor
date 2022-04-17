/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2022): sharky72 (https://github.com/KocourKuba)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "pch.h"
#include "ImageContainer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void ImageContainer::set_image(CImage& val)
{
	if (icon)
		icon.Destroy();

	if (val)
		icon.Attach(val.Detach());
}

void ImageContainer::set_image(const CImage& src)
{
	if (src.IsNull())
		return;

	//Source image parameters
	BYTE* srcPtr = (BYTE*)src.GetBits();
	int srcBitsCount = src.GetBPP();
	int srcWidth = src.GetWidth();
	int srcHeight = src.GetHeight();
	int srcPitch = src.GetPitch();
	//Destroy the original image
	if (!icon.IsNull())
	{
		icon.Destroy();
	}
	//Create new image
	if (srcBitsCount == 32)
	{
		//Support alpha channel
		icon.Create(srcWidth, srcHeight, srcBitsCount, 1);
	}
	else
	{
		icon.Create(srcWidth, srcHeight, srcBitsCount, 0);
	}

	//Load palette
	if (srcBitsCount <= 8 && icon.IsIndexed())
	{
		//Need palette
		RGBQUAD pal[256] = {};
		int nColors = src.GetMaxColorTableEntries();
		if (nColors > 0)
		{
			//Copy palette program
			src.GetColorTable(0, nColors, pal);
			icon.SetColorTable(0, nColors, pal);
		}
	}
	//Target image parameters
	BYTE* destPtr = (BYTE*)icon.GetBits();
	int destPitch = icon.GetPitch();
	//Copy image data
	for (int i = 0; i < srcHeight; i++)
	{
		memcpy(destPtr + i * destPitch, srcPtr + i * srcPitch, abs(srcPitch));
	}
}
