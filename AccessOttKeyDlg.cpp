// SettingsDlg.cpp : implementation file
//

#include "StdAfx.h"
#include <afxdialogex.h>
#include "IPTVChannelEditor.h"
#include "AccessOttKeyDlg.h"
#include "PlayListEntry.h"

// CAccessDlg dialog

IMPLEMENT_DYNAMIC(CAccessOttKeyDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CAccessOttKeyDlg, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT_PLAYLIST_URL, &CAccessOttKeyDlg::OnEnChangeEditPlaylistUrl)
	ON_BN_CLICKED(IDC_BUTTON_GET, &CAccessOttKeyDlg::OnBnClickedBtnGet)
END_MESSAGE_MAP()


CAccessOttKeyDlg::CAccessOttKeyDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_ACCESS_INFO, pParent)
	, m_streamType(StreamType::enEdem)
{

}

void CAccessOttKeyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_EMBED, m_bEmbed);
	DDX_Control(pDX, IDC_EDIT_PLAYLIST_URL, m_wndUrl);
	DDX_Control(pDX, IDC_BUTTON_GET, m_wndGet);
	DDX_Text(pDX, IDC_EDIT_KEY, m_accessKey);
	DDX_Text(pDX, IDC_EDIT_DOMAIN, m_domain);
	DDX_Text(pDX, IDC_EDIT_PLAYLIST_URL, m_url);
}

BOOL CAccessOttKeyDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CAccessOttKeyDlg::OnEnChangeEditPlaylistUrl()
{
	UpdateData(TRUE);

	m_wndGet.EnableWindow(!m_url.IsEmpty());
}

void CAccessOttKeyDlg::OnBnClickedBtnGet()
{
	UpdateData(TRUE);

	m_accessKey.Empty();
	m_domain.Empty();
	m_status.Empty();
	std::vector<BYTE> data;
	std::unique_ptr<std::istream> pl_stream;
	if (utils::CrackUrl(m_url.GetString()))
	{
		if (utils::DownloadFile(m_url.GetString(), data))
		{
			utils::vector_to_streambuf<char> buf(data);
			pl_stream = std::make_unique<std::istream>(&buf);
		}
	}
	else
	{
		pl_stream = std::make_unique<std::ifstream>(m_url.GetString());
	}

	if (!pl_stream || !pl_stream->good()) return;

	int step = 0;
	std::string line;
	auto entry = std::make_unique<PlaylistEntry>(m_streamType, theApp.GetAppPath(utils::PLUGIN_ROOT));
	while (std::getline(*pl_stream, line))
	{
		utils::string_rtrim(line, "\r");
		if (!entry->Parse(line)) continue;

		const auto& access_key = entry->get_uri_stream()->get_token();
		const auto& domain = entry->get_uri_stream()->get_domain();
		if (!access_key.empty() && !domain.empty() && access_key != L"00000000000000" && domain != L"localhost")
		{
			m_accessKey = access_key.c_str();
			m_domain = domain.c_str();
			m_status = _T("ok");
			break;
		}
	}

	UpdateData(FALSE);
}
