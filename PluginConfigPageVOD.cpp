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
#include "PluginConfigPageVOD.h"
#include "FillParamsInfoDlg.h"

#include "UtilsLib/inet_utils.h"

// CPluginConfigPageVOD dialog

IMPLEMENT_DYNAMIC(CPluginConfigPageVOD, CTooltipPropertyPage)

BEGIN_MESSAGE_MAP(CPluginConfigPageVOD, CTooltipPropertyPage)
	ON_CBN_SELCHANGE(IDC_COMBO_VOD_TEMPLATE, &CPluginConfigPageVOD::OnCbnSelchangeComboVodTemplate)
	ON_BN_CLICKED(IDC_CHECK_PLAYLIST_SHOW_LINK, &CPluginConfigPageVOD::OnBnClickedCheckPlaylistShowLink)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_VOD_TEMPLATES, &CPluginConfigPageVOD::OnBnClickedButtonEditVodTemplates)
	ON_BN_CLICKED(IDC_BUTTON_VOD_TEMPLATE, &CPluginConfigPageVOD::OnBnClickedButtonVodTemplate)
	ON_BN_CLICKED(IDC_BUTTON_VOD_PARSE, &CPluginConfigPageVOD::OnBnClickedButtonVodParse)
	ON_CBN_DROPDOWN(IDC_COMBO_VOD_TEMPLATE, &CPluginConfigPageVOD::SaveParameters)
	ON_EN_CHANGE(IDC_EDIT_PLAYLIST_DOMAIN, &CPluginConfigPageVOD::SaveParameters)
	ON_EN_CHANGE(IDC_EDIT_PROVIDER_VOD_URL, &CPluginConfigPageVOD::SaveParameters)
	ON_EN_CHANGE(IDC_EDIT_VOD_REGEX, &CPluginConfigPageVOD::SaveParameters)
	ON_BN_CLICKED(IDC_CHECK_VOD_SUPPORT, &CPluginConfigPageVOD::SaveParameters)
	ON_BN_CLICKED(IDC_CHECK_VOD_M3U, &CPluginConfigPageVOD::SaveParameters)
	ON_BN_CLICKED(IDC_CHECK_VOD_FILTER, &CPluginConfigPageVOD::SaveParameters)
	ON_EN_CHANGE(IDC_EDIT_VOD_PREFIX, &CPluginConfigPageVOD::SaveParameters)
	ON_EN_CHANGE(IDC_EDIT_VOD_PARAMS, &CPluginConfigPageVOD::SaveParameters)
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
	DDX_Control(pDX, IDC_EDIT_PLAYLIST_DOMAIN, m_wndVodPlaylistDomain);
	DDX_Text(pDX, IDC_EDIT_PLAYLIST_DOMAIN, m_VodPlaylistDomain);
	DDX_Control(pDX, IDC_EDIT_PROVIDER_VOD_URL, m_wndVodUrlTemplate);
	DDX_Text(pDX, IDC_EDIT_PROVIDER_VOD_URL, m_VodPlaylistTemplate);
	DDX_Control(pDX, IDC_EDIT_VOD_REGEX, m_wndVodRegex);
	DDX_Text(pDX, IDC_EDIT_VOD_REGEX, m_VodParseRegex);
	DDX_Control(pDX, IDC_BUTTON_VOD_PARSE, m_wndBtnVodParseTest);
	DDX_Control(pDX, IDC_CHECK_PLAYLIST_SHOW_LINK, m_wndBtnPlaylistShow);
	DDX_Control(pDX, IDC_BUTTON_VOD_TEMPLATE, m_wndBtnVodTemplateTest);
	DDX_Control(pDX, IDC_EDIT_VOD_PREFIX, m_wndVodUrlPrefix);
	DDX_Text(pDX, IDC_EDIT_VOD_PREFIX, m_VodUrlPrefix);
	DDX_Control(pDX, IDC_EDIT_VOD_PARAMS, m_wndVodUrlParams);
	DDX_Text(pDX, IDC_EDIT_VOD_PARAMS, m_VodUrlParams);
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
	AddTooltip(IDC_CHECK_PLAYLIST_SHOW_LINK, IDS_STRING_BUTTON_PLAYLIST_SHOW_LINK);
	AddTooltip(IDC_BUTTON_VOD_TEMPLATE, IDS_STRING_BUTTON_VOD_TEMPLATE);
	AddTooltip(IDC_EDIT_PLAYLIST_DOMAIN, IDS_STRING_EDIT_VOD_PLAYLIST_DOMAIN);

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
		REPL_API_URL,
		REPL_VOD_DOMAIN,
		REPL_SUBDOMAIN,
		REPL_LOGIN,
		REPL_PASSWORD,
		REPL_TOKEN,
		REPL_SERVER_ID,
		REPL_DEVICE_ID,
		REPL_QUALITY_ID,
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

	std::vector<std::wstring> url_prefix =
	{
		REPL_CGI_BIN,
	};

	m_wndVodUrlPrefix.SetTemplateParams(url_prefix);
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

	bool readOnly = GetPropertySheet()->GetSelectedConfig().empty();

	bool enableVod = m_wndChkEnableVOD.GetCheck() != 0;
	bool enableM3U = m_wndChkVodM3U.GetCheck() != 0;

	m_wndChkEnableVOD.EnableWindow(!readOnly);
	m_wndChkFilterSupport.EnableWindow(!readOnly && enableVod);
	m_wndChkVodM3U.EnableWindow(!readOnly && enableVod);
	m_wndVodPlaylistDomain.EnableWindow(enableVod);
	m_wndVodPlaylistDomain.SetReadOnly(readOnly);
	m_wndVodTemplates.EnableWindow(enableVod);
	m_wndBtnPlaylistShow.EnableWindow(readOnly);
	m_wndBtnEditVodTemplates.EnableWindow(!readOnly && enableVod);
	m_wndVodUrlTemplate.EnableWindow(enableVod);
	m_wndVodUrlTemplate.SetReadOnly(readOnly);
	m_wndVodRegex.EnableWindow(enableVod && enableM3U);
	m_wndVodRegex.SetReadOnly(readOnly);
	m_wndVodUrlPrefix.SetReadOnly(readOnly);
	m_wndVodUrlParams.SetReadOnly(readOnly);
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
	m_VodPlaylistDomain = plugin->get_vod_domain(vod_idx).c_str();
	m_VodPlaylistTemplate = plugin->get_vod_template(vod_idx).c_str();
	m_VodParseRegex = plugin->get_vod_parse_regex(vod_idx).c_str();
	m_VodUrlPrefix = plugin->get_vod_url_prefix(vod_idx).c_str();
	m_VodUrlParams = plugin->get_vod_url_params(vod_idx).c_str();

	UpdateData(FALSE);

	UpdateControls();
}

