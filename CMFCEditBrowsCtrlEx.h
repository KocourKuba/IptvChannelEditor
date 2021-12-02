#pragma once
#include <afxeditbrowsectrl.h>

class CMFCEditBrowseCtrlEx : public CMFCEditBrowseCtrl
{
public:
	void OnDrawBrowseButton(CDC* pDC, CRect rect, BOOL bIsButtonPressed, BOOL bIsButtonHot) override
	{
		CMemDC dc(*pDC, rect);
		__super::OnDrawBrowseButton(&dc.GetDC(), rect, bIsButtonPressed, bIsButtonHot);
	}
};