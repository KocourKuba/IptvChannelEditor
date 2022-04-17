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
#include "EditableListCtrl.h"
#include "InPlaceEdit.h"

#define EDIT_CTRL_ID 30000

// Determine the row index and column index for a mouse cursor position
// Returns row index or -1 if cursor not positioned over a row
int CEditableListCtrl::GetRowFromPoint(CPoint& point, int* col) const
{
	int row = HitTest(point, nullptr);

	if (col)
		*col = 0;

	// Make sure that the ListView is in LVS_REPORT
	if ((GetWindowLong(m_hWnd, GWL_STYLE) & LVS_TYPEMASK) != LVS_REPORT)
	{
		return row;
	}

	// Get the top and bottom row visible
	row = GetTopIndex();
	int bottom = row + GetCountPerPage();

	if (bottom > GetItemCount())
	{
		bottom = GetItemCount();
	}

	// Get the number of columns
	CHeaderCtrl* pHeader = (CHeaderCtrl*)GetDlgItem(0);
	int nColumnCount = pHeader->GetItemCount();

	// Loop through the visible rows
	for (; row <= bottom; row++)
	{
		// Get bounding rectangle of item and check whether point falls in it.
		CRect rect;
		GetItemRect(row, &rect, LVIR_BOUNDS);

		if (!rect.PtInRect(point)) continue;

		// Find the column
		for (int column = 0; column < nColumnCount; column++)
		{
			int colwidth = GetColumnWidth(column);

			if (point.x >= rect.left && point.x <= (rect.left + colwidth))
			{
				if (col)
					*col = column;

				return row;
			}

			rect.left += colwidth;
		}
	}

	return -1;
}

// Commence the editing of the sub item label
void CEditableListCtrl::EditCell(int nItem, int nCol)
{
	// The returned pointer should not be saved, make sure item visible
	if (!EnsureVisible(nItem, TRUE))
		return;

	// Make sure that column number is valid
	CHeaderCtrl* pHeader = (CHeaderCtrl*)GetDlgItem(0);
	int nColumnCount = pHeader->GetItemCount();
	if (nCol >= nColumnCount || GetColumnWidth(nCol) < 5)
		return;

	// Get the column offset
	int offset = 0;
	for (int i = 0; i < nCol; i++)
	{
		offset += GetColumnWidth(i);
	}

	CRect rect;
	GetItemRect(nItem, &rect, LVIR_BOUNDS);

	// Scroll horizontally if we need to expose the column
	CRect rcClient;
	GetClientRect(&rcClient);

	if (offset + rect.left < 0 || offset + rect.left > rcClient.right)
	{
		CSize size;
		size.cx = offset + rect.left;
		size.cy = 0;
		Scroll(size);
		rect.left -= size.cx;
	}

	// Get Column alignment
	LV_COLUMN lvcol{};
	lvcol.mask = LVCF_FMT;
	GetColumn(nCol, &lvcol);
	DWORD dwStyle;

	if ((lvcol.fmt & LVCFMT_JUSTIFYMASK) == LVCFMT_LEFT)
	{
		dwStyle = ES_LEFT;
	}
	else if ((lvcol.fmt & LVCFMT_JUSTIFYMASK) == LVCFMT_RIGHT)
	{
		dwStyle = ES_RIGHT;
	}
	else
	{
		dwStyle = ES_CENTER;
	}

	rect.left += offset + 4;
	rect.right = rect.left + GetColumnWidth(nCol) - 3;

	if (rect.right > rcClient.right)
	{
		rect.right = rcClient.right;
	}

	dwStyle |= WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL;

	// destroyed when kills focus
	CEdit* pEdit = new CInPlaceEdit(nItem, nCol, GetItemText(nItem, nCol));
	pEdit->Create(dwStyle, rect, this, EDIT_CTRL_ID);
} //-V773

// Handle the horizontal scroll messages
void CEditableListCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (GetFocus() != this)
		SetFocus();

	CListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}

// Handle the vertical scroll messages
void CEditableListCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (GetFocus() != this)
		SetFocus();

	CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}

// Handle the end of the label edit
void CEditableListCtrl::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* plvDispInfo = (LV_DISPINFO*)pNMHDR;

	LV_ITEM* plvItem = &plvDispInfo->item;

	if (plvItem->pszText != nullptr)
	{
		SetItemText(plvItem->iItem, plvItem->iSubItem, plvItem->pszText);
	}

	*pResult = FALSE;
}

// Add the means for the user to initiate edit of Description field in Recording Attachments table
void CEditableListCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	CListCtrl::OnLButtonDown(nFlags, point);

	ModifyStyle(0, LVS_EDITLABELS);

	int colnum = 0;
	int index = GetRowFromPoint(point, &colnum);
	if (index != -1)
	{
		UINT flag = LVIS_FOCUSED;

		if ((GetItemState(index, flag) & flag) == flag)
		{
			// Add check for LVS_EDITLABELS
			if (GetWindowLong(m_hWnd, GWL_STYLE) & LVS_EDITLABELS)
			{
				EditCell(index, colnum);
			}
		}
		else
		{
			SetItemState(index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		}
	}
}