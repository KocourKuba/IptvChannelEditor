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
#include "base_plugin.h"
#include "AccessInfoPage.h"
#include "MenuEdit.h"

class CPluginConfigPage : public CMFCPropertyPage
{
	DECLARE_DYNAMIC(CPluginConfigPage)

public:
	CPluginConfigPage(std::vector<std::wstring>& configs);   // standard constructor
	virtual ~CPluginConfigPage() = default;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PLUGIN_CONFIG };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	BOOL OnInitDialog() override;

	void AddTooltip(UINT ctrlID, UINT textID);
	void AssignMacros();

	BOOL PreTranslateMessage(MSG* pMsg) override;
	BOOL OnApply() override;

	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedButtonToggleEditConfig();
	afx_msg void OnBnClickedButtonSaveConfig();
	afx_msg void OnBnClickedButtonSaveAsConfig();
	afx_msg void OnBnClickedButtonEpgTest();
	afx_msg void OnBnClickedButtonEditTemplates();
	afx_msg void OnCbnSelchangeComboPlaylistTemplate();
	afx_msg void OnCbnDropdownComboPlaylistTemplate();
	afx_msg void OnBnClickedButtonPlaylistShow();
	afx_msg void OnBnClickedButtonStreamRegexTest();
	afx_msg void OnCbnSelchangeComboVodTemplate();
	afx_msg void OnCbnDropdownComboVodTemplate();
	afx_msg void OnBnClickedButtonEditVodTemplates();
	afx_msg void OnBnClickedButtonVodTemplate();
	afx_msg void OnEnChangeEditProviderVodUrl();
	afx_msg void OnEnChangeEditVodRegex();
	afx_msg void OnBnClickedCheckVodSupport();
	afx_msg void OnBnClickedCheckVodM3U();
	afx_msg void OnBnClickedButtonEditServers();
	afx_msg void OnBnClickedButtonEditDevices();
	afx_msg void OnBnClickedButtonEditQuality();
	afx_msg void OnBnClickedButtonEditProfiles();
	afx_msg void OnBnClickedCheckMapTagToId();
	afx_msg void OnBnClickedCheckPerChannelToken();

	afx_msg void OnBnClickedCheckStaticServers();
	afx_msg void OnBnClickedCheckStaticDevices();
	afx_msg void OnBnClickedCheckStaticQualities();
	afx_msg void OnBnClickedCheckStaticProfiles();

	afx_msg void OnCbnSelchangeComboAccessType();
	afx_msg void OnCbnDropdownComboAccessType();

	afx_msg void OnCbnSelchangeComboPluginConfig();

	afx_msg void OnCbnSelchangeComboStreamType();
	afx_msg void OnCbnDropdownComboStreamType();

	afx_msg void OnCbnSelchangeComboEpgType();
	afx_msg void OnCbnDropdownComboEpgType();

	afx_msg void OnEnChangeEditParsePattern();

	afx_msg void OnDtnDatetimechangeDatetimepickerDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnChangeEditUtc();
	afx_msg void OnChanges();

	afx_msg BOOL OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult);

private:
	void AllowSave(bool val = true);
	void EnableControls();
	void FillConfigs();
	void FillControlsCommon();
	void SaveControlsCommon();
	void FillControlsStream();
	void SaveControlsStream();
	void FillControlsEpg();
	void SaveControlsEpg();
	std::wstring GetSelectedConfig();
	void UpdateDateTimestamp(bool dateToUtc);

public:
	Credentials m_initial_cred;
	uri_stream* m_CurrentStream = nullptr;
	std::shared_ptr<base_plugin> m_plugin;

protected:
	CToolTipCtrl m_wndToolTipCtrl;

	CDateTimeCtrl m_wndDate;

	CEdit m_wndName;
	CEdit m_wndTitle;
	CEdit m_wndShortName;
	CEdit m_wndProviderUrl;
	CMenuEdit m_wndPlaylistTemplate;
	CMenuEdit m_wndParseStream;
	CEdit m_wndSubst;
	CEdit m_wndDuration;
	CMenuEdit m_wndStreamTemplate;
	CMenuEdit m_wndStreamArchiveTemplate;
	CMenuEdit m_wndCustomStreamArchiveTemplate;
	CMenuEdit m_wndEpgUrl;
	CEdit m_wndEpgRoot;
	CEdit m_wndEpgName;
	CEdit m_wndEpgDesc;
	CEdit m_wndEpgStart;
	CEdit m_wndEpgEnd;
	CMenuEdit m_wndDateFormat;
	CMenuEdit m_wndEpgStartFormat;
	CEdit m_wndEpgTimezone;
	CEdit m_wndSetID;
	CEdit m_wndToken;
	CMenuEdit m_wndVodUrlTemplate;
	CMenuEdit m_wndVodRegex;

	CComboBox m_wndAccessType;
	CComboBox m_wndPlaylistTemplates;
	CComboBox m_wndVodTemplates;
	CComboBox m_wndStreamType;
	CComboBox m_wndCatchupType;
	CComboBox m_wndEpgType;
	CComboBox m_wndPluginConfigs;
	CComboBox m_wndTags;

	CButton m_wndBtnSaveConf;
	CButton m_wndBtnSaveAsConf;
	CButton m_wndBtnEditTemplates;
	CButton m_wndChkPerChannelToken;
	CButton m_wndChkStaticServers;
	CButton m_wndBtnServers;
	CButton m_wndChkStaticDevices;
	CButton m_wndBtnDevices;
	CButton m_wndChkStaticQualities;
	CButton m_wndChkStaticProfiles;
	CButton m_wndBtnQualities;
	CButton m_wndChkSquareIcons;
	CButton m_wndChkFilterSupport;
	CButton m_wndBtnProfiles;
	CButton m_wndBtnToggleEdit;
	CButton m_wndBtnEpgTest;
	CButton m_wndBtnPlaylistTest;
	CButton m_wndBtnStreamParseTest;
	CButton m_wndChkEnableVOD;
	CButton m_wndChkVodM3U;
	CButton m_wndBtnVodParseTest;
	CButton m_wndBtnVodTemplateTest;
	CButton m_wndChkUseDuration;
	CButton m_wndCheckMapTags;
	CButton m_wndBtnEditVodTemplates;

	CString m_Name;
	CString m_Title;
	CString m_ShortName;
	CString m_ProviderUrl;
	CString m_PlaylistTemplate;
	CString m_ParseStream;
	CString m_Subst;
	CString m_StreamTemplate;
	CString m_StreamArchiveTemplate;
	CString m_CustomStreamArchiveTemplate;
	CString m_EpgUrl;
	CString m_EpgRoot;
	CString m_EpgName;
	CString m_EpgDesc;
	CString m_EpgStart;
	CString m_EpgEnd;
	CString m_EpgDateFormat;
	CString m_EpgTimeFormat;
	CString m_Token;
	CString m_SetID;
	CString m_VodPlaylistTemplate;
	CString m_VodParseRegex;

	COleDateTime m_Date;

	time_t m_UTC = 0;
	int m_Duration = 0;
	int m_EpgTimezone = 0;

private:
	bool m_allow_save = false;
	bool m_allow_edit = false;
	std::map<CWnd*, std::wstring> m_tooltips_info;

	std::vector<std::wstring>& m_configs;
	std::array<StreamParameters, 2> m_supported_streams;
	std::array<EpgParameters, 2> m_epg_parameters;
};
