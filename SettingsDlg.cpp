// SettingsDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "EdemChannelEditor.h"
#include "SettingsDlg.h"
#include "afxdialogex.h"


// CSettingsDlg dialog

IMPLEMENT_DYNAMIC(CSettingsDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CSettingsDlg, CDialogEx)
END_MESSAGE_MAP()


CSettingsDlg::CSettingsDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_SETTINGS, pParent)
{

}

void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_MFCEDITBROWSE_PLAYER, m_player);
	DDX_Text(pDX, IDC_EDIT_KEY, m_accessKey);
	DDX_Text(pDX, IDC_EDIT_DOMAIN, m_domain);
}

BOOL CSettingsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_player = theApp.GetProfileString(_T("Setting"), _T("Player"));
	m_accessKey = theApp.GetProfileString(_T("Setting"), _T("AccessKey"));
	m_domain = theApp.GetProfileString(_T("Setting"), _T("Domain"));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CSettingsDlg::OnOK()
{
	CDialogEx::OnOK();

	theApp.WriteProfileString(_T("Setting"), _T("Player"), m_player);
	theApp.WriteProfileString(_T("Setting"), _T("AccessKey"), m_accessKey);
	theApp.WriteProfileString(_T("Setting"), _T("Domain"), m_domain);
}
