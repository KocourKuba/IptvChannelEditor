/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2024): sharky72 (https://github.com/KocourKuba)

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
#include "MenuEdit.h"
#include "TooltipPropertyPage.h"

class CPluginConfigPage : public CTooltipPropertyPage
{
	DECLARE_DYNAMIC(CPluginConfigPage)

public:
	CPluginConfigPage();   // standard constructor
	virtual ~CPluginConfigPage() = default;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PLUGIN_CONFIG };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	BOOL OnInitDialog() override;
	BOOL OnSetActive() override;

	void FillControls() override;

	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedButtonEditServers();
	afx_msg void OnBnClickedButtonEditDevices();
	afx_msg void OnBnClickedButtonEditQuality();
	afx_msg void OnBnClickedButtonEditProfiles();
	afx_msg void OnBnClickedButtonEditDomains();

	afx_msg void OnCbnSelchangeComboAccessType();

	afx_msg void OnBnClickedButtonEditExtFiles();
	afx_msg void OnBnClickedButtonEditManifest();

	afx_msg void SaveParameters();

private:
	void UpdateControls();

protected:

	CDateTimeCtrl m_wndDate;

	CEdit m_wndName;
	CEdit m_wndTitle;
	CEdit m_wndUserAgent;
	CEdit m_wndProviderUrl;
	CEdit m_wndClassName;
	CEdit m_wndProviderApiUrl;

	CComboBox m_wndAccessType;
	CButton m_wndBtnExtFiles;
	CButton m_wndBtnManifest;
	CButton m_wndBtnServers;
	CButton m_wndChkStaticServers;
	CButton m_wndBtnDevices;
	CButton m_wndChkStaticDevices;
	CButton m_wndBtnQualities;
	CButton m_wndChkStaticQualities;
	CButton m_wndBtnProfiles;
	CButton m_wndChkStaticProfiles;
	CButton m_wndBtnDomains;
	CButton m_wndChkStaticDomains;
	CButton m_wndChkSquareIcons;
	CButton m_wndChkEnableBalance;

	CString m_Name;
	CString m_Title;
	CString m_UserAgent;
	CString m_ProviderUrl;
	CString m_ClassName;
	CString m_ProviderApiUrl;
};
