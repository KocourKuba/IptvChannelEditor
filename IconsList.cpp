// IconsList.cpp : implementation file
//

#include "StdAfx.h"
#include "IPTVChannelEditor.h"
#include "IconsList.h"
#include "afxdialogex.h"


// CIconsList dialog

IMPLEMENT_DYNAMIC(CIconsList, CDialogEx)

CIconsList::CIconsList(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_ICONS_LIST, pParent)
{
}

void CIconsList::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LIST_ICONS, m_wndIconsList);
}


BEGIN_MESSAGE_MAP(CIconsList, CDialogEx)
	ON_NOTIFY(HDN_GETDISPINFO, 0, &CIconsList::OnHdnGetdispinfoListIcons)
END_MESSAGE_MAP()


// CIconsList message handlers


BOOL CIconsList::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CIconsList::OnHdnGetdispinfoListIcons(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHDDISPINFO pDispInfo = reinterpret_cast<LPNMHDDISPINFO>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}
