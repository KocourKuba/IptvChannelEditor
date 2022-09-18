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
#include "AccessInfoDlg.h"
#include "IPTVChannelEditor.h"
#include "PlayListEntry.h"
#include "UrlDlg.h"

#include "UtilsLib\inet_utils.h"

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

IMPLEMENT_DYNAMIC(CAccessInfoDlg, CMFCPropertyPage)

BEGIN_MESSAGE_MAP(CAccessInfoDlg, CMFCPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CAccessInfoDlg::OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, &CAccessInfoDlg::OnBnClickedButtonRemove)
	ON_BN_CLICKED(IDC_BUTTON_NEW_FROM_URL, &CAccessInfoDlg::OnBnClickedButtonNewFromUrl)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_ACCOUNTS, &CAccessInfoDlg::OnNMDblClickList)
	ON_MESSAGE(WM_NOTIFY_END_EDIT, &CAccessInfoDlg::OnNotifyDescriptionEdited)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_ACCOUNTS, &CAccessInfoDlg::OnLvnItemchangedListAccounts)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_CHANNELS, &CAccessInfoDlg::OnLvnItemchangedListChannels)
	ON_CBN_SELCHANGE(IDC_COMBO_DEVICE_ID, &CAccessInfoDlg::OnCbnSelchangeComboDeviceId)
	ON_CBN_SELCHANGE(IDC_COMBO_PROFILE, &CAccessInfoDlg::OnCbnSelchangeComboProfile)
	ON_CBN_SELCHANGE(IDC_COMBO_QUALITY, &CAccessInfoDlg::OnCbnSelchangeComboQuality)
	ON_BN_CLICKED(IDC_CHECK_EMBED, &CAccessInfoDlg::OnBnClickedCheckEmbed)
	ON_EN_CHANGE(IDC_EDIT_PLUGIN_CAPTION, &CAccessInfoDlg::OnEnChangeEditPluginCaption)
	ON_EN_CHANGE(IDC_EDIT_PLUGIN_SUFFIX, &CAccessInfoDlg::OnEnChangeEditPluginSuffix)
	ON_EN_CHANGE(IDC_MFCEDITBROWSE_PLUGIN_LOGO, &CAccessInfoDlg::OnEnChangeMfceditbrowsePluginLogo)
	ON_EN_CHANGE(IDC_MFCEDITBROWSE_PLUGIN_BGND, &CAccessInfoDlg::OnEnChangeMfceditbrowsePluginBgnd)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, &CAccessInfoDlg::OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, &CAccessInfoDlg::OnToolTipText)
	ON_EN_CHANGE(IDC_EDIT_PLUGIN_UPDATE_VERSION, &CAccessInfoDlg::OnEnChangeEditPluginUpdateVersion)
	ON_BN_CLICKED(IDC_CHECK_AUTOINCREMENT_VERSION, &CAccessInfoDlg::OnBnClickedCheckAutoincrementVersion)
	ON_EN_CHANGE(IDC_EDIT_PLUGIN_UPDATE_URL, &CAccessInfoDlg::OnEnChangeEditPluginUpdateUrl)
	ON_EN_CHANGE(IDC_EDIT_PLUGIN_UPDATE_FILE_URL, &CAccessInfoDlg::OnEnChangeEditPluginUpdateFileUrl)
	ON_BN_CLICKED(IDC_CHECK_CUSTOM_UPDATE_NAME, &CAccessInfoDlg::OnBnClickedCheckCustomUpdateName)
	ON_BN_CLICKED(IDC_CHECK_CUSTOM_PACKAGE_NAME, &CAccessInfoDlg::OnBnClickedCheckCustomPackageName)
	ON_EN_CHANGE(IDC_EDIT_PLUGIN_UPDATE_NAME, &CAccessInfoDlg::OnEnChangeEditPluginUpdateName)
	ON_EN_CHANGE(IDC_EDIT_PLUGIN_PACKAGE_NAME, &CAccessInfoDlg::OnEnChangeEditPluginPackageName)
	ON_EN_CHANGE(IDC_EDIT_PLUGIN_CHANNELS_WEB_PATH, &CAccessInfoDlg::OnEnChangeEditPluginChannelsWebPath)
