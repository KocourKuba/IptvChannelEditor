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

#include "pch.h"
#include "AccessInfoPage.h"
#include "IPTVChannelEditor.h"
#include "AccountSettings.h"
#include "PlayListEntry.h"
#include "UrlDlg.h"
#include "Constants.h"
#include "StreamContainer.h"

#include "PluginConfigPage.h"
#include "PluginConfigPageTV.h"
#include "PluginConfigPageEPG.h"
#include "PluginConfigPageVOD.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

constexpr int min_cache_ttl = 5;

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

inline std::wstring get_utf16(const std::string& value)
{
	return utils::utf8_to_utf16(value);
}

inline std::wstring get_utf16(const char* value)
{
	return utils::utf8_to_utf16(std::string_view(value));
}

// CAccessDlg dialog

IMPLEMENT_DYNAMIC(CAccessInfoPage, CTooltipPropertyPage)

BEGIN_MESSAGE_MAP(CAccessInfoPage, CTooltipPropertyPage)
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
	ON_EN_CHANGE(IDC_EDIT_PLUGIN_CAPTION, &CAccessInfoPage::OnEnChangeEditPluginCaption)
	ON_BN_CLICKED(IDC_CHECK_CUSTOM_PLUGIN_NAME, &CAccessInfoPage::OnBnClickedCheckCustomPluginNameTemplate)
	ON_EN_CHANGE(IDC_EDIT_PLUGIN_ARCHIVE_TEMPLATE, &CAccessInfoPage::OnEnChangeEditPluginNameTemplate)
	ON_BN_CLICKED(IDC_CHECK_CUSTOM_PLUGIN_LOGO, &CAccessInfoPage::OnBnClickedCheckCustomPluginLogo)
	ON_EN_CHANGE(IDC_MFCEDITBROWSE_PLUGIN_LOGO, &CAccessInfoPage::OnEnChangeMfceditbrowsePluginLogo)
	ON_BN_CLICKED(IDC_CHECK_CUSTOM_PLUGIN_BGND, &CAccessInfoPage::OnBnClickedCheckCustomPluginBackground)
	ON_EN_CHANGE(IDC_MFCEDITBROWSE_PLUGIN_BGND, &CAccessInfoPage::OnEnChangeMfceditbrowsePluginBackground)
	ON_EN_CHANGE(IDC_EDIT_PLUGIN_CHANNELS_WEB_PATH, &CAccessInfoPage::OnEnChangeEditPluginChannelsWebPath)
	ON_WM_CUSTOM_MFC_BROWSE(IDC_MFCEDITBROWSE_PLUGIN_CHANNELS_DIRECT, &CAccessInfoPage::OnBnClickedButtonBrowseDirectLink)
	ON_EN_CHANGE(IDC_EDIT_PLUGIN_UPDATE_VERSION, &CAccessInfoPage::OnEnChangeEditPluginUpdateVersion)
	ON_BN_CLICKED(IDC_CHECK_AUTOINCREMENT_VERSION, &CAccessInfoPage::OnBnClickedCheckAutoincrementVersion)
	ON_WM_CUSTOM_MFC_BROWSE(IDC_MFCEDITBROWSE_PLUGIN_UPDATE_URL, &CAccessInfoPage::OnBnClickedButtonBrowseUpdateDesc)
	ON_WM_CUSTOM_MFC_BROWSE(IDC_MFCEDITBROWSE_PLUGIN_UPDATE_FILE_URL, &CAccessInfoPage::OnBnClickedButtonBrowseUpdateFile)
	ON_BN_CLICKED(IDC_CHECK_CUSTOM_UPDATE_NAME, &CAccessInfoPage::OnBnClickedCheckCustomUpdateNameTemplate)
	ON_EN_CHANGE(IDC_EDIT_PLUGIN_UPDATE_NAME_TEMPLATE, &CAccessInfoPage::OnEnChangeEditPluginUpdateNameTemplate)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_CONFIG, &CAccessInfoPage::OnBnClickedButtonEditConfig)
	ON_BN_CLICKED(IDC_CHECK_USE_DROPBOX, &CAccessInfoPage::OnBnClickedCheckUseDropbox)
	ON_BN_CLICKED(IDC_CHECK_CUSTOM_PLUGIN_CAPTION, &CAccessInfoPage::OnBnClickedCheckCustomPluginCaption)
END_MESSAGE_MAP()


