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

#include "pch.h"
#include <regex>
#include "AccessInfoPage.h"
#include "IPTVChannelEditor.h"
#include "AccountSettings.h"
#include "PlayListEntry.h"
#include "UrlDlg.h"
#include "Constants.h"

#include "UtilsLib\inet_utils.h"
#include "ResizedPropertySheet.h"
#include "PluginConfigPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

inline std::string get_utf8(const std::wstring& value)
{
	return utils::utf16_to_utf8(value);
}

inline std::string get_utf8(const CString& value)
{
	return utils::utf16_to_utf8(value.GetString(), value.GetLength());
}

inline std::string get_utf8(const wchar_t* value)
{
	return utils::utf16_to_utf8(std::wstring_view(value));
}

// CAccessDlg dialog

IMPLEMENT_DYNAMIC(CAccessInfoPage, CMFCPropertyPage)

BEGIN_MESSAGE_MAP(CAccessInfoPage, CMFCPropertyPage)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, &CAccessInfoPage::OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, &CAccessInfoPage::OnToolTipText)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CAccessInfoPage::OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, &CAccessInfoPage::OnBnClickedButtonRemove)
	ON_BN_CLICKED(IDC_BUTTON_NEW_FROM_URL, &CAccessInfoPage::OnBnClickedButtonNewFromUrl)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_ACCOUNTS, &CAccessInfoPage::OnNMDblClickList)
	ON_MESSAGE(WM_NOTIFY_END_EDIT, &CAccessInfoPage::OnNotifyEndEdit)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_ACCOUNTS, &CAccessInfoPage::OnLvnItemchangedListAccounts)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_CHANNELS, &CAccessInfoPage::OnLvnItemchangedListChannels)
	ON_CBN_SELCHANGE(IDC_COMBO_CONFIGS, &CAccessInfoPage::OnCbnSelchangeComboConfigs)
	ON_CBN_SELCHANGE(IDC_COMBO_DEVICE_ID, &CAccessInfoPage::OnCbnSelchangeComboDeviceId)
	ON_CBN_SELCHANGE(IDC_COMBO_SERVER_ID, &CAccessInfoPage::OnCbnSelchangeComboServerId)
	ON_CBN_SELCHANGE(IDC_COMBO_PROFILE, &CAccessInfoPage::OnCbnSelchangeComboProfile)
	ON_CBN_SELCHANGE(IDC_COMBO_QUALITY, &CAccessInfoPage::OnCbnSelchangeComboQuality)
	ON_BN_CLICKED(IDC_CHECK_EMBED, &CAccessInfoPage::OnBnClickedCheckEmbed)
	ON_EN_CHANGE(IDC_EDIT_PLUGIN_SUFFIX, &CAccessInfoPage::OnEnChangeEditPluginSuffix)
	ON_EN_CHANGE(IDC_EDIT_PLUGIN_CAPTION, &CAccessInfoPage::OnEnChangeEditPluginCaption)
	ON_EN_CHANGE(IDC_MFCEDITBROWSE_PLUGIN_LOGO, &CAccessInfoPage::OnEnChangeMfceditbrowsePluginLogo)
	ON_EN_CHANGE(IDC_MFCEDITBROWSE_PLUGIN_BGND, &CAccessInfoPage::OnEnChangeMfceditbrowsePluginBgnd)
	ON_EN_CHANGE(IDC_EDIT_PLUGIN_UPDATE_VERSION, &CAccessInfoPage::OnEnChangeEditPluginUpdateVersion)
	ON_BN_CLICKED(IDC_CHECK_AUTOINCREMENT_VERSION, &CAccessInfoPage::OnBnClickedCheckAutoincrementVersion)
	ON_EN_CHANGE(IDC_EDIT_PLUGIN_UPDATE_URL, &CAccessInfoPage::OnEnChangeEditPluginUpdateUrl)
	ON_EN_CHANGE(IDC_EDIT_PLUGIN_UPDATE_FILE_URL, &CAccessInfoPage::OnEnChangeEditPluginUpdateFileUrl)
	ON_BN_CLICKED(IDC_CHECK_CUSTOM_UPDATE_NAME, &CAccessInfoPage::OnBnClickedCheckCustomUpdateName)
	ON_BN_CLICKED(IDC_CHECK_CUSTOM_PACKAGE_NAME, &CAccessInfoPage::OnBnClickedCheckCustomPackageName)
	ON_EN_CHANGE(IDC_EDIT_PLUGIN_UPDATE_NAME, &CAccessInfoPage::OnEnChangeEditPluginUpdateName)
	ON_EN_CHANGE(IDC_EDIT_PLUGIN_PACKAGE_NAME, &CAccessInfoPage::OnEnChangeEditPluginPackageName)
	ON_EN_CHANGE(IDC_EDIT_PLUGIN_CHANNELS_WEB_PATH, &CAccessInfoPage::OnEnChangeEditPluginChannelsWebPath)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_CONFIG, &CAccessInfoPage::OnBnClickedButtonEditConfig)
END_MESSAGE_MAP()


CAccessInfoPage::CAccessInfoPage(std::vector<std::wstring>& configs)
	: CMFCPropertyPage(IDD_DIALOG_ACCESS_INFO)
	, m_configs(configs)
{
}

