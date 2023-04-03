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
#include "MenuEdit.h"
#include "TooltipPropertyPage.h"

class CPluginConfigPageVOD : public CTooltipPropertyPage
{
	DECLARE_DYNAMIC(CPluginConfigPageVOD)

public:
	CPluginConfigPageVOD();   // standard constructor
	virtual ~CPluginConfigPageVOD() = default;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PLUGIN_CONFIG_VOD };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	BOOL OnInitDialog() override;
	BOOL OnSetActive() override;

	void AssignMacros();
	void FillControls() override;

	DECLARE_MESSAGE_MAP()

	afx_msg void OnCbnSelchangeComboVodTemplate();
	afx_msg void OnCbnDropdownComboVodTemplate();
	afx_msg void OnBnClickedButtonEditVodTemplates();
	afx_msg void OnBnClickedButtonVodTemplate();
	afx_msg void OnEnChangeEditProviderVodUrl();
	afx_msg void OnEnChangeEditVodRegex();
	afx_msg void OnBnClickedCheckVodSupport();
	afx_msg void OnBnClickedCheckVodM3U();
	afx_msg void OnBnClickedCheckVodFilter();
	afx_msg void OnEnChangeEditVodPrefix();
	afx_msg void OnEnChangeEditVodParams();

private:
	void UpdateControls();

protected:
	CMenuEdit m_wndVodUrlTemplate;
	CMenuEdit m_wndVodRegex;
	CMenuEdit m_wndVodUrlPrefix;
	CMenuEdit m_wndVodUrlParams;

	CComboBox m_wndVodTemplates;

	CButton m_wndChkEnableVOD;
	CButton m_wndChkVodM3U;
	CButton m_wndChkFilterSupport;
	CButton m_wndBtnVodParseTest;
	CButton m_wndBtnVodTemplateTest;
	CButton m_wndBtnEditVodTemplates;

	CString m_VodPlaylistTemplate;
	CString m_VodParseRegex;
	CString m_VodUrlPrefix;
	CString m_VodUrlParams;
};
