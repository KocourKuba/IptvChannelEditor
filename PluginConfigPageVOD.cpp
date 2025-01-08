/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2025): sharky72 (https://github.com/KocourKuba)

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
#include "PluginEnums.h"
#include "Constants.h"

#include "UtilsLib/inet_utils.h"

// CPluginConfigPageVOD dialog

IMPLEMENT_DYNAMIC(CPluginConfigPageVOD, CTooltipPropertyPage)

BEGIN_MESSAGE_MAP(CPluginConfigPageVOD, CTooltipPropertyPage)
	ON_CBN_SELCHANGE(IDC_COMBO_VOD_ENGINE, &CPluginConfigPageVOD::SaveParameters)
	ON_CBN_SELCHANGE(IDC_COMBO_VOD_TEMPLATE, &CPluginConfigPageVOD::OnCbnSelchangeComboVodTemplate)
	ON_BN_CLICKED(IDC_CHECK_PLAYLIST_SHOW_LINK, &CPluginConfigPageVOD::OnBnClickedCheckPlaylistShowLink)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_VOD_TEMPLATES, &CPluginConfigPageVOD::OnBnClickedButtonEditVodTemplates)
	ON_BN_CLICKED(IDC_BUTTON_VOD_TEMPLATE, &CPluginConfigPageVOD::OnBnClickedButtonVodTemplate)
	ON_BN_CLICKED(IDC_BUTTON_VOD_PARSE, &CPluginConfigPageVOD::OnBnClickedButtonVodParse)
	ON_CBN_DROPDOWN(IDC_COMBO_VOD_TEMPLATE, &CPluginConfigPageVOD::SaveParameters)
	ON_EN_CHANGE(IDC_EDIT_PROVIDER_VOD_URL, &CPluginConfigPageVOD::SaveParameters)
	ON_EN_CHANGE(IDC_EDIT_VOD_REGEX, &CPluginConfigPageVOD::SaveParameters)
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

	DDX_Control(pDX, IDC_COMBO_VOD_ENGINE, m_wndVodEngine);
	DDX_Control(pDX, IDC_CHECK_VOD_FILTER, m_wndChkFilterSupport);
	DDX_Control(pDX, IDC_COMBO_VOD_TEMPLATE, m_wndVodTemplates);
	DDX_Control(pDX, IDC_BUTTON_EDIT_VOD_TEMPLATES, m_wndBtnEditVodTemplates);
	DDX_Control(pDX, IDC_EDIT_PROVIDER_VOD_URL, m_wndVodUrlTemplate);
	DDX_Text(pDX, IDC_EDIT_PROVIDER_VOD_URL, m_VodPlaylistTemplate);
	DDX_Control(pDX, IDC_EDIT_VOD_REGEX, m_wndVodRegex);
	DDX_Text(pDX, IDC_EDIT_VOD_REGEX, m_VodParseRegexTitle);
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

	AddTooltip(IDC_COMBO_VOD_TEMPLATE, IDS_STRING_COMBO_VOD_TEMPLATE);
	AddTooltip(IDC_BUTTON_EDIT_VOD_TEMPLATES, IDS_STRING_BUTTON_EDIT_VOD_TEMPLATES);
	AddTooltip(IDC_EDIT_VOD_REGEX, IDS_STRING_EDIT_VOD_REGEX);
	AddTooltip(IDC_EDIT_PROVIDER_VOD_URL, IDS_STRING_EDIT_PROVIDER_VOD_URL);
	AddTooltip(IDC_BUTTON_VOD_PARSE, IDS_STRING_BUTTON_VOD_PARSE);
	AddTooltip(IDC_CHECK_PLAYLIST_SHOW_LINK, IDS_STRING_BUTTON_PLAYLIST_SHOW_LINK);
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
		REPL_API_URL,
		REPL_SUBDOMAIN,
		REPL_LOGIN,
		REPL_PASSWORD,
		REPL_TOKEN,
		REPL_DOMAIN_ID,
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

	bool enableVod = m_wndVodEngine.GetCurSel() != (int)VodEngine::enNone;
	bool enableM3U = m_wndVodEngine.GetCurSel() == (int)VodEngine::enM3U;

	m_wndVodEngine.EnableWindow(!readOnly);
	m_wndChkFilterSupport.EnableWindow(!readOnly && enableVod);
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

	m_wndVodEngine.ResetContent();
	for (auto it = (size_t)VodEngine::enNone; it != (size_t)VodEngine::enLast; ((size_t&)it)++)
	{
		m_wndVodEngine.AddString(enum_to_string((VodEngine)it).c_str());
	}

	m_wndChkFilterSupport.SetCheck(plugin->get_vod_filter() != false);
	m_wndVodEngine.SetCurSel((int)plugin->get_vod_engine());

	int vod_idx = (int)plugin->get_vod_info_idx();
	m_wndVodTemplates.ResetContent();
	for (const auto& entry : plugin->get_vod_infos())
	{
		m_wndVodTemplates.AddString(entry.get_name().c_str());
	}

	if (vod_idx >= m_wndVodTemplates.GetCount())
	{
		vod_idx = 0;
		plugin->set_vod_info_idx(vod_idx);
	}

	m_wndVodTemplates.SetCurSel(vod_idx);
	const auto& info = plugin->get_vod_info(vod_idx);
	m_VodPlaylistTemplate = info.get_pl_template().c_str();
	m_VodParseRegexTitle = info.get_parse_regex_title().c_str();
	m_VodUrlPrefix = info.get_url_prefix().c_str();
	m_VodUrlParams = info.get_url_params().c_str();

	UpdateData(FALSE);

	UpdateControls();
}