CAccessInfoPage::CAccessInfoPage() : CTooltipPropertyPage(IDD_DIALOG_ACCESS_INFO)
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
	DDX_Control(pDX, IDC_EDIT_PLUGIN_ARCHIVE_TEMPLATE, m_wndPluginNameTemplate);
	DDX_Control(pDX, IDC_CHECK_CUSTOM_PLUGIN_NAME, m_wndCustomPluginName);
	DDX_Text(pDX, IDC_EDIT_PLUGIN_ARCHIVE_NAME, m_pluginName);
	DDX_Text(pDX, IDC_EDIT_PLUGIN_ARCHIVE_TEMPLATE, m_pluginNameTemplate);
	DDX_Control(pDX, IDC_CHECK_CUSTOM_PLUGIN_CAPTION, m_wndCustomCaption);
	DDX_Control(pDX, IDC_EDIT_PLUGIN_CAPTION, m_wndCaption);
	DDX_Text(pDX, IDC_EDIT_PLUGIN_CAPTION, m_caption);
	DDX_Control(pDX, IDC_CHECK_CUSTOM_PLUGIN_LOGO, m_wndCustomLogo);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_PLUGIN_LOGO, m_wndLogo);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_PLUGIN_LOGO, m_logo);
	DDX_Control(pDX, IDC_CHECK_CUSTOM_PLUGIN_BGND, m_wndCustomBackground);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_PLUGIN_BGND, m_wndBackground);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_PLUGIN_BGND, m_background);
	DDX_Text(pDX, IDC_EDIT_PLUGIN_CHANNELS_WEB_PATH, m_channelsWebPath);
	DDX_Control(pDX, IDC_EDIT_PLUGIN_CHANNELS_WEB_PATH, m_wndChannelsWebPath);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_PLUGIN_CHANNELS_DIRECT, m_wndDirectLink);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_PLUGIN_UPDATE_URL, m_wndUpdateUrl);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_PLUGIN_UPDATE_FILE_URL, m_wndUpdatePackageUrl);
	DDX_Text(pDX, IDC_EDIT_PLUGIN_UPDATE_VERSION, m_versionIdx);
	DDX_Control(pDX, IDC_EDIT_PLUGIN_UPDATE_VERSION, m_wndVersionID);
	DDX_Control(pDX, IDC_CHECK_AUTOINCREMENT_VERSION, m_wndAutoIncrement);
	DDX_Control(pDX, IDC_CHECK_CUSTOM_UPDATE_NAME, m_wndCustomUpdateName);
	DDX_Control(pDX, IDC_EDIT_PLUGIN_UPDATE_NAME_TEMPLATE, m_wndUpdateNameTemplate);
	DDX_Text(pDX, IDC_EDIT_PLUGIN_UPDATE_NAME_TEMPLATE, m_updateNameTemplate);
	DDX_Text(pDX, IDC_EDIT_PLUGIN_UPDATE_NAME, m_updateName);
	DDX_Control(pDX, IDC_BUTTON_EDIT_CONFIG, m_wndEditConfig);
	DDX_Control(pDX, IDC_CHECK_USE_DROPBOX, m_wndUseDropboxUpdate);
}