void CAccessInfoPage::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_MFCLINK_PROVIDER, m_wndProviderLink);
	DDX_Control(pDX, IDC_BUTTON_REMOVE, m_wndRemove);
	DDX_Control(pDX, IDC_BUTTON_NEW_FROM_URL, m_wndNewFromUrl);
	DDX_Control(pDX, IDC_LIST_ACCOUNTS, m_wndAccounts);
	DDX_Control(pDX, IDC_LIST_INFO, m_wndInfo);
	DDX_Control(pDX, IDC_LIST_CHANNELS, m_wndChLists);
	DDX_Control(pDX, IDC_COMBO_SERVER_ID, m_wndServers);
	DDX_Control(pDX, IDC_COMBO_DEVICE_ID, m_wndDevices);
	DDX_Control(pDX, IDC_COMBO_PROFILE, m_wndProfiles);
	DDX_Control(pDX, IDC_COMBO_QUALITY, m_wndQualities);
	DDX_Control(pDX, IDC_CHECK_EMBED, m_wndEmbed);
	DDX_Control(pDX, IDC_COMBO_CONFIGS, m_wndConfigs);
	DDX_Control(pDX, IDC_EDIT_PLUGIN_SUFFIX, m_wndSuffix);
	DDX_Text(pDX, IDC_EDIT_PLUGIN_SUFFIX, m_suffix);
	DDX_Control(pDX, IDC_EDIT_PLUGIN_CAPTION, m_wndCaption);
	DDX_Text(pDX, IDC_EDIT_PLUGIN_CAPTION, m_caption);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_PLUGIN_LOGO, m_wndLogo);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_PLUGIN_LOGO, m_logo);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_PLUGIN_BGND, m_wndBackground);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_PLUGIN_BGND, m_background);
	DDX_Text(pDX, IDC_EDIT_PLUGIN_CHANNELS_WEB_PATH, m_channelsWebPath);
	DDX_Text(pDX, IDC_EDIT_PLUGIN_UPDATE_URL, m_updateInfoUrl);
	DDX_Control(pDX, IDC_EDIT_PLUGIN_UPDATE_URL, m_wndUpdateUrl);
	DDX_Text(pDX, IDC_EDIT_PLUGIN_UPDATE_FILE_URL, m_updatePackageUrl);
	DDX_Control(pDX, IDC_EDIT_PLUGIN_UPDATE_FILE_URL, m_wndUpdatePackageUrl);
	DDX_Text(pDX, IDC_EDIT_PLUGIN_UPDATE_VERSION, m_versionIdx);
	DDX_Control(pDX, IDC_EDIT_PLUGIN_UPDATE_VERSION, m_wndVersionID);
	DDX_Control(pDX, IDC_CHECK_AUTOINCREMENT_VERSION, m_wndAutoIncrement);
	DDX_Control(pDX, IDC_CHECK_CUSTOM_UPDATE_NAME, m_wndCustomUpdateName);
	DDX_Control(pDX, IDC_EDIT_PLUGIN_UPDATE_NAME, m_wndUpdateName);
	DDX_Text(pDX, IDC_EDIT_PLUGIN_UPDATE_NAME, m_updateInfoName);
	DDX_Control(pDX, IDC_CHECK_CUSTOM_PACKAGE_NAME, m_wndCustomPackageName);
	DDX_Control(pDX, IDC_EDIT_PLUGIN_PACKAGE_NAME, m_wndPackageName);
	DDX_Text(pDX, IDC_EDIT_PLUGIN_PACKAGE_NAME, m_packageName);
	DDX_Control(pDX, IDC_BUTTON_EDIT_CONFIG, m_wndEditConfig);
}

BOOL CAccessInfoPage::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_LBUTTONDOWN
		|| pMsg->message == WM_LBUTTONUP
		|| pMsg->message == WM_MOUSEMOVE)
	{
		HWND hWnd = pMsg->hwnd;
		LPARAM lParam = pMsg->lParam;

		POINT pt{};
		pt.x = LOWORD(pMsg->lParam);  // horizontal position of cursor
		pt.y = HIWORD(pMsg->lParam);  // vertical position of cursor

		for (CWnd* wnd = GetWindow(GW_CHILD); wnd != NULL; wnd = wnd->GetWindow(GW_HWNDNEXT))
		{
			CRect rect;
			wnd->GetWindowRect(&rect);
			ScreenToClient(&rect);

			if (rect.PtInRect(pt)) {
				pMsg->hwnd = wnd->m_hWnd;

				ClientToScreen(&pt);
				wnd->ScreenToClient(&pt);
				pMsg->lParam = MAKELPARAM(pt.x, pt.y);
				break;
			}
		}

		m_wndToolTipCtrl.RelayEvent(pMsg);
		m_wndToolTipCtrl.Activate(TRUE);

		pMsg->hwnd = hWnd;
		pMsg->lParam = lParam;
	}

	return __super::PreTranslateMessage(pMsg);
}

