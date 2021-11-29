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
	DDX_Text(pDX, IDC_EDIT_STREAM_THREADS, m_MaxThreads);
	DDX_Control(pDX, IDC_EDIT_STREAM_THREADS, m_wndMaxThreads);
	DDX_Control(pDX, IDC_SPIN_STREAM_THREADS, m_wndSpinMaxThreads);
	DDX_Check(pDX, IDC_CHECK_AUTO_SYNC_CHANNELS, m_bAutoSync);
	DDX_Check(pDX, IDC_CHECK_AUTO_HIDE, m_bAutoHide);

	DDX_Check(pDX, IDC_CHECK_CMP_TITLE, m_bCmpTitle);
	DDX_Check(pDX, IDC_CHECK_CMP_ICON, m_bCmpIcon);
	DDX_Check(pDX, IDC_CHECK_CMP_ARCHIVE, m_bCmpArchive);
	DDX_Check(pDX, IDC_CHECK_CMP_EPG1, m_bCmpEpg1);
	DDX_Check(pDX, IDC_CHECK_CMP_EPG2, m_bCmpEpg2);
}

BOOL CMainSettingsPage::OnInitDialog()
{
	__super::OnInitDialog();

	m_bAutoSync = GetConfig().get_int(true, REG_AUTO_SYNC);
	m_bAutoHide = GetConfig().get_int(true, REG_AUTO_HIDE);
	m_MaxThreads = GetConfig().get_int(true, REG_MAX_THREADS);
	m_nLang = GetConfig().get_int(true, REG_LANGUAGE);
	int flags = GetConfig().get_int(true, REG_CMP_FLAGS, CMP_FLAG_ALL);

	m_bCmpTitle   = (flags & CMP_FLAG_TITLE) ? TRUE : FALSE;
	m_bCmpIcon    = (flags & CMP_FLAG_ICON) ? TRUE : FALSE;
	m_bCmpArchive = (flags & CMP_FLAG_ARCHIVE) ? TRUE : FALSE;
	m_bCmpEpg1    = (flags & CMP_FLAG_EPG1) ? TRUE : FALSE;
	m_bCmpEpg2    = (flags & CMP_FLAG_EPG2) ? TRUE : FALSE;

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

	int flags = 0;
	flags |= (m_bCmpTitle ? CMP_FLAG_TITLE : 0);
	flags |= (m_bCmpIcon ? CMP_FLAG_ICON : 0);
	flags |= (m_bCmpArchive ? CMP_FLAG_ARCHIVE : 0);
	flags |= (m_bCmpEpg1 ? CMP_FLAG_EPG1 : 0);
	flags |= (m_bCmpEpg2 ? CMP_FLAG_EPG2 : 0);
	GetConfig().set_int(true, REG_CMP_FLAGS, flags);

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