BOOL CAccessInfoPage::OnInitDialog()
{
	__super::OnInitDialog();

	AddTooltip(IDC_BUTTON_ADD, IDS_STRING_BUTTON_ADD);
	AddTooltip(IDC_BUTTON_REMOVE, IDS_STRING_BUTTON_REMOVE);
	AddTooltip(IDC_BUTTON_NEW_FROM_URL, IDS_STRING_BUTTON_NEW_FROM_URL);
	AddTooltip(IDC_COMBO_SERVER_ID, IDS_STRING_COMBO_SERVER_ID);
	AddTooltip(IDC_COMBO_DEVICE_ID, IDS_STRING_COMBO_DEVICE_ID);
	AddTooltip(IDC_COMBO_PROFILE, IDS_STRING_COMBO_PROFILE);
	AddTooltip(IDC_COMBO_QUALITY, IDS_STRING_QUALITY_ID);
	AddTooltip(IDC_CHECK_EMBED, IDS_STRING_CHECK_EMBED);
	AddTooltip(IDC_CHECK_CUSTOM_PLUGIN_LOGO, IDS_STRING_EDIT_ICON);
	AddTooltip(IDC_MFCEDITBROWSE_PLUGIN_LOGO, IDS_STRING_EDIT_ICON);
	AddTooltip(IDC_CHECK_CUSTOM_PLUGIN_BGND, IDS_STRING_EDIT_BACKGROUND);
	AddTooltip(IDC_MFCEDITBROWSE_PLUGIN_BGND, IDS_STRING_EDIT_BACKGROUND);
	AddTooltip(IDC_CHECK_CUSTOM_PLUGIN_CAPTION, IDS_STRING_EDIT_CAPTION);
	AddTooltip(IDC_EDIT_PLUGIN_CAPTION, IDS_STRING_EDIT_CAPTION);
	AddTooltip(IDC_CHECK_CUSTOM_PLUGIN_NAME, IDS_STRING_CHECK_CUSTOM_PLUGIN_NAME);
	AddTooltip(IDC_EDIT_PLUGIN_ARCHIVE_TEMPLATE, IDS_STRING_EDIT_PLUGIN_ARCHIVE_TEMPLATE);
	AddTooltip(IDC_EDIT_PLUGIN_ARCHIVE_NAME, IDS_STRING_EDIT_PLUGIN_ARCHIVE_NAME);
	AddTooltip(IDC_MFCEDITBROWSE_PLUGIN_CHANNELS_DIRECT, IDS_STRING_MFCEDITBROWSE_PLUGIN_CHANNELS_DIRECT);
	AddTooltip(IDC_EDIT_PLUGIN_CHANNELS_WEB_PATH, IDS_STRING_EDIT_PLUGIN_CHANNELS_WEB_PATH);
	AddTooltip(IDC_MFCEDITBROWSE_PLUGIN_UPDATE_URL, IDS_STRING_EDIT_PLUGIN_UPDATE_URL);
	AddTooltip(IDC_MFCEDITBROWSE_PLUGIN_UPDATE_FILE_URL, IDS_STRING_EDIT_PLUGIN_UPDATE_FILE_URL);
	AddTooltip(IDC_CHECK_AUTOINCREMENT_VERSION, IDS_STRING_CHECK_AUTOINCREMENT_VERSION);
	AddTooltip(IDC_EDIT_PLUGIN_UPDATE_VERSION, IDS_STRING_EDIT_PLUGIN_UPDATE_VERSION);
	AddTooltip(IDC_EDIT_PLUGIN_UPDATE_NAME, IDS_STRING_EDIT_PLUGIN_UPDATE_NAME);
	AddTooltip(IDC_CHECK_CUSTOM_UPDATE_NAME, IDS_STRING_EDIT_PLUGIN_UPDATE_NAME_TEMPLATE);
	AddTooltip(IDC_EDIT_PLUGIN_UPDATE_NAME_TEMPLATE, IDS_STRING_EDIT_PLUGIN_UPDATE_NAME_TEMPLATE);
	AddTooltip(IDC_BUTTON_EDIT_CONFIG, IDS_STRING_BUTTON_EDIT_CONFIG);
	AddTooltip(IDC_COMBO_CONFIGS, IDS_STRING_COMBO_CONFIGS);
	AddTooltip(IDC_CHECK_USE_DROPBOX, IDS_STRING_CHECK_USE_DROPBOX);

	SetButtonImage(IDB_PNG_EDIT, m_wndEditConfig);

	std::wstring provider_url = m_plugin->get_provider_url();
	m_wndProviderLink.SetURL(provider_url.c_str());
	m_wndProviderLink.SetWindowText(provider_url.c_str());
	m_wndDirectLink.EnableWindow(FALSE);
	m_wndDirectLink.EnableBrowseButton();
	m_wndUpdateUrl.EnableBrowseButton();
	m_wndUpdatePackageUrl.EnableBrowseButton();

	CString logo_filter(_T("PNG file(*.png)|*.png|All Files (*.*)|*.*||"));
	m_wndLogo.EnableFileBrowseButton(nullptr,
									 logo_filter.GetString(),
									 OFN_EXPLORER | OFN_ENABLESIZING | OFN_LONGNAMES | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST);

	CString bg_filter(_T("JPG file(*.jpg)|*.jpg|PNG file(*.png)|*.png|All Files (*.*)|*.*||"));
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

	std::vector<std::wstring> macroses =
	{
		REPL_TYPE,
		REPL_NAME,
		REPL_COMMENT,
		REPL_YEAR,
		REPL_MONTH,
		REPL_DAY,
		REPL_HOUR,
		REPL_MIN,
		REPL_TIMESTAMP,
		REPL_VERSION,
		REPL_VERSION_INDEX,
	};

	m_wndPluginNameTemplate.SetTemplateParams(macroses);
	m_wndUpdateNameTemplate.SetTemplateParams(macroses);

	FillConfigs();
	CreateAccountsList();
	CreateAccountInfo();
	CreateChannelsList();

	int account_idx = GetConfig().get_int(false, REG_ACTIVE_ACCOUNT);
	m_wndAccounts.SetCheck(account_idx, TRUE);
	m_wndAccounts.SetItemState(account_idx, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	m_wndAccounts.EnsureVisible(account_idx, FALSE);
	m_wndRemove.EnableWindow(m_wndAccounts.GetSelectionMark() != -1);

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CAccessInfoPage::CreateAccountsList()
{
	m_wndAccounts.DeleteAllItems();
	for (int i = m_wndAccounts.GetHeaderCtrl()->GetItemCount() - 1; i >=0; i--)
		m_wndAccounts.DeleteColumn(i);

	CRect rect;
	m_wndAccounts.GetClientRect(&rect);
	int vWidth = rect.Width() - GetSystemMetrics(SM_CXVSCROLL) - 22;

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
			break;
		default: break;
	}

	m_wndNewFromUrl.EnableWindow(!m_plugin->get_playlist_infos().empty() && !m_plugin->get_playlist_info(0).get_pl_parse_regex().empty());

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
	int vWidth = rect.Width() - GetSystemMetrics(SM_CXVSCROLL) - 24;

	m_wndChLists.InsertColumn(0, L"", LVCFMT_LEFT, 22, 0);
	m_wndChLists.InsertColumn(1, load_string_resource(IDS_STRING_COL_CHANNEL_NAME).c_str(), LVCFMT_LEFT, vWidth, 0);
	FillChannelsList();
}

void CAccessInfoPage::FillConfigs()
{
	m_wndConfigs.ResetContent();
	for (const auto& entry : m_configs)
	{
		int idx = m_wndConfigs.AddString(entry.c_str());
		if (entry == m_selected_cred.get_config())
			m_initial_config = idx;
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
		m_wndChLists.InsertItem(idx, L"", 0);
		m_wndChLists.SetItemText(idx, 1, channel.c_str());
		m_wndChLists.SetCheck(idx++, (!all && std::find(ch_list.begin(), ch_list.end(), get_utf8(channel)) != ch_list.end()));
	}

	m_wndChLists.EnableWindow(TRUE);
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
	params.login = selected.get_login();
	params.password = selected.get_password();
	params.subdomain = selected.get_subdomain();
	params.server_idx = selected.server_id;
	params.device_idx = selected.device_id;
	params.profile_idx = selected.profile_id;
	params.quality_idx = selected.quality_id;

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

	GetConfig().set_int(false, REG_ACTIVE_ACCOUNT, GetCheckedAccountIdx());

	nlohmann::json j_serialize = m_all_credentials;
	GetConfig().set_string(false, REG_ACCOUNT_DATA, utils::utf8_to_utf16(nlohmann::to_string(j_serialize)));

	m_selected_cred = selected;

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
		CWaitCursor cur;
		m_status.Empty();

		const auto& info = m_plugin->get_playlist_info(0);
		boost::wregex re(info.get_pl_parse_regex());
		std::wstring url = dlg.m_url.GetString();
		boost::wsmatch m;

		switch (m_plugin->get_plugin_type())
		{
			case PluginType::enEdem:
			{
				std::stringstream data;
				std::wstring url = dlg.m_url.GetString();
				if (!m_plugin->download_url(url, data))
				{
					std::ifstream instream(url);
					data << instream.rdbuf();
				}

				std::istringstream stream(data.str());
				if (!stream.good()) return;

				Credentials cred;
				auto playlist = std::make_unique<Playlist>();
				auto entry = std::make_unique<PlaylistEntry>(m_plugin, playlist, GetAppPath(utils::PLUGIN_ROOT));
				std::string line;
				while (std::getline(stream, line))
				{
					utils::string_rtrim(line, "\r");
					if (!entry->Parse(line)) continue;

					const auto& access_key = entry->get_token();
					const auto& subdomain = entry->get_subdomain();
					if (!access_key.empty() && !subdomain.empty() && access_key != L"00000000000000" && subdomain != L"localhost")
					{
						int cnt = m_wndAccounts.GetItemCount();

						m_wndAccounts.InsertItem(cnt, L"", 0);
						m_wndAccounts.SetItemText(cnt, 1, access_key.c_str());
						m_wndAccounts.SetItemText(cnt, 2, subdomain.c_str());

						Credentials cred;
						cred.set_token(access_key);
						cred.set_subdomain(subdomain);
						m_all_credentials.emplace_back(cred);
						break;
					}
				}
			}
			break;

			// Login/Password
			case PluginType::enGlanz:
			case PluginType::enFilmax:
			case PluginType::enFox:
			case PluginType::enKineskop:
			case PluginType::enMymagic:
			case PluginType::enOttIptv:
			case PluginType::enPing:
			case PluginType::enSmile:
			case PluginType::enSharaclub:
			case PluginType::enYossoTV:
			case PluginType::en101film:
			{
				if (boost::regex_match(url, m, re))
				{
					int cnt = m_wndAccounts.GetItemCount();
					m_wndAccounts.InsertItem(cnt, L"", 0);
					m_wndAccounts.SetItemText(cnt, 1, m[1].str().c_str());
					m_wndAccounts.SetItemText(cnt, 2, m[2].str().c_str());

					Credentials cred;
					cred.set_login(m[1].str());
					cred.set_password(m[2].str());
					m_all_credentials.emplace_back(cred);
				}
			}
			break;

			// Pin
			case PluginType::enAntifriz:
			case PluginType::enBcuMedia:
			case PluginType::enCbilling:
			case PluginType::enIptvOnline:
			case PluginType::enIpstream:
			case PluginType::enItv:
			case PluginType::enOneCent:
			case PluginType::enTVClub:
			case PluginType::enLightIptv:
			case PluginType::enOneUsd:
			case PluginType::enOttclub:
			case PluginType::enRusskoeTV:
			case PluginType::enSharavoz:
			case PluginType::enVidok:
			case PluginType::enVipLime:
			{
				if (boost::regex_match(url, m, re))
				{
					int cnt = m_wndAccounts.GetItemCount();
					m_wndAccounts.InsertItem(cnt, L"", 0);
					m_wndAccounts.SetItemText(cnt, 1, m[1].str().c_str());

					Credentials cred;
					cred.set_password(m[1].str());
					m_all_credentials.emplace_back(cred);
				}
			}
			break;
			default:
				break;
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
	bool changed = true;
	switch (m_plugin->get_access_type())
	{
		case AccountAccessType::enPin:
			switch (dispinfo->item.iSubItem)
			{
				case 1:
					cred.set_password(CString(dispinfo->item.pszText));
					break;
				case 2:
					cred.set_comment(CString(dispinfo->item.pszText));
					break;
				default:
					changed = false;
					break;
			}
			break;

		case AccountAccessType::enLoginPass:
			switch (dispinfo->item.iSubItem)
			{
				case 1:
					cred.set_login(CString(dispinfo->item.pszText));
					break;
				case 2:
					cred.set_password(CString(dispinfo->item.pszText));
					break;
				case 3:
					cred.set_comment(CString(dispinfo->item.pszText));
					break;
				default:
					changed = false;
					break;
			}
			break;

		case AccountAccessType::enOtt:
			switch (dispinfo->item.iSubItem)
			{
				case 1:
					cred.set_token(CString(dispinfo->item.pszText));
					break;
				case 2:
					cred.set_subdomain(CString(dispinfo->item.pszText));
					break;
				case 3:
					cred.set_portal(CString(dispinfo->item.pszText));
					break;
				case 4:
					cred.set_comment(CString(dispinfo->item.pszText));
					break;
				default:
					changed = false;
					break;
			}
			break;

		default:
			switch (dispinfo->item.iSubItem)
			{
				case 1:
					cred.set_comment(CString(dispinfo->item.pszText));
					break;
				default:
					changed = false;
					break;
			}
			break;
	}

	if (dispinfo->item.iItem == GetCheckedAccountIdx())
	{
		GetAccountInfo();
	}
	GetParent()->GetDlgItem(IDOK)->EnableWindow(changed);

	return 0;
}

void CAccessInfoPage::OnLvnItemchangedListAccounts(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;

	if (!(pNMLV->uChanged & LVIF_STATE))
		return;

	int selected = 0;
	if ((pNMLV->uNewState & LVIS_SELECTED) == LVIS_SELECTED)
	{
		m_wndAccounts.SetCheck(pNMLV->iItem, TRUE);
		selected = LVIS_SELECTED;
	}
	else if (pNMLV->uNewState == 0)
	{
		if (m_wndAccounts.GetItemState(pNMLV->iItem, LVIS_SELECTED) == 0)
		{
			m_wndAccounts.SetCheck(pNMLV->iItem, 0);
			selected = 0;
		}
	}
	else
	{
		selected = m_wndAccounts.GetCheck(pNMLV->iItem) ? LVIS_SELECTED : 0;
		m_wndAccounts.SetItemState(pNMLV->iItem, selected, LVIS_SELECTED);
	}

	m_wndRemove.EnableWindow(selected != 0);

	if (selected != 0)
	{
		m_plugin->clear_servers_list();
		m_plugin->clear_device_list();
		m_plugin->clear_profiles_list();
		m_plugin->clear_qualities_list();
		FillChannelsList();

		m_wndInfo.DeleteAllItems();
		GetAccountInfo();
	}

	BOOL enable = m_wndAccounts.GetCheck(pNMLV->iItem);

	UpdateOptionalControls(enable);

	GetParent()->GetDlgItem(IDOK)->EnableWindow(enable);
}

void CAccessInfoPage::UpdateOptionalControls(BOOL enable)
{
	auto& selected = GetCheckedAccount();
	if (selected.not_valid)
	{
		enable = FALSE;
	}

	m_wndConfigs.EnableWindow(enable);
	m_wndEditConfig.EnableWindow(enable);
	m_wndChLists.EnableWindow(enable);
	m_wndServers.EnableWindow(enable);
	m_wndProfiles.EnableWindow(enable);
	m_wndEmbed.EnableWindow(enable);
	m_wndCustomLogo.EnableWindow(enable);
	m_wndCustomBackground.EnableWindow(enable);
	m_wndCustomCaption.EnableWindow(enable);
	m_wndCustomPluginName.EnableWindow(enable);
	m_wndDirectLink.EnableWindow(enable);
	m_wndUpdatePackageUrl.EnableWindow(enable);
	m_wndUpdateUrl.EnableWindow(enable);
	m_wndChannelsWebPath.EnableWindow(enable);
	m_wndAutoIncrement.EnableWindow(enable);
	m_wndCustomUpdateName.EnableWindow(enable);
	m_wndUseDropboxUpdate.EnableWindow(enable);

	if (selected.not_valid)
	{
		m_wndCaption.EnableWindow(enable);
		m_wndPluginNameTemplate.EnableWindow(FALSE);
		m_wndLogo.EnableWindow(FALSE);
		m_wndBackground.EnableWindow(FALSE);
		m_wndVersionID.EnableWindow(FALSE);
		m_wndUpdateNameTemplate.EnableWindow(FALSE);
		return;
	}

	auto it = std::find(m_configs.begin(), m_configs.end(), selected.get_config());
	int sel_idx = 0;
	if (it != m_configs.end())
	{
		sel_idx = std::distance(m_configs.begin(), it);
		m_plugin->load_plugin_parameters(selected.get_config());
	}
	m_wndConfigs.SetCurSel(sel_idx);

	TemplateParams params;
	params.login = selected.get_login();
	params.password = selected.get_password();
	params.subdomain = selected.get_subdomain();
	params.server_idx = selected.server_id;
	params.device_idx = selected.device_id;
	params.profile_idx = selected.profile_id;
	params.quality_idx = selected.quality_id;

	m_plugin->fill_servers_list(&params);
	m_servers = m_plugin->get_servers_list();
	m_wndServers.ResetContent();
	m_wndServers.EnableWindow(!m_servers.empty() && enable);

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

	m_plugin->fill_devices_list(&params);
	m_devices = m_plugin->get_devices_list();
	m_wndDevices.EnableWindow(!m_devices.empty() && enable);
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

	m_plugin->fill_qualities_list(&params);
	m_qualities = m_plugin->get_qualities_list();
	m_wndQualities.EnableWindow(!m_qualities.empty() && enable);
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

	m_plugin->fill_profiles_list(&params);
	m_profiles = m_plugin->get_profiles_list();
	m_wndProfiles.EnableWindow(!m_profiles.empty() && enable);
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

	m_wndCustomCaption.SetCheck(selected.custom_caption);
	m_wndCaption.EnableWindow(selected.custom_caption && enable);
	m_caption = selected.get_caption().c_str();

	m_wndCustomPluginName.SetCheck(selected.custom_plugin_name);
	m_wndPluginNameTemplate.EnableWindow(selected.custom_plugin_name && enable);
	m_pluginNameTemplate = (selected.custom_plugin_name ? selected.get_plugin_name().c_str() : utils::DUNE_PLUGIN_FILE_NAME);
	if (m_pluginNameTemplate.IsEmpty())
		m_pluginNameTemplate = utils::DUNE_UPDATE_INFO_NAME;

	m_pluginName = fmt::format(L"dune_plugin_{:s}.zip", m_plugin->compile_name_template(m_pluginNameTemplate.GetString(), selected)).c_str();

	m_wndCustomLogo.SetCheck(selected.custom_logo);
	m_wndLogo.EnableWindow(selected.custom_logo && enable);

	if (!selected.get_logo().empty() && std::filesystem::path(selected.get_logo()).parent_path().empty())
		m_logo = fmt::format(LR"({:s}\{:s})", GetAppPath(utils::PLUGIN_ROOT) + L"plugins_images", selected.get_logo()).c_str();
	else
		m_logo = selected.get_logo().c_str();

	m_wndCustomBackground.SetCheck(selected.custom_background);
	m_wndBackground.EnableWindow(selected.custom_background && enable);

	if (!selected.get_background().empty() && std::filesystem::path(selected.get_background()).parent_path().empty())
		m_background = fmt::format(LR"({:s}\{:s})", GetAppPath(utils::PLUGIN_ROOT) + L"plugins_images", selected.get_background()).c_str();
	else
		m_background = selected.get_background().c_str();

	m_channelsWebPath = selected.get_ch_web_path().c_str();

	m_wndAutoIncrement.SetCheck(selected.custom_increment);
	m_wndVersionID.EnableWindow(selected.custom_increment && enable);

	m_wndCustomUpdateName.SetCheck(selected.custom_update_name);
	m_wndUpdateNameTemplate.EnableWindow(selected.custom_update_name && enable);

	m_updateNameTemplate = (selected.custom_update_name ? selected.get_update_name().c_str() : utils::DUNE_UPDATE_INFO_NAME);
	if (m_updateNameTemplate.IsEmpty())
		m_updateNameTemplate = utils::DUNE_UPDATE_INFO_NAME;

	m_updateName = m_plugin->compile_name_template(m_updateNameTemplate.GetString(), selected).c_str();

	if (selected.custom_increment)
	{
		m_versionIdx = selected.get_version_id().c_str();
	}
	else
	{
		COleDateTime date = COleDateTime::GetCurrentTime();
		m_versionIdx = fmt::format(L"{:d}{:02d}{:02d}{:02d}", date.GetYear(), date.GetMonth(), date.GetDay(), date.GetHour()).c_str();
	}

	m_wndUpdatePackageUrl.SetWindowText(selected.get_update_package_url().c_str());
	m_wndUpdateUrl.SetWindowText(selected.get_update_url().c_str());
	m_wndUseDropboxUpdate.SetCheck(selected.use_dropbox ? 1 : 0);

	UpdateData(FALSE);
}

void CAccessInfoPage::OnLvnItemchangedListChannels(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	if ((pNMLV->uChanged & LVIF_STATE))
	{
		auto& selected = GetCheckedAccount();
		if (selected.not_valid)
			return;

		if (pNMLV->uOldState == 0 && (pNMLV->uNewState & LVIS_SELECTED) == LVIS_SELECTED)
		{
			if (const auto& pair = selected.m_direct_links.find(get_utf8(m_all_channels_lists[pNMLV->iItem])); pair != selected.m_direct_links.end())
			{
				m_wndDirectLink.SetWindowText(get_utf16(pair->second).c_str());
			}
			else
			{
				m_wndDirectLink.SetWindowText(L"");
			}

			m_wndDirectLink.EnableWindow(TRUE);
		}
		else if (pNMLV->uNewState == 0 && (pNMLV->uOldState & LVIS_SELECTED) == LVIS_SELECTED)
		{
			m_wndDirectLink.EnableWindow(FALSE);
		}

		int cnt = m_wndChLists.GetItemCount();
		std::vector<std::string> ch_list;
		for (int nItem = 0; nItem < cnt; nItem++)
		{
			if (m_wndChLists.GetCheck(nItem))
			{
				ch_list.emplace_back(get_utf8(m_all_channels_lists[nItem]));
			}
		}

		selected.ch_list.swap(ch_list);
	}

	*pResult = 0;
}

void CAccessInfoPage::GetAccountInfo()
{
	UpdateData(TRUE);

	m_status = load_string_resource(IDS_STRING_STATUS_TEXT).c_str();
	m_wndProfiles.ResetContent();
	m_wndProfiles.EnableWindow(FALSE);
	m_wndInfo.DeleteAllItems();

	auto& selected_cred = GetCheckedAccount();
	if (selected_cred.not_valid)
		return;

	m_wndEmbed.SetCheck(selected_cred.embed);
	m_wndEmbed.EnableWindow(TRUE);

	if (m_plugin->get_access_type() == AccountAccessType::enNone)
		return;

	const auto& plugin_type = m_plugin->get_plugin_type();
	// reset template flag for new parse
	auto playlist = std::make_unique<Playlist>();
	auto entry = std::make_shared<PlaylistEntry>(m_plugin, playlist, GetAppPath(utils::PLUGIN_ROOT));
	entry->set_is_template(false);

	TemplateParams params;
	params.login = selected_cred.get_login();
	params.password = selected_cred.get_password();
	params.subdomain = selected_cred.get_subdomain();
	params.server_idx = selected_cred.server_id;
	params.device_idx = selected_cred.device_id;
	params.profile_idx = selected_cred.profile_id;
	params.quality_idx = selected_cred.quality_id;

	if (plugin_type == PluginType::enTVClub || plugin_type == PluginType::enVidok)
	{
		params.token = m_plugin->get_api_token(selected_cred);
	}
	else
	{
		params.token = selected_cred.get_token();
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
				selected_cred.set_token(it->value);
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

	CWaitCursor cur;
	std::stringstream data;
	if (!pl_url.empty() && m_plugin->download_url(pl_url, data, min_cache_ttl))
	{
		std::istringstream stream(data.str());

		if (stream.good())
		{
			int step = 0;
			std::string line;
			while (std::getline(stream, line))
			{
				utils::string_rtrim(line, "\r");
				if (entry->Parse(line) && !entry->get_token().empty())
				{
					// do not override fake ott and domain for edem
					if (m_plugin->get_access_type() != AccountAccessType::enOtt)
					{
						selected_cred.set_token(entry->get_token());
						selected_cred.set_subdomain(entry->get_domain());
					}
					m_status = _T("Ok");
					break;
				}
			}
		}
	}

	if (plugin_type == PluginType::enSmile || plugin_type == PluginType::en101film)
	{
		static std::vector<std::string> skipped_tag = {
			"bitrates", "url-tvg", "x-tvg-url", "servers"
		};

		const auto& tags = playlist->m3u_header.get_ext_tags();
		for (const auto& tag : tags)
		{
			if (std::find(skipped_tag.begin(), skipped_tag.end(), tag.first) != skipped_tag.end()) continue;

			AccountInfo info{ utils::utf8_to_utf16(tag.first), utils::utf8_to_utf16(tag.second) };
			acc_info.emplace_back(info);
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

int CAccessInfoPage::GetSelectedList()
{
	POSITION pos = m_wndChLists.GetFirstSelectedItemPosition();
	return (pos == nullptr) ? -1 : m_wndChLists.GetNextSelectedItem(pos);
}

void CAccessInfoPage::OnCbnSelchangeComboConfigs()
{
	int account_idx = GetCheckedAccountIdx();
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
		selected.set_config(value);
		m_plugin->load_plugin_parameters(selected.get_config());
		CreateAccountsList();
		m_wndAccounts.SetCheck(account_idx);
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

void CAccessInfoPage::OnBnClickedCheckCustomPluginCaption()
{
	auto& selected = GetCheckedAccount();
	selected.custom_caption = m_wndCustomCaption.GetCheck();

	m_wndCaption.EnableWindow(selected.custom_caption);
}

void CAccessInfoPage::OnEnChangeEditPluginCaption()
{
	UpdateData(TRUE);
	m_caption.Trim();
	auto& selected = GetCheckedAccount();
	selected.set_caption(m_caption);
}

void CAccessInfoPage::OnBnClickedCheckCustomPluginNameTemplate()
{
	auto& selected = GetCheckedAccount();
	selected.custom_plugin_name = m_wndCustomPluginName.GetCheck();

	m_wndPluginNameTemplate.EnableWindow(selected.custom_plugin_name);

	OnEnChangeEditPluginNameTemplate();
}

void CAccessInfoPage::OnEnChangeEditPluginNameTemplate()
{
	UpdateData(TRUE);
	auto& selected = GetCheckedAccount();
	if (utils::is_ascii(m_pluginNameTemplate.GetString()))
	{
		m_pluginNameTemplate.Trim();
		selected.set_plugin_name(m_pluginNameTemplate);
	}
	else
	{
		AfxMessageBox(IDS_STRING_WRN_NON_ASCII, MB_ICONERROR | MB_OK);

		m_pluginNameTemplate = selected.get_plugin_name().c_str();
	}

	m_pluginName = fmt::format(L"dune_plugin_{:s}.zip", m_plugin->compile_name_template(m_pluginNameTemplate.GetString(), selected)).c_str();

	UpdateData(FALSE);
}

void CAccessInfoPage::OnBnClickedCheckCustomPluginLogo()
{
	auto& selected = GetCheckedAccount();
	selected.custom_logo = m_wndCustomLogo.GetCheck();

	m_wndLogo.EnableWindow(selected.custom_logo);
}

void CAccessInfoPage::OnEnChangeMfceditbrowsePluginLogo()
{
	UpdateData(TRUE);
	auto& selected = GetCheckedAccount();
	m_logo.Trim();
	if (utils::is_ascii(std::filesystem::path(m_logo.GetString()).filename().wstring().c_str()))
	{
		selected.set_logo(m_logo);
	}
	else
	{
		AfxMessageBox(IDS_STRING_WRN_NON_ASCII, MB_ICONERROR | MB_OK);
		m_logo.Empty();
	}
	UpdateData(FALSE);
}

void CAccessInfoPage::OnBnClickedCheckCustomPluginBackground()
{
	auto& selected = GetCheckedAccount();
	selected.custom_background = m_wndCustomBackground.GetCheck();

	m_wndBackground.EnableWindow(selected.custom_background);
}

void CAccessInfoPage::OnEnChangeMfceditbrowsePluginBackground()
{
	UpdateData(TRUE);
	auto& selected = GetCheckedAccount();
	if (utils::is_ascii(std::filesystem::path(m_background.GetString()).filename().wstring().c_str()))
	{
		m_background.Trim();
		selected.set_background(m_background);
	}
	else
	{
		AfxMessageBox(IDS_STRING_WRN_NON_ASCII, MB_ICONERROR | MB_OK);
		m_background.Empty();
	}
	UpdateData(FALSE);
}

void CAccessInfoPage::OnBnClickedCheckAutoincrementVersion()
{
	auto& selected = GetCheckedAccount();
	selected.custom_increment = m_wndAutoIncrement.GetCheck();

	m_wndVersionID.EnableWindow(selected.custom_increment);
}

void CAccessInfoPage::OnEnChangeEditPluginUpdateVersion()
{
	UpdateData(TRUE);
	auto& selected = GetCheckedAccount();

	selected.set_version_id(m_versionIdx);
}

void CAccessInfoPage::OnBnClickedCheckCustomUpdateNameTemplate()
{
	auto& selected = GetCheckedAccount();
	selected.custom_update_name = m_wndCustomUpdateName.GetCheck();

	m_wndUpdateNameTemplate.EnableWindow(selected.custom_update_name);

	OnEnChangeEditPluginUpdateNameTemplate();
}

void CAccessInfoPage::OnEnChangeEditPluginUpdateNameTemplate()
{
	UpdateData(TRUE);

	auto& selected = GetCheckedAccount();
	if (utils::is_ascii(m_updateNameTemplate.GetString()))
	{
		m_updateNameTemplate.Trim();
		selected.set_update_name(m_updateNameTemplate);
	}
	else
	{
		AfxMessageBox(IDS_STRING_WRN_NON_ASCII, MB_ICONERROR | MB_OK);
		m_updateNameTemplate = selected.get_update_name().c_str();
	}

	m_updateName = m_plugin->compile_name_template(m_updateNameTemplate.GetString(), selected).c_str();

	UpdateData(FALSE);
}

void CAccessInfoPage::OnEnChangeEditPluginChannelsWebPath()
{
	UpdateData(TRUE);

	auto& selected = GetCheckedAccount();
	selected.set_ch_web_path(m_channelsWebPath);
}

void CAccessInfoPage::OnBnClickedButtonEditConfig()
{
	auto& selected = GetCheckedAccount();

	auto pSheet = std::make_unique<CPluginConfigPropertySheet>(m_configs, load_string_resource(IDS_STRING_PLUGIN_CONFIG).c_str(), REG_PLUGIN_CFG_WINDOW_POS);
	pSheet->m_psh.dwFlags |= PSH_NOAPPLYNOW;
	pSheet->m_psh.dwFlags &= ~PSH_HASHELP;
	pSheet->m_plugin = StreamContainer::get_instance(m_plugin->get_plugin_type());
	if (m_wndConfigs.GetCurSel() != m_initial_config)
		pSheet->m_plugin->load_plugin_parameters(selected.get_config());
	else
		pSheet->m_plugin->copy(m_plugin.get());
	pSheet->m_CurrentStream = m_CurrentStream;
	pSheet->m_selected_cred = selected;

	CPluginConfigPage dlgCfg;
	dlgCfg.m_psp.dwFlags &= ~PSP_HASHELP;

	CPluginConfigPageTV dlgCfgTV;
	dlgCfgTV.m_psp.dwFlags &= ~PSP_HASHELP;

	CPluginConfigPageEPG dlgCfgEPG;
	dlgCfgEPG.m_psp.dwFlags &= ~PSP_HASHELP;

	CPluginConfigPageVOD dlgCfgVOD;
	dlgCfgVOD.m_psp.dwFlags &= ~PSP_HASHELP;

	pSheet->AddPage(&dlgCfg);
	pSheet->AddPage(&dlgCfgTV);
	pSheet->AddPage(&dlgCfgEPG);
	pSheet->AddPage(&dlgCfgVOD);

	if (pSheet->DoModal() == IDOK)
	{
		m_plugin->copy(pSheet->m_plugin.get());
		CreateAccountsList();
		m_wndAccounts.SetCheck(GetConfig().get_int(false, REG_ACTIVE_ACCOUNT), TRUE);
	}
}

void CAccessInfoPage::OnBnClickedButtonBrowseDirectLink()
{
	auto& selectedAccount = GetCheckedAccount();
	if (selectedAccount.not_valid)
		return;

	int selectedList = GetSelectedList();
	if (selectedList == -1)
		return;

	CUrlDlg dlg;
	m_wndDirectLink.GetWindowText(dlg.m_url);
	if (dlg.DoModal() != IDOK)
		return;

	const auto& ch_list = get_utf8(m_all_channels_lists[selectedList]);
	if (dlg.m_url.IsEmpty())
	{
		selectedAccount.m_direct_links.erase(ch_list);
	}
	else
	{
		if (dlg.m_url.Find(L"www.dropbox.com") != -1)
		{
			dlg.m_url.Replace(L"www.dropbox.com", L"dl.dropboxusercontent.com");
			int pos = dlg.m_url.Find('?');
			if (pos != -1)
			{
				int new_pos = dlg.m_url.Find(L"?rlkey");
				if (new_pos == -1)
				{
					dlg.m_url = dlg.m_url.Mid(0, pos);
				}
				else
				{
					dlg.m_url.Replace(L"&dl=0", L"");
				}
			}
			selectedAccount.m_direct_links[ch_list] = get_utf8(dlg.m_url);
		}
		else
		{
			selectedAccount.m_direct_links[ch_list] = get_utf8(dlg.m_url);
		}
	}

	m_wndDirectLink.SetWindowText(dlg.m_url);
}

void CAccessInfoPage::OnBnClickedButtonBrowseUpdateDesc()
{
	auto& selected = GetCheckedAccount();
	if (selected.not_valid)
		return;

	CUrlDlg dlg;
	m_wndUpdateUrl.GetWindowText(dlg.m_url);
	if (dlg.DoModal() != IDOK)
		return;

	std::wstring url(dlg.m_url.GetString());
	CString updateName = m_updateNameTemplate + L".xml";
	if (!TransformDropboxPath(url, m_plugin->compile_name_template(updateName.GetString(), selected)))
		return;

	m_wndUpdateUrl.SetWindowText(url.c_str());
	selected.set_update_url(url);
}

void CAccessInfoPage::OnBnClickedButtonBrowseUpdateFile()
{
	auto& selected = GetCheckedAccount();
	if (selected.not_valid)
		return;

	CUrlDlg dlg;
	m_wndUpdatePackageUrl.GetWindowText(dlg.m_url);
	if (dlg.DoModal() != IDOK)
		return;

	std::wstring url(dlg.m_url.GetString());
	CString updateName = m_updateNameTemplate + L".tar.gz";
	if (!TransformDropboxPath(url, m_plugin->compile_name_template(updateName.GetString(), selected)))
		return;

	m_wndUpdatePackageUrl.SetWindowText(url.c_str());
	selected.set_update_package_url(url);
}

bool CAccessInfoPage::TransformDropboxPath(std::wstring& dropbox_link, const std::wstring& file)
{
	utils::CrackedUrl cracked;
	if (!dropbox_link.empty() && !cracked.CrackUrl(dropbox_link))
	{
		AfxMessageBox(IDS_STRING_ERR_WRONG_URL, MB_ICONERROR | MB_OK);
		return false;
	}

	if (cracked.host == L"www.dropbox.com" || cracked.host == L"dl.dropboxusercontent.com")
	{
		cracked.host = L"dl.dropboxusercontent.com";
		std::filesystem::path file_path(cracked.path);
		if (file_path.filename() != file)
		{
			AfxMessageBox(IDS_STRING_ERR_WRONG_UPLOADED_NAME, MB_ICONERROR | MB_OK);
			return false;
		}

		if (cracked.extra_info.find(L"?rlkey") != std::wstring::npos)
		{
			utils::string_replace_inplace<wchar_t>(cracked.extra_info, L"&dl=0", L"");
			dropbox_link = fmt::format(L"{:s}://{:s}{:s}{:s}", cracked.scheme, cracked.host, file_path.wstring(), cracked.extra_info);
		}
		else
		{
			dropbox_link = fmt::format(L"{:s}://{:s}{:s}/", cracked.scheme, cracked.host, file_path.parent_path().wstring());
		}
	}

	return true;
}

void CAccessInfoPage::UpdateTemplatedFields(const Credentials& selected)
{
	m_pluginNameTemplate = (selected.custom_plugin_name ? selected.get_plugin_name().c_str() : utils::DUNE_PLUGIN_FILE_NAME);
	if (m_pluginNameTemplate.IsEmpty())
		m_pluginNameTemplate = utils::DUNE_UPDATE_INFO_NAME;

	m_pluginName = fmt::format(L"dune_plugin_{:s}.zip", m_plugin->compile_name_template(m_pluginNameTemplate.GetString(), selected)).c_str();

	m_updateNameTemplate = (selected.custom_update_name ? selected.get_update_name().c_str() : utils::DUNE_UPDATE_INFO_NAME);
	if (m_updateNameTemplate.IsEmpty())
		m_updateNameTemplate = utils::DUNE_UPDATE_INFO_NAME;

	m_updateName = m_plugin->compile_name_template(m_updateNameTemplate.GetString(), selected).c_str();

	UpdateData(FALSE);
}

void CAccessInfoPage::OnBnClickedCheckUseDropbox()
{
	auto& selected = GetCheckedAccount();
	if (!selected.not_valid)
	{
		selected.use_dropbox = m_wndUseDropboxUpdate.GetCheck() != 0;
	}
}
