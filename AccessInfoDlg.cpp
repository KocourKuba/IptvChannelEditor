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
	ON_BN_CLICKED(IDC_CHECK_EMBED, &CAccessInfoDlg::OnBnClickedCheckEmbed)
	ON_EN_CHANGE(IDC_EDIT_PLUGIN_CAPTION, &CAccessInfoDlg::OnEnChangeEditPluginCaption)
	ON_EN_CHANGE(IDC_EDIT_PLUGIN_ICON, &CAccessInfoDlg::OnEnChangeEditPluginIcon)
	ON_EN_CHANGE(IDC_EDIT_PLUGIN_BACKGROUND, &CAccessInfoDlg::OnEnChangeEditPluginBackground)
	ON_EN_CHANGE(IDC_EDIT_PLUGIN_SUFFIX, &CAccessInfoDlg::OnEnChangeEditPluginSuffix)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, &CAccessInfoDlg::OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, &CAccessInfoDlg::OnToolTipText)
END_MESSAGE_MAP()


CAccessInfoDlg::CAccessInfoDlg() : CMFCPropertyPage(IDD_DIALOG_ACCESS_INFO)
{
}

void CAccessInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_MFCLINK_PROVIDER, m_wndProviderLink);
	DDX_Control(pDX, IDC_BUTTON_REMOVE, m_wndRemove);
	DDX_Control(pDX, IDC_BUTTON_NEW_FROM_URL, m_wndNewFromUrl);
	DDX_Control(pDX, IDC_COMBO_DEVICE_ID, m_wndDeviceID);
	DDX_Control(pDX, IDC_LIST_ACCOUNTS, m_wndAccounts);
	DDX_Control(pDX, IDC_LIST_INFO, m_wndInfo);
	DDX_Control(pDX, IDC_LIST_CHANNELS, m_wndChLists);
	DDX_Control(pDX, IDC_COMBO_PROFILE, m_wndProfile);
	DDX_Control(pDX, IDC_CHECK_EMBED, m_wndEmbed);
	DDX_Control(pDX, IDC_EDIT_PLUGIN_CAPTION, m_wndCaption);
	DDX_Text(pDX, IDC_EDIT_PLUGIN_CAPTION, m_caption);
	DDX_Control(pDX, IDC_EDIT_PLUGIN_ICON, m_wndLogo);
	DDX_Text(pDX, IDC_EDIT_PLUGIN_ICON, m_logo);
	DDX_Control(pDX, IDC_EDIT_PLUGIN_BACKGROUND, m_wndBackground);
	DDX_Text(pDX, IDC_EDIT_PLUGIN_BACKGROUND, m_background);
	DDX_Control(pDX, IDC_EDIT_PLUGIN_SUFFIX, m_wndSuffix);
	DDX_Text(pDX, IDC_EDIT_PLUGIN_SUFFIX, m_suffix);
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
		{ IDC_CHECK_EMBED, load_string_resource(IDS_STRING_CHECK_EMBED) },
		{ IDC_EDIT_PLUGIN_CAPTION, load_string_resource(IDS_STRING_EDIT_CAPTION) },
		{ IDC_EDIT_PLUGIN_ICON, load_string_resource(IDS_STRING_EDIT_ICON) },
		{ IDC_EDIT_PLUGIN_BACKGROUND, load_string_resource(IDS_STRING_EDIT_BACKGROUND) },
		{ IDC_EDIT_PLUGIN_SUFFIX, load_string_resource(IDS_STRING_EDIT_SUFFIX) },
	};

	m_plugin_type = GetConfig().get_plugin_type();
	m_access_type = GetConfig().get_plugin_account_access_type();
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

	LoadAccounts();
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
	int selected = GetCheckedAccount();
	if (selected == -1)
		return;

	const auto& cred = m_all_credentials[selected];
	TemplateParams params;
	params.login = utils::utf8_to_utf16(cred.login);
	params.password = utils::utf8_to_utf16(cred.password);
	params.domain = utils::utf8_to_utf16(cred.domain);
	params.server = cred.device_id;
	params.profile = cred.profile_id;

	UINT static_text = IDS_STRING_SERVER_ID;
	switch (m_plugin_type)
	{
		case StreamType::enCbilling:
			static_text = IDS_STRING_DEVICE_ID;
			break;
		case StreamType::enSharaclub:
			m_list_domain = GetConfig().get_string(false, REG_LIST_DOMAIN);
			m_epg_domain = GetConfig().get_string(false, REG_EPG_DOMAIN);
			params.domain = m_list_domain;
			break;
		case StreamType::enVipLime:
			static_text = IDS_STRING_QUALITY_ID;
			break;
		default:
			break;
	}

	m_servers = m_plugin->get_servers_list(params);

	auto wndStaticDevice = GetDlgItem(IDC_STATIC_DEVICE_ID);
	wndStaticDevice->SetWindowText(load_string_resource(static_text).c_str());
	wndStaticDevice->EnableWindow(!m_servers.empty());

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

	m_caption = cred.get_caption().c_str();
	m_logo = cred.get_logo().c_str();
	m_background = cred.get_background().c_str();
	m_suffix = cred.get_suffix().c_str();

	UpdateData(FALSE);
}

