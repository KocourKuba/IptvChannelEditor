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

void CFilterDialog::OnOK()
{
	UpdateData(TRUE);

	try
	{
		// Check expression
		std::wregex re(m_filterString.GetString());
		UNUSED_ALWAYS(re);
	}
	catch (std::regex_error& ex)
	{
		CString error;
		error.Format(_T("Error in regular expression: %hs"), ex.what());
		AfxMessageBox(error, MB_OK | MB_ICONERROR);
		return;
	}

	__super::OnOK();
}


