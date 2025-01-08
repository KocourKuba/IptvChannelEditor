/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2025): sharky72 (https://github.com/KocourKuba)

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


#pragma once

#define TVGN_EX_ALL   0x000F

// CTreeCtrlEx window

class CTreeCtrlEx : public CTreeCtrl
{
	struct CLRFONT
	{
		LOGFONT logfont{};	// A LOGFONT object that represents the tree item font.
		COLORREF color;		// An RGB value that represents the text color for a tree item.
		COLORREF colorBack; // An RGB value that represents the background color for a tree item.
		CLRFONT() : color((COLORREF)-1), colorBack((COLORREF)-1) {}
	};
	using CColorFontMap = CMap<void*, void*, CLRFONT, CLRFONT&>;

	DECLARE_DYNAMIC(CTreeCtrlEx)

public:
	CTreeCtrlEx() = default;
	virtual ~CTreeCtrlEx() = default;

	BOOL Create(DWORD dwStyle, DWORD dwExStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

public:
	UINT GetSelectedCount() const;
	HTREEITEM GetPrevItem(HTREEITEM hItem) const;
	HTREEITEM GetNextItem(HTREEITEM hItem) const;
	HTREEITEM GetNextItem(HTREEITEM hItem, UINT nCode) const;
	HTREEITEM GetLastItem(HTREEITEM hItem) const;
	HTREEITEM GetFirstSelectedItem() const;
	HTREEITEM GetLastSelectedItem() const;
	HTREEITEM GetNextSelectedItem(HTREEITEM hItem) const;
	HTREEITEM GetPrevSelectedItem(HTREEITEM hItem) const;
	std::vector<HTREEITEM> GetSelectedItems() const;

	BOOL IsSelected(HTREEITEM hItem) const { return !!(TVIS_SELECTED & GetItemState(hItem, TVIS_SELECTED)); }
	BOOL SelectItemEx(HTREEITEM hItem, BOOL bSelect = TRUE);

	BOOL SelectItems(HTREEITEM hFromItem, HTREEITEM hToItem);
	void ClearSelection(BOOL bMultiOnly = FALSE);

	BOOL CreateDragImageEx(CPoint ptDragPoint);

	void ScrollUp();
	void DeleteDragImageEx();

	virtual void SetItemFont(HTREEITEM hItem, LOGFONT& logfont);
	virtual BOOL GetItemFont(HTREEITEM hItem, LOGFONT* plogfont);
	virtual void SetItemBold(HTREEITEM hItem, BOOL bBold);
	virtual BOOL GetItemBold(HTREEITEM hItem);
	virtual void SetItemColor(HTREEITEM hItem, COLORREF color);
	virtual void SetItemBackColor(HTREEITEM hItem, COLORREF color);
	virtual COLORREF GetItemColor(HTREEITEM hItem);
	virtual COLORREF GetItemBackColor(HTREEITEM hItem);

	void OnPaint();

protected:
	void SelectMultiple(HTREEITEM hClickedItem, UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnItemexpanding(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnSetfocus(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnKillfocus(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnDeleteItem(NMHDR* pNMHDR, LRESULT* pResult);

protected:
	CPoint m_ptHotSpot;
	CImageList m_CurrentDragImage;
	CPoint    m_ptClick;
	BOOL      m_bSelectPending = FALSE;
	HTREEITEM m_hClickedItem = nullptr;
	HTREEITEM m_hFirstSelectedItem = nullptr;
	BOOL      m_bSelectionComplete = TRUE;
	BOOL      m_bEditLabelPending = FALSE;
	UINT_PTR  m_idTimer = 0;
	CColorFontMap m_mapColorFont; // Maps HTREEITEM handles with CLRFONT structures that contains
								  // the color and logfont information for the tree item.
};
