#pragma once

class CEditableListCtrl : public CListCtrl
{
public:
	int GetRowFromPoint(CPoint& point, int* col) const;

	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

protected:
	void EditCell(int nItem, int nCol);
};
