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
	ON_BN_CLICKED(IDC_CHECK_REGEX_SHOW, &CFilterDialog::OnUpdateControls)
	ON_BN_CLICKED(IDC_CHECK_CASE_SHOW, &CFilterDialog::OnUpdateControls)
	ON_BN_CLICKED(IDC_CHECK_REGEX_HIDE, &CFilterDialog::OnUpdateControls)
	ON_BN_CLICKED(IDC_CHECK_CASE_HIDE, &CFilterDialog::OnUpdateControls)
END_MESSAGE_MAP()

CFilterDialog::CFilterDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_FILTER, pParent)
{
}

void CFilterDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_SHOW_MATCHED, m_filterState[0]);
	DDX_Control(pDX, IDC_COMBO_STRING_SHOW, m_wndFilterString[0]);
	DDX_Control(pDX, IDC_CHECK_REGEX_SHOW, m_wndFilterRegex[0]);
	DDX_Check(pDX, IDC_CHECK_REGEX_SHOW, m_filterRegex[0]);
	DDX_Control(pDX, IDC_CHECK_CASE_SHOW, m_wndFilterCase[0]);
	DDX_Check(pDX, IDC_CHECK_CASE_SHOW, m_filterCase[0]);

	DDX_Check(pDX, IDC_CHECK_HIDE_MATCHED, m_filterState[1]);
	DDX_Control(pDX, IDC_COMBO_STRING_HIDE, m_wndFilterString[1]);
	DDX_Control(pDX, IDC_CHECK_REGEX_HIDE, m_wndFilterRegex[1]);
	DDX_Check(pDX, IDC_CHECK_REGEX_HIDE, m_filterRegex[1]);
	DDX_Control(pDX, IDC_CHECK_CASE_HIDE, m_wndFilterCase[1]);
	DDX_Check(pDX, IDC_CHECK_CASE_HIDE, m_filterCase[1]);
}

BOOL CFilterDialog::OnInitDialog()
{
	__super::OnInitDialog();

	m_filterState[0] = GetConfig().get_int(false, REG_FILTER_STATE_S);
	m_filterRegex[0] = GetConfig().get_int(false, REG_FILTER_REGEX_S);
	m_filterCase[0] = GetConfig().get_int(false, REG_FILTER_CASE_S);
	m_wndFilterString[0].LoadHistoryFromText(GetConfig().get_string(false, REG_FILTER_STRING_LST_S).c_str(),
											 GetConfig().get_string(false, REG_FILTER_STRING_S).c_str());

	m_filterState[1] = GetConfig().get_int(false, REG_FILTER_STATE_H);
	m_filterRegex[1] = GetConfig().get_int(false, REG_FILTER_REGEX_H);
	m_filterCase[1] = GetConfig().get_int(false, REG_FILTER_CASE_H);
	m_wndFilterString[1].LoadHistoryFromText(GetConfig().get_string(false, REG_FILTER_STRING_LST_H).c_str(),
											 GetConfig().get_string(false, REG_FILTER_STRING_H).c_str());

	UpdateData(FALSE);

	OnUpdateControls();

	return TRUE;
}

void CFilterDialog::OnOK()
{
	UpdateData(TRUE);

	std::array<CString, 2> filterString;
	for (int i = 0; i < 2; i++)
	{
		m_wndFilterString[i].GetWindowText(filterString[i]);

		try
		{
			// Check expression
			if (m_filterRegex[i])
			{
				std::wregex re(filterString[i].GetString());
				UNUSED_ALWAYS(re);
			}
		}
		catch (std::regex_error& ex)
		{
			CString error;
			error.Format(IDS_STRING_ERR_REGEX, ex.what());
			AfxMessageBox(error, MB_OK | MB_ICONERROR);
			return;
		}
	}

	GetConfig().set_string(false, REG_FILTER_STRING_S, filterString[0].GetString());
	GetConfig().set_string(false, REG_FILTER_STRING_LST_S, m_wndFilterString[0].SaveHistoryToText().GetString());
	GetConfig().set_int(false, REG_FILTER_REGEX_S, m_filterRegex[0]);
	GetConfig().set_int(false, REG_FILTER_CASE_S, m_filterCase[0]);
	GetConfig().set_int(false, REG_FILTER_STATE_S, m_filterState[0]);

	GetConfig().set_string(false, REG_FILTER_STRING_H, filterString[1].GetString());
	GetConfig().set_string(false, REG_FILTER_STRING_LST_H, m_wndFilterString[1].SaveHistoryToText().GetString());
	GetConfig().set_int(false, REG_FILTER_REGEX_H, m_filterRegex[1]);
	GetConfig().set_int(false, REG_FILTER_CASE_H, m_filterCase[1]);
	GetConfig().set_int(false, REG_FILTER_STATE_H, m_filterState[1]);

	__super::OnOK();
}

void CFilterDialog::OnUpdateControls()
{
	UpdateData(TRUE);

	for (int i = 0; i < 2; i++)
	{
		if (m_filterRegex[i])
		{
			m_filterCase[i] = FALSE;
			m_wndFilterCase[i].EnableWindow(FALSE);
		}
		m_wndFilterCase[i].EnableWindow(!m_filterRegex[i]);

		if (m_filterCase[i])
		{
			m_filterRegex[i] = FALSE;
			m_wndFilterRegex[i].EnableWindow(FALSE);
		}
		m_wndFilterRegex[i].EnableWindow(!m_filterCase[i]);
	}

	UpdateData(FALSE);
}