void CPluginConfigPageVOD::SaveParameters()
{
	UpdateData(TRUE);

	auto& plugin = GetPropertySheet()->m_plugin;
	int idx = m_wndVodTemplates.GetCurSel();

	plugin->set_vod_domain(idx, m_VodPlaylistDomain.GetString());
	plugin->set_vod_template(idx, m_VodPlaylistTemplate.GetString());
	plugin->set_vod_parse_regex(idx, m_VodParseRegex.GetString());
	plugin->set_vod_url_prefix(idx, m_VodUrlPrefix.GetString());
	plugin->set_vod_url_params(idx, m_VodUrlParams.GetString());

	plugin->set_vod_support(m_wndChkEnableVOD.GetCheck() != 0);
	plugin->set_vod_m3u(m_wndChkVodM3U.GetCheck() != 0);
	plugin->set_vod_filter(m_wndChkFilterSupport.GetCheck() != 0);

	m_wndBtnVodTemplateTest.EnableWindow(!m_VodPlaylistTemplate.IsEmpty());
	m_wndBtnVodParseTest.EnableWindow(!m_VodParseRegex.IsEmpty());

	UpdateControls();

	AllowSave();
}

void CPluginConfigPageVOD::OnBnClickedButtonVodTemplate()
{
	auto& cred = GetPropertySheet()->m_selected_cred;

	TemplateParams params;
	params.s_token = GetPropertySheet()->m_plugin->get_api_token(cred);
	params.login = cred.get_login();
	params.password = cred.get_password();
	params.ott_key = cred.get_ott_key();
	params.subdomain = cred.get_subdomain();
	params.server_idx = cred.server_id;
	params.device_idx = cred.device_id;
	params.profile_idx = cred.profile_id;
	params.quality_idx = cred.quality_id;
	params.playlist_idx = m_wndVodTemplates.GetCurSel();

	CWaitCursor cur;
	const auto& url = GetPropertySheet()->m_plugin->get_vod_url(params);
	std::stringstream data;
	if (GetPropertySheet()->m_plugin->download_url(url, data))
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
		AfxMessageBox(GetPropertySheet()->m_plugin->get_download_error().c_str(), MB_ICONERROR | MB_OK);
	}
}

