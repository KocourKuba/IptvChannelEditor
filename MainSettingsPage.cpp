// SettingsDlg.cpp : implementation file
//

#include "pch.h"
#include <afxdialogex.h>
#include "IPTVChannelEditor.h"
#include "MainSettingsPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CMainSettingsPage dialog

IMPLEMENT_DYNAMIC(CMainSettingsPage, CPropertyPage)

BEGIN_MESSAGE_MAP(CMainSettingsPage, CPropertyPage)
	ON_EN_CHANGE(IDC_EDIT_STREAM_THREADS, &CMainSettingsPage::OnEnChangeEditStreamThreads)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_STREAM_THREADS, &CMainSettingsPage::OnDeltaposSpinStreamThreads)
	ON_CBN_SELCHANGE(IDC_COMBO_LANG, &CMainSettingsPage::OnCbnSelchangeComboLang)
END_MESSAGE_MAP()


CMainSettingsPage::CMainSettingsPage() : CPropertyPage(IDD_MAIN_SETTINGS_PAGE)
{
}

void CMainSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_COMBO_LANG, m_wndLanguage);
	DDX_Check(pDX, IDC_CHECK_AUTO_SYNC_CHANNELS, m_bAutoSync);
	DDX_Check(pDX, IDC_CHECK_AUTO_HIDE, m_bAutoHide);
	DDX_Text(pDX, IDC_EDIT_STREAM_THREADS, m_MaxThreads);
	DDX_Control(pDX, IDC_EDIT_STREAM_THREADS, m_wndMaxThreads);
	DDX_Control(pDX, IDC_SPIN_STREAM_THREADS, m_wndSpinMaxThreads);
}

BOOL CMainSettingsPage::OnInitDialog()
{
	__super::OnInitDialog();

	m_bAutoSync = GetConfig().get_int(true, REG_AUTO_SYNC);
	m_bAutoHide = GetConfig().get_int(true, REG_AUTO_HIDE);
	m_MaxThreads = GetConfig().get_int(true, REG_MAX_THREADS);
	m_nLang = GetConfig().get_int(true, REG_LANGUAGE);

	int nCurrent = 0;
	for (const auto& pair : theApp.m_LangMap)
	{
		int nIdx = m_wndLanguage.AddString(pair.second.csLang);
		m_wndLanguage.SetItemData(nIdx, pair.first);
		if (pair.first == m_nLang)
			nCurrent = nIdx;
	}

	if (m_wndLanguage.GetCount())
	{
		m_wndLanguage.SetCurSel(nCurrent);
	}

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CMainSettingsPage::OnOK()
{
	UpdateData(TRUE);

	GetConfig().set_int(true, REG_AUTO_SYNC, m_bAutoSync);
	GetConfig().set_int(true, REG_AUTO_HIDE, m_bAutoHide);
	GetConfig().set_int(true, REG_MAX_THREADS, m_MaxThreads);
	GetConfig().set_int(true, REG_LANGUAGE, m_nLang);

	GetConfig().SaveAppSettingsRegistry();

	__super::OnOK();
}

void CMainSettingsPage::OnEnChangeEditStreamThreads()
{
	UpdateData(TRUE);

	if (m_MaxThreads < 1)
		m_MaxThreads = 1;

	if (m_MaxThreads > 10)
		m_MaxThreads = 10;

	UpdateData(FALSE);
}

void CMainSettingsPage::OnDeltaposSpinStreamThreads(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateData(TRUE);
	m_MaxThreads -= reinterpret_cast<LPNMUPDOWN>(pNMHDR)->iDelta;
	UpdateData(FALSE);
	OnEnChangeEditStreamThreads();
	*pResult = 0;
}

void CMainSettingsPage::OnCbnSelchangeComboLang()
{
	AfxMessageBox(IDS_STRING_INFO_RESTART_NEED, MB_OK | MB_ICONINFORMATION);
	m_nLang = (WORD)m_wndLanguage.GetItemData(m_wndLanguage.GetCurSel());
}
