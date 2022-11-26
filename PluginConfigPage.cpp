/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2022): sharky72 (https://github.com/KocourKuba)

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
#include "NewConfigDlg.h"
#include "AccountSettings.h"
#include "Constants.h"

#include "UtilsLib/inet_utils.h"

// CPluginConfigPage dialog

IMPLEMENT_DYNAMIC(CPluginConfigPage, CTooltipPropertyPage)

BEGIN_MESSAGE_MAP(CPluginConfigPage, CTooltipPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_CONFIG, &CPluginConfigPage::OnBnClickedButtonToggleEditConfig)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_CONFIG, &CPluginConfigPage::OnBnClickedButtonSaveConfig)
	ON_CBN_SELCHANGE(IDC_COMBO_ACCESS_TYPE, &CPluginConfigPage::OnCbnSelchangeComboAccessType)
	ON_BN_CLICKED(IDC_CHECK_STATIC_SERVERS, &CPluginConfigPage::OnBnClickedCheckStaticServers)
	ON_BN_CLICKED(IDC_CHECK_STATIC_DEVICES, &CPluginConfigPage::OnBnClickedCheckStaticDevices)
	ON_BN_CLICKED(IDC_CHECK_STATIC_QUALITIES, &CPluginConfigPage::OnBnClickedCheckStaticQualities)
	ON_BN_CLICKED(IDC_CHECK_STATIC_PROFILES, &CPluginConfigPage::OnBnClickedCheckStaticProfiles)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_SERVERS, &CPluginConfigPage::OnBnClickedButtonEditServers)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_DEVICES, &CPluginConfigPage::OnBnClickedButtonEditDevices)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_QUALITY, &CPluginConfigPage::OnBnClickedButtonEditQuality)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_PROFILES, &CPluginConfigPage::OnBnClickedButtonEditProfiles)
	ON_CBN_SELCHANGE(IDC_COMBO_PLUGIN_CONFIG, &CPluginConfigPage::OnCbnSelchangeComboPluginConfig)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_AS_CONFIG, &CPluginConfigPage::OnBnClickedButtonSaveAsConfig)
	ON_BN_CLICKED(IDC_CHECK_SQUARE_ICONS, &CPluginConfigPage::OnBnClickedCheckSquareIcons)
	ON_EN_CHANGE(IDC_EDIT_PLUGIN_NAME, &CPluginConfigPage::OnEnChangeEditPluginName)
	ON_EN_CHANGE(IDC_EDIT_TITLE, &CPluginConfigPage::OnEnChangeEditTitle)
	ON_EN_CHANGE(IDC_EDIT_PROVIDER_URL, &CPluginConfigPage::OnEnChangeEditProviderUrl)
END_MESSAGE_MAP()

CPluginConfigPage::CPluginConfigPage() : CTooltipPropertyPage(IDD_DIALOG_PLUGIN_CONFIG)
{
}

