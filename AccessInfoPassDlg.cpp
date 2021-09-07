#include "StdAfx.h"
#include "AccessInfoPassDlg.h"
#include "IPTVChannelEditor.h"
#include "utils.h"
#include "json.hpp"
#include "PlayListEntry.h"

using json = nlohmann::json;

static constexpr auto ACCESS_TEMPLATE_SHARACLUB = "http://list.playtv.pro/api/dune-api5m.php?subscr={:s}-{:s}";
static constexpr auto PLAYLIST_TEMPLATE_SHARACLUB = "http://list.playtv.pro/tv_live-m3u8/{:s}-{:s}";
static constexpr auto PLAYLIST_TEMPLATE_GLANZ = "http://pl.ottglanz.tv/get.php?username={:s}&password={:s}&type=m3u&output=hls";
static constexpr auto PLAYLIST_TEMPLATE_ANTIFRIZ = "https://antifriz.tv/playlist/{:s}.m3u8";

// CAccessDlg dialog

IMPLEMENT_DYNAMIC(CAccessInfoPassDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CAccessInfoPassDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_GET, &CAccessInfoPassDlg::OnBnClickedBtnGet)
END_MESSAGE_MAP()


CAccessInfoPassDlg::CAccessInfoPassDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_ACCESS_INFO_PASS, pParent)
{
}

void CAccessInfoPassDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_EDIT_LOGIN, m_wndLogin);
	DDX_Text(pDX, IDC_EDIT_LOGIN, m_login);
	DDX_Control(pDX, IDC_EDIT_PASSWORD, m_wndPassword);
	DDX_Text(pDX, IDC_EDIT_PASSWORD, m_password);
	DDX_Text(pDX, IDC_EDIT_STATUS, m_status);
	DDX_Text(pDX, IDC_EDIT_SUBSCRIPTION, m_subscription);
	DDX_Text(pDX, IDC_EDIT_BALANCE, m_balance);
	DDX_Text(pDX, IDC_EDIT_PACKAGES_PRICE, m_packages_price);
	DDX_Control(pDX, IDC_BUTTON_GET, m_wndGet);
}

BOOL CAccessInfoPassDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_login = m_entry->get_stream_uri()->get_login().c_str();
	m_password = m_entry->get_stream_uri()->get_password().c_str();

	if (m_entry->get_stream_type() == StreamType::enAntifriz)
	{
		m_wndLogin.EnableWindow(FALSE);
	}

	UpdateData(FALSE);

	OnBnClickedBtnGet();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CAccessInfoPassDlg::OnOK()
{
	UpdateData(TRUE);

	m_entry->get_stream_uri()->set_login(utils::utf16_to_utf8(m_login.GetString()));
	m_entry->get_stream_uri()->set_password(utils::utf16_to_utf8(m_password.GetString()));

	__super::OnOK();
}

void CAccessInfoPassDlg::OnBnClickedBtnGet()
{
	UpdateData(TRUE);

	const auto& login = utils::utf16_to_utf8(m_login.GetString());
	const auto& password = utils::utf16_to_utf8(m_password.GetString());

	if (m_entry->get_stream_type() == StreamType::enSharaclub)
	{
		const auto& access_url = fmt::format(ACCESS_TEMPLATE_SHARACLUB, login.c_str(), password.c_str());
		std::vector<BYTE> data;
		if (!utils::DownloadFile(access_url, data) || data.empty())
			return;

		try
		{
			json js = json::parse(data);
			json js_data = js["data"];

			m_status = js.value("status", "").c_str();
			if (m_status == _T("ok"))
			{
				m_subscription = utils::utf8_to_utf16(js_data.value("abon", "")).c_str();
				m_balance.Format(_T("%hs RUR"), js_data.value("money", "").c_str());
				m_packages_price.Format(_T("%hs RUR"), js_data.value("money_need", "").c_str());
			}
		}
		catch (const json::parse_error&)
		{
			// parse errors are ok, because input may be random bytes
		}
		catch (const json::out_of_range&)
		{
			// out of range errors may happen if provided sizes are excessive
		}
	}

	LPCSTR tpl = nullptr;
	std::string pl_url;
	switch (m_entry->get_stream_type())
	{
		case StreamType::enSharaclub:
			pl_url = fmt::format(PLAYLIST_TEMPLATE_SHARACLUB, login.c_str(), password.c_str());
			break;
		case StreamType::enGlanz:
			pl_url = fmt::format(PLAYLIST_TEMPLATE_GLANZ, login.c_str(), password.c_str());
			break;
		case StreamType::enAntifriz:
			pl_url = fmt::format(PLAYLIST_TEMPLATE_ANTIFRIZ, password.c_str());
			break;
		default:
			return;
	}

	std::vector<BYTE> data;
	std::unique_ptr<std::istream> pl_stream;
	if (utils::DownloadFile(pl_url, data))
	{
		utils::vector_to_streambuf<char> buf(data);
		pl_stream = std::make_unique<std::istream>(&buf);

		if (pl_stream && pl_stream->good())
		{
			int step = 0;
			std::string line;
			while (std::getline(*pl_stream, line))
			{
				utils::string_rtrim(line, "\r");
				if (m_entry->Parse(line) && !m_entry->get_stream_uri()->get_token().empty())
				{
					m_status = _T("Ok");
					break;
				}
			}
		}
	}

	UpdateData(FALSE);
}
