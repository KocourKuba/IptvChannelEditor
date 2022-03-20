#pragma once

class CInPlaceEdit : public CEdit
{

public:
	CInPlaceEdit(int iItem, int iSubItem, LPCTSTR initText);
	virtual ~CInPlaceEdit() = default;

public:
		BOOL PreTranslateMessage(MSG* pMsg) override;

protected:
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnNcDestroy();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()

private:
	int m_iItem;
	int m_iSubItem;
	CString m_initText;
};