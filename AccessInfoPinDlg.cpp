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
	ON_EN_CHANGE(IDC_EDIT_PASSWORD, &CAccessInfoPinDlg::OnEnChangeEditLogin)
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
	DDX_Control(pDX, IDC_LIST_INFO, m_wndInfo);
	DDX_Text(pDX, IDC_EDIT_PASSWORD, m_password);
}

BOOL CAccessInfoPinDlg::OnInitDialog()
{
	__super::OnInitDialog();

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

	m_password = m_entry->stream_uri->get_password().c_str();

	switch (m_type)
	{
		case StreamType::enCbilling:
		case StreamType::enShuraTV:
		{
			int max_etry = 3;
			CString text;
			text.LoadString(IDS_STRING_DEVICE_ID);
			if (m_type == StreamType::enShuraTV)
			{
				max_etry = 2;
				text.LoadString(IDS_STRING_SERVER_ID);
				GetDlgItem(IDC_STATIC_DEVICE_ID)->SetWindowText(text);
			}

			GetDlgItem(IDC_STATIC_DEVICE_ID)->ShowWindow(SW_SHOW);
			for (int i = 1; i <= max_etry; i++)
			{
				text.Format(_T("%d"), i);
				m_wndDeviceID.AddString(text);
			}
			m_wndDeviceID.SetCurSel(m_device_id - 1);
			m_wndDeviceID.ShowWindow(SW_SHOW);
			break;
		}
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
		case StreamType::enShuraTV:
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

	m_status.LoadString(IDS_STRING_STATUS_TEXT);

	// reset templated flag for new parse
	m_entry->stream_uri->set_template(false);
	m_device_id = m_wndDeviceID.GetCurSel() + 1;

	const auto& pl_url = fmt::format(m_entry->stream_uri->get_playlist_template(), m_password.GetString(), m_device_id);

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
				if (m_entry->Parse(line, m3uEntry) && !m_entry->stream_uri->get_token().empty())
				{
					m_status = _T("Ok");
					break;
				}
			}
		}
	}

	m_wndInfo.DeleteAllItems();

	CString txt;
	txt.LoadString(IDS_STRING_STATUS);
	int idx = 0;
	m_wndInfo.InsertItem(idx, txt);
	m_wndInfo.SetItemText(idx++, 1, m_status);

	if (m_type == StreamType::enItv || m_type == StreamType::enCbilling)
	{
		// currently supported only in ITV & Cbilling
		std::wstring header;
		if (!m_entry->stream_uri->get_access_info_header().empty())
			header = fmt::format(m_entry->stream_uri->get_access_info_header(), m_password.GetString());

		std::vector<BYTE> data;
		if (utils::DownloadFile(m_entry->stream_uri->get_access_url(L"", m_password.GetString()), data, &header) && !data.empty())
		{
			std::list<AccountParams> params;
			m_entry->stream_uri->parse_access_info(data, params);
			for (const auto& item : params)
			{
				m_wndInfo.InsertItem(idx, item.name.c_str());
				m_wndInfo.SetItemText(idx++, 1, item.value.c_str());
			}
		}
	}

	GetDlgItem(IDOK)->EnableWindow(TRUE);

	UpdateData(FALSE);
}

void CAccessInfoPinDlg::OnEnChangeEditLogin()
{
	GetDlgItem(IDOK)->EnableWindow(FALSE);
}
