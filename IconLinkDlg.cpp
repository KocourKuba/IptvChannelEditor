/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2022): sharky72 (https://github.com/KocourKuba)

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
#include <afxdialogex.h>
#include "IconLinkDlg.h"
#include "IPTVChannelEditor.h"
#include "IconCache.h"


// CIconLink dialog

IMPLEMENT_DYNAMIC(CIconLinkDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CIconLinkDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_GET, &CIconLinkDlg::OnBnClickedButtonGet)
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()

CIconLinkDlg::CIconLinkDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_ICON_LINK, pParent)
	, m_url(_T(""))
{

}

CIconLinkDlg::~CIconLinkDlg()
{
}

BOOL CIconLinkDlg::OnInitDialog()
{
	__super::OnInitDialog();

	OnBnClickedButtonGet();

	return TRUE;
}

void CIconLinkDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_URL, m_url);
	DDX_Control(pDX, IDC_STATIC_ICON, m_wndIcon);
}

// CIconLink message handlers

void CIconLinkDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	CMFCDynamicLayout* layout = GetDynamicLayout();

	if (layout)
	{
		CSize size = layout->GetMinSize();
		CRect rect(0, 0, size.cx, size.cy);
		AdjustWindowRect(&rect, GetStyle(), FALSE);
		lpMMI->ptMinTrackSize.x = rect.Width();
		lpMMI->ptMinTrackSize.y = rect.Height();
	}

	__super::OnGetMinMaxInfo(lpMMI);
}

void CIconLinkDlg::OnBnClickedButtonGet()
{
	UpdateData(TRUE);

	if (m_url.IsEmpty())
	{
		m_wndIcon.SetBitmap(nullptr);
		return;
	}

	const auto& img = GetIconCache().get_icon(m_url.GetString());
	SetImage(img, m_wndIcon);
}
