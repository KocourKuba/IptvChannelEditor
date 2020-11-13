#include "StdAfx.h"
#include "ColorTreeCtrl.h"
#include "ColoringProperty.h"
#include "ChannelInfo.h"
#include "PlayListEntry.h"
#include "ChannelCategory.h"

BEGIN_MESSAGE_MAP(CColorTreeCtrl, CTreeCtrl)
	ON_WM_PAINT()
END_MESSAGE_MAP()

void CColorTreeCtrl::SetItemBold(HTREEITEM hItem, BOOL bBold)
{
	SetItemState(hItem, bBold ? TVIS_BOLD : 0, TVIS_BOLD);
}

BOOL CColorTreeCtrl::GetItemBold(HTREEITEM hItem)
{
	return GetItemState(hItem, TVIS_BOLD) & TVIS_BOLD;
}

void CColorTreeCtrl::OnPaint()
{
	CPaintDC dc(this);

	// Create a memory DC compatible with the paint DC
	CDC memDC;
	memDC.CreateCompatibleDC(&dc);

	CRect rcClip, rcClient;
	dc.GetClipBox(&rcClip);
	GetClientRect(&rcClient);

	// Select a compatible bitmap into the memory DC
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(&dc, rcClient.Width(), rcClient.Height());
	memDC.SelectObject(&bitmap);

	// Set clip region to be same as that in paint DC
	CRgn rgn;
	rgn.CreateRectRgnIndirect(&rcClip);
	memDC.SelectClipRgn(&rgn);
	rgn.DeleteObject();

	// First let the control do its default drawing.
	CWnd::DefWindowProc(WM_PAINT, (WPARAM)memDC.m_hDC, 0);

	HTREEITEM hItem = GetFirstVisibleItem();

	int n = GetVisibleCount() + 1;
	while (hItem && n--)
	{
		// Do not meddle with selected items or drop highlighted items
		UINT selflag = TVIS_DROPHILITED | TVIS_SELECTED;
		UINT state = GetItemState(hItem, selflag);
		BOOL root = GetParentItem(hItem) == nullptr;
		if (!(state & selflag))
		{
			ColoringProperty* container = nullptr;
			if (class_hash == typeid(ChannelInfo).hash_code() && !root)
			{
				container = dynamic_cast<ColoringProperty*>((ChannelInfo*)GetItemData(hItem));
			}
			else if (class_hash == typeid(PlaylistEntry).hash_code())
			{
				container = dynamic_cast<ColoringProperty*>((PlaylistEntry*)GetItemData(hItem));
			}

			bool isColored = container ? container->is_colored() : false;
			bool isDisabled = container ? container->is_disabled() : false;
			// No font specified, so use window font
			CFont* pFont = GetFont();
			LOGFONT logfont;
			pFont->GetLogFont(&logfont);

			if (GetItemBold(hItem))
				logfont.lfWeight = 700;

			CFont fontDC;
			fontDC.CreateFontIndirect(&logfont);
			CFont* pFontDC = memDC.SelectObject(&fontDC);

			if (isDisabled)
				memDC.SetTextColor(::GetSysColor(COLOR_GRAYTEXT));
			else if (isColored)
				memDC.SetTextColor(m_color);
			else
				memDC.SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));

			const auto& sItem = GetItemText(hItem);

			CRect rect;
			GetItemRect(hItem, &rect, TRUE);
			memDC.SetBkColor(GetSysColor(COLOR_WINDOW));
			memDC.TextOut(rect.left + 2, rect.top + 1, sItem);
			memDC.SelectObject(pFontDC);
		}

		hItem = GetNextVisibleItem(hItem);
	}

	dc.BitBlt(rcClip.left, rcClip.top, rcClip.Width(), rcClip.Height(), &memDC, rcClip.left, rcClip.top, SRCCOPY);
}
