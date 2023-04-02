/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2023): sharky72 (https://github.com/KocourKuba)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "pch.h"
#include <iosfwd>
#include "IPTVChannelEditor.h"
#include "PluginConfigPage.h"
#include "StreamContainer.h"
#include "FillParamsInfoDlg.h"
#include "AccountSettings.h"
#include "Constants.h"

#include "UtilsLib/inet_utils.h"

// CPluginConfigPage dialog

IMPLEMENT_DYNAMIC(CPluginConfigPage, CTooltipPropertyPage)

BEGIN_MESSAGE_MAP(CPluginConfigPage, CTooltipPropertyPage)
	ON_CBN_SELCHANGE(IDC_COMBO_ACCESS_TYPE, &CPluginConfigPage::OnCbnSelchangeComboAccessType)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_EXT_FILES, &CPluginConfigPage::OnBnClickedButtonEditExtFiles)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_EXT_SCRIPTS, &CPluginConfigPage::OnBnClickedButtonEditExtScripts)
	ON_BN_CLICKED(IDC_CHECK_STATIC_SERVERS, &CPluginConfigPage::OnBnClickedCheckStaticServers)
	ON_BN_CLICKED(IDC_CHECK_STATIC_DEVICES, &CPluginConfigPage::OnBnClickedCheckStaticDevices)
	ON_BN_CLICKED(IDC_CHECK_STATIC_QUALITIES, &CPluginConfigPage::OnBnClickedCheckStaticQualities)
	ON_BN_CLICKED(IDC_CHECK_STATIC_PROFILES, &CPluginConfigPage::OnBnClickedCheckStaticProfiles)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_SERVERS, &CPluginConfigPage::OnBnClickedButtonEditServers)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_DEVICES, &CPluginConfigPage::OnBnClickedButtonEditDevices)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_QUALITY, &CPluginConfigPage::OnBnClickedButtonEditQuality)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_PROFILES, &CPluginConfigPage::OnBnClickedButtonEditProfiles)
	ON_BN_CLICKED(IDC_CHECK_SQUARE_ICONS, &CPluginConfigPage::OnBnClickedCheckSquareIcons)
	ON_EN_CHANGE(IDC_EDIT_PLUGIN_NAME, &CPluginConfigPage::OnEnChangeEditPluginName)
	ON_EN_CHANGE(IDC_EDIT_TITLE, &CPluginConfigPage::OnEnChangeEditTitle)
	ON_EN_CHANGE(IDC_EDIT_USER_AGENT, &CPluginConfigPage::OnEnChangeEditUserAgent)
	ON_EN_CHANGE(IDC_EDIT_PROVIDER_URL, &CPluginConfigPage::OnEnChangeEditProviderUrl)
END_MESSAGE_MAP()

CPluginConfigPage::CPluginConfigPage() : CTooltipPropertyPage(IDD_DIALOG_PLUGIN_CONFIG)
{
}

void CPluginConfigPage::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_EDIT_PLUGIN_NAME, m_wndName);
	DDX_Text(pDX, IDC_EDIT_PLUGIN_NAME, m_Name);
	DDX_Control(pDX, IDC_EDIT_TITLE, m_wndTitle);
	DDX_Text(pDX, IDC_EDIT_TITLE, m_Title);
	DDX_Control(pDX, IDC_EDIT_USER_AGENT, m_wndUserAgent);
	DDX_Text(pDX, IDC_EDIT_USER_AGENT, m_UserAgent);
	DDX_Control(pDX, IDC_EDIT_PROVIDER_URL, m_wndProviderUrl);
	DDX_Text(pDX, IDC_EDIT_PROVIDER_URL, m_ProviderUrl);
	DDX_Control(pDX, IDC_COMBO_ACCESS_TYPE, m_wndAccessType);
	DDX_Control(pDX, IDC_CHECK_SQUARE_ICONS, m_wndChkSquareIcons);
	DDX_Control(pDX, IDC_BUTTON_EDIT_EXT_FILES, m_wndBtnExtFiles);
	DDX_Control(pDX, IDC_BUTTON_EDIT_EXT_SCRIPTS, m_wndBtnExtScripts);
	DDX_Control(pDX, IDC_CHECK_STATIC_SERVERS, m_wndChkStaticServers);
	DDX_Control(pDX, IDC_BUTTON_EDIT_SERVERS, m_wndBtnServers);
	DDX_Control(pDX, IDC_CHECK_STATIC_DEVICES, m_wndChkStaticDevices);
	DDX_Control(pDX, IDC_BUTTON_EDIT_DEVICES, m_wndBtnDevices);
	DDX_Control(pDX, IDC_CHECK_STATIC_QUALITIES, m_wndChkStaticQualities);
	DDX_Control(pDX, IDC_BUTTON_EDIT_QUALITY, m_wndBtnQualities);
	DDX_Control(pDX, IDC_CHECK_STATIC_PROFILES, m_wndChkStaticProfiles);
	DDX_Control(pDX, IDC_BUTTON_EDIT_PROFILES, m_wndBtnProfiles);
}

