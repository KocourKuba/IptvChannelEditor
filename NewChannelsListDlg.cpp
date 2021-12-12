// NewChannelsList.cpp : implementation file
//

#include "pch.h"
#include "resource.h"
#include "NewChannelsListDlg.h"
#include "afxdialogex.h"


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
