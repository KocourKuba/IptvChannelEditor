// FilterDialog.cpp : implementation file
//

#include "pch.h"
#include <afxdialogex.h>
#include "IPTVChannelEditor.h"
#include "FilterDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CFilterDialog dialog

IMPLEMENT_DYNAMIC(CFilterDialog, CDialogEx)

BEGIN_MESSAGE_MAP(CFilterDialog, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK_REGEX, &CFilterDialog::OnUpdateControls)
	ON_BN_CLICKED(IDC_CHECK_CASE, &CFilterDialog::OnUpdateControls)
END_MESSAGE_MAP()

CFilterDialog::CFilterDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_FILTER, pParent)
{

}

void CFilterDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_STRING, m_filterString);
	DDX_Control(pDX, IDC_CHECK_REGEX, m_wndFilterRegex);
	DDX_Check(pDX, IDC_CHECK_REGEX, m_filterRegex);
	DDX_Control(pDX, IDC_CHECK_CASE, m_wndFilterCase);
	DDX_Check(pDX, IDC_CHECK_CASE, m_filterCase);
	DDX_Check(pDX, IDC_CHECK_NOT_ADDED, m_filterNotAdded);
}

BOOL CFilterDialog::OnInitDialog()
{
	__super::OnInitDialog();

	OnUpdateControls();

	return TRUE;
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


void CFilterDialog::OnUpdateControls()
{
	UpdateData(TRUE);

	if (m_filterRegex)
	{
		m_filterCase = FALSE;
		m_wndFilterCase.EnableWindow(FALSE);
	}
	m_wndFilterCase.EnableWindow(!m_filterRegex);

	if (m_filterCase)
	{
		m_filterRegex = FALSE;
		m_wndFilterRegex.EnableWindow(FALSE);
	}
	m_wndFilterRegex.EnableWindow(!m_filterCase);

	UpdateData(FALSE);
}
