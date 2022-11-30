#pragma once
#include "ResizedPropertySheet.h"
#include "Credentials.h"
#include "uri_stream.h"
#include "base_plugin.h"

class CPluginConfigPropertySheet : public CMFCPropertySheet
{
	DECLARE_DYNAMIC(CPluginConfigPropertySheet)

public:
	CPluginConfigPropertySheet() = default;
	CPluginConfigPropertySheet(LPCTSTR pszSection,
							   CWnd* pParentWnd = nullptr,
							   UINT iSelectPage = 0)
		: CMFCPropertySheet(_T(""), pParentWnd, iSelectPage), m_posKey(pszSection) {}

	CPluginConfigPropertySheet(LPCTSTR pszCaption,
							   LPCTSTR pszSection,
							   CWnd* pParentWnd = nullptr,
							   UINT iSelectPage = 0)
		: CMFCPropertySheet(pszCaption, pParentWnd, iSelectPage), m_posKey(pszSection) {}

	~CPluginConfigPropertySheet() override = default;

public:
	BOOL OnInitDialog() override;
	INT_PTR DoModal() override;

	std::wstring GetSelectedConfig();
	void UpdateControls();
	void AllowSave(bool val = true);

protected:
	DECLARE_MESSAGE_MAP()

	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnCbnSelchangeComboPluginConfig();
	afx_msg void OnBnClickedButtonSaveConfig();
	afx_msg void OnBnClickedButtonSaveAsConfig();

protected:
	void SetupDynamicLayout();
	void FillConfigs();

public:
	bool m_configPages = false;
	Credentials m_initial_cred;
	uri_stream* m_CurrentStream = nullptr;
	bool m_allow_save = false;
	std::shared_ptr<base_plugin> m_plugin;
	std::vector<std::wstring> m_configs;

protected:
	CComboBox m_wndPluginConfigs;

	CButton m_wndBtnSaveConf;
	CButton m_wndBtnSaveAsConf;

	CRect m_min_rc;
	CString m_posKey;
	int m_nHeaderHeight = 30;
};