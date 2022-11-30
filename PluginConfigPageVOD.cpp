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
#include "PluginConfigPageVOD.h"
#include "FillParamsInfoDlg.h"

#include "UtilsLib/inet_utils.h"

// CPluginConfigPageVOD dialog

IMPLEMENT_DYNAMIC(CPluginConfigPageVOD, CTooltipPropertyPage)

BEGIN_MESSAGE_MAP(CPluginConfigPageVOD, CTooltipPropertyPage)
	ON_CBN_SELCHANGE(IDC_COMBO_VOD_TEMPLATE, &CPluginConfigPageVOD::OnCbnSelchangeComboVodTemplate)
	ON_CBN_DROPDOWN(IDC_COMBO_VOD_TEMPLATE, &CPluginConfigPageVOD::OnCbnDropdownComboVodTemplate)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_VOD_TEMPLATES, &CPluginConfigPageVOD::OnBnClickedButtonEditVodTemplates)
	ON_EN_CHANGE(IDC_EDIT_PROVIDER_VOD_URL, &CPluginConfigPageVOD::OnEnChangeEditProviderVodUrl)
	ON_BN_CLICKED(IDC_BUTTON_VOD_TEMPLATE, &CPluginConfigPageVOD::OnBnClickedButtonVodTemplate)
	ON_EN_CHANGE(IDC_EDIT_VOD_REGEX, &CPluginConfigPageVOD::OnEnChangeEditVodRegex)
	ON_BN_CLICKED(IDC_CHECK_VOD_SUPPORT, &CPluginConfigPageVOD::OnBnClickedCheckVodSupport)
	ON_BN_CLICKED(IDC_CHECK_VOD_M3U, &CPluginConfigPageVOD::OnBnClickedCheckVodM3U)
	ON_BN_CLICKED(IDC_CHECK_VOD_FILTER, &CPluginConfigPageVOD::OnBnClickedCheckVodFilter)
END_MESSAGE_MAP()

CPluginConfigPageVOD::CPluginConfigPageVOD() : CTooltipPropertyPage(IDD_DIALOG_PLUGIN_CONFIG_VOD)
{
}

void CPluginConfigPageVOD::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_CHECK_VOD_SUPPORT, m_wndChkEnableVOD);
	DDX_Control(pDX, IDC_CHECK_VOD_M3U, m_wndChkVodM3U);
	DDX_Control(pDX, IDC_CHECK_VOD_FILTER, m_wndChkFilterSupport);
	DDX_Control(pDX, IDC_COMBO_VOD_TEMPLATE, m_wndVodTemplates);
	DDX_Control(pDX, IDC_BUTTON_EDIT_VOD_TEMPLATES, m_wndBtnEditVodTemplates);
	DDX_Control(pDX, IDC_EDIT_PROVIDER_VOD_URL, m_wndVodUrlTemplate);
	DDX_Text(pDX, IDC_EDIT_PROVIDER_VOD_URL, m_VodPlaylistTemplate);
	DDX_Control(pDX, IDC_EDIT_VOD_REGEX, m_wndVodRegex);
	DDX_Text(pDX, IDC_EDIT_VOD_REGEX, m_VodParseRegex);
	DDX_Control(pDX, IDC_BUTTON_VOD_PARSE, m_wndBtnVodParseTest);
	DDX_Control(pDX, IDC_BUTTON_VOD_TEMPLATE, m_wndBtnVodTemplateTest);
}

