#pragma once
#include <typeinfo>

class CColorTreeCtrl : public CTreeCtrl
{
	DECLARE_MESSAGE_MAP()

	// Construction
public:
	void SetItemBold(HTREEITEM hItem, BOOL bBold);
	BOOL GetItemBold(HTREEITEM hItem);
	void OnPaint();

public:
	COLORREF m_color = ::GetSysColor(COLOR_WINDOWTEXT);
	size_t class_hash;
};