END_MESSAGE_MAP()


CAccessInfoDlg::CAccessInfoDlg() : CMFCPropertyPage(IDD_DIALOG_ACCESS_INFO)
, m_channelsWebPath(_T(""))
{
}

void CAccessInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_MFCLINK_PROVIDER, m_wndProviderLink);
	DDX_Control(pDX, IDC_BUTTON_REMOVE, m_wndRemove);
	DDX_Control(pDX, IDC_BUTTON_NEW_FROM_URL, m_wndNewFromUrl);
	DDX_Control(pDX, IDC_LIST_ACCOUNTS, m_wndAccounts);
	DDX_Control(pDX, IDC_LIST_INFO, m_wndInfo);
	DDX_Control(pDX, IDC_LIST_CHANNELS, m_wndChLists);
	DDX_Control(pDX, IDC_COMBO_DEVICE_ID, m_wndDeviceID);
	DDX_Control(pDX, IDC_COMBO_PROFILE, m_wndProfile);
	DDX_Control(pDX, IDC_COMBO_QUALITY, m_wndQuality);
	DDX_Control(pDX, IDC_CHECK_EMBED, m_wndEmbed);
	DDX_Control(pDX, IDC_EDIT_PLUGIN_CAPTION, m_wndCaption);
	DDX_Text(pDX, IDC_EDIT_PLUGIN_CAPTION, m_caption);
	DDX_Control(pDX, IDC_EDIT_PLUGIN_SUFFIX, m_wndSuffix);
	DDX_Text(pDX, IDC_EDIT_PLUGIN_SUFFIX, m_suffix);
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
}

BOOL CAccessInfoDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_LBUTTONDOWN
		|| pMsg->message == WM_LBUTTONUP
		|| pMsg->message == WM_MOUSEMOVE)
	{
		m_wndToolTipCtrl.RelayEvent(pMsg);
	}

	return __super::PreTranslateMessage(pMsg);
}

