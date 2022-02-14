// SettingsDlg.cpp : implementation file
//

#include "pch.h"
#include <afxdialogex.h>
#include "IPTVChannelEditor.h"
#include "AccessOttKeyDlg.h"
#include "PlayListEntry.h"

#include "UtilsLib\inet_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CAccessDlg dialog

IMPLEMENT_DYNAMIC(CAccessOttKeyDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CAccessOttKeyDlg, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT_PLAYLIST_URL, &CAccessOttKeyDlg::OnEnChangeEditPlaylistUrl)
	ON_BN_CLICKED(IDC_BUTTON_GET, &CAccessOttKeyDlg::OnBnClickedBtnGet)
	ON_BN_CLICKED(IDC_CHECK_EMBED_VP, &CAccessOttKeyDlg::OnBnClickedCheckEmbedVp)
END_MESSAGE_MAP()

CAccessOttKeyDlg::CAccessOttKeyDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_ACCESS_INFO, pParent)
	, m_streamType(StreamType::enEdem)
{

}

void CAccessOttKeyDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_EMBED, m_bEmbed);
	DDX_Control(pDX, IDC_EDIT_PLAYLIST_URL, m_wndUrl);
	DDX_Control(pDX, IDC_BUTTON_GET, m_wndGet);
	DDX_Text(pDX, IDC_EDIT_KEY, m_accessKey);
	DDX_Text(pDX, IDC_EDIT_DOMAIN, m_domain);
	DDX_Text(pDX, IDC_EDIT_PLAYLIST_URL, m_url);
	DDX_Text(pDX, IDC_EDIT_VP_KEY, m_vportal);
	DDX_Control(pDX, IDC_EDIT_VP_KEY, m_wndVPortal);
	DDX_Check(pDX, IDC_CHECK_EMBED_VP, m_bEmbed_vp);
}

BOOL CAccessOttKeyDlg::OnInitDialog()
{
	__super::OnInitDialog();

	OnBnClickedCheckEmbedVp();

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
	if (!utils::DownloadFile(m_url.GetString(), data))
	{
		std::ifstream instream(m_url.GetString());
		data.assign((std::istreambuf_iterator<char>(instream)), std::istreambuf_iterator<char>());
	}

	const auto& wbuf = utils::utf8_to_utf16((char*)data.data(), data.size());
	std::wistringstream stream(wbuf);
	if (!stream.good()) return;

	auto entry = std::make_unique<PlaylistEntry>(m_streamType, GetAppPath(utils::PLUGIN_ROOT));
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
			m_accessKey = access_key.c_str();
			m_domain = domain.c_str();
			m_status = _T("Ok");
			break;
		}
	}

	UpdateData(FALSE);
}

void CAccessOttKeyDlg::OnBnClickedCheckEmbedVp()
{
	UpdateData(TRUE);

	m_wndVPortal.EnableWindow(m_bEmbed_vp);
}