void CPluginConfigPage::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_COMBO_PLUGIN_CONFIG, m_wndPluginConfigs);
	DDX_Control(pDX, IDC_BUTTON_EDIT_CONFIG, m_wndBtnToggleEdit);
	DDX_Control(pDX, IDC_BUTTON_SAVE_CONFIG, m_wndBtnSaveConf);
	DDX_Control(pDX, IDC_BUTTON_SAVE_AS_CONFIG, m_wndBtnSaveAsConf);
	DDX_Control(pDX, IDC_EDIT_PLUGIN_NAME, m_wndName);
	DDX_Text(pDX, IDC_EDIT_PLUGIN_NAME, m_Name);
	DDX_Control(pDX, IDC_EDIT_TITLE, m_wndTitle);
	DDX_Text(pDX, IDC_EDIT_TITLE, m_Title);
	DDX_Control(pDX, IDC_EDIT_PROVIDER_URL, m_wndProviderUrl);
	DDX_Text(pDX, IDC_EDIT_PROVIDER_URL, m_ProviderUrl);
	DDX_Control(pDX, IDC_COMBO_ACCESS_TYPE, m_wndAccessType);
	DDX_Control(pDX, IDC_CHECK_SQUARE_ICONS, m_wndChkSquareIcons);
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

	AddTooltip(IDC_COMBO_PLUGIN_CONFIG, IDS_STRING_COMBO_CONFIG);
	AddTooltip(IDC_BUTTON_EDIT_CONFIG, IDS_STRING_BUTTON_EDIT_CONFIG);
	AddTooltip(IDC_BUTTON_SAVE_CONFIG, IDS_STRING_BUTTON_SAVE_CONFIG);
	AddTooltip(IDC_BUTTON_SAVE_AS_CONFIG, IDS_STRING_BUTTON_SAVE_AS_CONFIG);
	AddTooltip(IDC_EDIT_PLUGIN_NAME, IDS_STRING_EDIT_PLUGIN_NAME);
	AddTooltip(IDC_EDIT_TITLE, IDS_STRING_EDIT_TITLE);
	AddTooltip(IDC_EDIT_PROVIDER_URL, IDS_STRING_EDIT_PROVIDER_URL);
	AddTooltip(IDC_CHECK_SQUARE_ICONS, IDS_STRING_CHECK_SQUARE_ICONS);
	AddTooltip(IDC_CHECK_STATIC_SERVERS, IDS_STRING_CHECK_STATIC_SERVERS);
	AddTooltip(IDC_BUTTON_EDIT_SERVERS, IDS_STRING_BUTTON_EDIT_SERVERS);
	AddTooltip(IDC_CHECK_STATIC_DEVICES, IDS_STRING_CHECK_STATIC_DEVICES);
	AddTooltip(IDC_BUTTON_EDIT_DEVICES, IDS_STRING_BUTTON_EDIT_DEVICES);
	AddTooltip(IDC_CHECK_STATIC_QUALITIES, IDS_STRING_CHECK_STATIC_QUALITIES);
	AddTooltip(IDC_BUTTON_EDIT_QUALITY, IDS_STRING_BUTTON_EDIT_QUALITY);
	AddTooltip(IDC_CHECK_STATIC_PROFILES, IDS_STRING_CHECK_STATIC_PROFILES);
	AddTooltip(IDC_BUTTON_EDIT_PROFILES, IDS_STRING_BUTTON_EDIT_PROFILES);
	AddTooltip(IDC_COMBO_ACCESS_TYPE, IDS_STRING_COMBO_ACCESS_TYPE);

	SetButtonImage(IDB_PNG_EDIT, m_wndBtnToggleEdit);
	SetButtonImage(IDB_PNG_SAVE, m_wndBtnSaveConf);
	SetButtonImage(IDB_PNG_SAVE_AS, m_wndBtnSaveAsConf);

	SetButtonImage(IDB_PNG_EDIT, m_wndBtnServers);
	SetButtonImage(IDB_PNG_EDIT, m_wndBtnDevices);
	SetButtonImage(IDB_PNG_EDIT, m_wndBtnQualities);
	SetButtonImage(IDB_PNG_EDIT, m_wndBtnProfiles);

	m_pSaveButton = &m_wndBtnSaveConf;

	m_wndBtnToggleEdit.EnableWindow(TRUE);
	GetPropertySheet()->m_plugin->load_plugin_parameters(GetPropertySheet()->m_initial_cred.get_config());

	FillConfigs();
	FillControlsCommon();

	RestoreWindowPos(GetSafeHwnd(), REG_CONFIG_WINDOW_POS);
	AllowSave(false);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPluginConfigPage::OnSetActive()
{
	__super::OnSetActive();

	FillControlsCommon();

	return TRUE;
}

void CPluginConfigPage::FillConfigs()
{
	m_wndPluginConfigs.ResetContent();
	int cur_idx = 0;
	for (const auto& entry : GetPropertySheet()->m_configs)
	{
		if (!GetPropertySheet()->m_initial_cred.config.empty() && entry == GetPropertySheet()->m_initial_cred.get_config())
		{
			std::wstring name = entry + L" (Current)";
			int idx = m_wndPluginConfigs.AddString(name.c_str());
			cur_idx = idx;
		}
		else
		{
			m_wndPluginConfigs.AddString(entry.c_str());
		}
	}

	m_wndPluginConfigs.SetCurSel(cur_idx);
}

std::wstring CPluginConfigPage::GetSelectedConfig()
{
	size_t idx = (size_t)m_wndPluginConfigs.GetCurSel();
	const auto& configs = GetPropertySheet()->m_configs;
	if (idx < 1 || idx >= configs.size()) return L"";

	return configs[idx];
}

void CPluginConfigPage::UpdateControls()
{
	UpdateData(TRUE);

	const auto& plugin = GetPropertySheet()->m_plugin;
	bool custom = plugin->get_plugin_type() == PluginType::enCustom;
	bool enable = GetPropertySheet()->m_allow_edit;

	m_wndPluginConfigs.EnableWindow(!enable);

	// buttons
	m_wndBtnSaveConf.EnableWindow(enable && GetPropertySheet()->m_allow_save && !GetSelectedConfig().empty());

	// common
	m_wndName.EnableWindow(enable);
	m_wndTitle.EnableWindow(enable);
	m_wndProviderUrl.EnableWindow(enable);
	m_wndChkSquareIcons.EnableWindow(enable);
	m_wndAccessType.EnableWindow(enable && custom);

	// servers
	m_wndChkStaticServers.SetCheck(plugin->get_static_servers());
	m_wndChkStaticServers.EnableWindow(enable);
	m_wndBtnServers.EnableWindow(enable && plugin->get_static_servers());

	// devices
	m_wndChkStaticDevices.SetCheck(plugin->get_static_devices());
	m_wndChkStaticDevices.EnableWindow(enable);
	m_wndBtnDevices.EnableWindow(enable && plugin->get_static_devices());

	// qualities
	m_wndChkStaticQualities.SetCheck(plugin->get_static_qualities());
	m_wndChkStaticQualities.EnableWindow(enable);
	m_wndBtnQualities.EnableWindow(enable && plugin->get_static_qualities());

	// profiles
	m_wndChkStaticProfiles.SetCheck(plugin->get_static_profiles());
	m_wndChkStaticProfiles.EnableWindow(enable);
	m_wndBtnProfiles.EnableWindow(enable && plugin->get_static_profiles());
}

