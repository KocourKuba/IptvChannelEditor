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

constexpr auto REG_FILTER_REGEX     = _T("FilterUseRegex");
constexpr auto REG_FILTER_CASE      = _T("FilterUseCase");
constexpr auto REG_FILTER_NOT_ADDED = _T("FilterNotAdded");

// CFilterDialog dialog

IMPLEMENT_DYNAMIC(CFilterDialog, CDialogEx)

BEGIN_MESSAGE_MAP(CFilterDialog, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK_REGEX, &CFilterDialog::OnUpdateControls)
	ON_BN_CLICKED(IDC_CHECK_CASE, &CFilterDialog::OnUpdateControls)
	ON_BN_CLICKED(IDC_CHECK_NOT_ADDED, &CFilterDialog::OnUpdateControls)
	ON_BN_CLICKED(IDC_CHECK_CHANGED, &CFilterDialog::OnUpdateControls)
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
	DDX_Control(pDX, IDC_CHECK_NOT_ADDED, m_wndFilterNotAdded);
	DDX_Check(pDX, IDC_CHECK_NOT_ADDED, m_filterNotAdded);
	DDX_Control(pDX, IDC_CHECK_CHANGED, m_wndFilterChanged);
	DDX_Check(pDX, IDC_CHECK_CHANGED, m_filterChanged);
}

BOOL CFilterDialog::OnInitDialog()
{
	__super::OnInitDialog();

	m_filterString = GetConfig().get_string(false, REG_FILTER_STRING).c_str();
	int flags = GetConfig().get_int(false, REG_FILTER_FLAGS);
	m_filterRegex    = (flags & FILTER_FLAG_REGEX) ? TRUE : FALSE;
	m_filterCase     = (flags & FILTER_FLAG_CASE) ? TRUE : FALSE;
	m_filterNotAdded = (flags & FILTER_FLAG_NOT_ADDED) ? TRUE : FALSE;
	m_filterChanged  = (flags & FILTER_FLAG_CHANGED) ? TRUE : FALSE;

	UpdateData(FALSE);

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

	int flags = 0;
	flags |= m_filterRegex ? FILTER_FLAG_REGEX : 0;
	flags |= m_filterCase ? FILTER_FLAG_CASE : 0;
	flags |= m_filterNotAdded ? FILTER_FLAG_NOT_ADDED : 0;
	flags |= m_filterChanged ? FILTER_FLAG_CHANGED : 0;

	GetConfig().set_string(false, REG_FILTER_STRING, m_filterString.GetString());
	GetConfig().set_int(false, REG_FILTER_FLAGS, flags);

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

	if (m_filterNotAdded)
	{
		m_filterChanged = FALSE;
		m_wndFilterChanged.EnableWindow(FALSE);
	}
	m_wndFilterChanged.EnableWindow(!m_filterNotAdded);

	if (m_filterChanged)
	{
		m_filterNotAdded = FALSE;
		m_wndFilterNotAdded.EnableWindow(FALSE);
	}
	m_wndFilterNotAdded.EnableWindow(!m_filterChanged);

	UpdateData(FALSE);
}
