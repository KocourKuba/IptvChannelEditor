#include "pch.h"
#include <afxdialogex.h>
#include "resource.h"
#include "AccessInfoDlg.h"
#include "IPTVChannelEditor.h"
#include "PlayListEntry.h"

#include "UtilsLib\inet_utils.h"
#include "UrlDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CAccessDlg dialog

IMPLEMENT_DYNAMIC(CAccessInfoDlg, CPropertyPage)

BEGIN_MESSAGE_MAP(CAccessInfoDlg, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_GET, &CAccessInfoDlg::GetAccountInfo)
	ON_BN_CLICKED(IDC_BUTTON_NEW, &CAccessInfoDlg::OnBnClickedButtonNew)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, &CAccessInfoDlg::OnBnClickedButtonRemove)
	ON_BN_CLICKED(IDC_BUTTON_NEW_FROM_URL, &CAccessInfoDlg::OnBnClickedButtonNewFromUrl)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_ACCOUNTS, &CAccessInfoDlg::OnNMDblClickList)
	ON_MESSAGE(WM_NOTIFY_END_EDIT, &CAccessInfoDlg::OnNotifyDescriptionEdited)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_ACCOUNTS, &CAccessInfoDlg::OnLvnItemchangedListAccounts)
END_MESSAGE_MAP()


CAccessInfoDlg::CAccessInfoDlg() : CPropertyPage(IDD_DIALOG_ACCESS_INFO)
{
}

void CAccessInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_EMBED, m_bEmbed);
	DDX_Control(pDX, IDC_BUTTON_GET, m_wndGet);
	DDX_Control(pDX, IDC_BUTTON_REMOVE, m_wndRemove);
	DDX_Control(pDX, IDC_BUTTON_NEW_FROM_URL, m_wndNewFromUrl);
	DDX_Control(pDX, IDC_COMBO_DEVICE_ID, m_wndDeviceID);
	DDX_Control(pDX, IDC_LIST_INFO, m_wndInfo);
	DDX_Control(pDX, IDC_LIST_ACCOUNTS, m_wndAccounts);
	DDX_Control(pDX, IDC_CHECK_EMBED_VP, m_wndEmbedPortal);
	DDX_Check(pDX, IDC_CHECK_EMBED_VP, m_bEmbed_vp);
}