BOOL CAccessInfoPage::OnInitDialog()
{
	__super::OnInitDialog();

	if (!m_wndToolTipCtrl.Create(this, TTS_ALWAYSTIP))
	{
		TRACE(_T("Unable To create ToolTip\n"));
		return FALSE;
	}

	m_tooltips_info_account =
	{
		{ IDC_BUTTON_ADD, load_string_resource(IDS_STRING_BUTTON_ADD) },
		{ IDC_BUTTON_REMOVE, load_string_resource(IDS_STRING_BUTTON_REMOVE) },
		{ IDC_BUTTON_NEW_FROM_URL, load_string_resource(IDS_STRING_BUTTON_NEW_FROM_URL) },
		{ IDC_COMBO_SERVER_ID, load_string_resource(IDS_STRING_COMBO_SERVER_ID) },
		{ IDC_COMBO_DEVICE_ID, load_string_resource(IDS_STRING_COMBO_DEVICE_ID) },
		{ IDC_COMBO_PROFILE, load_string_resource(IDS_STRING_COMBO_PROFILE) },
		{ IDC_COMBO_QUALITY, load_string_resource(IDS_STRING_QUALITY_ID) },
		{ IDC_CHECK_EMBED, load_string_resource(IDS_STRING_CHECK_EMBED) },
		{ IDC_MFCEDITBROWSE_PLUGIN_LOGO, load_string_resource(IDS_STRING_EDIT_ICON) },
		{ IDC_MFCEDITBROWSE_PLUGIN_BGND, load_string_resource(IDS_STRING_EDIT_BACKGROUND) },
		{ IDC_EDIT_PLUGIN_SUFFIX, load_string_resource(IDS_STRING_EDIT_SUFFIX) },
		{ IDC_EDIT_PLUGIN_CAPTION, load_string_resource(IDS_STRING_EDIT_CAPTION) },
		{ IDC_EDIT_PLUGIN_CHANNELS_WEB_PATH, load_string_resource(IDS_STRING_EDIT_PLUGIN_CHANNELS_WEB_PATH) },
		{ IDC_EDIT_PLUGIN_UPDATE_URL, load_string_resource(IDS_STRING_EDIT_PLUGIN_UPDATE_URL) },
		{ IDC_EDIT_PLUGIN_UPDATE_FILE_URL, load_string_resource(IDS_STRING_EDIT_PLUGIN_UPDATE_FILE_URL) },
		{ IDC_CHECK_AUTOINCREMENT_VERSION, load_string_resource(IDS_STRING_CHECK_AUTOINCREMENT_VERSION) },
		{ IDC_EDIT_PLUGIN_UPDATE_VERSION, load_string_resource(IDS_STRING_EDIT_PLUGIN_UPDATE_VERSION) },
		{ IDC_CHECK_CUSTOM_UPDATE_NAME, load_string_resource(IDS_STRING_CHECK_CUSTOM_UPDATE_NAME) },
		{ IDC_EDIT_PLUGIN_UPDATE_NAME, load_string_resource(IDS_STRING_EDIT_PLUGIN_UPDATE_NAME) },
		{ IDC_CHECK_CUSTOM_PACKAGE_NAME, load_string_resource(IDS_STRING_CHECK_CUSTOM_PACKAGE_NAME) },
		{ IDC_EDIT_PLUGIN_PACKAGE_NAME, load_string_resource(IDS_STRING_EDIT_PLUGIN_PACKAGE_NAME) },
		{ IDC_BUTTON_EDIT_CONFIG, load_string_resource(IDS_STRING_BUTTON_EDIT_CONFIG) },
		{ IDC_COMBO_CONFIGS, load_string_resource(IDS_STRING_COMBO_CONFIGS) },
	};

	m_wndToolTipCtrl.SetDelayTime(TTDT_AUTOPOP, 10000);
	m_wndToolTipCtrl.SetDelayTime(TTDT_INITIAL, 500);
	m_wndToolTipCtrl.SetMaxTipWidth(300);

	SetButtonImage(IDB_PNG_EDIT, m_wndEditConfig);

	for (const auto& pair : m_tooltips_info_account)
	{
		m_wndToolTipCtrl.AddTool(GetDlgItem(pair.first), LPSTR_TEXTCALLBACK);
	}

	m_wndToolTipCtrl.Activate(TRUE);

	std::wstring provider_url = m_plugin->get_provider_url();
	m_wndProviderLink.SetURL(provider_url.c_str());
	m_wndProviderLink.SetWindowText(provider_url.c_str());

	CString logo_filter(_T("PNG file(*.png)|*.png||"));
	m_wndLogo.EnableFileBrowseButton(nullptr,
									 logo_filter.GetString(),
									 OFN_EXPLORER | OFN_ENABLESIZING | OFN_LONGNAMES | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST);

	CString bg_filter(_T("JPG file(*.jpg)|*.jpg||"));
	m_wndBackground.EnableFileBrowseButton(nullptr,
										   bg_filter.GetString(),
										   OFN_EXPLORER | OFN_ENABLESIZING | OFN_LONGNAMES | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST);

	m_wndAccounts.SetExtendedStyle(m_wndAccounts.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
	CHeaderCtrl* header = m_wndAccounts.GetHeaderCtrl();
	header->ModifyStyle(0, HDS_CHECKBOXES);

	nlohmann::json creds;
	JSON_ALL_TRY;
	{
		creds = nlohmann::json::parse(GetConfig().get_string(false, REG_ACCOUNT_DATA));
	}
	JSON_ALL_CATCH;
	for (const auto& item : creds.items())
	{
		const auto& val = item.value();
		if (val.empty()) continue;

		JSON_ALL_TRY;
		{
			auto cred = val.get<Credentials>();
			m_all_credentials.emplace_back(cred);
		}
		JSON_ALL_CATCH;
	}

	CreateAccountsList();
	CreateAccountInfo();
	CreateChannelsList();
	FillConfigs();

	m_wndAccounts.SetCheck(GetConfig().get_int(false, REG_ACTIVE_ACCOUNT), TRUE);
	m_wndRemove.EnableWindow(m_wndAccounts.GetSelectionMark() != -1);

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CAccessInfoPage::OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult)
{
	// if there is a top level routing frame then let it handle the message
	if (GetRoutingFrame() != nullptr)
		return FALSE;

	// to be thorough we will need to handle UNICODE versions of the message also !!

	UINT nID = pNMHDR->idFrom;
	TOOLTIPTEXT* pTTT = (TOOLTIPTEXT*)pNMHDR;

	if (pNMHDR->code == TTN_NEEDTEXT && (pTTT->uFlags & TTF_IDISHWND))
	{
		// idFrom is actually the HWND of the tool
		nID = ::GetDlgCtrlID((HWND)nID);
	}

	if (nID != 0) // will be zero on a separator
	{
		auto& pair = m_tooltips_info_account.find(nID);
		if (pair != m_tooltips_info_account.end())
		{
			pTTT->lpszText = pair->second.data();
			*pResult = 0;
			return TRUE;
		}
	}

	return FALSE;
}

void CAccessInfoPage::UpdateOptionalControls()
{
	auto& selected = GetCheckedAccount();
	if (selected.not_valid)
		return;

	TemplateParams params;
	params.login = selected.get_login();
	params.password = selected.get_password();
	params.subdomain = selected.get_subdomain();
	params.server_idx = selected.server_id;
	params.device_idx = selected.device_id;
	params.profile_idx = selected.profile_id;
	params.quality_idx = selected.quality_id;

	if (m_plugin->get_plugin_type() == PluginType::enSharaclub)
	{
		m_list_domain = GetConfig().get_string(false, REG_LIST_DOMAIN);
		m_epg_domain = GetConfig().get_string(false, REG_EPG_DOMAIN);
		params.subdomain = m_list_domain;
	}

	m_plugin->fill_servers_list(params);
	m_servers = m_plugin->get_servers_list();
	m_wndServers.ResetContent();
	m_wndServers.EnableWindow(!m_servers.empty());

	if (!m_servers.empty())
	{
		for (const auto& info : m_servers)
		{
			m_wndServers.AddString(info.get_name().c_str());
		}

		if (params.server_idx >= (int)m_servers.size())
		{
			params.server_idx = 0;
		}

		m_wndServers.SetCurSel(params.server_idx);
	}

	m_plugin->fill_devices_list(params);
	m_devices = m_plugin->get_devices_list();
	m_wndDevices.EnableWindow(!m_devices.empty());
	m_wndDevices.ResetContent();
	if (!m_devices.empty())
	{
		for (const auto& info : m_devices)
		{
			m_wndDevices.AddString(info.get_name().c_str());
		}

		if (params.device_idx >= (int)m_devices.size())
		{
			params.device_idx = 0;
		}

		m_wndDevices.SetCurSel(params.device_idx);
	}

	m_plugin->fill_qualities_list(params);
	m_qualities = m_plugin->get_qualities_list();
	m_wndQualities.EnableWindow(!m_qualities.empty());
	m_wndQualities.ResetContent();
	if (!m_qualities.empty())
	{
		for (const auto& info : m_qualities)
		{
			m_wndQualities.AddString(info.get_name().c_str());
		}

		if (params.quality_idx >= (int)m_qualities.size())
		{
			params.quality_idx = 0;
		}

		m_wndQualities.SetCurSel(params.quality_idx);
	}

	m_plugin->fill_profiles_list(params);
	m_profiles = m_plugin->get_profiles_list();
	m_wndProfiles.EnableWindow(!m_qualities.empty());
	m_wndProfiles.ResetContent();

	if (!m_profiles.empty())
	{
		for (const auto& info : m_profiles)
		{
			m_wndProfiles.AddString(info.get_name().c_str());
		}

		if (params.profile_idx >= (int)m_profiles.size())
		{
			params.profile_idx = 0;
		}

		m_wndProfiles.SetCurSel(params.profile_idx);
	}

	auto it = std::find(m_configs.begin(), m_configs.end(), selected.get_config());
	int sel_idx = 0;
	if (it != m_configs.end())
		sel_idx = std::distance(m_configs.begin(), it);
	m_wndConfigs.SetCurSel(sel_idx);

	m_suffix = selected.get_suffix().c_str();
	m_caption = selected.get_caption().c_str();

	if (!selected.get_logo().empty() && std::filesystem::path(selected.get_logo()).parent_path().empty())
		m_logo = fmt::format(LR"({:s}\{:s})", GetAppPath(utils::PLUGIN_ROOT) + L"plugins_images", selected.get_logo()).c_str();
	else
		m_logo = selected.get_logo().c_str();

	if (!selected.get_background().empty() && std::filesystem::path(selected.get_background()).parent_path().empty())
		m_background = fmt::format(LR"({:s}\{:s})", GetAppPath(utils::PLUGIN_ROOT) + L"plugins_images", selected.get_background()).c_str();
	else
		m_background = selected.get_background().c_str();

	m_channelsWebPath = selected.get_ch_web_path().c_str();

	UpdateData(FALSE);
}

void CAccessInfoPage::CreateAccountsList()
{
	m_wndAccounts.DeleteAllItems();
	for (int i = m_wndAccounts.GetHeaderCtrl()->GetItemCount() - 1; i >=0; i--)
		m_wndAccounts.DeleteColumn(i);

	CRect rect;
	m_wndAccounts.GetClientRect(&rect);
	int vWidth = rect.Width() - GetSystemMetrics(SM_CXVSCROLL) - 22;

	m_wndNewFromUrl.EnableWindow(FALSE);
	int last = 0;
	m_wndAccounts.InsertColumn(last++, L"", LVCFMT_LEFT, 22, 0);
	switch (m_plugin->get_access_type())
	{
		case AccountAccessType::enPin:
			vWidth /= 2;
			m_wndAccounts.InsertColumn(last++, load_string_resource(IDS_STRING_COL_PASSWORD).c_str(), LVCFMT_LEFT, vWidth, 0);
			break;
		case AccountAccessType::enLoginPass:
			vWidth /= 3;
			m_wndAccounts.InsertColumn(last++, load_string_resource(IDS_STRING_COL_LOGIN).c_str(), LVCFMT_LEFT, vWidth, 0);
			m_wndAccounts.InsertColumn(last++, load_string_resource(IDS_STRING_COL_PASSWORD).c_str(), LVCFMT_LEFT, vWidth, 0);
			break;
		case AccountAccessType::enOtt:
			vWidth /= 4;
			m_wndAccounts.InsertColumn(last++, load_string_resource(IDS_STRING_COL_TOKEN).c_str(), LVCFMT_LEFT, vWidth, 0);
			m_wndAccounts.InsertColumn(last++, load_string_resource(IDS_STRING_COL_DOMAIN).c_str(), LVCFMT_LEFT, vWidth, 0);
			m_wndAccounts.InsertColumn(last++, load_string_resource(IDS_STRING_COL_VPORTAL).c_str(), LVCFMT_LEFT, vWidth, 0);
			m_wndNewFromUrl.EnableWindow(TRUE);
			break;
		default: break;
	}

	m_wndAccounts.InsertColumn(last++, load_string_resource(IDS_STRING_COL_COMMENT).c_str(), LVCFMT_LEFT, vWidth, 0);

	int idx = 0;
	for (const auto& cred : m_all_credentials)
	{
		m_wndAccounts.InsertItem(idx, L"", 0);

		size_t sub_idx = 0;
		switch (m_plugin->get_access_type())
		{
			case AccountAccessType::enPin:
				m_wndAccounts.SetItemText(idx, ++sub_idx, cred.get_password().c_str());
				break;

			case AccountAccessType::enLoginPass:
				m_wndAccounts.SetItemText(idx, ++sub_idx, cred.get_login().c_str());
				m_wndAccounts.SetItemText(idx, ++sub_idx, cred.get_password().c_str());
				break;

			case AccountAccessType::enOtt:
				m_wndAccounts.SetItemText(idx, ++sub_idx, cred.get_token().c_str());
				m_wndAccounts.SetItemText(idx, ++sub_idx, cred.get_subdomain().c_str());
				m_wndAccounts.SetItemText(idx, ++sub_idx, cred.get_portal().c_str());
				break;

			default:break;
		}

		m_wndAccounts.SetItemText(idx, ++sub_idx, cred.get_comment().c_str());
		m_wndAccounts.SetItemText(idx, ++sub_idx, cred.get_suffix().c_str());

		++idx;
	}
}

void CAccessInfoPage::CreateAccountInfo()
{
	m_wndInfo.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_GRIDLINES);

	CRect rect;
	m_wndInfo.GetClientRect(&rect);
	int vWidth = rect.Width() - GetSystemMetrics(SM_CXVSCROLL) - 1;
	int nWidth = vWidth / 4;

	m_wndInfo.InsertColumn(0, load_string_resource(IDS_STRING_COL_INFO).c_str(), LVCFMT_LEFT, nWidth);
	m_wndInfo.InsertColumn(1, load_string_resource(IDS_STRING_COL_DATA).c_str(), LVCFMT_LEFT, vWidth - nWidth);
}

