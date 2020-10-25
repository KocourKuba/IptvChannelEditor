﻿// ColorListBox.cpp : implementation file

#include "StdAfx.h"
#include "ColorListBox.h"
#include "PlayListEntry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColorListBox

void CColorListBox::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);

	CRect rcItem = lpDIS->rcItem;

	if ((lpDIS->itemID != (UINT)-1) && (lpDIS->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)))
	{
		COLORREF clrWindow = ::GetSysColor(COLOR_WINDOW);
		COLORREF clrColored = RGB(200, 0, 0);
		bool colored = false;
		if (lpDIS->itemData)
		{
			colored = ((PlaylistEntry*)lpDIS->itemData)->notexist;

		}

		COLORREF clText = colored ? clrColored : ::GetSysColor(COLOR_WINDOWTEXT);
		COLORREF clrWindowText = IsWindowEnabled() ? clText : ::GetSysColor(COLOR_GRAYTEXT);
		BOOL bSelected = ((lpDIS->itemState & ODS_SELECTED) != 0);
		CRect rcText(rcItem);
		rcText.DeflateRect(0, 0);

		if (bSelected)
		{
			clrWindow = IsWindowEnabled() ? ::GetSysColor(COLOR_HIGHLIGHT) : ::GetSysColor(COLOR_GRAYTEXT);
			clrWindowText = colored ? clrColored : ::GetSysColor(COLOR_HIGHLIGHTTEXT);
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
