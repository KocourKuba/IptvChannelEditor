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
#include <afxdialogex.h>
#include "resource.h"
#include "CustomPlaylistDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CCustomPlaylist dialog

IMPLEMENT_DYNAMIC(CCustomPlaylistDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CCustomPlaylistDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK_FILE, &CCustomPlaylistDlg::OnBnClickedCheckFile)
END_MESSAGE_MAP()

CCustomPlaylistDlg::CCustomPlaylistDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CUSTOM_PLAYLIST, pParent)
{
}

void CCustomPlaylistDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_CUSTOM_PLAYLIST, m_wndUrl);
	DDX_Text(pDX, IDC_CUSTOM_PLAYLIST, m_url);
	DDX_Check(pDX, IDC_CHECK_FILE, m_isFile);
}

BOOL CCustomPlaylistDlg::OnInitDialog()
{
	__super::OnInitDialog();

	OnBnClickedCheckFile();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CCustomPlaylistDlg::OnBnClickedCheckFile()
{
	UpdateData(TRUE);

	if (m_isFile)
	{
		CString filter;
		filter.LoadString(IDS_STRING_LOAD_PLAYLIST);
		m_wndUrl.EnableFileBrowseButton(_T("*.m3u8"),
										filter,
										OFN_EXPLORER | OFN_ENABLESIZING | OFN_LONGNAMES | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NONETWORKBUTTON);
	}
	else
	{
		m_wndUrl.EnableBrowseButton(FALSE);
	}
}
