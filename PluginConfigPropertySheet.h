/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2023): sharky72 (https://github.com/KocourKuba)

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
#include "ResizedPropertySheet.h"
#include "Credentials.h"
#include "uri_stream.h"
#include "base_plugin.h"

class CPluginConfigPropertySheet : public CMFCPropertySheet
{
	DECLARE_DYNAMIC(CPluginConfigPropertySheet)

public:
	CPluginConfigPropertySheet() = default;
	CPluginConfigPropertySheet(std::vector<std::wstring>& configs,
							   LPCTSTR pszSection,
							   CWnd* pParentWnd = nullptr,
							   UINT iSelectPage = 0)
		: CMFCPropertySheet(_T(""), pParentWnd, iSelectPage), m_posKey(pszSection)
		, m_configs(configs) {}

	CPluginConfigPropertySheet(std::vector<std::wstring>& configs,
							   LPCTSTR pszCaption,
							   LPCTSTR pszSection,
							   CWnd* pParentWnd = nullptr,
							   UINT iSelectPage = 0)
		: CMFCPropertySheet(pszCaption, pParentWnd, iSelectPage), m_posKey(pszSection)
		, m_configs(configs) {}

	~CPluginConfigPropertySheet() override = default;

public:
	BOOL OnInitDialog() override;
	INT_PTR DoModal() override;
	BOOL PreTranslateMessage(MSG* pMsg) override;

	std::wstring GetSelectedConfig();
	void UpdateControls();
	void AllowSave(bool val = true);

protected:
	DECLARE_MESSAGE_MAP()

	afx_msg BOOL OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnCbnSelchangeComboPluginConfig();
	afx_msg void OnBnClickedButtonSaveConfig();
	afx_msg void OnBnClickedButtonSaveAsConfig();

protected:
	void AddTooltip(UINT ctrlID, UINT textID);
	void SetupDynamicLayout();
	void FillConfigs();

public:
	bool m_configPages = false;
	bool m_allow_save = false;
	Credentials m_selected_cred;
	uri_stream* m_CurrentStream = nullptr;
	std::shared_ptr<base_plugin> m_plugin;

protected:
	CComboBox m_wndPluginConfigs;

	CButton m_wndBtnSaveConf;
	CButton m_wndBtnSaveAsConf;

	CRect m_min_rc;
	CString m_posKey;
	int m_gapHeight = 30;

	std::vector<std::wstring>& m_configs;

	CToolTipCtrl m_wndToolTipCtrl;
	std::map<CWnd*, std::wstring> m_tooltips_info;
};