BOOL CAccessInfoDlg::OnInitDialog()
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
		{ IDC_COMBO_DEVICE_ID, load_string_resource(IDS_STRING_COMBO_DEVICE_ID) },
		{ IDC_COMBO_PROFILE, load_string_resource(IDS_STRING_COMBO_PROFILE) },
		{ IDC_COMBO_QUALITY, load_string_resource(IDS_STRING_QUALITY_ID) },
		{ IDC_CHECK_EMBED, load_string_resource(IDS_STRING_CHECK_EMBED) },
		{ IDC_EDIT_PLUGIN_CAPTION, load_string_resource(IDS_STRING_EDIT_CAPTION) },
		{ IDC_MFCEDITBROWSE_PLUGIN_LOGO, load_string_resource(IDS_STRING_EDIT_ICON) },
		{ IDC_MFCEDITBROWSE_PLUGIN_BGND, load_string_resource(IDS_STRING_EDIT_BACKGROUND) },
		{ IDC_EDIT_PLUGIN_SUFFIX, load_string_resource(IDS_STRING_EDIT_SUFFIX) },
		{ IDC_EDIT_PLUGIN_CHANNELS_WEB_PATH, load_string_resource(IDS_STRING_EDIT_PLUGIN_CHANNELS_WEB_PATH) },
		{ IDC_EDIT_PLUGIN_UPDATE_URL, load_string_resource(IDS_STRING_EDIT_PLUGIN_UPDATE_URL) },
		{ IDC_EDIT_PLUGIN_UPDATE_FILE_URL, load_string_resource(IDS_STRING_EDIT_PLUGIN_UPDATE_FILE_URL) },
		{ IDC_CHECK_AUTOINCREMENT_VERSION, load_string_resource(IDS_STRING_CHECK_AUTOINCREMENT_VERSION) },
		{ IDC_EDIT_PLUGIN_UPDATE_VERSION, load_string_resource(IDS_STRING_EDIT_PLUGIN_UPDATE_VERSION) },
		{ IDC_CHECK_CUSTOM_UPDATE_NAME, load_string_resource(IDS_STRING_CHECK_CUSTOM_UPDATE_NAME) },
		{ IDC_EDIT_PLUGIN_UPDATE_NAME, load_string_resource(IDS_STRING_EDIT_PLUGIN_UPDATE_NAME) },
		{ IDC_CHECK_CUSTOM_PACKAGE_NAME, load_string_resource(IDS_STRING_CHECK_CUSTOM_PACKAGE_NAME) },
		{ IDC_EDIT_PLUGIN_PACKAGE_NAME, load_string_resource(IDS_STRING_EDIT_PLUGIN_PACKAGE_NAME) },
	};

	m_plugin_type = GetConfig().get_plugin_type();
	m_plugin = StreamContainer::get_instance(m_plugin_type);

	m_wndToolTipCtrl.SetDelayTime(TTDT_AUTOPOP, 10000);
	m_wndToolTipCtrl.SetDelayTime(TTDT_INITIAL, 500);
	m_wndToolTipCtrl.SetMaxTipWidth(300);

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

	m_wndAccounts.SetCheck(GetConfig().get_int(false, REG_ACTIVE_ACCOUNT), TRUE);
	m_wndRemove.EnableWindow(m_wndAccounts.GetSelectionMark() != -1);

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CAccessInfoDlg::OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult)
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

void CAccessInfoDlg::UpdateOptionalControls()
{
	auto& selected = GetCheckedAccount();
	if (selected.not_valid)
		return;

	TemplateParams params;
	params.login = utils::utf8_to_utf16(selected.login);
	params.password = utils::utf8_to_utf16(selected.password);
	params.subdomain = utils::utf8_to_utf16(selected.domain);
	params.server = selected.device_id;
	params.profile = selected.profile_id;
	params.quality = selected.quality_id;

	UINT static_text = IDS_STRING_SERVER_ID;
	switch (m_plugin_type)
	{
		case StreamType::enCbilling:
			static_text = IDS_STRING_DEVICE_ID;
			break;
		case StreamType::enSharaclub:
			m_list_domain = GetConfig().get_string(false, REG_LIST_DOMAIN);
			m_epg_domain = GetConfig().get_string(false, REG_EPG_DOMAIN);
			params.subdomain = m_list_domain;
			break;
		default:
			break;
	}

	GetDlgItem(IDC_STATIC_DEVICE_ID)->SetWindowText(load_string_resource(static_text).c_str());

	m_servers = m_plugin->get_servers_list(params);
	m_wndDeviceID.ResetContent();

	for (const auto& info : m_servers)
	{
		m_wndDeviceID.AddString(info.name.c_str());
	}

	m_wndDeviceID.EnableWindow(!m_servers.empty());
	if (!m_servers.empty())
	{
		if (params.server >= (int)m_servers.size())
		{
			params.server = 0;
		}

		m_wndDeviceID.SetCurSel(params.server);
	}

	m_qualities = m_plugin->get_quality_list(params);

	m_wndQuality.EnableWindow(!m_qualities.empty());

	m_wndQuality.ResetContent();

	for (const auto& info : m_qualities)
	{
		m_wndQuality.AddString(info.name.c_str());
	}

	if (!m_qualities.empty())
	{
		if (params.quality >= (int)m_qualities.size())
		{
			params.quality = 0;
		}

		m_wndQuality.SetCurSel(params.quality);
	}

	m_caption = selected.get_caption().c_str();
	m_suffix = selected.get_suffix().c_str();
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

void CAccessInfoDlg::CreateAccountsList()
{
	m_wndAccounts.SetExtendedStyle(m_wndAccounts.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
	CHeaderCtrl* header = m_wndAccounts.GetHeaderCtrl();
	header->ModifyStyle(0, HDS_CHECKBOXES);

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
			m_wndNewFromUrl.ShowWindow(SW_SHOW);
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
				m_wndAccounts.SetItemText(idx, ++sub_idx, utils::utf8_to_utf16(cred.password).c_str());
				break;

			case AccountAccessType::enLoginPass:
				m_wndAccounts.SetItemText(idx, ++sub_idx, utils::utf8_to_utf16(cred.login).c_str());
				m_wndAccounts.SetItemText(idx, ++sub_idx, utils::utf8_to_utf16(cred.password).c_str());
				break;

			case AccountAccessType::enOtt:
				m_wndAccounts.SetItemText(idx, ++sub_idx, utils::utf8_to_utf16(cred.token).c_str());
				m_wndAccounts.SetItemText(idx, ++sub_idx, utils::utf8_to_utf16(cred.domain).c_str());
				m_wndAccounts.SetItemText(idx, ++sub_idx, utils::utf8_to_utf16(cred.portal).c_str());
				break;

			default:break;
		}

		m_wndAccounts.SetItemText(idx, ++sub_idx, utils::utf8_to_utf16(cred.comment).c_str());
		m_wndAccounts.SetItemText(idx, ++sub_idx, utils::utf8_to_utf16(cred.suffix).c_str());

		++idx;
	}
}

