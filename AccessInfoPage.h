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
#include "EditableListCtrl.h"
#include "CMFCEditBrowsCtrlEx.h"
#include "TooltipPropertyPage.h"
#include "Credentials.h"
#include "uri_stream.h"
#include "MenuEdit.h"

class CAccessInfoPage : public CTooltipPropertyPage
{
	DECLARE_DYNAMIC(CAccessInfoPage)

public:
	CAccessInfoPage();   // standard constructor
	virtual ~CAccessInfoPage() = default;

	Credentials& GetCheckedAccount();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ACCESS_INFO };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	BOOL OnInitDialog() override;
	BOOL OnApply() override;

	DECLARE_MESSAGE_MAP()

	afx_msg void OnNMDblClickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnItemchangedListAccounts(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnItemchangedListChannels(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnBnClickedButtonRemove();
	afx_msg void OnBnClickedButtonNewFromUrl();
	afx_msg void OnBnClickedCheckEmbed();
	afx_msg void OnBnClickedButtonEditConfig();
	afx_msg void OnBnClickedButtonBrowseDirectLink();
	afx_msg void OnBnClickedButtonBrowseUpdateDesc();
	afx_msg void OnBnClickedButtonBrowseUpdateFile();
	afx_msg void OnCbnSelchangeComboConfigs();
	afx_msg void OnCbnSelchangeComboServerId();
	afx_msg void OnCbnSelchangeComboDeviceId();
	afx_msg void OnCbnSelchangeComboProfile();
	afx_msg void OnCbnSelchangeComboQuality();
	afx_msg void OnBnClickedCheckCustomPluginCaption();
	afx_msg void OnEnChangeEditPluginCaption();
	afx_msg void OnBnClickedCheckCustomPluginNameTemplate();
	afx_msg void OnEnChangeEditPluginNameTemplate();
	afx_msg void OnBnClickedCheckCustomPluginLogo();
	afx_msg void OnEnChangeMfceditbrowsePluginLogo();
	afx_msg void OnBnClickedCheckCustomPluginBackground();
	afx_msg void OnEnChangeMfceditbrowsePluginBackground();
	afx_msg void OnBnClickedCheckCustomUpdateNameTemplate();
	afx_msg void OnEnChangeEditPluginUpdateNameTemplate();
	afx_msg void OnBnClickedCheckAutoincrementVersion();
	afx_msg void OnEnChangeEditPluginUpdateVersion();
	afx_msg void OnBnClickedCheckUseDropbox();
	afx_msg void OnEnChangeEditPluginChannelsWebPath();
	afx_msg LRESULT OnNotifyEndEdit(WPARAM, LPARAM);

public:
	void CreateAccountsList();

private:
	void UpdateOptionalControls(BOOL enable);
	int GetCheckedAccountIdx();
	int GetSelectedList();
	void GetAccountInfo();
	void CreateAccountInfo();
	void CreateChannelsList();
	void FillChannelsList();
	void FillConfigs();
	bool TransformDropboxPath(std::wstring& dropbox_link, const std::wstring& file);
	void UpdateTemplatedFields(const Credentials& selected);

public:
	CString m_status;

	std::wstring m_list_domain;
	std::wstring m_epg_domain;

	Credentials m_selected_cred;
	uri_stream* m_CurrentStream = nullptr;
	std::vector<std::wstring> m_all_channels_lists;
	std::shared_ptr<base_plugin> m_plugin;
	std::vector<std::wstring> m_configs;

protected:
	CButton m_wndRemove;
	CButton m_wndNewFromUrl;
	CButton m_wndEmbed;
	CButton m_wndEditConfig;
	CButton m_wndUseDropboxUpdate;
	CComboBox m_wndServers;
	CComboBox m_wndDevices;
	CComboBox m_wndQualities;
	CComboBox m_wndProfiles;
	CComboBox m_wndConfigs;
	CButton m_wndCustomCaption;
	CEdit m_wndCaption;
	CMenuEdit m_wndPluginNameTemplate;
	CButton m_wndCustomLogo;
	CMFCEditBrowseCtrlEx m_wndLogo;
	CButton m_wndCustomBackground;
	CMFCEditBrowseCtrlEx m_wndBackground;
	CEditableListCtrl m_wndAccounts;
	CListCtrl m_wndInfo;
	CListCtrl m_wndChLists;
	CMFCLinkCtrl m_wndProviderLink;
	CButton m_wndCustomPluginName;
	CButton m_wndAutoIncrement;
	CMFCEditBrowseCtrlEx m_wndDirectLink;
	CMFCEditBrowseCtrlEx m_wndUpdateUrl;
	CMFCEditBrowseCtrlEx m_wndUpdatePackageUrl;
	CEdit m_wndChannelsWebPath;
	CEdit m_wndVersionID;
	CButton m_wndCustomUpdateName;
	CMenuEdit m_wndUpdateNameTemplate;

private:
	int m_initial_config = 0;

	CString m_logo;
	CString m_background;
	CString m_caption;
	CString m_pluginNameTemplate;
	CString m_pluginName;
	CString m_updateNameTemplate;
	CString m_updateName;
	CString m_versionIdx;
	CString m_channelsWebPath;

	std::vector<DynamicParamsInfo> m_servers;
	std::vector<DynamicParamsInfo> m_devices;
	std::vector<DynamicParamsInfo> m_profiles;
	std::vector<DynamicParamsInfo> m_qualities;
	std::vector<Credentials> m_all_credentials;
};
