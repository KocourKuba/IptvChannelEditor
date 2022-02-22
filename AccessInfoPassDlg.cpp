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
	ON_EN_CHANGE(IDC_EDIT_LOGIN, &CAccessInfoPassDlg::OnEnChangeEditLogin)
	ON_EN_CHANGE(IDC_EDIT_PASSWORD, &CAccessInfoPassDlg::OnEnChangeEditLogin)
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

	CString str;
	str.LoadString(IDS_STRING_COL_INFO);
	m_wndInfo.InsertColumn(0, str, LVCFMT_LEFT, nWidth, 0);
	str.LoadString(IDS_STRING_COL_DATA);
	m_wndInfo.InsertColumn(1, str, LVCFMT_LEFT, vWidth - nWidth, 0);

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

	// reset templated flag for new parse
	auto& uri = m_entry->stream_uri;
	uri->set_template(false);

	PlaylistTemplateParams params;
	params.login = m_login.GetString();
	params.password = m_password.GetString();
	std::wstring url = uri->get_playlist_template(params);

	std::list<AccountInfo> acc_info;
	if (uri->parse_access_info(params, acc_info))
	{
		// currently supported only in sharaclub, oneott use this to obtain token
		for (auto it = acc_info.begin(); it != acc_info.end(); )
		{
			if (it->name == (L"token"))
			{
				m_token = it->value.c_str();
				it = acc_info.erase(it);
			}
			else if (it->name == (L"url"))
			{
				url = it->value;
				it = acc_info.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	std::vector<BYTE> data;
	if (!url.empty() && utils::DownloadFile(url, data))
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
	for (const auto& item : acc_info)
	{
		m_wndInfo.InsertItem(idx, item.name.c_str());
		m_wndInfo.SetItemText(idx++, 1, item.value.c_str());
	}

	GetDlgItem(IDOK)->EnableWindow(TRUE);

	UpdateData(FALSE);
}

void CAccessInfoPassDlg::OnEnChangeEditLogin()
{
	GetDlgItem(IDOK)->EnableWindow(FALSE);
}
