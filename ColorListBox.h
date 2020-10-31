﻿#pragma once

template<class T>
class CColorListBox : public CListBox
{
// Construction
public:
	void DrawItem(LPDRAWITEMSTRUCT lpDIS) override
	{
		CDC* pDC = CDC::FromHandle(lpDIS->hDC);

		CRect rcItem = lpDIS->rcItem;

		if ((lpDIS->itemID != (UINT)-1) && (lpDIS->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)))
		{
			COLORREF clrWindow = ::GetSysColor(COLOR_WINDOW);
			bool isColored = false;
			if (lpDIS->itemData)
			{
				auto container = dynamic_cast<ColoringProperty*>((T*)lpDIS->itemData);
				if (container)
					isColored = container->is_colored();
			}

			COLORREF clText = isColored ? m_color : ::GetSysColor(COLOR_WINDOWTEXT);
			COLORREF clrWindowText = IsWindowEnabled() ? clText : ::GetSysColor(COLOR_GRAYTEXT);
			BOOL bSelected = ((lpDIS->itemState & ODS_SELECTED) != 0);
			CRect rcText(rcItem);
			rcText.DeflateRect(0, 0);

			if (bSelected)
			{
				clrWindow = IsWindowEnabled() ? ::GetSysColor(COLOR_HIGHLIGHT) : ::GetSysColor(COLOR_GRAYTEXT);
				clrWindowText = isColored ? m_color : ::GetSysColor(COLOR_HIGHLIGHTTEXT);
			}

			// set the text and text background colors, then repaint the item.
			pDC->SetBkColor(clrWindow);
			pDC->SetTextColor(clrWindowText);
			if (clrWindow != COLORREF(-1))
				pDC->FillSolidRect(&rcItem, clrWindow);

			CString strText;
			GetText(lpDIS->itemID, strText);

			pDC->DrawText(strText, &rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_EXPANDTABS);
		}

		if ((lpDIS->itemAction & ODA_FOCUS))
			pDC->DrawFocusRect(&lpDIS->rcItem);
	}

	COLORREF m_color = ::GetSysColor(COLOR_WINDOWTEXT);
};

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
};
