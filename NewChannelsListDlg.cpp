/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2024): sharky72 (https://github.com/KocourKuba)

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
#include "NewChannelsListDlg.h"


// CNewChannelsListDlg dialog

IMPLEMENT_DYNAMIC(CNewChannelsListDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CNewChannelsListDlg, CDialogEx)
END_MESSAGE_MAP()

CNewChannelsListDlg::CNewChannelsListDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_NEW_CHANNELS_LIST, pParent)
{
}

void CNewChannelsListDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_NAME, m_name);
	DDX_Check(pDX, IDC_CHECK_MAKE_COPY, m_MakeCopy);
}

void CNewChannelsListDlg::OnOK()
{
	UpdateData(TRUE);

	int len = m_name.GetLength();
	if (!len) return;

	for (int i = 0; i < len; i++)
	{
		if (m_name[i] > 127)
		{
			AfxMessageBox(IDS_STRING_WRN_NON_ASCII_LIST, MB_ICONERROR | MB_OK);
			return;
		}
	}

	__super::OnOK();
}
