// FilterDialog.cpp : implementation file
//

#include "StdAfx.h"
#include <afxdialogex.h>
#include "EdemChannelEditor.h"
#include "FilterDialog.h"


// CFilterDialog dialog

IMPLEMENT_DYNAMIC(CFilterDialog, CDialogEx)

BEGIN_MESSAGE_MAP(CFilterDialog, CDialogEx)
END_MESSAGE_MAP()

CFilterDialog::CFilterDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_FILTER, pParent)
{

}

void CFilterDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_STRING, m_filterString);
	DDX_Check(pDX, IDC_CHECK_REGEX, m_filterRegex);
	DDX_Check(pDX, IDC_CHECK_CASE, m_filterCase);
}

BOOL CFilterDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_filterString = theApp.GetProfileString(_T("Setting"), _T("FilterString"));
	m_filterRegex = theApp.GetProfileInt(_T("Setting"), _T("FilterUseRegex"), FALSE);
	m_filterCase = theApp.GetProfileInt(_T("Setting"), _T("FilterUseCase"), FALSE);

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CFilterDialog::OnOK()
{
	UpdateData(TRUE);

	try
	{
		std::wregex re(m_filterString.GetString());
	}
	catch (std::regex_error& ex)
	{
		CString error;
		error.Format(_T("Error in regular expression: %hs"), ex.what());
		AfxMessageBox(error, MB_OK | MB_ICONERROR);
		return;
	}

	theApp.WriteProfileString(_T("Setting"), _T("FilterString"), m_filterString);
	theApp.WriteProfileInt(_T("Setting"), _T("FilterUseRegex"), m_filterRegex);
	theApp.WriteProfileInt(_T("Setting"), _T("FilterUseCase"), m_filterCase);

	__super::OnOK();
}