void CPluginConfigPageVOD::OnBnClickedButtonVodParse()
{
	const auto& url = fmt::format(L"https://regex101.com/?regex={:s}", utils::string_replace<wchar_t>(m_VodParseRegex.GetString(), L"+", L"%2B"));
	ShellExecute(nullptr, _T("open"), url.c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
}

void CPluginConfigPageVOD::OnCbnSelchangeComboVodTemplate()
{
	int idx = m_wndVodTemplates.GetCurSel();

	const auto& plugin = GetPropertySheet()->m_plugin;
	m_VodPlaylistTemplate = plugin->get_vod_template(idx).c_str();
	m_VodParseRegex = plugin->get_vod_parse_regex(idx).c_str();

	UpdateData(FALSE);

	SaveParameters();
}

void CPluginConfigPageVOD::OnBnClickedButtonEditVodTemplates()
{
	std::vector<CFillParamsInfoDlg::variantInfo> info;
	for (const auto& item : GetPropertySheet()->m_plugin->get_vod_templates())
	{
		info.emplace_back(item);
	}

	CFillParamsInfoDlg dlg;
	dlg.m_type = DynamicParamsType::enPlaylistVOD;
	dlg.m_paramsList = std::move(info);
	dlg.m_readonly = GetPropertySheet()->GetSelectedConfig().empty();

	if (dlg.DoModal() == IDOK)
	{
		std::vector<PlaylistTemplateInfo> playlists;
		for (const auto& item : dlg.m_paramsList)
		{
			playlists.emplace_back(std::get<PlaylistTemplateInfo>(item));
		}
		GetPropertySheet()->m_plugin->set_vod_templates(playlists);

		FillControls();

		AllowSave();
	}
}

void CPluginConfigPageVOD::OnBnClickedCheckPlaylistShowLink()
{
	BOOL show = m_wndBtnPlaylistShow.GetCheck();

	if (show)
	{
		auto& cred = GetPropertySheet()->m_selected_cred;

		TemplateParams params;
		params.s_token = GetPropertySheet()->m_plugin->get_api_token(cred);
		params.login = cred.get_login();
		params.password = cred.get_password();
		params.ott_key = cred.get_ott_key();
		params.subdomain = cred.get_subdomain();
		params.server_idx = cred.server_id;
		params.device_idx = cred.device_id;
		params.profile_idx = cred.profile_id;
		params.quality_idx = cred.quality_id;
		params.playlist_idx = m_wndVodTemplates.GetCurSel();

		m_VodPlaylistTemplate = GetPropertySheet()->m_plugin->get_vod_url(params).c_str();
	}
	else
	{
		FillControls();
	}

	UpdateData(FALSE);
}
