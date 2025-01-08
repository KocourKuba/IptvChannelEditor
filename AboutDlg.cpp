/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2025): sharky72 (https://github.com/KocourKuba)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "pch.h"
#include "resource.h"		// main symbols
#include "IPTVChannelEditor.h"
#include "AboutDlg.h"

#include "UtilsLib\utils.h"
#include "UtilsLib\inet_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
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
	std::string encoded = load_string_resource_a(IDS_STRING_PAYPAL);
	enc.Decode(encoded.c_str(), (int)encoded.size());
	//https://www.paypal.com/donate/?business=5DY7PESZL4D8L&amount=5&no_recurring=0&currency_code=USD
	m_paypal.SetURL(utils::utf8_to_utf16(enc.GetResultString()).c_str());

	encoded = load_string_resource_a(IDS_STRING_YOOMONEY);
	enc.Decode(encoded.c_str(), (int)encoded.size());
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