BOOL CAccessInfoDlg::OnInitDialog()
{
	__super::OnInitDialog();

	auto plugin_type = GetConfig().get_plugin_type();
	auto account_type = GetConfig().get_plugin_account_type();
	CRect rect;

	m_wndAccounts.SetExtendedStyle(m_wndAccounts.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
	CHeaderCtrl* header = m_wndAccounts.GetHeaderCtrl();
	header->ModifyStyle(0, HDS_CHECKBOXES);
	HDITEM hdi = { 0 };
	hdi.mask = HDI_FORMAT;
	header->GetItem(0, &hdi);
	hdi.fmt |= HDF_CHECKBOX;
	header->SetItem(0, &hdi);

	m_wndAccounts.GetClientRect(&rect);
	int vWidth = rect.Width() - /*GetSystemMetrics(SM_CXVSCROLL) -*/ 1 - 22;

	m_wndAccounts.InsertColumn(0, L"", LVCFMT_LEFT, 22, 0);
	CString str;
	if (account_type == AccountType::enPin)
	{
		str.LoadString(IDS_STRING_COL_PASSWORD);
		m_wndAccounts.InsertColumn(1, str, LVCFMT_LEFT, vWidth, 0);
	}
	else if (account_type == AccountType::enLoginPass)
	{
		vWidth /= 2;
		str.LoadString(IDS_STRING_COL_LOGIN);
		m_wndAccounts.InsertColumn(1, str, LVCFMT_LEFT, vWidth, 0);
		str.LoadString(IDS_STRING_COL_PASSWORD);
		m_wndAccounts.InsertColumn(2, str, LVCFMT_LEFT, vWidth, 0);
	}
	else if (account_type == AccountType::enOtt)
	{
		vWidth /= 3;
		str.LoadString(IDS_STRING_COL_TOKEN);
		m_wndAccounts.InsertColumn(1, str, LVCFMT_LEFT, vWidth, 0);
		str.LoadString(IDS_STRING_COL_DOMAIN);
		m_wndAccounts.InsertColumn(2, str, LVCFMT_LEFT, vWidth, 0);
		str.LoadString(IDS_STRING_COL_VPORTAL);
		m_wndAccounts.InsertColumn(3, str, LVCFMT_LEFT, vWidth, 0);

		m_wndEmbedPortal.ShowWindow(SW_SHOW);
		m_wndNewFromUrl.ShowWindow(SW_SHOW);
	}

	m_wndInfo.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_GRIDLINES);

	m_wndInfo.GetClientRect(&rect);
	vWidth = rect.Width() - GetSystemMetrics(SM_CXVSCROLL) - 1;
	int nWidth = vWidth / 4;

	str.LoadString(IDS_STRING_COL_INFO);
	m_wndInfo.InsertColumn(0, str, LVCFMT_LEFT, nWidth, 0);
	str.LoadString(IDS_STRING_COL_DATA);
	m_wndInfo.InsertColumn(1, str, LVCFMT_LEFT, vWidth - nWidth, 0);

	if (plugin_type == StreamType::enCbilling || plugin_type == StreamType::enShuraTV)
	{
		int max_entry = 3;
		CString text;
		text.LoadString(IDS_STRING_DEVICE_ID);
		if (plugin_type == StreamType::enShuraTV)
		{
			max_entry = 2;
			text.LoadString(IDS_STRING_SERVER_ID);
			GetDlgItem(IDC_STATIC_DEVICE_ID)->SetWindowText(text);
		}

		GetDlgItem(IDC_STATIC_DEVICE_ID)->ShowWindow(SW_SHOW);
		for (int i = 1; i <= max_entry; i++)
		{
			text.Format(_T("%d"), i);
			m_wndDeviceID.AddString(text);
		}
		m_wndDeviceID.SetCurSel(GetConfig().get_int(false, REG_DEVICE_ID, 1) - 1);
		m_wndDeviceID.ShowWindow(SW_SHOW);
	}

	const auto& credentials = GetConfig().get_string(false, REG_CREDENTIALS);
	auto& all_accounts = utils::string_split(credentials, L';');
	if (all_accounts.empty())
	{
		if (account_type == AccountType::enPin)
		{
			all_accounts.emplace_back(m_password);
		}
		else if (account_type == AccountType::enLoginPass)
		{
			all_accounts.emplace_back(fmt::format(L"\"{:s}\",\"{:s}\"", m_login, m_password));
		}
		else if (account_type == AccountType::enOtt)
		{
			all_accounts.emplace_back(fmt::format(L"\"{:s}\",\"{:s}\",\"{:s}\"", m_token, m_domain, m_portal));
		}
	}

	int selected = -1;
	int idx = 0;
	for (const auto& account : all_accounts)
	{
		auto& creds = utils::string_split(account, L',');
		if (creds.empty() || creds.front().empty()) continue;

		m_wndAccounts.InsertItem(idx, L"", 0);

		std::wstring str(creds[0]);
		utils::string_trim(str, utils::EMSLiterals<wchar_t>::quote);
		m_wndAccounts.SetItemText(idx, 1, str.c_str());
		if (account_type == AccountType::enPin)
		{
			if (str == m_password)
			{
				selected = idx;
			}
		}
		else if (account_type == AccountType::enLoginPass)
		{
			if (str == m_login)
			{
				selected = idx;
			}
			str = creds.size() > 1 ? creds[1] : L"";
			utils::string_trim(str, utils::EMSLiterals<wchar_t>::quote);
			m_wndAccounts.SetItemText(idx, 2, str.c_str());
		}
		else if (account_type == AccountType::enOtt)
		{
			if (str == m_token)
			{
				selected = idx;
			}

			str = creds.size() > 1 ? creds[1] : L"";
			utils::string_trim(str, utils::EMSLiterals<wchar_t>::quote);
			m_wndAccounts.SetItemText(idx, 2, str.c_str());

			str = creds.size() > 2 ? creds[2] : L"";
			utils::string_trim(str, utils::EMSLiterals<wchar_t>::quote);
			m_wndAccounts.SetItemText(idx, 3, str.c_str());
		}
		++idx;
	}

	m_wndAccounts.SetCheck(selected, TRUE);
	m_wndGet.EnableWindow(selected != -1);
	m_wndRemove.EnableWindow(m_wndAccounts.GetSelectionMark() != -1);

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CAccessInfoDlg::OnOK()
{
	UpdateData(TRUE);

	auto account_type = GetConfig().get_plugin_account_type();
	std::wstring credentials;
	int sel = GetChecked();
	int cnt = m_wndAccounts.GetItemCount();

	for (int i = 0; i < cnt; i++)
	{
		if (account_type == AccountType::enPin)
		{
			credentials += fmt::format(L"\"{:s}\";", m_wndAccounts.GetItemText(i, 0).GetString());
			if (i == sel)
			{
				m_password = m_wndAccounts.GetItemText(i, 1).GetString();
			}
		}
		else if (account_type == AccountType::enLoginPass)
		{
			credentials += fmt::format(L"\"{:s}\",\"{:s}\";",
									   m_wndAccounts.GetItemText(i, 1).GetString(),
									   m_wndAccounts.GetItemText(i, 2).GetString());
			if (i == sel)
			{
				m_login = m_wndAccounts.GetItemText(i, 1).GetString();
				m_password = m_wndAccounts.GetItemText(i, 2).GetString();
			}
		}
		else if (account_type == AccountType::enOtt)
		{
			credentials += fmt::format(L"\"{:s}\",\"{:s}\",\"{:s}\";",
									   m_wndAccounts.GetItemText(i, 1).GetString(),
									   m_wndAccounts.GetItemText(i, 2).GetString(),
									   m_wndAccounts.GetItemText(i, 3).GetString());
			if (i == sel)
			{
				m_token = m_wndAccounts.GetItemText(i, 1).GetString();
				m_domain = m_wndAccounts.GetItemText(i, 2).GetString();
				m_portal = m_wndAccounts.GetItemText(i, 3).GetString();
			}
		}
	}

	if (account_type == AccountType::enOtt)
	{
		GetConfig().set_string(false, REG_PORTAL, m_portal);
	}

	GetConfig().set_string(false, REG_DOMAIN, m_domain);
	GetConfig().set_string(false, REG_TOKEN, m_token);
	GetConfig().set_string(false, REG_LOGIN, m_login);
	GetConfig().set_string(false, REG_PASSWORD, m_password);
	GetConfig().set_string(false, REG_HOST, m_host);
	GetConfig().set_string(false, REG_CREDENTIALS, credentials);

	const auto plugin_type = GetConfig().get_plugin_type();
	if (plugin_type == StreamType::enCbilling || plugin_type == StreamType::enShuraTV)
	{
		GetConfig().set_int(false, REG_DEVICE_ID, m_wndDeviceID.GetCurSel() + 1);
	}

	__super::OnOK();
}

void CAccessInfoDlg::OnBnClickedButtonNew()
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

			auto account_type = GetConfig().get_plugin_account_type();
			if (account_type == AccountType::enPin)
			{
				m_password = m_wndAccounts.GetItemText(pNMLV->iItem, 1).GetString();
			}
			else if (account_type == AccountType::enLoginPass)
			{
				m_login = m_wndAccounts.GetItemText(pNMLV->iItem, 1).GetString();
				m_password = m_wndAccounts.GetItemText(pNMLV->iItem, 2).GetString();
			}
			else if (account_type == AccountType::enOtt)
			{
				m_token = m_wndAccounts.GetItemText(pNMLV->iItem, 1).GetString();
				m_domain = m_wndAccounts.GetItemText(pNMLV->iItem, 2).GetString();
				m_portal = m_wndAccounts.GetItemText(pNMLV->iItem, 3).GetString();
			}

			m_wndGet.EnableWindow(TRUE);
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
			m_wndGet.EnableWindow(FALSE);
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

	int checked = GetChecked();
	if (checked == -1)
		return;

	// reset templated flag for new parse
	auto entry = std::make_shared<PlaylistEntry>(GetConfig().get_plugin_type(), GetAppPath(utils::PLUGIN_ROOT));
	auto& uri = entry->stream_uri;
	uri->set_template(false);
	uri->set_login(m_login);
	uri->set_password(m_password);
	uri->set_token(m_token);
	uri->set_domain(m_domain);
	uri->set_host(m_host);

	PlaylistTemplateParams params;
	params.device = m_wndDeviceID.GetCurSel() + 1;
	params.login = m_login;
	params.password = m_password;
	std::wstring pl_url = uri->get_playlist_template(params);

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
	CString txt;
	txt.LoadString(IDS_STRING_STATUS);
	m_wndInfo.DeleteAllItems();
	m_wndInfo.InsertItem(idx, txt);
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