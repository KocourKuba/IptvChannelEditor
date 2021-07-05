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
	, m_bAutoSync(FALSE)
{

}

void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_MFCEDITBROWSE_PLAYER, m_player);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_PROBE, m_probe);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_PROBE, m_wndProbe);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_PLAYER, m_wndPlayer);
	DDX_Check(pDX, IDC_CHECK_AUTO_SYNC_CHANNELS, m_bAutoSync);
}

BOOL CSettingsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_player = theApp.GetProfileString(_T("Setting"), _T("Player"));
	m_probe = theApp.GetProfileString(_T("Setting"), _T("FFProbe"));
	m_bAutoSync = theApp.GetProfileInt(_T("Setting"), _T("AutoSyncChannel"), FALSE);

	CString filter(_T("EXE file(*.exe)|*.exe|All Files (*.*)|*.*||"));
	m_wndPlayer.EnableFileBrowseButton(nullptr, filter.GetString(), OFN_EXPLORER | OFN_ENABLESIZING | OFN_LONGNAMES | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST);
	m_wndProbe.EnableFileBrowseButton(nullptr, filter.GetString(), OFN_EXPLORER | OFN_ENABLESIZING | OFN_LONGNAMES | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST);

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CSettingsDlg::OnOK()
{
	CDialogEx::OnOK();

	UpdateData(TRUE);

	theApp.WriteProfileString(_T("Setting"), _T("Player"), m_player);
	theApp.WriteProfileString(_T("Setting"), _T("FFProbe"), m_probe);
	theApp.WriteProfileInt(_T("Setting"), _T("AutoSyncChannel"), m_bAutoSync);
}
