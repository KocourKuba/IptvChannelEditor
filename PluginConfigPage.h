/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2022): sharky72 (https://github.com/KocourKuba)

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
#include "uri_stream.h"
#include "AccessInfoPage.h"

class CPluginConfigPage : public CMFCPropertyPage
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
	BOOL PreTranslateMessage(MSG* pMsg) override;
	BOOL OnApply() override;

	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedButtonEditConfig();
	afx_msg void OnBnClickedButtonLoadConfig();
	afx_msg void OnBnClickedButtonSaveConfig();
	afx_msg void OnBnClickedButtonEpgTest();
	afx_msg void OnBnClickedButtonPlaylistShow();
	afx_msg void OnBnClickedButtonStreamParse();
	afx_msg void OnBnClickedButtonStreamIdParse();
	afx_msg void OnBnClickedButtonEditServers();
	afx_msg void OnBnClickedButtonEditDevices();
	afx_msg void OnBnClickedButtonEditQuality();
	afx_msg void OnBnClickedButtonEditProfiles();

	afx_msg void OnCbnSelchangeComboStreamSubType();
	afx_msg void OnCbnSelchangeComboEpgType();
	afx_msg void OnCbnSelchangeComboPluginType();

	afx_msg void OnEnChangeEditParsePattern();
	afx_msg void OnEnChangeEditParsePatternID();

	afx_msg BOOL OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult);

private:
	void EnableControls(bool enable);
	void FillControlsCommon();
	void FillControlsStream();
	void FillControlsEpg();

public:
	PluginType m_plugin_type = PluginType::enCustom;
	bool m_single = false;
	bool m_readonly = true;
	CString m_SetID;
	CAccessInfoPage* m_pAccessPage = nullptr;
	std::shared_ptr<uri_stream> m_plugin;

protected:
	CToolTipCtrl m_wndToolTipCtrl;
	CStatic m_wndDurationCaption;

	CEdit m_wndName;
	CEdit m_wndTitle;
	CEdit m_wndShortName;
	CEdit m_wndProviderUrl;
	CEdit m_wndPlaylistTemplate;
	CEdit m_wndParseStream;
	CEdit m_wndParseStreamID;
	CEdit m_wndSubst;
	CEdit m_wndDuration;
	CEdit m_wndStreamTemplate;
	CEdit m_wndStreamArchiveTemplate;
	CEdit m_wndEpgUrl;
	CEdit m_wndEpgRoot;
	CEdit m_wndEpgName;
	CEdit m_wndEpgDesc;
	CEdit m_wndEpgStart;
	CEdit m_wndEpgEnd;
	CEdit m_wndDateFormat;
	CEdit m_wndEpgTimeFormat;
	CEdit m_wndEpgTimezone;
	CEdit m_wndSetID;

	CComboBox m_wndAccessType;
	CComboBox m_wndStreamSubType;
	CComboBox m_wndCatchupType;
	CComboBox m_wndEpgType;
	CComboBox m_wndPluginType;

	CButton m_wndChkStaticServers;
	CButton m_wndBtnServers;
	CButton m_wndChkStaticDevices;
	CButton m_wndBtnDevices;
	CButton m_wndChkStaticQualities;
	CButton m_wndBtnQualities;
	CButton m_wndChkStaticProfiles;
	CButton m_wndBtnProfiles;
	CButton m_wndToggleEdit;
	CButton m_wndBtnLoadConf;
	CButton m_wndSaveConf;
	CButton m_wndSquareIcons;
	CButton m_wndEpgTest;
	CButton m_wndPlaylistShow;
	CButton m_wndBtnStreamParse;
	CButton m_wndBtnStreamParseID;

	CString m_Name;
	CString m_Title;
	CString m_ShortName;
	CString m_ProviderUrl;
	CString m_PlaylistTemplate;
	CString m_ParseStream;
	CString m_ParseStreamID;
	CString m_Subst;
	CString m_StreamTemplate;
	CString m_StreamArchiveTemplate;
	CString m_EpgUrl;
	CString m_EpgRoot;
	CString m_EpgName;
	CString m_EpgDesc;
	CString m_EpgStart;
	CString m_EpgEnd;
	CString m_EpgDateFormat;
	CString m_EpgTimeFormat;
	CString m_Token;

	COleDateTime m_Date;

	int m_Duration = 0;
	int m_EpgTimezone = 0;

private:
	std::map<UINT, std::wstring> m_tooltips_info_account;
	bool allowEdit = false;
};