void CPluginConfigPage::FillControlsCommon()
{
	const auto& plugin = GetPropertySheet()->m_plugin;
	if (!plugin) return;

	m_wndAccessType.SetCurSel((int)plugin->get_access_type());
	m_wndChkSquareIcons.SetCheck(plugin->get_square_icons() != false);

	m_Name = plugin->get_name().c_str();
	m_Title = plugin->get_title().c_str();
	m_ProviderUrl = plugin->get_provider_url().c_str();

	UpdateData(FALSE);

	UpdateControls();
}

void CPluginConfigPage::OnBnClickedButtonToggleEditConfig()
{
	if (GetPropertySheet()->m_allow_edit && GetPropertySheet()->m_allow_save)
	{
		if (IDNO == AfxMessageBox(L"Changes not saved. Continue?", MB_ICONWARNING | MB_YESNO))
			return;

		AllowSave(false);
	}

	GetPropertySheet()->m_allow_edit = !GetPropertySheet()->m_allow_edit;

	AllowSave(false);

	UpdateControls();
}

void CPluginConfigPage::OnBnClickedButtonSaveConfig()
{
	auto name = GetSelectedConfig();
	if (name.empty()) return;

	if (GetPropertySheet()->m_plugin->save_plugin_parameters(name))
	{
		AllowSave(false);
	}
	else
	{
		AfxMessageBox(IDS_STRING_ERR_SAVE_CONFIG, MB_ICONERROR | MB_OK);
	}
}

void CPluginConfigPage::OnBnClickedButtonSaveAsConfig()
{
	CNewConfigDlg dlg;
	if (dlg.DoModal() != IDOK || dlg.m_name.IsEmpty())
		return;

	std::filesystem::path new_conf = dlg.m_name.GetString();
	if (new_conf.extension().empty())
		new_conf += (L".json");

	if (!GetPropertySheet()->m_plugin->save_plugin_parameters(new_conf))
	{
		AfxMessageBox(IDS_STRING_ERR_SAVE_CONFIG, MB_ICONERROR | MB_OK);
	}
	else
	{
		AllowSave(false);
		GetPropertySheet()->m_configs.emplace_back(new_conf);
		FillConfigs();
		m_wndPluginConfigs.SetCurSel((int)GetPropertySheet()->m_configs.size() - 1);
		UpdateData(FALSE);
	}
}

void CPluginConfigPage::OnCbnSelchangeComboAccessType()
{
	GetPropertySheet()->m_plugin->set_access_type((AccountAccessType)m_wndAccessType.GetCurSel());
	AllowSave();
	FillControlsCommon();
}

void CPluginConfigPage::OnBnClickedButtonEditServers()
{
	CFillParamsInfoDlg dlg;
	dlg.m_type = 0;
	dlg.m_paramsList = GetPropertySheet()->m_plugin->get_servers_list();
	dlg.m_readonly = !GetPropertySheet()->m_allow_edit;

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
	dlg.m_readonly = !GetPropertySheet()->m_allow_edit;

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
	dlg.m_readonly = !GetPropertySheet()->m_allow_edit;

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
	dlg.m_readonly = !GetPropertySheet()->m_allow_edit;

	if (dlg.DoModal() == IDOK)
	{
		AllowSave();
		GetPropertySheet()->m_plugin->set_profiles_list(dlg.m_paramsList);
	}
}

void CPluginConfigPage::OnCbnSelchangeComboPluginConfig()
{
	AllowSave(false);
	GetPropertySheet()->m_plugin->load_plugin_parameters(GetSelectedConfig());
	FillControlsCommon();
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
	AllowSave();
	GetPropertySheet()->m_plugin->set_name(m_Name.GetString());
}

void CPluginConfigPage::OnEnChangeEditTitle()
{
	AllowSave();
	GetPropertySheet()->m_plugin->set_title(m_Title.GetString());
}

void CPluginConfigPage::OnEnChangeEditProviderUrl()
{
	AllowSave();
	GetPropertySheet()->m_plugin->set_provider_url(m_ProviderUrl.GetString());
}