void CAccessInfoDlg::LoadAccounts()
{
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
	switch (m_access_type)
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
		switch (m_access_type)
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
	int selected = GetCheckedAccount();
	if (selected == -1)
		return;

	auto& cred = m_all_credentials[selected];

	int idx = 0;
	auto ch_list = cred.ch_list;
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

void CAccessInfoDlg::OnOK()
{
	UpdateData(TRUE);

	int selected = GetCheckedAccount();
	const auto& cred = m_all_credentials[selected];

	TemplateParams params;
	params.login = utils::utf8_to_utf16(cred.login);
	params.password = utils::utf8_to_utf16(cred.password);
	params.domain = utils::utf8_to_utf16(cred.domain);
	params.server = cred.device_id;
	params.profile = cred.profile_id;
	if (m_plugin_type == StreamType::enSharaclub)
	{
		params.domain = m_list_domain;
	}

	if (m_wndDeviceID.GetCount())
	{
		m_plugin->set_server(params);
	}

	if (m_wndProfile.GetCount())
	{
		m_plugin->set_profile(params);
	}

	GetConfig().set_int(false, REG_ACTIVE_ACCOUNT, selected);

	nlohmann::json j_serialize = m_all_credentials;
	GetConfig().set_string(false, REG_ACCOUNT_DATA, utils::utf8_to_utf16(nlohmann::to_string(j_serialize)));

	m_initial_cred = m_all_credentials[selected];

	__super::OnOK();
}

void CAccessInfoDlg::OnBnClickedButtonAdd()
{
	m_wndAccounts.InsertItem(m_wndAccounts.GetItemCount(), L"new");

	Credentials cred;
	static constexpr auto newVal = "new";
	switch (m_access_type)
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
	dlg.m_url = GetConfig().get_string(false, REG_ACCESS_URL).c_str();

	if (dlg.DoModal() == IDOK)
	{
		m_status.Empty();
		std::vector<BYTE> data;
		GetConfig().set_string(false, REG_ACCESS_URL, dlg.m_url.GetString());
		if (!utils::DownloadFile(dlg.m_url.GetString(), data))
		{
			std::ifstream instream(dlg.m_url.GetString());
			data.assign((std::istreambuf_iterator<char>(instream)), std::istreambuf_iterator<char>());
		}

		const auto& wbuf = utils::utf8_to_utf16((char*)data.data(), data.size());
		std::wistringstream stream(wbuf);
		if (!stream.good()) return;

		Credentials cred;
		auto entry = std::make_unique<PlaylistEntry>(GetConfig().get_plugin_type(), GetAppPath(utils::PLUGIN_ROOT));
		std::wstring line;
		while (std::getline(stream, line))
		{
			utils::string_rtrim(line, L"\r");
			m3u_entry m3uEntry(line);
			if (!entry->Parse(line, m3uEntry)) continue;

			const auto& access_key = entry->get_uri_stream()->get_token();
			const auto& domain = entry->get_uri_stream()->get_domain();
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
	switch (m_access_type)
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
	}

	*pResult = 0;
}

void CAccessInfoDlg::OnLvnItemchangedListChannels(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	if ((pNMLV->uChanged & LVIF_STATE))
	{
		int account_idx = GetCheckedAccount();
		if (account_idx != -1)
		{
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

			m_all_credentials[account_idx].ch_list.swap(ch_list);

			UpdateOptionalControls();
		}
	}

	*pResult = 0;
}

void CAccessInfoDlg::GetAccountInfo()
{
	UpdateData(TRUE);

	m_status.LoadString(IDS_STRING_STATUS_TEXT);
	m_wndProfile.ResetContent();
	auto wndStaticProfile = GetDlgItem(IDC_STATIC_PROFILE);
	wndStaticProfile->EnableWindow(FALSE);
	m_wndProfile.EnableWindow(FALSE);

	int checked = GetCheckedAccount();
	if (checked == -1)
		return;

	auto& cred = m_all_credentials[checked];
	if (!m_servers.empty())
	{
		m_wndDeviceID.EnableWindow(TRUE);
		m_wndDeviceID.SetCurSel(cred.device_id);
	}

	m_wndEmbed.SetCheck(cred.embed);
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

	switch (m_access_type)
	{
		case AccountAccessType::enPin:
			password = utils::utf8_to_utf16(cred.password);
			break;

		case AccountAccessType::enLoginPass:
			login = utils::utf8_to_utf16(cred.login);
			password = utils::utf8_to_utf16(cred.password);
			break;

		case AccountAccessType::enOtt:
			token = utils::utf8_to_utf16(cred.token);
			domain = utils::utf8_to_utf16(cred.domain);
			portal = utils::utf8_to_utf16(cred.portal);
			break;

		default: break;
	}

	// reset templated flag for new parse
	auto entry = std::make_shared<PlaylistEntry>(GetConfig().get_plugin_type(), GetAppPath(utils::PLUGIN_ROOT));
	auto& uri = entry->stream_uri;
	uri->set_template(false);
	uri->set_login(login);
	uri->set_password(password);
	uri->set_token(token);
	uri->set_domain(domain);
	uri->set_host(m_host);

	TemplateParams params;
	params.login = std::move(login);
	params.password = std::move(password);
	params.domain = m_list_domain;
	params.server = cred.device_id;
	params.profile = cred.profile_id;

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
			cred.profile_id = 0;
		}

		m_wndProfile.SetCurSel(params.profile);
	}

	wndStaticProfile->EnableWindow(m_profiles.size() > 1);
	m_wndProfile.EnableWindow(m_profiles.size() > 1);

	std::wstring pl_url = uri->get_playlist_url(params);

	std::list<AccountInfo> acc_info;
	if (uri->parse_access_info(params, acc_info))
	{
		for (auto it = acc_info.begin(); it != acc_info.end(); )
		{
			// currently supported only in sharaclub, oneott use this to obtain token
			if (it->name == (L"token"))
			{
				cred.token = get_utf8(it->value);
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
				if (entry->Parse(line, m3uEntry) && !uri->get_token().empty())
				{
					// do not override fake ott and domain for edem
					if (m_access_type != AccountAccessType::enOtt)
					{
						cred.token = get_utf8(uri->get_token());
						cred.domain = get_utf8(uri->get_domain());
					}
					m_host = uri->get_host();
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

int CAccessInfoDlg::GetCheckedAccount()
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
	int selected = GetCheckedAccount();
	if (selected != -1)
	{
		m_all_credentials[selected].device_id = m_wndDeviceID.GetCurSel();
	}
}

void CAccessInfoDlg::OnCbnSelchangeComboProfile()
{
	int selected = GetCheckedAccount();
	if (selected != -1)
	{
		m_all_credentials[selected].profile_id = m_wndProfile.GetCurSel();
	}
}

void CAccessInfoDlg::OnBnClickedCheckEmbed()
{
	int selected = GetCheckedAccount();
	if (selected != -1)
	{
		m_all_credentials[selected].embed = m_wndEmbed.GetCheck();
	}
}

void CAccessInfoDlg::OnEnChangeEditPluginCaption()
{
	UpdateData(TRUE);
	int selected = GetCheckedAccount();
	if (selected != -1)
	{
		m_all_credentials[selected].caption = get_utf8(m_caption.GetString());
	}
}

void CAccessInfoDlg::OnEnChangeEditPluginIcon()
{
	UpdateData(TRUE);
	int selected = GetCheckedAccount();
	if (selected != -1)
	{
		m_all_credentials[selected].logo = get_utf8(m_logo.GetString());
	}
}

void CAccessInfoDlg::OnEnChangeEditPluginBackground()
{
	UpdateData(TRUE);
	int selected = GetCheckedAccount();
	if (selected != -1)
	{
		m_all_credentials[selected].background = get_utf8(m_background.GetString());
	}
}

void CAccessInfoDlg::OnEnChangeEditPluginSuffix()
{
	UpdateData(TRUE);
	int selected = GetCheckedAccount();
	if (selected != -1)
	{
		m_all_credentials[selected].suffix = get_utf8(m_suffix.GetString());
	}
}