void CAccessInfoDlg::CreateAccountInfo()
{
	m_wndInfo.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_GRIDLINES);

	CRect rect;
	m_wndInfo.GetClientRect(&rect);
	int vWidth = rect.Width() - GetSystemMetrics(SM_CXVSCROLL) - 1;
	int nWidth = vWidth / 4;

	m_wndInfo.InsertColumn(0, load_string_resource(IDS_STRING_COL_INFO).c_str(), LVCFMT_LEFT, nWidth);
	m_wndInfo.InsertColumn(1, load_string_resource(IDS_STRING_COL_DATA).c_str(), LVCFMT_LEFT, vWidth - nWidth);
}

void CAccessInfoDlg::CreateChannelsList()
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

void CAccessInfoDlg::FillChannelsList()
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

void CAccessInfoDlg::SetWebUpdate()
{
	UpdateData(TRUE);

	auto& selected = GetCheckedAccount();
	if (selected.not_valid)
		return;

	m_wndAutoIncrement.SetCheck(selected.custom_increment);

	m_wndVersionID.EnableWindow(selected.custom_increment);
	m_wndUpdateName.EnableWindow(selected.custom_update_name);
	m_wndPackageName.EnableWindow(selected.custom_package_name);

	const auto& plugin_info = GetConfig().get_plugin_info();
	const auto& short_name_w = utils::utf8_to_utf16(plugin_info.short_name);
	const auto& suffix = utils::utf8_to_utf16(selected.suffix);

	if (selected.custom_update_name)
	{
		m_updateInfoName = selected.update_name.c_str();
	}
	else
	{
		m_updateInfoName = fmt::format(utils::DUNE_UPDATE_NAME, plugin_info.short_name, (selected.suffix.empty()) ? "mod" : selected.suffix).c_str();
		m_updateInfoName += L".txt";
	}

	if (selected.custom_package_name)
	{
		m_packageName = selected.package_name.c_str();
	}
	else
	{
		m_packageName = fmt::format(utils::DUNE_UPDATE_NAME, plugin_info.short_name, (selected.suffix.empty()) ? "mod" : selected.suffix).c_str();
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

BOOL CAccessInfoDlg::OnApply()
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
	params.subdomain = utils::utf8_to_utf16(selected.domain);
	params.server = selected.device_id;
	params.profile = selected.profile_id;
	params.quality = selected.quality_id;

	if (m_plugin_type == StreamType::enSharaclub)
	{
		params.subdomain = m_list_domain;
	}

	if (m_wndDeviceID.GetCount())
	{
		m_plugin->set_server(params);
	}

	if (m_wndProfile.GetCount())
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

void CAccessInfoDlg::OnBnClickedButtonAdd()
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

void CAccessInfoDlg::OnBnClickedButtonRemove()
{
	int idx = m_wndAccounts.GetSelectionMark();
	if (idx != -1)
	{
		m_wndAccounts.DeleteItem(idx);
		m_all_credentials.erase(m_all_credentials.begin() + idx);
	}
}

void CAccessInfoDlg::OnBnClickedButtonNewFromUrl()
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

		if (m_plugin_type == StreamType::enKineskop)
		{
			// http://knkp.in/2119490/6cd0ff4249f49957/de/1
			static std::wregex re_url(LR"(^https?:\/\/[^\/]+\/([^\/]+)\/([^\/]+)\/?.*$)");
			std::wsmatch m;
			if (std::regex_match(url, m, re_url))
			{
				Credentials cred;
				cred.login = get_utf8(m[1].str());
				cred.password = get_utf8(m[2].str());
				m_all_credentials.emplace_back(cred);

				int cnt = m_wndAccounts.GetItemCount();
				m_wndAccounts.InsertItem(cnt, L"", 0);
				m_wndAccounts.SetItemText(cnt, 1, m[1].str().c_str());
				m_wndAccounts.SetItemText(cnt, 2, m[2].str().c_str());
				return;
			}
		}

		Credentials cred;
		auto entry = std::make_unique<PlaylistEntry>(GetConfig().get_plugin_type(), GetAppPath(utils::PLUGIN_ROOT));
		std::wstring line;
		while (std::getline(stream, line))
		{
			utils::string_rtrim(line, L"\r");
			m3u_entry m3uEntry(line);
			if (!entry->Parse(line, m3uEntry)) continue;

			const auto& access_key = entry->get_uri_stream()->get_parser().token;
			const auto& domain = entry->get_uri_stream()->get_parser().domain;
			if (!access_key.empty() && !domain.empty() && access_key != L"00000000000000" && domain != L"localhost")
			{
				int cnt = m_wndAccounts.GetItemCount();

				m_wndAccounts.InsertItem(cnt, L"", 0);
				m_wndAccounts.SetItemText(cnt, 1, access_key.c_str());
				m_wndAccounts.SetItemText(cnt, 2, domain.c_str());

				Credentials cred;
				cred.token = get_utf8(access_key);
				cred.domain = get_utf8(domain);
				m_all_credentials.emplace_back(cred);
				break;
			}
		}
	}
}

