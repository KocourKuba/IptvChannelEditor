#include "StdAfx.h"
#include <afxdialogex.h>
#include "AccessInfoPassDlg.h"
#include "resource.h"
#include "utils.h"
#include "json.hpp"
#include "PlayListEntry.h"

using json = nlohmann::json;

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
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_EDIT_LOGIN, m_wndLogin);
	DDX_Control(pDX, IDC_EDIT_PASSWORD, m_wndPassword);
	DDX_Control(pDX, IDC_BUTTON_GET, m_wndGet);
	DDX_Check(pDX, IDC_CHECK_EMBED, m_bEmbed);
	DDX_Text(pDX, IDC_EDIT_LOGIN, m_login);
	DDX_Text(pDX, IDC_EDIT_PASSWORD, m_password);
	DDX_Text(pDX, IDC_EDIT_STATUS, m_status);
	DDX_Text(pDX, IDC_EDIT_SUBSCRIPTION, m_subscription);
	DDX_Text(pDX, IDC_EDIT_BALANCE, m_balance);
	DDX_Text(pDX, IDC_EDIT_PACKAGES_PRICE, m_packages_price);
}

BOOL CAccessInfoPassDlg::OnInitDialog()
{
	__super::OnInitDialog();

	m_login = m_entry->stream_uri->get_login().c_str();
	m_password = m_entry->stream_uri->get_password().c_str();

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

	__super::OnOK();
}

void CAccessInfoPassDlg::OnBnClickedBtnGet()
{
	UpdateData(TRUE);

	m_status = _T("Unknown");

	const auto& login = m_login.GetString();
	const auto& password = m_password.GetString();

	// reset templated flag for new parse
	m_entry->stream_uri->set_template(false);

	if (m_entry->stream_uri->isHasAccessInfo())
	{
		// currently supported only in sharaclub
		std::vector<BYTE> data;
		if (!utils::DownloadFile(m_entry->stream_uri->get_access_url(login, password), data) || data.empty())
			return;

		try
		{
			json js = json::parse(data);
			json js_data = js["data"];

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

	const auto& pl_url = fmt::format(m_entry->stream_uri->get_playlist_template(), login, password);

	std::vector<BYTE> data;
	std::unique_ptr<std::istream> pl_stream;
	if (!pl_url.empty() && utils::DownloadFile(pl_url, data))
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
				m3u_entry m3uEntry(line);
				if (m_entry->Parse(line, m3uEntry) && !m_entry->stream_uri->get_login().empty() && !m_entry->stream_uri->get_token().empty())
				{
					m_status = _T("Ok");
					break;
				}
			}
		}
	}

	UpdateData(FALSE);
}
