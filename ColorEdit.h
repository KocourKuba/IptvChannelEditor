#pragma once

class CColorEdit : public CEdit
{
// Construction
public:
	CColorEdit();
	virtual ~CColorEdit() = default;

	void SetBkColor(COLORREF crColor);
	void SetTextColor(COLORREF crColor);
	BOOL SetReadOnly(BOOL flag = TRUE);

	DECLARE_MESSAGE_MAP()
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);

protected:
	CBrush m_brBkgnd;
	COLORREF m_crBkColor;
	COLORREF m_crTextColor;
};