void CAccessInfoPage::CreateChannelsList()
{
	m_wndChLists.SetExtendedStyle(m_wndAccounts.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
	CHeaderCtrl* header = m_wndAccounts.GetHeaderCtrl();
	header->ModifyStyle(0, HDS_CHECKBOXES);

	CRect rect;
	m_wndChLists.GetClientRect(&rect);
	int vWidth = rect.Width() - 22;

	m_wndChLists.InsertColumn(0, L"", LVCFMT_LEFT, 22, 0);
	m_wndChLists.InsertColumn(1, load_string_resource(IDS_STRING_COL_CHANNEL_NAME).c_str(), LVCFMT_LEFT, vWidth, 0);
	FillChannelsList();
}

void CAccessInfoPage::FillConfigs()
{
	m_wndConfigs.ResetContent();
	for (const auto& entry : m_configs)
	{
		m_wndConfigs.AddString(entry.c_str());
	}
	m_wndConfigs.SetCurSel(0);
}

void CAccessInfoPage::FillChannelsList()
{
	auto& selected = GetCheckedAccount();
	if (selected.not_valid)
		return;

	int idx = 0;
	auto ch_list(selected.ch_list);
	bool all = ch_list.empty();
	m_wndChLists.DeleteAllItems();
	for (const auto& channel : m_all_channels_lists)
	{
		bool check = all;
		if (!check)
		{
			for (const auto& item : ch_list)
			{
				if (item == get_utf8(channel))
				{
					check = true;
					break;
				}
			}
		}
		m_wndChLists.InsertItem(idx, L"", 0);
		m_wndChLists.SetItemText(idx, 1, channel.c_str());
		m_wndChLists.SetCheck(idx++, check);
	}

	m_wndChLists.EnableWindow(TRUE);
}

void CAccessInfoPage::SetWebUpdate()
{
	UpdateData(TRUE);

	auto& selected = GetCheckedAccount();
	if (selected.not_valid)
		return;

	m_wndAutoIncrement.SetCheck(selected.custom_increment);

	m_wndVersionID.EnableWindow(selected.custom_increment);
	m_wndUpdateName.EnableWindow(selected.custom_update_name);
	m_wndPackageName.EnableWindow(selected.custom_package_name);

	const auto& short_name_w = utils::utf8_to_utf16(m_plugin->get_short_name());
	const auto& suffix = utils::utf8_to_utf16(selected.suffix);

	if (selected.custom_update_name)
	{
		m_updateInfoName = selected.update_name.c_str();
	}
	else
	{
		m_updateInfoName = fmt::format(utils::DUNE_UPDATE_NAME, m_plugin->get_short_name(), (selected.suffix.empty()) ? "mod" : selected.suffix).c_str();
		m_updateInfoName += L".txt";
	}

	if (selected.custom_package_name)
	{
		m_packageName = selected.package_name.c_str();
	}
	else
	{
		m_packageName = fmt::format(utils::DUNE_UPDATE_NAME, m_plugin->get_short_name(), (selected.suffix.empty()) ? "mod" : selected.suffix).c_str();
		m_packageName += L".tar.gz";
	}

	if (selected.custom_increment)
	{
		m_versionIdx = selected.version_id.c_str();
	}
	else
	{
		COleDateTime date = COleDateTime::GetCurrentTime();
		m_versionIdx = fmt::format(L"{:d}{:02d}{:02d}{:02d}", date.GetYear(), date.GetMonth(), date.GetDay(), date.GetHour()).c_str();
	}

	m_updatePackageUrl = selected.update_package_url.c_str();
	m_updateInfoUrl = selected.update_url.c_str();

	UpdateData(FALSE);
}

BOOL CAccessInfoPage::OnApply()
{
	UpdateData(TRUE);

	auto& selected = GetCheckedAccount();
	if (selected.not_valid)
	{
		return TRUE;
	}

	if (selected.update_url.empty() != selected.update_package_url.empty())
	{
		AfxMessageBox(IDS_STRING_BOTH_PATH_MUST_FILLED, MB_ICONERROR | MB_OK);
		return FALSE;
	}

	TemplateParams params;
	params.login = utils::utf8_to_utf16(selected.login);
	params.password = utils::utf8_to_utf16(selected.password);
	params.subdomain = utils::utf8_to_utf16(selected.subdomain);
	params.server_idx = selected.server_id;
	params.device_idx = selected.device_id;
	params.profile_idx = selected.profile_id;
	params.quality_idx = selected.quality_id;

	if (m_plugin->get_plugin_type() == PluginType::enSharaclub)
	{
		params.subdomain = m_list_domain;
	}

	if (m_wndServers.GetCount())
	{
		m_plugin->set_server(params);
	}

	if (m_wndQualities.GetCount())
	{
		m_plugin->set_quality(params);
	}

	if (m_wndProfiles.GetCount())
	{
		m_plugin->set_profile(params);
	}

	selected.custom_increment = m_wndAutoIncrement.GetCheck();
	if (selected.custom_increment)
	{
		selected.version_id = get_utf8(m_versionIdx);
	}

	selected.custom_update_name = m_wndCustomUpdateName.GetCheck();
	if (selected.custom_update_name)
	{
		selected.update_name = get_utf8(m_updateInfoName);
	}

	selected.custom_package_name = m_wndCustomPackageName.GetCheck();
	if (selected.custom_package_name)
	{
		selected.package_name = get_utf8(m_packageName);
	}

	selected.update_package_url = get_utf8(m_updatePackageUrl);
	selected.update_url = get_utf8(m_updateInfoUrl);

	GetConfig().set_int(false, REG_ACTIVE_ACCOUNT, GetCheckedAccountIdx());

	nlohmann::json j_serialize = m_all_credentials;
	GetConfig().set_string(false, REG_ACCOUNT_DATA, utils::utf8_to_utf16(nlohmann::to_string(j_serialize)));

	m_initial_cred = selected;

	return TRUE;
}

void CAccessInfoPage::OnBnClickedButtonAdd()
{
	m_wndAccounts.InsertItem(m_wndAccounts.GetItemCount(), L"new");

	Credentials cred;
	static constexpr auto newVal = "new";
	switch (m_plugin->get_access_type())
	{
		case AccountAccessType::enPin:
			cred.password = newVal;
			break;

		case AccountAccessType::enLoginPass:
			cred.login = newVal;
			break;

		case AccountAccessType::enOtt:
			cred.token = newVal;
			break;

		default:break;
	}

	m_all_credentials.emplace_back(cred);
}

void CAccessInfoPage::OnBnClickedButtonRemove()
{
	int idx = m_wndAccounts.GetSelectionMark();
	if (idx != -1)
	{
		m_wndAccounts.DeleteItem(idx);
		m_all_credentials.erase(m_all_credentials.begin() + idx);
	}
}

void CAccessInfoPage::OnBnClickedButtonNewFromUrl()
{
	CUrlDlg dlg;

	if (dlg.DoModal() == IDOK)
	{
		m_status.Empty();
		std::vector<BYTE> data;
		std::wstring url = dlg.m_url.GetString();
		if (!utils::DownloadFile(url, data))
		{
			std::ifstream instream(url);
			data.assign((std::istreambuf_iterator<char>(instream)), std::istreambuf_iterator<char>());
		}

		const auto& wbuf = utils::utf8_to_utf16((char*)data.data(), data.size());
		std::wistringstream stream(wbuf);
		if (!stream.good()) return;

		Credentials cred;
		auto entry = std::make_unique<PlaylistEntry>(m_plugin, GetAppPath(utils::PLUGIN_ROOT));
		std::wstring line;
		while (std::getline(stream, line))
		{
			utils::string_rtrim(line, L"\r");
			m3u_entry m3uEntry(line);
			if (!entry->Parse(line, m3uEntry)) continue;

			const auto& access_key = entry->get_token();
			const auto& subdomain = entry->get_subdomain();
			if (!access_key.empty() && !subdomain.empty() && access_key != L"00000000000000" && subdomain != L"localhost")
			{
				int cnt = m_wndAccounts.GetItemCount();

				m_wndAccounts.InsertItem(cnt, L"", 0);
				m_wndAccounts.SetItemText(cnt, 1, access_key.c_str());
				m_wndAccounts.SetItemText(cnt, 2, subdomain.c_str());

				Credentials cred;
				cred.token = get_utf8(access_key);
				cred.subdomain = get_utf8(subdomain);
				m_all_credentials.emplace_back(cred);
				break;
			}
		}
	}
}

void CAccessInfoPage::OnNMDblClickList(NMHDR* pNMHDR, LRESULT* pResult)
{
	DWORD pos = GetMessagePos();
	CPoint pt(LOWORD(pos), HIWORD(pos));
	ScreenToClient(&pt);

	CRect rect;
	m_wndAccounts.GetWindowRect(&rect);
	ScreenToClient(&rect);

	pt.x -= rect.left;
	pt.y -= rect.top;

	m_wndAccounts.OnLButtonDown(MK_LBUTTON, pt);

	*pResult = 0;
}

LRESULT CAccessInfoPage::OnNotifyEndEdit(WPARAM wParam, LPARAM lParam)
{
	// Get the changed Description field text via the callback
	NMLVDISPINFO* dispinfo = reinterpret_cast<NMLVDISPINFO*>(lParam);

	// Persist the selected attachment details upon updating its text
	m_wndAccounts.SetItemText(dispinfo->item.iItem, dispinfo->item.iSubItem, dispinfo->item.pszText);
	auto& cred = m_all_credentials[dispinfo->item.iItem];
	switch (m_plugin->get_access_type())
	{
		case AccountAccessType::enPin:
			switch (dispinfo->item.iSubItem)
			{
				case 1:
					cred.password = get_utf8(dispinfo->item.pszText);
					break;
				case 2:
					cred.comment = get_utf8(dispinfo->item.pszText);
					break;
				default:
					break;
			}
			break;

		case AccountAccessType::enLoginPass:
			switch (dispinfo->item.iSubItem)
			{
				case 1:
					cred.login = get_utf8(dispinfo->item.pszText);
					break;
				case 2:
					cred.password = get_utf8(dispinfo->item.pszText);
					break;
				case 3:
					cred.comment = get_utf8(dispinfo->item.pszText);
					break;
				default:
					break;
			}
			break;

		case AccountAccessType::enOtt:
			switch (dispinfo->item.iSubItem)
			{
				case 1:
					cred.token = get_utf8(dispinfo->item.pszText);
					break;
				case 2:
					cred.subdomain = get_utf8(dispinfo->item.pszText);
					break;
				case 3:
					cred.portal = get_utf8(dispinfo->item.pszText);
					break;
				case 4:
					cred.comment = get_utf8(dispinfo->item.pszText);
					break;
				default:
					break;
			}
			break;

		default:break;
	}

	if (dispinfo->item.iItem == GetCheckedAccountIdx())
	{
		GetAccountInfo();
	}

	return 0;
}

void CAccessInfoPage::OnLvnItemchangedListAccounts(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	if ((pNMLV->uChanged & LVIF_STATE))
	{
		m_wndRemove.EnableWindow(pNMLV->uNewState & LVIS_SELECTED);

		UpdateOptionalControls();

		if ((pNMLV->uNewState & 0x2000) && (pNMLV->uOldState & 0x1000))
		{
			int cnt = m_wndAccounts.GetItemCount();
			for (int nItem = 0; nItem < cnt; nItem++)
			{
				if (nItem != pNMLV->iItem)
				{
					m_wndAccounts.SetCheck(nItem, FALSE);
				}
			}

			GetAccountInfo();
			FillChannelsList();
			GetParent()->GetDlgItem(IDOK)->EnableWindow(TRUE);
		}
		else if ((pNMLV->uNewState & 0x1000) && (pNMLV->uOldState & 0x2000))
		{
			m_wndInfo.DeleteAllItems();
			m_wndChLists.EnableWindow(FALSE);
			m_wndServers.EnableWindow(FALSE);
			m_wndProfiles.EnableWindow(FALSE);
			m_wndEmbed.EnableWindow(FALSE);
			m_wndLogo.EnableWindow(FALSE);
			m_wndBackground.EnableWindow(FALSE);
			m_wndSuffix.EnableWindow(FALSE);
			m_wndCaption.EnableWindow(FALSE);
			GetParent()->GetDlgItem(IDOK)->EnableWindow(FALSE);
		}

		SetWebUpdate();
	}

	*pResult = 0;
}

void CAccessInfoPage::OnLvnItemchangedListChannels(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	if ((pNMLV->uChanged & LVIF_STATE))
	{
		auto& selected = GetCheckedAccount();
		if (selected.not_valid)
			return;

		int cnt = m_wndChLists.GetItemCount();
		std::vector<std::string> ch_list;
		for (int nItem = 0; nItem < cnt; nItem++)
		{
			if (m_wndChLists.GetCheck(nItem))
			{
				ch_list.emplace_back(get_utf8(m_all_channels_lists[nItem]));
			}
		}

		if (ch_list.size() == m_all_channels_lists.size())
		{
			ch_list.clear();
		}

		selected.ch_list.swap(ch_list);
	}

	*pResult = 0;
}

void CAccessInfoPage::GetAccountInfo()
{
	UpdateData(TRUE);

	m_status.LoadString(IDS_STRING_STATUS_TEXT);
	m_wndProfiles.ResetContent();
	m_wndProfiles.EnableWindow(FALSE);
	m_wndInfo.DeleteAllItems();

	auto& selected_cred = GetCheckedAccount();
	if (selected_cred.not_valid)
		return;

	m_wndEmbed.SetCheck(selected_cred.embed);
	m_wndEmbed.EnableWindow(TRUE);
	m_wndLogo.EnableWindow(TRUE);
	m_wndBackground.EnableWindow(TRUE);
	m_wndSuffix.EnableWindow(TRUE);
	m_wndCaption.EnableWindow(TRUE);

	std::wstring login;
	std::wstring password;
	std::wstring token;
	std::wstring domain;
	std::wstring portal;

	switch (m_plugin->get_access_type())
	{
		case AccountAccessType::enPin:
			password = utils::utf8_to_utf16(selected_cred.password);
			break;

		case AccountAccessType::enLoginPass:
			login = utils::utf8_to_utf16(selected_cred.login);
			password = utils::utf8_to_utf16(selected_cred.password);
			break;

		case AccountAccessType::enOtt:
			token = utils::utf8_to_utf16(selected_cred.token);
			domain = utils::utf8_to_utf16(selected_cred.subdomain);
			portal = utils::utf8_to_utf16(selected_cred.portal);
			break;

		case AccountAccessType::enNone:
			return;
		default: break;
	}

	// reset template flag for new parse
	auto entry = std::make_shared<PlaylistEntry>(m_plugin, GetAppPath(utils::PLUGIN_ROOT));
	entry->set_is_template(false);

	TemplateParams params;
	params.login = std::move(login);
	params.password = std::move(password);
	params.subdomain = m_list_domain;
	params.server_idx = selected_cred.server_id;
	params.device_idx = selected_cred.device_id;
	params.profile_idx = selected_cred.profile_id;
	params.quality_idx = selected_cred.quality_id;

	if (m_plugin->get_plugin_type() == PluginType::enTVClub || m_plugin->get_plugin_type() == PluginType::enVidok)
	{
		params.token = m_plugin->get_api_token(selected_cred);
	}

	auto& pl_url = m_plugin->get_playlist_url(params);
	std::list<AccountInfo> acc_info;
	if (m_plugin->parse_access_info(params, acc_info))
	{
		for (auto it = acc_info.begin(); it != acc_info.end(); )
		{
			// currently supported only in sharaclub, oneott use this to obtain token
			if (it->name == (L"token"))
			{
				selected_cred.token = get_utf8(it->value);
				it = acc_info.erase(it);
			}
			else if (it->name == (L"url"))
			{
				pl_url = it->value;
				it = acc_info.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	std::vector<BYTE> data;
	if (!pl_url.empty() && utils::DownloadFile(pl_url, data))
	{
		const auto& wbuf = utils::utf8_to_utf16((char*)data.data(), data.size());
		std::wistringstream stream(wbuf);

		if (stream.good())
		{
			int step = 0;
			std::wstring line;
			while (std::getline(stream, line))
			{
				utils::string_rtrim(line, L"\r");
				m3u_entry m3uEntry(line);
				if (entry->Parse(line, m3uEntry) && !entry->get_token().empty())
				{
					// do not override fake ott and domain for edem
					if (m_plugin->get_access_type() != AccountAccessType::enOtt)
					{
						selected_cred.token = get_utf8(entry->get_token());
						selected_cred.subdomain = get_utf8(entry->get_domain());
					}
					m_status = _T("Ok");
					break;
				}
			}
		}
	}

	int idx = 0;
	m_wndInfo.InsertItem(idx, load_string_resource(IDS_STRING_STATUS).c_str());
	m_wndInfo.SetItemText(idx, 1, m_status);
	for (const auto& item : acc_info)
	{
		m_wndInfo.InsertItem(++idx, item.name.c_str());
		m_wndInfo.SetItemText(idx, 1, item.value.c_str());
	}

	UpdateData(FALSE);
}

Credentials& CAccessInfoPage::GetCheckedAccount()
{
	static Credentials empty;
	empty.not_valid = true;

	int selected = GetCheckedAccountIdx();
	return selected != -1 ? m_all_credentials[selected] : empty;
}

int CAccessInfoPage::GetCheckedAccountIdx()
{
	int cnt = m_wndAccounts.GetItemCount();
	for (int i = 0; i < cnt; i++)
	{
		if (m_wndAccounts.GetCheck(i))
		{
			return i;
		}
	}

	return -1;
}

void CAccessInfoPage::OnCbnSelchangeComboConfigs()
{
	auto& selected = GetCheckedAccount();
	int idx = m_wndConfigs.GetCurSel();
	if (idx < 1)
	{
		selected.config.clear();
	}
	else
	{
		CString value;
		m_wndConfigs.GetLBText(idx, value);
		selected.set_config(value.GetString());
	}
}

void CAccessInfoPage::OnCbnSelchangeComboServerId()
{
	auto& selected = GetCheckedAccount();
	selected.server_id = m_wndServers.GetCurSel();
}

void CAccessInfoPage::OnCbnSelchangeComboDeviceId()
{
	auto& selected = GetCheckedAccount();
	selected.device_id = m_wndDevices.GetCurSel();
}

void CAccessInfoPage::OnCbnSelchangeComboProfile()
{
	auto& selected = GetCheckedAccount();
	selected.profile_id = m_wndProfiles.GetCurSel();
}

void CAccessInfoPage::OnCbnSelchangeComboQuality()
{
	auto& selected = GetCheckedAccount();
	selected.quality_id = m_wndQualities.GetCurSel();
}

void CAccessInfoPage::OnBnClickedCheckEmbed()
{
	auto& selected = GetCheckedAccount();
	selected.embed = m_wndEmbed.GetCheck();
}

void CAccessInfoPage::OnEnChangeEditPluginSuffix()
{
	UpdateData(TRUE);
	auto& selected = GetCheckedAccount();
	m_suffix.Trim();
	if (utils::is_ascii(m_suffix.GetString()))
	{
		selected.suffix = get_utf8(m_suffix);
	}
	else
	{
		AfxMessageBox(IDS_STRING_WRN_NON_ASCII, MB_ICONERROR | MB_OK);
		m_suffix.Empty();
		UpdateData(FALSE);
	}
}

void CAccessInfoPage::OnEnChangeEditPluginCaption()
{
	UpdateData(TRUE);
	m_caption.Trim();
	auto& selected = GetCheckedAccount();
	selected.caption = get_utf8(m_caption);
}

void CAccessInfoPage::OnEnChangeMfceditbrowsePluginLogo()
{
	UpdateData(TRUE);
	auto& selected = GetCheckedAccount();
	m_logo.Trim();
	if (utils::is_ascii(std::filesystem::path(m_logo.GetString()).filename().wstring().c_str()))
	{
		selected.logo = get_utf8(m_logo);
	}
	else
	{
		AfxMessageBox(IDS_STRING_WRN_NON_ASCII, MB_ICONERROR | MB_OK);
		m_logo.Empty();
		UpdateData(FALSE);
	}
}

void CAccessInfoPage::OnEnChangeMfceditbrowsePluginBgnd()
{
	UpdateData(TRUE);
	auto& selected = GetCheckedAccount();
	m_background.Trim();
	if (utils::is_ascii(std::filesystem::path(m_background.GetString()).filename().wstring().c_str()))
	{
		selected.background = get_utf8(m_background);
	}
	else
	{
		AfxMessageBox(IDS_STRING_WRN_NON_ASCII, MB_ICONERROR | MB_OK);
		m_background.Empty();
		UpdateData(FALSE);
	}
}

void CAccessInfoPage::OnEnChangeEditPluginUpdateVersion()
{
	UpdateData(TRUE);
	auto& selected = GetCheckedAccount();
	selected.version_id = get_utf8(m_versionIdx);
}

void CAccessInfoPage::OnBnClickedCheckAutoincrementVersion()
{
	m_wndVersionID.EnableWindow(m_wndAutoIncrement.GetCheck());
}

void CAccessInfoPage::OnEnChangeEditPluginUpdateUrl()
{
	UpdateData(TRUE);
	auto& selected = GetCheckedAccount();
	selected.update_url = get_utf8(m_updateInfoUrl);
}

void CAccessInfoPage::OnEnChangeEditPluginUpdateFileUrl()
{
	UpdateData(TRUE);
	auto& selected = GetCheckedAccount();
	selected.update_package_url = get_utf8(m_updatePackageUrl);
}

void CAccessInfoPage::OnBnClickedCheckCustomUpdateName()
{
	m_wndUpdateName.EnableWindow(m_wndCustomUpdateName.GetCheck());
}

void CAccessInfoPage::OnBnClickedCheckCustomPackageName()
{
	m_wndPackageName.EnableWindow(m_wndCustomPackageName.GetCheck());
}

void CAccessInfoPage::OnEnChangeEditPluginUpdateName()
{
	UpdateData(TRUE);
	if (!utils::is_ascii(m_updateInfoName.GetString()))
	{
		AfxMessageBox(IDS_STRING_WRN_NON_ASCII, MB_ICONERROR | MB_OK);
	}
}

void CAccessInfoPage::OnEnChangeEditPluginPackageName()
{
	UpdateData(TRUE);
	if (!utils::is_ascii(m_packageName.GetString()))
	{
		AfxMessageBox(IDS_STRING_WRN_NON_ASCII, MB_ICONERROR | MB_OK);
	}
}

void CAccessInfoPage::OnEnChangeEditPluginChannelsWebPath()
{
	UpdateData(TRUE);

	auto& selected = GetCheckedAccount();
	selected.ch_web_path = get_utf8(m_channelsWebPath);
}

void CAccessInfoPage::OnBnClickedButtonEditConfig()
{
	auto pSheet = std::make_unique<CResizedPropertySheet>(L"", REG_PLUGIN_CFG_WINDOW_POS);
	pSheet->m_psh.dwFlags |= PSH_NOAPPLYNOW;
	pSheet->m_psh.dwFlags &= ~PSH_HASHELP;

	CPluginConfigPage dlgCfg(m_configs);
	dlgCfg.m_psp.dwFlags &= ~PSP_HASHELP;
	dlgCfg.m_plugin = m_plugin;
	dlgCfg.m_CurrentStream = m_CurrentStream;
	dlgCfg.m_initial_cred = m_initial_cred;

	pSheet->AddPage(&dlgCfg);

	auto res = (pSheet->DoModal() == IDOK);
	if (res)
	{
		m_plugin->load_plugin_parameters(m_initial_cred.get_config());
		CreateAccountsList();
	}
}
