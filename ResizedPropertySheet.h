#pragma once

class CResizedPropertySheet : public CMFCPropertySheet
{
	DECLARE_DYNAMIC(CResizedPropertySheet)

public:
	CResizedPropertySheet() = default;
	CResizedPropertySheet(UINT nIDCaption, LPCTSTR pszSection, CWnd* pParentWnd = nullptr, UINT iSelectPage = 0)
		: CMFCPropertySheet(nIDCaption, pParentWnd, iSelectPage), m_posKey(pszSection) {}
	CResizedPropertySheet(LPCTSTR pszCaption, LPCTSTR pszSection, CWnd* pParentWnd = nullptr, UINT iSelectPage = 0)
		: CMFCPropertySheet(pszCaption, pParentWnd, iSelectPage), m_posKey(pszSection) {}
	~CResizedPropertySheet() override = default;

public:
	BOOL OnInitDialog() override;
	INT_PTR DoModal() override;

protected:
	DECLARE_MESSAGE_MAP()

	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();

protected:
	void SetupDynamicLayout();

protected:
	CRect m_min_rc;
	CString m_posKey;
};
