#include "pch.h"
#include "resource.h"		// main symbols
#include "AboutDlg.h"

#include "UtilsLib\utils.h"
#include "UtilsLib\inet_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

CAboutDlg::CAboutDlg() : CDialog(IDD_ABOUTBOX)
{
}

BOOL CAboutDlg::OnInitDialog()
{
	__super::OnInitDialog();

	m_version.Format(_T("Version %d.%d.%d"), MAJOR, MINOR, BUILD);

	utils::CBase64Coder enc;
	CString str;
	str.LoadString(IDS_STRING_PAYPAL);
	std::string encoded = utils::utf16_to_utf8(str.GetString(), str.GetLength());
	enc.Decode(encoded.c_str(), encoded.size());
	// https://www.paypal.com/donate/?cmd=_donations&business=5DY7PESZL4D8L&currency_code=USD&amount=5
	m_paypal.SetURL(utils::utf8_to_utf16(enc.GetResultString()).c_str());

	str.LoadString(IDS_STRING_YOOMONEY);
	encoded = utils::utf16_to_utf8(str.GetString(), str.GetLength());
	enc.Decode(encoded.c_str(), encoded.size());
	// https://yoomoney.ru/to/41001913379027
	m_yoomoney.SetURL(utils::utf8_to_utf16(enc.GetResultString()).c_str());

	UpdateData(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_VERSION, m_version);
	DDX_Control(pDX, IDC_MFCLINK_DONATE_PAYPAL, m_paypal);
	DDX_Control(pDX, IDC_MFCLINK_DONATE_YOOMONEY, m_yoomoney);
}

void CAboutDlg::OnOK()
{
	__super::OnOK();
}