BOOL CPluginConfigPage::OnInitDialog()
{
	__super::OnInitDialog();

	AddTooltip(IDC_EDIT_PLUGIN_NAME, IDS_STRING_EDIT_PLUGIN_NAME);
	AddTooltip(IDC_EDIT_TITLE, IDS_STRING_EDIT_TITLE);
	AddTooltip(IDC_EDIT_PROVIDER_URL, IDS_STRING_EDIT_PROVIDER_URL);
	AddTooltip(IDC_CHECK_SQUARE_ICONS, IDS_STRING_CHECK_SQUARE_ICONS);
	AddTooltip(IDC_BUTTON_EDIT_EXT_FILES, IDS_STRING_BUTTON_EDIT_EXT_FILES);
	AddTooltip(IDC_BUTTON_EDIT_EXT_SCRIPTS, IDS_STRING_BUTTON_EDIT_EXT_SCRIPTS);
	AddTooltip(IDC_CHECK_STATIC_SERVERS, IDS_STRING_CHECK_STATIC_SERVERS);
	AddTooltip(IDC_BUTTON_EDIT_SERVERS, IDS_STRING_BUTTON_EDIT_SERVERS);
	AddTooltip(IDC_CHECK_STATIC_DEVICES, IDS_STRING_CHECK_STATIC_DEVICES);
	AddTooltip(IDC_BUTTON_EDIT_DEVICES, IDS_STRING_BUTTON_EDIT_DEVICES);
	AddTooltip(IDC_CHECK_STATIC_QUALITIES, IDS_STRING_CHECK_STATIC_QUALITIES);
	AddTooltip(IDC_BUTTON_EDIT_QUALITY, IDS_STRING_BUTTON_EDIT_QUALITY);
	AddTooltip(IDC_CHECK_STATIC_PROFILES, IDS_STRING_CHECK_STATIC_PROFILES);
	AddTooltip(IDC_BUTTON_EDIT_PROFILES, IDS_STRING_BUTTON_EDIT_PROFILES);
	AddTooltip(IDC_COMBO_ACCESS_TYPE, IDS_STRING_COMBO_ACCESS_TYPE);

	SetButtonImage(IDB_PNG_EDIT, m_wndBtnExtFiles);
	SetButtonImage(IDB_PNG_EDIT, m_wndBtnExtScripts);
	SetButtonImage(IDB_PNG_EDIT, m_wndBtnServers);
	SetButtonImage(IDB_PNG_EDIT, m_wndBtnDevices);
	SetButtonImage(IDB_PNG_EDIT, m_wndBtnQualities);
	SetButtonImage(IDB_PNG_EDIT, m_wndBtnProfiles);

	FillControls();

	RestoreWindowPos(GetSafeHwnd(), REG_CONFIG_WINDOW_POS);
	AllowSave(false);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPluginConfigPage::OnSetActive()
{
	__super::OnSetActive();

	FillControls();

	return TRUE;
}

void CPluginConfigPage::UpdateControls()
{
	UpdateData(TRUE);

	const auto& plugin = GetPropertySheet()->m_plugin;
	bool custom = plugin->get_plugin_type() == PluginType::enCustom;
	bool readOnly = GetPropertySheet()->GetSelectedConfig().empty();

	// common
	m_wndName.SetReadOnly(readOnly);
	m_wndTitle.SetReadOnly(readOnly);
	m_wndUserAgent.SetReadOnly(readOnly);
	m_wndProviderUrl.SetReadOnly(readOnly);
	m_wndChkSquareIcons.EnableWindow(!readOnly);
	m_wndAccessType.EnableWindow(custom);

	// ext files
	m_wndBtnExtFiles.EnableWindow(!readOnly || !plugin->get_files_list().empty());
	m_wndBtnExtScripts.EnableWindow(!readOnly || !plugin->get_scripts_list().empty());

	// servers
	m_wndChkStaticServers.SetCheck(plugin->get_static_servers());
	m_wndChkStaticServers.EnableWindow(!readOnly);
	m_wndBtnServers.EnableWindow(plugin->get_static_servers());

	// devices
	m_wndChkStaticDevices.SetCheck(plugin->get_static_devices());
	m_wndChkStaticDevices.EnableWindow(!readOnly);
	m_wndBtnDevices.EnableWindow(plugin->get_static_devices());

	// qualities
	m_wndChkStaticQualities.SetCheck(plugin->get_static_qualities());
	m_wndChkStaticQualities.EnableWindow(!readOnly);
	m_wndBtnQualities.EnableWindow(plugin->get_static_qualities());

	// profiles
	m_wndChkStaticProfiles.SetCheck(plugin->get_static_profiles());
	m_wndChkStaticProfiles.EnableWindow(!readOnly);
	m_wndBtnProfiles.EnableWindow(plugin->get_static_profiles());
}

void CPluginConfigPage::FillControls()
{
	const auto& plugin = GetPropertySheet()->m_plugin;
	if (!plugin) return;

	m_wndAccessType.SetCurSel((int)plugin->get_access_type());
	m_wndChkSquareIcons.SetCheck(plugin->get_square_icons() != false);

	m_Name = plugin->get_name().c_str();
	m_Title = plugin->get_title().c_str();
	m_UserAgent = plugin->get_user_agent().c_str();
	m_ProviderUrl = plugin->get_provider_url().c_str();

	UpdateData(FALSE);

	UpdateControls();
}

void CPluginConfigPage::OnCbnSelchangeComboAccessType()
{
	GetPropertySheet()->m_plugin->set_access_type((AccountAccessType)m_wndAccessType.GetCurSel());
	AllowSave();
	FillControls();
}

void CPluginConfigPage::OnBnClickedButtonEditExtFiles()
{
	CFillParamsInfoDlg dlg;
	dlg.m_type = 4;
	dlg.m_paramsList = GetPropertySheet()->m_plugin->get_files_list();
	dlg.m_readonly = GetPropertySheet()->GetSelectedConfig().empty();

	if (dlg.DoModal() == IDOK)
	{
		AllowSave();
		GetPropertySheet()->m_plugin->set_files_list(dlg.m_paramsList);
	}
}

void CPluginConfigPage::OnBnClickedButtonEditExtScripts()
{
	CFillParamsInfoDlg dlg;
	dlg.m_type = 4;
	dlg.m_paramsList = GetPropertySheet()->m_plugin->get_scripts_list();
	dlg.m_readonly = GetPropertySheet()->GetSelectedConfig().empty();

	if (dlg.DoModal() == IDOK)
	{
		AllowSave();
		GetPropertySheet()->m_plugin->set_scripts_list(dlg.m_paramsList);
	}
}

void CPluginConfigPage::OnBnClickedButtonEditServers()
{
	CFillParamsInfoDlg dlg;
	dlg.m_type = 0;
	dlg.m_paramsList = GetPropertySheet()->m_plugin->get_servers_list();
	dlg.m_readonly = GetPropertySheet()->GetSelectedConfig().empty();

	if (dlg.DoModal() == IDOK)
	{
		AllowSave();
		GetPropertySheet()->m_plugin->set_servers_list(dlg.m_paramsList);
	}
}

void CPluginConfigPage::OnBnClickedButtonEditDevices()
{
	CFillParamsInfoDlg dlg;
	dlg.m_type = 1;
	dlg.m_paramsList = GetPropertySheet()->m_plugin->get_devices_list();
	dlg.m_readonly = GetPropertySheet()->GetSelectedConfig().empty();

	if (dlg.DoModal() == IDOK)
	{
		AllowSave();
		GetPropertySheet()->m_plugin->set_devices_list(dlg.m_paramsList);
	}
}

void CPluginConfigPage::OnBnClickedButtonEditQuality()
{
	CFillParamsInfoDlg dlg;
	dlg.m_type = 2;
	dlg.m_paramsList = GetPropertySheet()->m_plugin->get_qualities_list();
	dlg.m_readonly = GetPropertySheet()->GetSelectedConfig().empty();

	if (dlg.DoModal() == IDOK)
	{
		AllowSave();
		GetPropertySheet()->m_plugin->set_qualities_list(dlg.m_paramsList);
	}
}

void CPluginConfigPage::OnBnClickedButtonEditProfiles()
{
	CFillParamsInfoDlg dlg;
	dlg.m_type = 3;
	dlg.m_paramsList = GetPropertySheet()->m_plugin->get_profiles_list();
	dlg.m_readonly = GetPropertySheet()->GetSelectedConfig().empty();

	if (dlg.DoModal() == IDOK)
	{
		AllowSave();
		GetPropertySheet()->m_plugin->set_profiles_list(dlg.m_paramsList);
	}
}

void CPluginConfigPage::OnBnClickedCheckStaticServers()
{
	AllowSave();
	GetPropertySheet()->m_plugin->set_static_servers(m_wndChkStaticServers.GetCheck() != 0);
	m_wndBtnServers.EnableWindow(m_wndChkStaticServers.GetCheck());
}

void CPluginConfigPage::OnBnClickedCheckStaticDevices()
{
	AllowSave();
	GetPropertySheet()->m_plugin->set_static_devices(m_wndChkStaticDevices.GetCheck() != 0);
	m_wndBtnDevices.EnableWindow(m_wndChkStaticDevices.GetCheck());
}

void CPluginConfigPage::OnBnClickedCheckStaticQualities()
{
	AllowSave();
	GetPropertySheet()->m_plugin->set_static_qualities(m_wndChkStaticQualities.GetCheck() != 0);
	m_wndBtnQualities.EnableWindow(m_wndChkStaticQualities.GetCheck());
}

void CPluginConfigPage::OnBnClickedCheckStaticProfiles()
{
	AllowSave();
	GetPropertySheet()->m_plugin->set_static_profiles(m_wndChkStaticProfiles.GetCheck() != 0);
	m_wndBtnProfiles.EnableWindow(m_wndChkStaticProfiles.GetCheck());
}

void CPluginConfigPage::OnBnClickedCheckSquareIcons()
{
	AllowSave();
	GetPropertySheet()->m_plugin->set_square_icons(m_wndChkSquareIcons.GetCheck() != 0);
}

void CPluginConfigPage::OnEnChangeEditPluginName()
{
	UpdateData(TRUE);
	AllowSave();
	GetPropertySheet()->m_plugin->set_name(m_Name.GetString());
}

void CPluginConfigPage::OnEnChangeEditTitle()
{
	UpdateData(TRUE);
	AllowSave();
	GetPropertySheet()->m_plugin->set_title(m_Title.GetString());
}

void CPluginConfigPage::OnEnChangeEditUserAgent()
{
	UpdateData(TRUE);
	AllowSave();
	GetPropertySheet()->m_plugin->set_user_agent(m_UserAgent.GetString());
}

void CPluginConfigPage::OnEnChangeEditProviderUrl()
{
	UpdateData(TRUE);
	AllowSave();
	GetPropertySheet()->m_plugin->set_provider_url(m_ProviderUrl.GetString());
}
