
#pragma once

#define TVGN_EX_ALL                     0x000F

// CTreeCtrlEx window

class CTreeCtrlEx : public CTreeCtrl
{
	DECLARE_DYNAMIC(CTreeCtrlEx)

public:
	CTreeCtrlEx() = default;
	virtual ~CTreeCtrlEx() = default;

	BOOL Create(DWORD dwStyle, DWORD dwExStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

public:
	UINT GetSelectedCount() const;
	HTREEITEM GetNextItem(HTREEITEM hItem, UINT nCode) const;
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

	void SetItemBold(HTREEITEM hItem, BOOL bBold);
	BOOL GetItemBold(HTREEITEM hItem);
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

public:
	COLORREF m_color = ::GetSysColor(COLOR_WINDOWTEXT);
	size_t class_hash;
	CPoint m_ptHotSpot;
	CImageList m_CurrentDragImage;

private:
	CPoint    m_ptClick;
	BOOL      m_bSelectPending = FALSE;
	HTREEITEM m_hClickedItem = nullptr;
	HTREEITEM m_hFirstSelectedItem = nullptr;
	BOOL      m_bSelectionComplete = TRUE;
	BOOL      m_bEditLabelPending = FALSE;
	UINT_PTR  m_idTimer = 0;

};
