// SettingsDlg.cpp : implementation file
//

#include "StdAfx.h"
#include <afxdialogex.h>
#include "IPTVChannelEditor.h"
#include "SettingsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CSettingsDlg dialog

IMPLEMENT_DYNAMIC(CSettingsDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CSettingsDlg, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT_STREAM_THREADS, &CSettingsDlg::OnEnChangeEditStreamThreads)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_STREAM_THREADS, &CSettingsDlg::OnDeltaposSpinStreamThreads)
	ON_CBN_SELCHANGE(IDC_COMBO_LANG, &CSettingsDlg::OnCbnSelchangeComboLang)
END_MESSAGE_MAP()


CSettingsDlg::CSettingsDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_SETTINGS, pParent)
{

}

void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_MFCEDITBROWSE_PLAYER, m_player);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_PLAYER, m_wndPlayer);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_PROBE, m_probe);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_PROBE, m_wndProbe);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_CH_LIST_PATH, m_wndListsPath);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_CH_LIST_PATH, m_lists_path);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_PLUGINS_PATH, m_wndPluginsPath);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_PLUGINS_PATH, m_plugins_path);
	DDX_Check(pDX, IDC_CHECK_AUTO_SYNC_CHANNELS, m_bAutoSync);
	DDX_Text(pDX, IDC_EDIT_STREAM_THREADS, m_MaxThreads);
	DDX_Check(pDX, IDC_CHECK_AUTO_HIDE, m_bAutoHide);
	DDX_Control(pDX, IDC_EDIT_STREAM_THREADS, m_wndMaxThreads);
	DDX_Control(pDX, IDC_SPIN_STREAM_THREADS, m_wndSpinMaxThreads);
	DDX_Control(pDX, IDC_COMBO_LANG, m_wndLanguage);
}

BOOL CSettingsDlg::OnInitDialog()
{
	__super::OnInitDialog();

	m_player = GetConfig().get_string(true, REG_PLAYER).c_str();
	m_probe = GetConfig().get_string(true, REG_FFPROBE).c_str();
	m_lists_path = GetConfig().get_string(true, REG_LISTS_PATH).c_str();
	m_plugins_path = GetConfig().get_string(true, REG_OUTPUT_PATH).c_str();
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

	CString filter(_T("EXE file(*.exe)|*.exe|All Files (*.*)|*.*||"));
	m_wndPlayer.EnableFileBrowseButton(nullptr, filter.GetString(), OFN_EXPLORER | OFN_ENABLESIZING | OFN_LONGNAMES | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST);
	m_wndProbe.EnableFileBrowseButton(nullptr, filter.GetString(), OFN_EXPLORER | OFN_ENABLESIZING | OFN_LONGNAMES | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST);

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CSettingsDlg::OnOK()
{
	UpdateData(TRUE);

	if (m_lists_path.Right(1) != '\\')
		m_lists_path += '\\';

	if (m_plugins_path.Right(1) != '\\')
		m_plugins_path += '\\';

	GetConfig().set_string(true, REG_PLAYER, m_player.GetString());
	GetConfig().set_string(true, REG_FFPROBE, m_probe.GetString());
	GetConfig().set_string(true, REG_LISTS_PATH, m_lists_path.GetString());
	GetConfig().set_string(true, REG_OUTPUT_PATH, m_plugins_path.GetString());
	GetConfig().set_int(true, REG_AUTO_SYNC, m_bAutoSync);
	GetConfig().set_int(true, REG_AUTO_HIDE, m_bAutoHide);
	GetConfig().set_int(true, REG_MAX_THREADS, m_MaxThreads);
	GetConfig().set_int(true, REG_LANGUAGE, m_nLang);

	GetConfig().SaveAppSettingsRegistry();

	__super::OnOK();
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

void CSettingsDlg::OnCbnSelchangeComboLang()
{
	AfxMessageBox(IDS_STRING_INFO_RESTART_NEED, MB_OK | MB_ICONINFORMATION);
	m_nLang = (WORD)m_wndLanguage.GetItemData(m_wndLanguage.GetCurSel());
}
