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
#include "plugin_config.h"

class CPluginConfigPageEPG : public CTooltipPropertyPage
{
	DECLARE_DYNAMIC(CPluginConfigPageEPG)

public:
	CPluginConfigPageEPG();   // standard constructor
	virtual ~CPluginConfigPageEPG() = default;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PLUGIN_CONFIG_EPG };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	BOOL OnInitDialog() override;

	BOOL OnSetActive() override;
	BOOL OnApply() override;

	void AssignMacros();
	void FillControls() override;

	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedButtonEpgTest();
	afx_msg void OnCbnSelchangeComboEpgType();
	afx_msg void OnCbnSelchangeComboEpgParserPreset();
	afx_msg void OnEnChangeEditUtc();
	afx_msg void OnDtnDatetimechangeDatetimepickerDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void SaveParameters();

private:
	void UpdateControls();
	void UpdateDateTimestamp(bool dateToUtc);
	EpgParameters& GetEpgParameters();

protected:
	utils::CUrlDownload m_dl;

	CDateTimeCtrl m_wndDate;

	CEdit m_wndEpgDomain;
	CEdit m_wndEpgAuth;
	CMenuEdit m_wndEpgUrl;
	CEdit m_wndEpgRoot;
	CEdit m_wndEpgName;
	CEdit m_wndEpgDesc;
	CEdit m_wndEpgStart;
	CEdit m_wndEpgEnd;
	CMenuEdit m_wndDateFormat;
	CMenuEdit m_wndEpgStartFormat;
	CEdit m_wndEpgTimezone;

	CComboBox m_wndCatchupType;
	CComboBox m_wndEpgType;
	CComboBox m_wndEpgPreset;

	CButton m_wndBtnEpgTest;
	CButton m_wndChkUseDuration;

	CString m_EpgDomain;
	CString m_EpgAuth;
	CString m_EpgUrl;
	CString m_EpgRoot;
	CString m_EpgName;
	CString m_EpgDesc;
	CString m_EpgStart;
	CString m_EpgEnd;
	CString m_EpgDateFormat;
	CString m_EpgStartTimeFormat;
	CString m_Token;
	CString m_SetEpgID;
	CString m_SetID;
	CString m_DuneIP;

	COleDateTime m_Date;

	time_t m_UTC = 0;
	int m_EpgTimezone = 0;
};
