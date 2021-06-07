// SettingsDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "EdemChannelEditor.h"
#include "AccessDlg.h"
#include "afxdialogex.h"


// CAccessDlg dialog

IMPLEMENT_DYNAMIC(CAccessDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CAccessDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK_GLOBAL, &CAccessDlg::OnBnClickedCheckGlobal)
END_MESSAGE_MAP()


CAccessDlg::CAccessDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_ACCESS_INFO, pParent)
	, m_bEmbedded(FALSE)
{

}

void CAccessDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_KEY, m_accessKey);
	DDX_Text(pDX, IDC_EDIT_DOMAIN, m_domain);
	DDX_Check(pDX, IDC_CHECK_GLOBAL, m_bEmbedded);
}

BOOL CAccessDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CAccessDlg::OnBnClickedCheckGlobal()
{
	UpdateData(TRUE);

	if (!m_bEmbedded)
	{
		m_accessKey = theApp.GetProfileString(_T("Setting"), _T("AccessKey"));
		m_domain = theApp.GetProfileString(_T("Setting"), _T("Domain"));
	}

	UpdateData(FALSE);
}
