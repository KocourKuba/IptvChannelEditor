#include "pch.h"
#include <afxdialogex.h>
#include "AccessInfoPinDlg.h"
#include "resource.h"
#include "PlayListEntry.h"

#include "UtilsLib\inet_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CAccessDlg dialog

IMPLEMENT_DYNAMIC(CAccessInfoPinDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CAccessInfoPinDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_GET, &CAccessInfoPinDlg::OnBnClickedBtnGet)
END_MESSAGE_MAP()


CAccessInfoPinDlg::CAccessInfoPinDlg(StreamType type, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_ACCESS_INFO_PIN, pParent), m_type(type)
{
}

void CAccessInfoPinDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_EMBED, m_bEmbed);
	DDX_Control(pDX, IDC_BUTTON_GET, m_wndGet);
	DDX_Control(pDX, IDC_COMBO_DEVICE_ID, m_wndDeviceID);
	DDX_Text(pDX, IDC_EDIT_PASSWORD, m_password);
	DDX_Text(pDX, IDC_EDIT_STATUS, m_status);
	DDX_Text(pDX, IDC_EDIT_SUBSCRIPTION, m_subscription);
	DDX_Text(pDX, IDC_EDIT_BALANCE, m_balance);
	DDX_Text(pDX, IDC_EDIT_DEVICE_INFO, m_dev_info);
}

BOOL CAccessInfoPinDlg::OnInitDialog()
{
	__super::OnInitDialog();

	m_password = m_entry->stream_uri->get_password().c_str();
	switch (m_type)
	{
		case StreamType::enItv:
			GetDlgItem(IDC_STATIC_SUBSCRIPTION)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_EDIT_SUBSCRIPTION)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_BALANCE)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_EDIT_BALANCE)->ShowWindow(SW_SHOW);
			break;
		case StreamType::enCbilling:
			GetDlgItem(IDC_STATIC_SUBSCRIPTION)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_EDIT_SUBSCRIPTION)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_BALANCE)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_EDIT_BALANCE)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_DEVICE_ID)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_COMBO_DEVICE_ID)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_DEVICE_INFO)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_EDIT_DEVICE_INFO)->ShowWindow(SW_SHOW);
			m_wndDeviceID.SetCurSel(m_device_id - 1);
			break;
		default:
			break;
	}

	UpdateData(FALSE);

	OnBnClickedBtnGet();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CAccessInfoPinDlg::OnOK()
{
	UpdateData(TRUE);

	m_entry->stream_uri->set_password(m_password.GetString());
	switch (m_type)
	{
		case StreamType::enCbilling:
			m_device_id = m_wndDeviceID.GetCurSel() + 1;
			break;
		default:
			break;
	}

	__super::OnOK();
}

void CAccessInfoPinDlg::OnBnClickedBtnGet()
{
	UpdateData(TRUE);

	m_status = _T("Unknown");

	// reset templated flag for new parse
	m_entry->stream_uri->set_template(false);

	if (m_type == StreamType::enItv || m_type == StreamType::enCbilling)
	{
		// currently supported only in ITV & Cbilling
		m_device_id = m_wndDeviceID.GetCurSel() + 1;
		std::wstring header;
		if (!m_entry->stream_uri->get_access_info_header().empty())
			header = fmt::format(m_entry->stream_uri->get_access_info_header(), m_password.GetString());

		std::vector<BYTE> data;
		if (!utils::DownloadFile(m_entry->stream_uri->get_access_url(L"", m_password.GetString()), data, &header) || data.empty())
			return;

		std::map<std::string, std::wstring> params;
		m_entry->stream_uri->parse_access_info(data, params);
		m_balance = params["balance"].c_str();
		m_subscription = params["subscription"].c_str();
		m_dev_info = params["devices_num"].c_str();
	}

	const auto& pl_url = fmt::format(m_entry->stream_uri->get_playlist_template(), m_password.GetString(), m_device_id);

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
