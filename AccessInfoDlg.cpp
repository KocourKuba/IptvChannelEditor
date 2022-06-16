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

std::map<UINT, UINT> tooltips_info_account =
{
	{ IDC_BUTTON_ADD, IDS_STRING_BUTTON_ADD },
	{ IDC_BUTTON_REMOVE, IDS_STRING_BUTTON_REMOVE },
	{ IDC_BUTTON_NEW_FROM_URL, IDS_STRING_BUTTON_NEW_FROM_URL },
	{ IDC_COMBO_DEVICE_ID, IDS_STRING_COMBO_DEVICE_ID },
	{ IDC_COMBO_PROFILE, IDS_STRING_COMBO_PROFILE },
	{ IDC_CHECK_EMBED, IDS_STRING_CHECK_EMBED },
};

// CAccessDlg dialog

IMPLEMENT_DYNAMIC(CAccessInfoDlg, CMFCPropertyPage)

BEGIN_MESSAGE_MAP(CAccessInfoDlg, CMFCPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CAccessInfoDlg::OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, &CAccessInfoDlg::OnBnClickedButtonRemove)
	ON_BN_CLICKED(IDC_BUTTON_NEW_FROM_URL, &CAccessInfoDlg::OnBnClickedButtonNewFromUrl)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_ACCOUNTS, &CAccessInfoDlg::OnNMDblClickList)
	ON_MESSAGE(WM_NOTIFY_END_EDIT, &CAccessInfoDlg::OnNotifyDescriptionEdited)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_ACCOUNTS, &CAccessInfoDlg::OnLvnItemchangedListAccounts)
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
	DDX_Control(pDX, IDC_LIST_INFO, m_wndInfo);
	DDX_Control(pDX, IDC_LIST_ACCOUNTS, m_wndAccounts);
	DDX_Control(pDX, IDC_COMBO_PROFILE, m_wndProfile);
	DDX_Check(pDX, IDC_CHECK_EMBED, m_bEmbed);
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

	m_plugin_type = GetConfig().get_plugin_type();
	m_access_type = GetConfig().get_plugin_account_access_type();
	m_plugin = StreamContainer::get_instance(m_plugin_type);

	CRect rect;

	m_wndAccounts.SetExtendedStyle(m_wndAccounts.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
	CHeaderCtrl* header = m_wndAccounts.GetHeaderCtrl();
	header->ModifyStyle(0, HDS_CHECKBOXES);
	HDITEM hdi = { 0 };
	hdi.mask = HDI_FORMAT;
	header->GetItem(0, &hdi);
	hdi.fmt |= HDF_CHECKBOX;
	header->SetItem(0, &hdi);

	m_wndToolTipCtrl.SetDelayTime(TTDT_AUTOPOP, 10000);
	m_wndToolTipCtrl.SetDelayTime(TTDT_INITIAL, 500);
	m_wndToolTipCtrl.SetMaxTipWidth(500);

	for (const auto& pair : tooltips_info_account)
	{
		m_wndToolTipCtrl.AddTool(GetDlgItem(pair.first), pair.second);
	}

	m_wndToolTipCtrl.Activate(TRUE);

	std::wstring provider_url = m_plugin->get_provider_url();
	m_wndProviderLink.SetURL(provider_url.c_str());
	m_wndProviderLink.SetWindowText(provider_url.c_str());

	m_wndAccounts.GetClientRect(&rect);
	int vWidth = rect.Width() - 23;

	int last = 0;
	m_wndAccounts.InsertColumn(last++, L"", LVCFMT_LEFT, 22, 0);
	switch(m_access_type)
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

	m_wndAccounts.InsertColumn(last, load_string_resource(IDS_STRING_COL_COMMENT).c_str(), LVCFMT_LEFT, vWidth, 0);

	m_wndInfo.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_GRIDLINES);

	m_wndInfo.GetClientRect(&rect);
	vWidth = rect.Width() - GetSystemMetrics(SM_CXVSCROLL) - 1;
	int nWidth = vWidth / 4;

	m_wndInfo.InsertColumn(0, load_string_resource(IDS_STRING_COL_INFO).c_str(), LVCFMT_LEFT, nWidth, 0);
	m_wndInfo.InsertColumn(1, load_string_resource(IDS_STRING_COL_DATA).c_str(), LVCFMT_LEFT, vWidth - nWidth, 0);

	TemplateParams params;
	params.login = m_login;
	params.password = m_password;
	params.domain = m_domain;
	params.server = GetConfig().get_int(false, REG_DEVICE_ID);
	params.profile = GetConfig().get_int(false, REG_PROFILE_ID);

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

	const auto& credentials = GetConfig().get_string(false, REG_CREDENTIALS);
	auto& all_accounts = utils::string_split(credentials, L';');
	if (all_accounts.empty())
	{
		switch (m_access_type)
		{
			case AccountAccessType::enPin:
				all_accounts.emplace_back(m_password);
				break;

			case AccountAccessType::enLoginPass:
				all_accounts.emplace_back(fmt::format(L"\"{:s}\",\"{:s}\"", m_login, m_password));
				break;

			case AccountAccessType::enOtt:
				all_accounts.emplace_back(fmt::format(L"\"{:s}\",\"{:s}\",\"{:s}\"", m_token, m_domain, m_portal));
				break;

			default: break;
		}
	}

	int selected = -1;
	int idx = 0;
	for (const auto& account : all_accounts)
	{
		auto& creds = utils::string_split(account, L',');
		if (creds.empty() || creds.front().empty()) continue;

		m_wndAccounts.InsertItem(idx, L"", 0);

		size_t last = 0;
		std::wstring str(creds[last]);
		utils::string_trim(str, utils::EMSLiterals<wchar_t>::quote);
		m_wndAccounts.SetItemText(idx, ++last, str.c_str());
		switch (m_access_type)
		{
			case AccountAccessType::enPin:
				if (str == m_password)
				{
					selected = idx;
				}
				break;

			case AccountAccessType::enLoginPass:
				if (str == m_login)
				{
					selected = idx;
				}
				str = creds.size() > last ? creds[last] : L"";
				utils::string_trim(str, utils::EMSLiterals<wchar_t>::quote);
				m_wndAccounts.SetItemText(idx, ++last, str.c_str());
				break;

			case AccountAccessType::enOtt:
				if (str == m_token)
				{
					selected = idx;
				}

				str = creds.size() > last ? creds[last] : L"";
				utils::string_trim(str, utils::EMSLiterals<wchar_t>::quote);
				m_wndAccounts.SetItemText(idx, ++last, str.c_str());

				str = creds.size() > last ? creds[last] : L"";
				utils::string_trim(str, utils::EMSLiterals<wchar_t>::quote);
				m_wndAccounts.SetItemText(idx, ++last, str.c_str());
				break;

			default:break;
		}

		str = creds.size() > last ? creds[last] : L"";
		utils::string_trim(str, utils::EMSLiterals<wchar_t>::quote);
		m_wndAccounts.SetItemText(idx, ++last, str.c_str());

		++idx;
	}

	m_wndAccounts.SetCheck(selected, TRUE);
	m_wndRemove.EnableWindow(m_wndAccounts.GetSelectionMark() != -1);

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CAccessInfoDlg::OnOK()
{
	UpdateData(TRUE);

	std::wstring credentials;
	int sel = GetChecked();
	int cnt = m_wndAccounts.GetItemCount();

	for (int i = 0; i < cnt; i++)
	{
		switch (m_access_type)
		{
			case AccountAccessType::enPin:
				credentials += fmt::format(L"\"{:s}\",\"{:s}\";",
										   m_wndAccounts.GetItemText(i, 1).GetString(),
										   m_wndAccounts.GetItemText(i, 2).GetString());
				if (i == sel)
				{
					m_password = m_wndAccounts.GetItemText(i, 1).GetString();
				}
				break;

			case AccountAccessType::enLoginPass:
				credentials += fmt::format(L"\"{:s}\",\"{:s}\",\"{:s}\";",
					m_wndAccounts.GetItemText(i, 1).GetString(),
					m_wndAccounts.GetItemText(i, 2).GetString(),
					m_wndAccounts.GetItemText(i, 3).GetString());
				if (i == sel)
				{
					m_login = m_wndAccounts.GetItemText(i, 1).GetString();
					m_password = m_wndAccounts.GetItemText(i, 2).GetString();
				}
				break;

			case AccountAccessType::enOtt:
				credentials += fmt::format(L"\"{:s}\",\"{:s}\",\"{:s}\",\"{:s}\";",
					m_wndAccounts.GetItemText(i, 1).GetString(),
					m_wndAccounts.GetItemText(i, 2).GetString(),
					m_wndAccounts.GetItemText(i, 3).GetString(),
					m_wndAccounts.GetItemText(i, 4).GetString());
				if (i == sel)
				{
					m_token = m_wndAccounts.GetItemText(i, 1).GetString();
					m_domain = m_wndAccounts.GetItemText(i, 2).GetString();
					m_portal = m_wndAccounts.GetItemText(i, 3).GetString();
					GetConfig().set_string(false, REG_VPORTAL, m_portal);
				}
				break;

			default: break;
		}
	}

	TemplateParams params;
	params.login = m_login;
	params.password = m_password;
	params.domain = m_domain;
	params.server = m_wndDeviceID.GetCurSel();
	params.profile = m_wndProfile.GetCurSel();
	if (m_plugin_type == StreamType::enSharaclub)
	{
		params.domain = m_list_domain;
	}

	if (m_wndDeviceID.GetCount() && m_wndDeviceID.GetCurSel() != GetConfig().get_int(false, REG_DEVICE_ID))
	{
		m_plugin->set_server(params);
		GetConfig().set_int(false, REG_DEVICE_ID, m_wndDeviceID.GetCurSel());
	}

	if (m_wndProfile.GetCount() && m_wndProfile.GetCurSel() != GetConfig().get_int(false, REG_PROFILE_ID))
	{
		m_plugin->set_profile(params);
		GetConfig().set_int(false, REG_PROFILE_ID, m_wndProfile.GetCurSel());
	}

	GetConfig().set_int(false, REG_EMBED_INFO, m_bEmbed);
	GetConfig().set_string(false, REG_DOMAIN, m_domain);
	GetConfig().set_string(false, REG_TOKEN, m_token);
	GetConfig().set_string(false, REG_LOGIN, m_login);
	GetConfig().set_string(false, REG_PASSWORD, m_password);
	GetConfig().set_string(false, REG_HOST, m_host);
	GetConfig().set_string(false, REG_CREDENTIALS, credentials);

	__super::OnOK();
}

