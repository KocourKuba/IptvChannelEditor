#include "StdAfx.h"
#include <afxdialogex.h>
#include "AccessInfoPinDlg.h"
#include "resource.h"
#include "utils.h"
#include "json.hpp"
#include "PlayListEntry.h"

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
}

BOOL CAccessInfoPinDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_password = m_entry->stream_uri->get_password().c_str();

	UpdateData(FALSE);

	OnBnClickedBtnGet();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CAccessInfoPinDlg::OnOK()
{
	UpdateData(TRUE);

	m_entry->stream_uri->set_password(utils::utf16_to_utf8(m_password.GetString()));

	__super::OnOK();
}

void CAccessInfoPinDlg::OnBnClickedBtnGet()
{
	UpdateData(TRUE);

	m_status = _T("Unknown");

	const auto& password = utils::utf16_to_utf8(m_password.GetString());
	const auto& pl_url = fmt::format(m_entry->stream_uri->get_playlist_template(), password.c_str());

	// reset templated flag for new parse
	m_entry->stream_uri->set_template(false);

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
				if (m_entry->Parse(line) && !m_entry->stream_uri->get_token().empty())
				{
					m_status = _T("Ok");
					break;
				}
			}
		}
	}

	UpdateData(FALSE);
}
