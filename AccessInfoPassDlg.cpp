#include "pch.h"
#include <afxdialogex.h>
#include "AccessInfoPassDlg.h"
#include "resource.h"
#include "PlayListEntry.h"

#include "UtilsLib\inet_utils.h"
#include "UtilsLib\md5.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CAccessDlg dialog

IMPLEMENT_DYNAMIC(CAccessInfoPassDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CAccessInfoPassDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_GET, &CAccessInfoPassDlg::OnBnClickedBtnGet)
END_MESSAGE_MAP()


CAccessInfoPassDlg::CAccessInfoPassDlg(StreamType type, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_ACCESS_INFO_PASS, pParent), m_type(type)
{
}

void CAccessInfoPassDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_EDIT_LOGIN, m_wndLogin);
	DDX_Control(pDX, IDC_EDIT_PASSWORD, m_wndPassword);
	DDX_Control(pDX, IDC_BUTTON_GET, m_wndGet);
	DDX_Check(pDX, IDC_CHECK_EMBED, m_bEmbed);
	DDX_Text(pDX, IDC_EDIT_LOGIN, m_login);
	DDX_Control(pDX, IDC_LIST_INFO, m_wndInfo);
	DDX_Text(pDX, IDC_EDIT_PASSWORD, m_password);
}

BOOL CAccessInfoPassDlg::OnInitDialog()
{
	__super::OnInitDialog();

	m_login = m_entry->stream_uri->get_login().c_str();
	m_password = m_entry->stream_uri->get_password().c_str();
	m_token = m_entry->stream_uri->get_token().c_str();

	m_wndInfo.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_GRIDLINES/* | LVS_EX_UNDERLINECOLD | LVS_EX_UNDERLINEHOT*/);
	CRect rect;
	m_wndInfo.GetClientRect(&rect);
	int vWidth = rect.Width() - GetSystemMetrics(SM_CXVSCROLL) - 1;
	int nWidth = vWidth / 4;

	m_wndInfo.InsertColumn(0, _T("Info"), LVCFMT_LEFT, nWidth, 0);
	m_wndInfo.InsertColumn(1, _T("Data"), LVCFMT_LEFT, vWidth - nWidth, 0);

	UpdateData(FALSE);

	OnBnClickedBtnGet();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CAccessInfoPassDlg::OnOK()
{
	UpdateData(TRUE);

	m_entry->stream_uri->set_login(m_login.GetString());
	m_entry->stream_uri->set_password(m_password.GetString());
	m_entry->stream_uri->set_token(m_token.GetString());

	__super::OnOK();
}

void CAccessInfoPassDlg::OnBnClickedBtnGet()
{
	UpdateData(TRUE);

	m_status.LoadString(IDS_STRING_STATUS_TEXT);

	const auto& login = std::wstring(m_login.GetString());
	const auto& password = std::wstring(m_password.GetString());

	// reset templated flag for new parse
	const auto& uri = m_entry->stream_uri;
	uri->set_template(false);

	std::wstring pl_url = fmt::format(uri->get_playlist_template(), login, password);

	if (m_type == StreamType::enVidok)
	{
		std::string login_a = utils::string_tolower(utils::utf16_to_utf8(login));
		std::string password_a = utils::utf16_to_utf8(password);
		const auto& token = utils::utf8_to_utf16(utils::md5_hash_hex(login_a + utils::md5_hash_hex(password_a)));
		pl_url = fmt::format(uri->get_playlist_template(), token);
	}

	std::map<std::wstring, std::wstring> params;
	if (m_type == StreamType::enOneOtt || m_type == StreamType::enSharaclub || m_type == StreamType::enVidok)
	{
		// currently supported only in sharaclub, oneott use this to obtain token
		std::vector<BYTE> data;
		if (utils::DownloadFile(uri->get_access_url(login, password), data) && !data.empty())
		{
			uri->parse_access_info(data, params);

			if (params.find(L"token") != params.end())
				m_token = params[L"token"].c_str();

			if (params.find(L"url") != params.end())
				pl_url = params[L"url"];
		}
	}

	std::vector<BYTE> data;
	if (!pl_url.empty() && utils::DownloadFile(pl_url, data))
	{
		utils::vector_to_streambuf<char> buf(data);
		std::istream stream(&buf);

		if (stream.good())
		{
			int step = 0;
			std::string line;
			while (std::getline(stream, line))
			{
				utils::string_rtrim(line, "\r");
				m3u_entry m3uEntry(line);
				if (m_entry->Parse(line, m3uEntry) && (!m_entry->stream_uri->get_login().empty() || !m_entry->stream_uri->get_token().empty()))
				{
					m_status = _T("Ok");
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
	for (const auto& pair : params)
	{
		m_wndInfo.InsertItem(idx, pair.first.c_str());
		m_wndInfo.SetItemText(idx++, 1, pair.second.c_str());
	}


	UpdateData(FALSE);
}