void CAccessInfoDlg::OnBnClickedButtonAdd()
{
	m_wndAccounts.InsertItem(m_wndAccounts.GetItemCount(), L"new");
}

void CAccessInfoDlg::OnBnClickedButtonRemove()
{
	int idx = m_wndAccounts.GetSelectionMark();
	if (idx != -1)
		m_wndAccounts.DeleteItem(idx);
}

void CAccessInfoDlg::OnBnClickedButtonNewFromUrl()
{
	CUrlDlg dlg;
	dlg.m_url = GetConfig().get_string(false, REG_ACCESS_URL).c_str();

	if (dlg.DoModal() == IDOK)
	{
		m_token.clear();
		m_domain.clear();
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
			GetParent()->GetDlgItem(IDOK)->EnableWindow(TRUE);
		}
		else if ((pNMLV->uNewState & 0x1000) && (pNMLV->uOldState & 0x2000))
		{
			m_login.clear();
			m_password.clear();
			m_token.clear();
			m_domain.clear();
			m_portal.clear();
			m_wndInfo.DeleteAllItems();
			GetParent()->GetDlgItem(IDOK)->EnableWindow(FALSE);
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

	int checked = GetChecked();
	if (checked == -1)
		return;

	switch (m_access_type)
	{
		case AccountAccessType::enPin:
			m_password = m_wndAccounts.GetItemText(checked, 1).GetString();
			break;

		case AccountAccessType::enLoginPass:
			m_login = m_wndAccounts.GetItemText(checked, 1).GetString();
			m_password = m_wndAccounts.GetItemText(checked, 2).GetString();
			break;

		case AccountAccessType::enOtt:
			m_token = m_wndAccounts.GetItemText(checked, 1).GetString();
			m_domain = m_wndAccounts.GetItemText(checked, 2).GetString();
			m_portal = m_wndAccounts.GetItemText(checked, 3).GetString();
			break;

		default: break;
	}

	// reset templated flag for new parse
	auto entry = std::make_shared<PlaylistEntry>(GetConfig().get_plugin_type(), GetAppPath(utils::PLUGIN_ROOT));
	auto& uri = entry->stream_uri;
	uri->set_template(false);
	uri->set_login(m_login);
	uri->set_password(m_password);
	uri->set_token(m_token);
	uri->set_domain(m_domain);
	uri->set_host(m_host);

	TemplateParams params;
	params.login = m_login;
	params.password = m_password;
	params.domain = m_list_domain;
	params.server = m_wndDeviceID.GetCurSel();

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
				m_token = it->value;
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
					m_status = _T("Ok");
					m_token = uri->get_token();
					m_domain = uri->get_domain();
					m_host = uri->get_host();
					break;
				}
			}
		}
	}

	int idx = 0;
	m_wndInfo.DeleteAllItems();
	m_wndInfo.InsertItem(idx, load_string_resource(IDS_STRING_STATUS).c_str());
	m_wndInfo.SetItemText(idx++, 1, m_status);
	for (const auto& item : acc_info)
	{
		m_wndInfo.InsertItem(idx, item.name.c_str());
		m_wndInfo.SetItemText(idx++, 1, item.value.c_str());
	}

	UpdateData(FALSE);
}

int CAccessInfoDlg::GetChecked()
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
