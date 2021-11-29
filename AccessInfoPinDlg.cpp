#include "pch.h"
#include <afxdialogex.h>
#include "AccessInfoPinDlg.h"
#include "resource.h"
#include "json.hpp"
#include "utils.h"
#include "PlayListEntry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using json = nlohmann::json;

// CAccessDlg dialog

IMPLEMENT_DYNAMIC(CAccessInfoPinDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CAccessInfoPinDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_GET, &CAccessInfoPinDlg::OnBnClickedBtnGet)
END_MESSAGE_MAP()


CAccessInfoPinDlg::CAccessInfoPinDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_ACCESS_INFO_PIN, pParent)
{
}

void CAccessInfoPinDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_EMBED, m_bEmbed);
	DDX_Control(pDX, IDC_BUTTON_GET, m_wndGet);
	DDX_Text(pDX, IDC_EDIT_PASSWORD, m_password);
	DDX_Text(pDX, IDC_EDIT_STATUS, m_status);
	DDX_Text(pDX, IDC_EDIT_SUBSCRIPTION, m_subscription);
	DDX_Text(pDX, IDC_EDIT_BALANCE, m_balance);
}

BOOL CAccessInfoPinDlg::OnInitDialog()
{
	__super::OnInitDialog();

	m_password = m_entry->stream_uri->get_password().c_str();

	UpdateData(FALSE);

	OnBnClickedBtnGet();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CAccessInfoPinDlg::OnOK()
{
	UpdateData(TRUE);

	m_entry->stream_uri->set_password(m_password.GetString());

	__super::OnOK();
}

void CAccessInfoPinDlg::OnBnClickedBtnGet()
{
	UpdateData(TRUE);

	m_status = _T("Unknown");

	// reset templated flag for new parse
	m_entry->stream_uri->set_template(false);

	if (m_entry->stream_uri->isHasAccessInfo())
	{
		// currently supported only in ITV
		std::vector<BYTE> data;
		if (!utils::DownloadFile(m_entry->stream_uri->get_access_url(L"", m_password.GetString()), data) || data.empty())
			return;

		try
		{
			json js = json::parse(data);

			json js_data = js["user_info"];

			m_balance.Format(_T("%hs $"), js_data.value("cash", "").c_str());

			if (!js.contains("package_info"))
			{
				m_subscription = _T("No packages");
			}
			else
			{
				json pkg_data = js["package_info"];
				for (const auto& item : pkg_data)
				{
					if (!m_subscription.IsEmpty())
						m_subscription += _T(", ");

					m_subscription.AppendFormat(_T("%s"), utils::utf8_to_utf16(item.value("name", "")).c_str());
				}
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

	const auto& pl_url = fmt::format(m_entry->stream_uri->get_playlist_template(), m_password.GetString());

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
				if (m_entry->Parse(line, m3uEntry) && !m_entry->stream_uri->get_token().empty())
				{
					m_status = _T("Ok");
					break;
				}
			}
		}
	}

	UpdateData(FALSE);
}