void CAccessInfoDlg::OnNMDblClickList(NMHDR* pNMHDR, LRESULT* pResult)
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

// OnNotifyDescriptionEdited()
LRESULT CAccessInfoDlg::OnNotifyDescriptionEdited(WPARAM wParam, LPARAM lParam)
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
					cred.domain = get_utf8(dispinfo->item.pszText);
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

	GetAccountInfo();

	return 0;
}

void CAccessInfoDlg::OnLvnItemchangedListAccounts(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	if ((pNMLV->uChanged & LVIF_STATE))
	{
		m_wndRemove.EnableWindow(pNMLV->uNewState & LVIS_SELECTED);

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
			m_wndDeviceID.EnableWindow(FALSE);
			m_wndProfile.EnableWindow(FALSE);
			m_wndEmbed.EnableWindow(FALSE);
			m_wndCaption.EnableWindow(FALSE);
			m_wndLogo.EnableWindow(FALSE);
			m_wndBackground.EnableWindow(FALSE);
			m_wndSuffix.EnableWindow(FALSE);
			GetParent()->GetDlgItem(IDOK)->EnableWindow(FALSE);
		}

		UpdateOptionalControls();
		SetWebUpdate();
	}

	*pResult = 0;
}

void CAccessInfoDlg::OnLvnItemchangedListChannels(NMHDR* pNMHDR, LRESULT* pResult)
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

