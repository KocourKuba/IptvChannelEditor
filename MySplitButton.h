#pragma once
#include <afxwin.h>

class CMySplitButton : public CSplitButton
{
	DECLARE_DYNAMIC(CMySplitButton)

public:
	CMySplitButton() = default;
	virtual ~CMySplitButton() = default;
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnDropDown(NMHDR* /*pNMHDR*/, LRESULT* pResult);
};
