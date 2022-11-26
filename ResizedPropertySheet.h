#pragma once
#include "Credentials.h"
#include "uri_stream.h"
#include "base_plugin.h"

class CResizedPropertySheet : public CMFCPropertySheet
{
	DECLARE_DYNAMIC(CResizedPropertySheet)

public:
	CResizedPropertySheet() = default;
	CResizedPropertySheet(std::vector<std::wstring>& configs,
						  LPCTSTR pszSection,
						  CWnd* pParentWnd = nullptr,
						  UINT iSelectPage = 0)
		: CMFCPropertySheet(_T(""), pParentWnd, iSelectPage)
		, m_posKey(pszSection)
		, m_configs(configs) {}

	CResizedPropertySheet(std::vector<std::wstring>& configs,
						  LPCTSTR pszCaption,
						  LPCTSTR pszSection,
						  CWnd* pParentWnd = nullptr,
						  UINT iSelectPage = 0)
		: CMFCPropertySheet(pszCaption, pParentWnd, iSelectPage), m_posKey(pszSection), m_configs(configs) {}

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

public:
	Credentials m_initial_cred;
	uri_stream* m_CurrentStream = nullptr;
	bool m_allow_save = false;
	bool m_allow_edit = false;
	std::shared_ptr<base_plugin> m_plugin;
	std::vector<std::wstring>& m_configs;

protected:
	CRect m_min_rc;
	CString m_posKey;
};