void CPluginConfigPageVOD::SaveParameters()
{
	UpdateData(TRUE);

	int idx = m_wndVodTemplates.GetCurSel();
	auto& plugin = GetPropertySheet()->m_plugin;
	auto& info = plugin->get_vod_info(idx);

	if (!m_wndBtnPlaylistShow.GetCheck())
	{
		info.set_pl_template(m_VodPlaylistTemplate.GetString());
	}

	info.set_parse_regex_title(m_VodParseRegexTitle.GetString());
	info.set_url_prefix(m_VodUrlPrefix.GetString());
	info.set_url_params(m_VodUrlParams.GetString());

	plugin->set_vod_engine((VodEngine)m_wndVodEngine.GetCurSel());
	plugin->set_vod_filter(m_wndChkFilterSupport.GetCheck() != 0);

	m_wndBtnVodTemplateTest.EnableWindow(!m_VodPlaylistTemplate.IsEmpty());
	m_wndBtnVodParseTest.EnableWindow(!m_VodParseRegexTitle.IsEmpty());

	UpdateControls();

	AllowSave();
}

void CPluginConfigPageVOD::OnBnClickedButtonVodTemplate()
{
	const auto& plugin = GetPropertySheet()->m_plugin;
	auto& cred = GetPropertySheet()->m_selected_cred;

	TemplateParams params;
	params.creds = cred;
	params.playlist_idx = m_wndVodTemplates.GetCurSel();

	plugin->get_api_token(params);
	plugin->update_provider_params(params);

	CWaitCursor cur;

	m_dl.SetUrl(plugin->get_vod_url(params));
	m_dl.SetUserAgent(plugin->get_user_agent());

	std::stringstream data;
	if (m_dl.DownloadFile(data))
	{
		const auto& out_file = std::filesystem::temp_directory_path().wstring() + L"vod.m3u8";

		std::ofstream out_stream(out_file, std::ofstream::binary);
		data.seekg(0);
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
		AfxMessageBox(m_dl.GetLastErrorMessage().c_str(), MB_ICONERROR | MB_OK);
	}
}

void CPluginConfigPageVOD::OnBnClickedButtonVodParse()
{
	const auto& url = fmt::format(L"https://regex101.com/?regex={:s}", utils::string_replace<wchar_t>(m_VodParseRegexTitle.GetString(), L"+", L"%2B"));
	ShellExecute(nullptr, _T("open"), url.c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
}

void CPluginConfigPageVOD::OnCbnSelchangeComboVodEngine()
{
	// TODO: Add your control notification handler code here
}

void CPluginConfigPageVOD::OnCbnSelchangeComboVodTemplate()
{
	int idx = m_wndVodTemplates.GetCurSel();

	auto& info = GetPropertySheet()->m_plugin->get_vod_info(idx);
	m_VodPlaylistTemplate = info.get_pl_template().c_str();
	m_VodParseRegexTitle = info.get_parse_regex_title().c_str();

	UpdateData(FALSE);

	SaveParameters();
}

void CPluginConfigPageVOD::OnBnClickedButtonEditVodTemplates()
{
	std::vector<CFillParamsInfoDlg::variantInfo> info;
	for (const auto& item : GetPropertySheet()->m_plugin->get_vod_infos())
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
		GetPropertySheet()->m_plugin->set_vod_infos(playlists);

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
		params.creds = cred;

		GetPropertySheet()->m_plugin->get_api_token(params);
		params.playlist_idx = m_wndVodTemplates.GetCurSel();

		GetPropertySheet()->m_plugin->update_provider_params(params);

		m_VodPlaylistTemplate = GetPropertySheet()->m_plugin->get_vod_url(params).c_str();
	}
	else
	{
		FillControls();
	}

	UpdateData(FALSE);
}