void CAccessInfoDlg::GetAccountInfo()
{
	UpdateData(TRUE);

	m_status.LoadString(IDS_STRING_STATUS_TEXT);
	m_wndProfile.ResetContent();
	m_wndProfile.EnableWindow(FALSE);

	auto& selected = GetCheckedAccount();
	if (selected.not_valid)
		return;

	if (!m_servers.empty())
	{
		m_wndDeviceID.EnableWindow(TRUE);
		m_wndDeviceID.SetCurSel(selected.device_id);
	}

	m_wndEmbed.SetCheck(selected.embed);
	m_wndEmbed.EnableWindow(TRUE);
	m_wndCaption.EnableWindow(TRUE);
	m_wndLogo.EnableWindow(TRUE);
	m_wndBackground.EnableWindow(TRUE);
	m_wndSuffix.EnableWindow(TRUE);

	std::wstring login;
	std::wstring password;
	std::wstring token;
	std::wstring domain;
	std::wstring portal;

	switch (m_plugin->get_access_type())
	{
		case AccountAccessType::enPin:
			password = utils::utf8_to_utf16(selected.password);
			break;

		case AccountAccessType::enLoginPass:
			login = utils::utf8_to_utf16(selected.login);
			password = utils::utf8_to_utf16(selected.password);
			break;

		case AccountAccessType::enOtt:
			token = utils::utf8_to_utf16(selected.token);
			domain = utils::utf8_to_utf16(selected.domain);
			portal = utils::utf8_to_utf16(selected.portal);
			break;

		default: break;
	}

	// reset templated flag for new parse
	auto entry = std::make_shared<PlaylistEntry>(GetConfig().get_plugin_type(), GetAppPath(utils::PLUGIN_ROOT));
	auto& uri = entry->stream_uri;
	uri->set_template(false);
	uri->get_parser().login = login;
	uri->get_parser().password = password;
	uri->get_parser().token = token;
	uri->get_parser().subdomain = domain;
	uri->get_parser().host = m_host;

	TemplateParams params;
	params.login = std::move(login);
	params.password = std::move(password);
	params.subdomain = m_list_domain;
	params.server = selected.device_id;
	params.profile = selected.profile_id;
	params.quality = selected.quality_id;

	if (m_plugin_type == StreamType::enTVClub || m_plugin_type == StreamType::enVidok)
	{
		params.token = m_plugin->get_api_token(params.login, params.password);
	}

	m_plugin->clear_profiles_list();
	m_profiles = m_plugin->get_profiles_list(params);
	for (const auto& info : m_profiles)
	{
		m_wndProfile.AddString(info.name.c_str());
	}

	if (!m_profiles.empty())
	{
		if (params.profile >= (int)m_profiles.size())
		{
			params.profile = 0;
			selected.profile_id = 0;
		}

		m_wndProfile.SetCurSel(params.profile);
	}

	m_wndProfile.EnableWindow(m_profiles.size() > 1);

	std::wstring pl_url;
	uri->get_playlist_url(pl_url, params);

	std::list<AccountInfo> acc_info;
	if (uri->parse_access_info(params, acc_info))
	{
		for (auto it = acc_info.begin(); it != acc_info.end(); )
		{
			// currently supported only in sharaclub, oneott use this to obtain token
			if (it->name == (L"token"))
			{
				selected.token = get_utf8(it->value);
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
				if (entry->Parse(line, m3uEntry) && !uri->get_parser().token.empty())
				{
					// do not override fake ott and domain for edem
					if (m_plugin->get_access_type() != AccountAccessType::enOtt)
					{
						selected.token = get_utf8(uri->get_parser().token);
						selected.domain = get_utf8(uri->get_parser().domain);
					}
					m_host = uri->get_parser().host;
					m_status = _T("Ok");
					break;
				}
			}
		}
	}

	m_wndInfo.DeleteAllItems();

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

Credentials& CAccessInfoDlg::GetCheckedAccount()
{
	static Credentials empty;
	empty.not_valid = true;

	int selected = GetCheckedAccountIdx();
	return selected != -1 ? m_all_credentials[selected] : empty;
}

int CAccessInfoDlg::GetCheckedAccountIdx()
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

void CAccessInfoDlg::OnCbnSelchangeComboDeviceId()
{
	auto& selected = GetCheckedAccount();
	selected.device_id = m_wndDeviceID.GetCurSel();
}

void CAccessInfoDlg::OnCbnSelchangeComboProfile()
{
	auto& selected = GetCheckedAccount();
	selected.profile_id = m_wndProfile.GetCurSel();
}

void CAccessInfoDlg::OnCbnSelchangeComboQuality()
{
	auto& selected = GetCheckedAccount();
	selected.quality_id = m_wndQuality.GetCurSel();
}

void CAccessInfoDlg::OnBnClickedCheckEmbed()
{
	auto& selected = GetCheckedAccount();
	selected.embed = m_wndEmbed.GetCheck();
}

void CAccessInfoDlg::OnEnChangeEditPluginCaption()
{
	UpdateData(TRUE);
	m_caption.Trim();
	auto& selected = GetCheckedAccount();
	selected.caption = get_utf8(m_caption);
}

void CAccessInfoDlg::OnEnChangeEditPluginSuffix()
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

void CAccessInfoDlg::OnEnChangeMfceditbrowsePluginLogo()
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

void CAccessInfoDlg::OnEnChangeMfceditbrowsePluginBgnd()
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

void CAccessInfoDlg::OnEnChangeEditPluginUpdateVersion()
{
	UpdateData(TRUE);
	auto& selected = GetCheckedAccount();
	selected.version_id = get_utf8(m_versionIdx);
}

void CAccessInfoDlg::OnBnClickedCheckAutoincrementVersion()
{
	m_wndVersionID.EnableWindow(m_wndAutoIncrement.GetCheck());
}

void CAccessInfoDlg::OnEnChangeEditPluginUpdateUrl()
{
	UpdateData(TRUE);
	auto& selected = GetCheckedAccount();
	selected.update_url = get_utf8(m_updateInfoUrl);
}

void CAccessInfoDlg::OnEnChangeEditPluginUpdateFileUrl()
{
	UpdateData(TRUE);
	auto& selected = GetCheckedAccount();
	selected.update_package_url = get_utf8(m_updatePackageUrl);
}

void CAccessInfoDlg::OnBnClickedCheckCustomUpdateName()
{
	m_wndUpdateName.EnableWindow(m_wndCustomUpdateName.GetCheck());
}

void CAccessInfoDlg::OnBnClickedCheckCustomPackageName()
{
	m_wndPackageName.EnableWindow(m_wndCustomPackageName.GetCheck());
}

void CAccessInfoDlg::OnEnChangeEditPluginUpdateName()
{
	UpdateData(TRUE);
	if (!utils::is_ascii(m_updateInfoName.GetString()))
	{
		AfxMessageBox(IDS_STRING_WRN_NON_ASCII, MB_ICONERROR | MB_OK);
	}
}

void CAccessInfoDlg::OnEnChangeEditPluginPackageName()
{
	UpdateData(TRUE);
	if (!utils::is_ascii(m_packageName.GetString()))
	{
		AfxMessageBox(IDS_STRING_WRN_NON_ASCII, MB_ICONERROR | MB_OK);
	}
}

void CAccessInfoDlg::OnEnChangeEditPluginChannelsWebPath()
{
	UpdateData(TRUE);

	auto& selected = GetCheckedAccount();
	selected.ch_web_path = get_utf8(m_channelsWebPath);
}
