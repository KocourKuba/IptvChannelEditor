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
#include "EditableListCtrl.h"
#include "uri_stream.h"
#include "Config.h"
#include "CMFCEditBrowsCtrlEx.h"

#include "UtilsLib\json_wrapper.h"

class CAccessInfoPage : public CMFCPropertyPage
{
	DECLARE_DYNAMIC(CAccessInfoPage)

public:
	CAccessInfoPage(std::vector<std::wstring>& configs);   // standard constructor
	virtual ~CAccessInfoPage() = default;

	Credentials& GetCheckedAccount();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ACCESS_INFO };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	BOOL OnInitDialog() override;

	void UpdateOptionalControls();

	BOOL PreTranslateMessage(MSG* pMsg) override;
	BOOL OnApply() override;

	DECLARE_MESSAGE_MAP()

	afx_msg void OnNMDblClickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnItemchangedListAccounts(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnItemchangedListChannels(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnBnClickedButtonRemove();
	afx_msg void OnBnClickedButtonNewFromUrl();
	afx_msg void OnBnClickedCheckEmbed();
	afx_msg void OnCbnSelchangeComboConfigs();
	afx_msg void OnCbnSelchangeComboServerId();
	afx_msg void OnCbnSelchangeComboDeviceId();
	afx_msg void OnCbnSelchangeComboProfile();
	afx_msg void OnCbnSelchangeComboQuality();
	afx_msg void OnEnChangeEditPluginCaption();
	afx_msg void OnEnChangeEditPluginSuffix();
	afx_msg void OnEnChangeMfceditbrowsePluginLogo();
	afx_msg void OnEnChangeMfceditbrowsePluginBgnd();
	afx_msg void OnEnChangeEditPluginUpdateVersion();
	afx_msg void OnEnChangeEditPluginUpdateUrl();
	afx_msg void OnEnChangeEditPluginUpdateName();
	afx_msg void OnEnChangeEditPluginPackageName();
	afx_msg void OnEnChangeEditPluginUpdateFileUrl();
	afx_msg void OnEnChangeEditPluginChannelsWebPath();
	afx_msg void OnBnClickedCheckAutoincrementVersion();
	afx_msg void OnBnClickedCheckCustomUpdateName();
	afx_msg void OnBnClickedCheckCustomPackageName();
	afx_msg BOOL OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnNotifyEndEdit(WPARAM, LPARAM);

private:
	int GetCheckedAccountIdx();
	void GetAccountInfo();
	void CreateAccountsList();
	void CreateAccountInfo();
	void CreateChannelsList();
	void FillChannelsList();
	void FillConfigs();
	void SetWebUpdate();

public:
	CString m_status;

	Credentials m_initial_cred;
	std::wstring m_list_domain;
	std::wstring m_epg_domain;
	std::vector<std::wstring> m_all_channels_lists;
	std::shared_ptr<uri_stream> m_plugin;
	PluginType m_plugin_type = PluginType::enBase;

protected:
	CButton m_wndRemove;
	CButton m_wndNewFromUrl;
	CButton m_wndEmbed;
	CComboBox m_wndServers;
	CComboBox m_wndDevices;
	CComboBox m_wndQualities;
	CComboBox m_wndProfiles;
	CComboBox m_wndConfigs;
	CEdit m_wndSuffix;
	CEdit m_wndCaption;
	CMFCEditBrowseCtrlEx m_wndLogo;
	CMFCEditBrowseCtrlEx m_wndBackground;
	CEditableListCtrl m_wndAccounts;
	CListCtrl m_wndInfo;
	CListCtrl m_wndChLists;
	CMFCLinkCtrl m_wndProviderLink;
	CToolTipCtrl m_wndToolTipCtrl;
	CButton m_wndAutoIncrement;
	CButton m_wndCustomPackageName;
	CButton m_wndCustomUpdateName;
	CEdit m_wndUpdateUrl;
	CEdit m_wndUpdatePackageUrl;
	CEdit m_wndVersionID;
	CEdit m_wndUpdateName;
	CEdit m_wndPackageName;

private:
	CString m_logo;
	CString m_background;
	CString m_suffix;
	CString m_caption;
	CString m_updateInfoUrl;
	CString m_updatePackageUrl;
	CString m_updateInfoName;
	CString m_packageName;
	CString m_versionIdx;
	CString m_channelsWebPath;

	std::vector<DynamicParamsInfo> m_servers;
	std::vector<DynamicParamsInfo> m_devices;
	std::vector<DynamicParamsInfo> m_profiles;
	std::vector<DynamicParamsInfo> m_qualities;
	std::vector<Credentials> m_all_credentials;
	std::vector<std::wstring>& m_configs;

	std::map<UINT, std::wstring> m_tooltips_info_account;
};

