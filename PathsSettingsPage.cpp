// SettingsPage.cpp : implementation file
//

#include "pch.h"
#include "IPTVChannelEditor.h"
#include "PathsSettingsPage.h"
#include "afxdialogex.h"


// CPathsSettingsPage dialog

IMPLEMENT_DYNAMIC(CPathsSettingsPage, CPropertyPage)

BEGIN_MESSAGE_MAP(CPathsSettingsPage, CPropertyPage)
END_MESSAGE_MAP()

CPathsSettingsPage::CPathsSettingsPage() : CPropertyPage(IDD_PATHS_SETTINGS_PAGE)
{
}

void CPathsSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_MFCEDITBROWSE_PLAYER, m_player);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_PLAYER, m_wndPlayer);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_PROBE, m_probe);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_PROBE, m_wndProbe);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_CH_LIST_PATH, m_wndListsPath);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_CH_LIST_PATH, m_lists_path);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_PLUGINS_PATH, m_wndPluginsPath);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_PLUGINS_PATH, m_plugins_path);
}

BOOL CPathsSettingsPage::OnInitDialog()
{
	__super::OnInitDialog();

	m_player = GetConfig().get_string(true, REG_PLAYER).c_str();
	m_probe = GetConfig().get_string(true, REG_FFPROBE).c_str();
	m_lists_path = GetConfig().get_string(true, REG_LISTS_PATH).c_str();
	m_plugins_path = GetConfig().get_string(true, REG_OUTPUT_PATH).c_str();

	CString filter(_T("EXE file(*.exe)|*.exe|All Files (*.*)|*.*||"));
	m_wndPlayer.EnableFileBrowseButton(nullptr, filter.GetString(), OFN_EXPLORER | OFN_ENABLESIZING | OFN_LONGNAMES | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST);
	m_wndProbe.EnableFileBrowseButton(nullptr, filter.GetString(), OFN_EXPLORER | OFN_ENABLESIZING | OFN_LONGNAMES | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST);

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CPathsSettingsPage::OnOK()
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

	GetConfig().SaveAppSettingsRegistry();

	__super::OnOK();
}