BOOL CPluginConfigPageVOD::OnInitDialog()
{
	__super::OnInitDialog();

	AddTooltip(IDC_CHECK_VOD_SUPPORT, IDS_STRING_CHECK_VOD_SUPPORT);
	AddTooltip(IDC_CHECK_VOD_M3U, IDS_STRING_CHECK_VOD_M3U);
	AddTooltip(IDC_COMBO_VOD_TEMPLATE, IDS_STRING_COMBO_VOD_TEMPLATE);
	AddTooltip(IDC_BUTTON_EDIT_VOD_TEMPLATES, IDS_STRING_BUTTON_EDIT_VOD_TEMPLATES);
	AddTooltip(IDC_EDIT_VOD_REGEX, IDS_STRING_EDIT_VOD_REGEX);
	AddTooltip(IDC_EDIT_PROVIDER_VOD_URL, IDS_STRING_EDIT_PROVIDER_VOD_URL);
	AddTooltip(IDC_BUTTON_VOD_PARSE, IDS_STRING_BUTTON_VOD_PARSE);
	AddTooltip(IDC_BUTTON_VOD_TEMPLATE, IDS_STRING_BUTTON_VOD_TEMPLATE);

	SetButtonImage(IDB_PNG_EDIT, m_wndBtnEditVodTemplates);

	AssignMacros();
	FillControls();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPluginConfigPageVOD::AssignMacros()
{
	std::vector<std::wstring> pl_params =
	{
		L"{SUBDOMAIN}",
		L"{LOGIN}",
		L"{PASSWORD}",
		L"{TOKEN}",
		L"{SERVER_ID}",
		L"{DEVICE_ID}",
		L"{QUALITY_ID}",
	};

	m_wndVodUrlTemplate.SetTemplateParams(pl_params);

	std::vector<std::wstring> re_params =
	{
		L"(?<title>)",
		L"(?<title_orig>)",
		L"(?<country>)",
		L"(?<year>)",
		L"(?<movie_time>)",
	};
	m_wndVodRegex.SetTemplateParams(re_params);
}

BOOL CPluginConfigPageVOD::OnSetActive()
{
	__super::OnSetActive();

	FillControls();

	return TRUE;
}

void CPluginConfigPageVOD::UpdateControls()
{
	UpdateData(TRUE);

	bool enable = !GetPropertySheet()->GetSelectedConfig().empty();

	bool enableVod = m_wndChkEnableVOD.GetCheck() != 0;
	bool enableM3U = m_wndChkVodM3U.GetCheck() != 0;

	m_wndChkEnableVOD.EnableWindow(enable);
	m_wndChkFilterSupport.EnableWindow(enable && enableVod);
	m_wndChkVodM3U.EnableWindow(enable && enableVod);
	m_wndVodTemplates.EnableWindow(enable && enableVod);
	m_wndBtnEditVodTemplates.EnableWindow(enable && enableVod);
	m_wndVodUrlTemplate.EnableWindow(enable && enableVod);
	m_wndVodRegex.EnableWindow(enable && enableVod && enableM3U);
}

void CPluginConfigPageVOD::FillControls()
{
	auto& plugin = GetPropertySheet()->m_plugin;
	if (!plugin) return;

	m_wndChkFilterSupport.SetCheck(plugin->get_vod_filter() != false);
	m_wndChkEnableVOD.SetCheck(plugin->get_vod_support() != false);
	m_wndChkVodM3U.SetCheck(plugin->get_vod_m3u() != false);

	int vod_idx = plugin->get_vod_template_idx();
	m_wndVodTemplates.ResetContent();
	for (const auto& entry : plugin->get_vod_templates())
	{
		m_wndVodTemplates.AddString(entry.get_name().c_str());
	}

	if (vod_idx >= m_wndVodTemplates.GetCount())
	{
		vod_idx = 0;
		plugin->set_vod_template_idx(vod_idx);
	}

	m_wndVodTemplates.SetCurSel(vod_idx);
	m_VodPlaylistTemplate = plugin->get_vod_template(vod_idx).c_str();
	m_VodParseRegex = plugin->get_vod_parse_regex(vod_idx).c_str();

	UpdateData(FALSE);

	UpdateControls();
}

void CPluginConfigPageVOD::OnBnClickedButtonVodTemplate()
{
	auto& cred = GetPropertySheet()->m_initial_cred;

	TemplateParams params;
	params.token = cred.get_token();
	params.login = cred.get_login();
	params.password = cred.get_password();
	params.subdomain = cred.get_subdomain();
	params.server_idx = cred.server_id;
	params.device_idx = cred.device_id;
	params.profile_idx = cred.profile_id;
	params.quality_idx = cred.quality_id;
	params.playlist_idx = m_wndVodTemplates.GetCurSel();

	CWaitCursor cur;
	const auto& url = GetPropertySheet()->m_plugin->get_vod_url(params);
	std::stringstream data;
	if (utils::DownloadFile(url, data))
	{
		const auto& out_file = std::filesystem::temp_directory_path().wstring() + L"vod.m3u8";

		std::ofstream out_stream(out_file, std::ofstream::binary);
		out_stream << data.rdbuf();
		out_stream.close();

		STARTUPINFO			si;
		PROCESS_INFORMATION pi;
		GetStartupInfo(&si);
		CString csCmd;
		csCmd.Format(_T("\"notepad.exe\" \"%s\""), out_file.c_str());
		CreateProcess(nullptr, csCmd.GetBuffer(0), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else
	{
		AfxMessageBox(IDS_STRING_ERR_CANT_DOWNLOAD_PLAYLIST, MB_OK|MB_ICONERROR);
	}
}

void CPluginConfigPageVOD::OnEnChangeEditProviderVodUrl()
{
	UpdateData(TRUE);
	GetPropertySheet()->m_plugin->set_vod_template(m_wndVodTemplates.GetCurSel(), m_VodPlaylistTemplate.GetString());
	m_wndBtnVodTemplateTest.EnableWindow(!m_VodPlaylistTemplate.IsEmpty());

	AllowSave();
}

void CPluginConfigPageVOD::OnEnChangeEditVodRegex()
{
	UpdateData(TRUE);
	GetPropertySheet()->m_plugin->set_vod_parse_regex(m_wndVodTemplates.GetCurSel(), m_VodParseRegex.GetString());
	m_wndBtnVodParseTest.EnableWindow(!m_VodParseRegex.IsEmpty());

	AllowSave();
}

void CPluginConfigPageVOD::OnBnClickedCheckVodSupport()
{
	GetPropertySheet()->m_plugin->set_vod_support(m_wndChkEnableVOD.GetCheck() != 0);

	UpdateControls();

	AllowSave();
}

void CPluginConfigPageVOD::OnBnClickedCheckVodM3U()
{
	GetPropertySheet()->m_plugin->set_vod_m3u(m_wndChkVodM3U.GetCheck() != 0);

	UpdateControls();

	AllowSave();
}

void CPluginConfigPageVOD::OnBnClickedCheckVodFilter()
{
	GetPropertySheet()->m_plugin->set_vod_filter(m_wndChkFilterSupport.GetCheck() != 0);

	UpdateControls();

	AllowSave();
}

void CPluginConfigPageVOD::OnCbnSelchangeComboVodTemplate()
{
	const auto& plugin = GetPropertySheet()->m_plugin;
	if (!plugin) return;

	int idx = m_wndVodTemplates.GetCurSel();

	plugin->set_vod_template_idx(idx);
	m_VodPlaylistTemplate = plugin->get_current_vod_template().c_str();
	m_VodParseRegex = plugin->get_current_vod_parse_regex().c_str();

	AllowSave();

	UpdateData(FALSE);
}

void CPluginConfigPageVOD::OnCbnDropdownComboVodTemplate()
{
	UpdateData(TRUE);

	auto& plugin = GetPropertySheet()->m_plugin;
	if (!plugin) return;

	int idx = m_wndVodTemplates.GetCurSel();
	plugin->set_vod_template(idx, m_VodPlaylistTemplate.GetString());
	plugin->set_vod_parse_regex(idx, m_VodParseRegex.GetString());
}

void CPluginConfigPageVOD::OnBnClickedButtonEditVodTemplates()
{
	std::vector<DynamicParamsInfo> info;
	for (const auto& item : GetPropertySheet()->m_plugin->get_vod_templates())
	{
		info.emplace_back(item.name, item.pl_template);
	}

	CFillParamsInfoDlg dlg;
	dlg.m_type = 4;
	dlg.m_paramsList = std::move(info);
	dlg.m_readonly = !GetPropertySheet()->GetSelectedConfig().empty();

	if (dlg.DoModal() == IDOK)
	{
		AllowSave();

		std::vector<PlaylistTemplateInfo> playlists;
		for (const auto& item : dlg.m_paramsList)
		{
			playlists.emplace_back(item.id, item.name);
		}
		GetPropertySheet()->m_plugin->set_vod_templates(playlists);

		FillControls();
	}
}