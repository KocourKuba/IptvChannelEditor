// UrlDlg.cpp : implementation file
//

#include "pch.h"
#include "IPTVChannelEditor.h"
#include "UrlDlg.h"
#include "afxdialogex.h"


// CUrlDlg dialog

IMPLEMENT_DYNAMIC(CUrlDlg, CDialogEx)

CUrlDlg::CUrlDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_URL, pParent)
{
}

void CUrlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_EDIT_PLAYLIST_URL, m_wndUrl);
	DDX_Text(pDX, IDC_EDIT_PLAYLIST_URL, m_url);
}


BEGIN_MESSAGE_MAP(CUrlDlg, CDialogEx)
END_MESSAGE_MAP()


// CUrlDlg message handlers
