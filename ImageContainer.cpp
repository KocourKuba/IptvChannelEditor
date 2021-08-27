#include "StdAfx.h"
#include "ImageContainer.h"

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
		RGBQUAD pal[256];
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
