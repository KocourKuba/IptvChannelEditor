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
#include "UtilsLib\json_wrapper.h"

class CAccessInfoDlg : public CMFCPropertyPage
{
	DECLARE_DYNAMIC(CAccessInfoDlg)

public:
	CAccessInfoDlg();   // standard constructor
	virtual ~CAccessInfoDlg() = default;

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ACCESS_INFO };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	BOOL OnInitDialog() override;

	void UpdateOptionalControls();

	BOOL PreTranslateMessage(MSG* pMsg) override;
	void OnOK() override;

	DECLARE_MESSAGE_MAP()

	afx_msg void OnNMDblClickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnItemchangedListAccounts(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnItemchangedListChannels(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnBnClickedButtonRemove();
	afx_msg void OnBnClickedButtonNewFromUrl();
	afx_msg void OnBnClickedCheckEmbed();
	afx_msg void OnCbnSelchangeComboDeviceId();
	afx_msg void OnCbnSelchangeComboProfile();
	afx_msg void OnEnChangeEditPluginCaption();
	afx_msg void OnEnChangeEditPluginIcon();
	afx_msg void OnEnChangeEditPluginBackground();
	afx_msg void OnEnChangeEditPluginSuffix();
	afx_msg LRESULT OnNotifyDescriptionEdited(WPARAM, LPARAM);

private:
	int GetCheckedAccount();
	void GetAccountInfo();
	void LoadAccounts();
	void CreateAccountsList();
	void CreateAccountInfo();
	void CreateChannelsList();
	void FillChannelsList();

public:
	CString m_status;

	Credentials m_initial_cred;
	std::wstring m_host;
	std::wstring m_list_domain;
	std::wstring m_epg_domain;
	std::vector<std::wstring> m_all_channels_lists;

protected:
	CButton m_wndRemove;
	CButton m_wndNewFromUrl;
	CButton m_wndEmbed;
	CComboBox m_wndDeviceID;
	CComboBox m_wndProfile;
	CEdit m_wndCaption;
	CEdit m_wndLogo;
	CEdit m_wndBackground;
	CEdit m_wndSuffix;
	CEditableListCtrl m_wndAccounts;
	CListCtrl m_wndInfo;
	CListCtrl m_wndChLists;
	CMFCLinkCtrl m_wndProviderLink;
	CToolTipCtrl m_wndToolTipCtrl;

private:
	CString m_caption;
	CString m_logo;
	CString m_background;
	CString m_suffix;

	std::unique_ptr<uri_stream> m_plugin;
	std::vector<ServersInfo> m_servers;
	std::vector<ProfilesInfo> m_profiles;
	std::vector<Credentials> m_all_credentials;
	StreamType m_plugin_type = StreamType::enBase;
	AccountAccessType m_access_type = AccountAccessType::enUnknown;
};

