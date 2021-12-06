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

	DDX_Control(pDX, IDC_COMBO_STRING, m_wndFilterString);
	DDX_Control(pDX, IDC_CHECK_REGEX, m_wndFilterRegex);
	DDX_Check(pDX, IDC_CHECK_REGEX, m_filterRegex);
	DDX_Control(pDX, IDC_CHECK_CASE, m_wndFilterCase);
	DDX_Check(pDX, IDC_CHECK_CASE, m_filterCase);
}

BOOL CFilterDialog::OnInitDialog()
{
	__super::OnInitDialog();

	CString stringList = GetConfig().get_string(false, REG_FILTER_STRING_LST).c_str();
	CString filterString = GetConfig().get_string(false, REG_FILTER_STRING).c_str();

	m_filterRegex  = GetConfig().get_int(false, REG_FILTER_REGEX);
	m_filterCase   = GetConfig().get_int(false, REG_FILTER_CASE);

	m_wndFilterString.LoadHistoryFromText(stringList, filterString);

	UpdateData(FALSE);

	OnUpdateControls();

	return TRUE;
}

void CFilterDialog::OnOK()
{
	UpdateData(TRUE);

	CString filterString;
	m_wndFilterString.GetWindowText(filterString);

	try
	{
		// Check expression
		std::wregex re(filterString.GetString());
		UNUSED_ALWAYS(re);
	}
	catch (std::regex_error& ex)
	{
		CString error;
		error.Format(_T("Error in regular expression: %hs"), ex.what());
		AfxMessageBox(error, MB_OK | MB_ICONERROR);
		return;
	}

	GetConfig().set_string(false, REG_FILTER_STRING, filterString.GetString());
	GetConfig().set_string(false, REG_FILTER_STRING_LST, m_wndFilterString.SaveHistoryToText().GetString());
	GetConfig().set_int(false, REG_FILTER_REGEX, m_filterRegex);
	GetConfig().set_int(false, REG_FILTER_CASE, m_filterCase);

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
