// NewChannelDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "EdemChannelEditor.h"
#include "NewChannelDlg.h"

// NewChannel dialog

IMPLEMENT_DYNAMIC(CNewChannelDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CNewChannelDlg, CDialogEx)
END_MESSAGE_MAP()

CNewChannelDlg::CNewChannelDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_NEW_CHANNEL, pParent)
{
}

void CNewChannelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_CATEGORY, m_name);
}

BOOL CNewChannelDlg::OnInitDialog()
{
	__super::OnInitDialog();

	return TRUE;
}
