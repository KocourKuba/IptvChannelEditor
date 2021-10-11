// SettingsDlg.cpp : implementation file
//

#include "StdAfx.h"
#include <afxdialogex.h>
#include "IPTVChannelEditor.h"
#include "SettingsDlg.h"

// CSettingsDlg dialog

IMPLEMENT_DYNAMIC(CSettingsDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CSettingsDlg, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT_STREAM_THREADS, &CSettingsDlg::OnEnChangeEditStreamThreads)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_STREAM_THREADS, &CSettingsDlg::OnDeltaposSpinStreamThreads)
END_MESSAGE_MAP()


CSettingsDlg::CSettingsDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_SETTINGS, pParent)
	, m_bAutoSync(FALSE)
{

}

void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_MFCEDITBROWSE_PLAYER, m_player);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_PROBE, m_probe);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_PROBE, m_wndProbe);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_PLAYER, m_wndPlayer);
	DDX_Check(pDX, IDC_CHECK_AUTO_SYNC_CHANNELS, m_bAutoSync);
	DDX_Text(pDX, IDC_EDIT_STREAM_THREADS, m_MaxThreads);
	DDX_Control(pDX, IDC_EDIT_STREAM_THREADS, m_wndMaxThreads);
	DDX_Control(pDX, IDC_SPIN_STREAM_THREADS, m_wndSpinMaxThreads);
}

BOOL CSettingsDlg::OnInitDialog()
{
	__super::OnInitDialog();

	CString filter(_T("EXE file(*.exe)|*.exe|All Files (*.*)|*.*||"));
	m_wndPlayer.EnableFileBrowseButton(nullptr, filter.GetString(), OFN_EXPLORER | OFN_ENABLESIZING | OFN_LONGNAMES | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST);
	m_wndProbe.EnableFileBrowseButton(nullptr, filter.GetString(), OFN_EXPLORER | OFN_ENABLESIZING | OFN_LONGNAMES | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST);

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CSettingsDlg::OnEnChangeEditStreamThreads()
{
	UpdateData(TRUE);

	if (m_MaxThreads < 1)
		m_MaxThreads = 1;

	if (m_MaxThreads > 10)
		m_MaxThreads = 10;

	UpdateData(FALSE);
}


void CSettingsDlg::OnDeltaposSpinStreamThreads(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateData(TRUE);
	m_MaxThreads -= reinterpret_cast<LPNMUPDOWN>(pNMHDR)->iDelta;
	UpdateData(FALSE);
	OnEnChangeEditStreamThreads();
	*pResult = 0;
}
