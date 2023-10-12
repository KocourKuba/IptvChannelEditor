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
#include "PluginConfigPageTV.h"
#include "FillParamsInfoDlg.h"

#include "UtilsLib/inet_utils.h"

// CPluginConfigPageTV dialog

IMPLEMENT_DYNAMIC(CPluginConfigPageTV, CTooltipPropertyPage)

BEGIN_MESSAGE_MAP(CPluginConfigPageTV, CTooltipPropertyPage)
	ON_CBN_SELCHANGE(IDC_COMBO_STREAM_TYPE, &CPluginConfigPageTV::OnCbnSelchangeComboStreamType)
	ON_BN_CLICKED(IDC_BUTTON_PLAYLIST_SHOW, &CPluginConfigPageTV::OnBnClickedButtonPlaylistShow)
	ON_BN_CLICKED(IDC_BUTTON_STREAM_PARSE, &CPluginConfigPageTV::OnBnClickedButtonStreamRegexTest)
	ON_CBN_SELCHANGE(IDC_COMBO_PLAYLIST_TEMPLATE, &CPluginConfigPageTV::OnCbnSelchangeComboPlaylistTemplate)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_TEMPLATES, &CPluginConfigPageTV::OnBnClickedButtonEditTemplates)
	ON_EN_CHANGE(IDC_EDIT_PLAYLIST_DOMAIN, &CPluginConfigPageTV::SaveParameters)
	ON_BN_CLICKED(IDC_CHECK_PER_CHANNEL_TOKEN, &CPluginConfigPageTV::SaveParameters)
	ON_BN_CLICKED(IDC_CHECK_EPG_ID_FROM_ID, &CPluginConfigPageTV::SaveParameters)
	ON_EN_CHANGE(IDC_EDIT_PARSE_PATTERN, &CPluginConfigPageTV::SaveParameters)
	ON_BN_CLICKED(IDC_CHECK_MAP_TAG_TO_ID, &CPluginConfigPageTV::SaveParameters)
	ON_EN_CHANGE(IDC_EDIT_PLAYLIST_TEMPLATE, &CPluginConfigPageTV::SaveParameters)
	ON_EN_CHANGE(IDC_EDIT_DURATION, &CPluginConfigPageTV::SaveParameters)
	ON_EN_CHANGE(IDC_EDIT_STREAM_TEMPLATE, &CPluginConfigPageTV::SaveParameters)
	ON_EN_CHANGE(IDC_EDIT_STREAM_ARC_TEMPLATE, &CPluginConfigPageTV::SaveParameters)
	ON_EN_CHANGE(IDC_EDIT_CUSTOM_STREAM_ARC_TEMPLATE, &CPluginConfigPageTV::SaveParameters)
	ON_EN_CHANGE(IDC_EDIT_DUNE_PARAMS, &CPluginConfigPageTV::SaveParameters)
	ON_CBN_SELCHANGE(IDC_COMBO_TAGS, &CPluginConfigPageTV::SaveParameters)
	ON_CBN_SELCHANGE(IDC_COMBO_CATCHUP_TYPE, &CPluginConfigPageTV::SaveParameters)
END_MESSAGE_MAP()

CPluginConfigPageTV::CPluginConfigPageTV() : CTooltipPropertyPage(IDD_DIALOG_PLUGIN_CONFIG_TV)
{
}

void CPluginConfigPageTV::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_EDIT_PARSE_PATTERN, m_wndParseStream);
	DDX_Text(pDX, IDC_EDIT_PARSE_PATTERN, m_ParseStream);
	DDX_Control(pDX, IDC_EDIT_DURATION, m_wndDuration);
	DDX_Text(pDX, IDC_EDIT_DURATION, m_Duration);
	DDX_Control(pDX, IDC_EDIT_STREAM_TEMPLATE, m_wndStreamTemplate);
	DDX_Text(pDX, IDC_EDIT_STREAM_TEMPLATE, m_StreamTemplate);
	DDX_Control(pDX, IDC_EDIT_STREAM_ARC_TEMPLATE, m_wndStreamArchiveTemplate);
	DDX_Text(pDX, IDC_EDIT_STREAM_ARC_TEMPLATE, m_StreamArchiveTemplate);
	DDX_Control(pDX, IDC_EDIT_CUSTOM_STREAM_ARC_TEMPLATE, m_wndCustomStreamArchiveTemplate);
	DDX_Text(pDX, IDC_EDIT_CUSTOM_STREAM_ARC_TEMPLATE, m_CustomStreamArchiveTemplate);
	DDX_Control(pDX, IDC_COMBO_STREAM_TYPE, m_wndStreamType);
	DDX_Control(pDX, IDC_COMBO_CATCHUP_TYPE, m_wndCatchupType);
	DDX_Control(pDX, IDC_COMBO_PLAYLIST_TEMPLATE, m_wndPlaylistTemplates);
	DDX_Control(pDX, IDC_BUTTON_EDIT_TEMPLATES, m_wndBtnEditTemplates);
	DDX_Control(pDX, IDC_EDIT_PLAYLIST_DOMAIN, m_wndPlaylistDomain);
	DDX_Text(pDX, IDC_EDIT_PLAYLIST_DOMAIN, m_PlaylistDomain);
	DDX_Control(pDX, IDC_EDIT_PLAYLIST_TEMPLATE, m_wndPlaylistTemplate);
	DDX_Text(pDX, IDC_EDIT_PLAYLIST_TEMPLATE, m_PlaylistTemplate);
	DDX_Control(pDX, IDC_BUTTON_PLAYLIST_SHOW, m_wndBtnPlaylistTest);
	DDX_Control(pDX, IDC_BUTTON_STREAM_PARSE, m_wndBtnStreamParseTest);
	DDX_Control(pDX, IDC_COMBO_TAGS, m_wndTags);
	DDX_Control(pDX, IDC_CHECK_MAP_TAG_TO_ID, m_wndCheckMapTags);
	DDX_Control(pDX, IDC_CHECK_PER_CHANNEL_TOKEN, m_wndChkPerChannelToken);
	DDX_Control(pDX, IDC_CHECK_EPG_ID_FROM_ID, m_wndChkEpgIdFromID);
	DDX_Control(pDX, IDC_EDIT_DUNE_PARAMS, m_wndDuneParams);
	DDX_Text(pDX, IDC_EDIT_DUNE_PARAMS, m_DuneParams);
}

BOOL CPluginConfigPageTV::OnInitDialog()
{
	__super::OnInitDialog();

	AddTooltip(IDC_EDIT_PLAYLIST_TEMPLATE, IDS_STRING_EDIT_PLAYLIST_TEMPLATE);
	AddTooltip(IDC_EDIT_PARSE_PATTERN, IDS_STRING_EDIT_PARSE_PATTERN);
	AddTooltip(IDC_BUTTON_PLAYLIST_SHOW, IDS_STRING_BUTTON_PLAYLIST_SHOW);
	AddTooltip(IDC_BUTTON_STREAM_PARSE, IDS_STRING_BUTTON_STREAM_PARSE);
	AddTooltip(IDC_EDIT_DURATION, IDS_STRING_EDIT_DURATION);
	AddTooltip(IDC_EDIT_STREAM_TEMPLATE, IDS_STRING_EDIT_STREAM_TEMPLATE);
	AddTooltip(IDC_EDIT_STREAM_ARC_TEMPLATE, IDS_STRING_EDIT_STREAM_ARC_TEMPLATE);
	AddTooltip(IDC_EDIT_CUSTOM_STREAM_ARC_TEMPLATE, IDS_STRING_EDIT_STREAM_ARC_TEMPLATE);
	AddTooltip(IDC_COMBO_STREAM_TYPE, IDS_STRING_COMBO_STREAM_TYPE);
	AddTooltip(IDC_COMBO_CATCHUP_TYPE, IDS_STRING_COMBO_CATCHUP_TYPE);
	AddTooltip(IDC_COMBO_PLAYLIST_TEMPLATE, IDS_STRING_COMBO_PLAYLIST_TEMPLATE);
	AddTooltip(IDC_BUTTON_EDIT_TEMPLATES, IDS_STRING_BUTTON_EDIT_TEMPLATES);
	AddTooltip(IDC_CHECK_PER_CHANNEL_TOKEN, IDS_STRING_CHECK_PER_CHANNEL_TOKEN);
	AddTooltip(IDC_COMBO_TAGS, IDS_STRING_COMBO_TAGS);
	AddTooltip(IDC_CHECK_MAP_TAG_TO_ID, IDS_STRING_CHECK_MAP_TAG_TO_ID);
	AddTooltip(IDC_CHECK_EPG_ID_FROM_ID, IDS_STRING_CHECK_EPG_ID_FROM_ID);
	AddTooltip(IDC_EDIT_DUNE_PARAMS, IDS_STRING_EDIT_DUNE_PARAMS);
	AddTooltip(IDC_EDIT_PLAYLIST_DOMAIN, IDS_STRING_EDIT_PLAYLIST_DOMAIN);

	m_wndToolTipCtrl.SetDelayTime(TTDT_AUTOPOP, 10000);
	m_wndToolTipCtrl.SetDelayTime(TTDT_INITIAL, 500);
	m_wndToolTipCtrl.SetMaxTipWidth(300);
	m_wndToolTipCtrl.Activate(TRUE);

	SetButtonImage(IDB_PNG_EDIT, m_wndBtnEditTemplates);

	AssignMacros();

	FillControls();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPluginConfigPageTV::OnSetActive()
{
	__super::OnSetActive();

	FillControls();

	return TRUE;
}

void CPluginConfigPageTV::AssignMacros()
{
	std::vector<std::wstring> stream_params =
	{
		L"(?<id>)",
		L"(?<epg_id>)",
		L"(?<scheme>)",
		L"(?<domain>)",
		L"(?<port>)",
		L"(?<login>)",
		L"(?<password>)",
		L"(?<subdomain>)",
		L"(?<token>)",
		L"(?<int_id>)",
		L"(?<quality>)",
		L"(?<host>)",
		L"(?<var1>)",
		L"(?<var2>)",
	};
	m_wndParseStream.SetTemplateParams(stream_params);

	std::vector<std::wstring> pl_params =
	{
		REPL_API_URL,
		REPL_PL_DOMAIN,
		REPL_SUBDOMAIN,
		REPL_LOGIN,
		REPL_PASSWORD,
		REPL_TOKEN,
		REPL_SERVER_ID,
		REPL_DEVICE_ID,
		REPL_QUALITY_ID,
	};

	m_wndPlaylistTemplate.SetTemplateParams(pl_params);

	std::vector<std::wstring> strm_params(std::move(pl_params));
	strm_params.insert(strm_params.end(),
					   {
						   REPL_CGI_BIN,
						   REPL_SCHEME,
						   REPL_DOMAIN,
						   REPL_PORT,
						   REPL_ID,
						   REPL_INT_ID,
						   REPL_HOST,
						   REPL_VAR1,
						   REPL_VAR2,
						   REPL_VAR3,
					   });

	m_wndStreamTemplate.SetTemplateParams(strm_params);

	std::vector<std::wstring> arc_params(std::move(strm_params));
	arc_params.insert(arc_params.end(),
					  {
						  REPL_LIVE_URL,
						  REPL_START,
						  REPL_STOP,
						  REPL_NOW,
						  REPL_DURATION,
						  REPL_OFFSET,
					  });

	m_wndStreamArchiveTemplate.SetTemplateParams(arc_params);
	m_wndCustomStreamArchiveTemplate.SetTemplateParams(arc_params);

	std::vector<std::wstring> dune_params =
	{
		L"{BUFFERING}",
	};
	m_wndDuneParams.SetTemplateParams(dune_params);
}

void CPluginConfigPageTV::FillControls()
{
	const auto& plugin = GetPropertySheet()->m_plugin;
	if (!plugin) return;

	m_wndPlaylistTemplates.ResetContent();
	size_t idx = 0;
	size_t current = 0;
	for (const auto& entry : plugin->get_playlist_infos())
	{
		auto name = entry.get_name();
		if (idx == plugin->get_playlist_idx())
		{
			current = idx;
			name += fmt::format(L" ({:s})", load_string_resource(IDS_STRING_CURRENT));
		}

		m_wndPlaylistTemplates.AddString(name.c_str());
		idx++;
	}

	m_wndPlaylistTemplates.SetCurSel(current);

	FillPlaylistSettings(current);

	m_wndStreamType.SetCurSel(0);

	FillControlsStream();

	UpdateControls();
}

void CPluginConfigPageTV::UpdateControls()
{

	bool readOnly = GetPropertySheet()->GetSelectedConfig().empty();

	// common
	m_wndPlaylistDomain.SetReadOnly(readOnly);
	m_wndBtnEditTemplates.EnableWindow(!readOnly);
	m_wndPlaylistTemplate.SetReadOnly(readOnly);
	m_wndChkPerChannelToken.EnableWindow(!readOnly);
	m_wndChkEpgIdFromID.EnableWindow(!readOnly);
	m_wndParseStream.SetReadOnly(readOnly);
	m_wndCheckMapTags.EnableWindow(!readOnly);
	m_wndTags.EnableWindow(m_wndCheckMapTags.GetCheck() != 0);

	// test
	m_wndBtnStreamParseTest.EnableWindow(!m_ParseStream.IsEmpty());

	// streams
	m_wndCatchupType.EnableWindow(!readOnly);
	m_wndDuration.SetReadOnly(readOnly);
	m_wndDuneParams.SetReadOnly(readOnly);
	m_wndStreamTemplate.SetReadOnly(readOnly);
	m_wndStreamArchiveTemplate.SetReadOnly(readOnly);
	m_wndCustomStreamArchiveTemplate.SetReadOnly(readOnly);
}

void CPluginConfigPageTV::FillPlaylistSettings(size_t index)
{
	const auto& plugin = GetPropertySheet()->m_plugin;
	if (!plugin) return;

	m_PlaylistDomain = plugin->get_playlist_domain(index).c_str();
	m_PlaylistTemplate = plugin->get_playlist_template(index).c_str();
	m_ParseStream = plugin->get_uri_parse_pattern(index).c_str();
	m_wndChkPerChannelToken.SetCheck(plugin->get_per_channel_token(index) != false);
	m_wndChkEpgIdFromID.SetCheck(plugin->get_epg_id_from_id(index) != false);

	if (plugin->get_tag_id_match(index).empty())
	{
		m_wndCheckMapTags.SetCheck(FALSE);
		m_wndTags.EnableWindow(FALSE);
	}
	else
	{
		int idx = m_wndTags.FindString(-1, plugin->get_tag_id_match(index).c_str());
		if (idx != CB_ERR)
		{
			m_wndTags.EnableWindow(TRUE);
			m_wndTags.SetCurSel(idx);
			m_wndCheckMapTags.SetCheck(TRUE);
		}
		else
		{
			m_wndTags.EnableWindow(FALSE);
			m_wndCheckMapTags.EnableWindow(FALSE);
		}
	}

	UpdateData(FALSE);
}

void CPluginConfigPageTV::FillControlsStream()
{
	const auto& stream = GetPropertySheet()->m_plugin->get_supported_stream(m_wndStreamType.GetCurSel());

	m_wndCatchupType.SetCurSel((int)stream.cu_type);

	m_Duration = stream.cu_duration;
	m_DuneParams = stream.dune_params.c_str();
	m_StreamTemplate = stream.uri_template.c_str();
	m_StreamArchiveTemplate = stream.uri_arc_template.c_str();
	m_CustomStreamArchiveTemplate = stream.uri_custom_arc_template.c_str();

	UpdateData(FALSE);
}

void CPluginConfigPageTV::SaveParameters()
{
	UpdateData(TRUE);

	auto& plugin = GetPropertySheet()->m_plugin;
	int idx = m_wndPlaylistTemplates.GetCurSel();

	plugin->set_uri_parse_pattern(idx, m_ParseStream.GetString());
	plugin->set_playlist_domain(idx, m_PlaylistDomain.GetString());
	plugin->set_playlist_template(idx, m_PlaylistTemplate.GetString());
	plugin->set_per_channel_token(idx, m_wndChkPerChannelToken.GetCheck() != 0);
	plugin->set_epg_id_from_id(idx, m_wndChkEpgIdFromID.GetCheck() != 0);

	auto& stream = GetPropertySheet()->m_plugin->get_supported_stream(m_wndStreamType.GetCurSel());

	stream.cu_duration = m_Duration;
	stream.set_uri_template(m_StreamTemplate.GetString());
	stream.set_uri_arc_template(m_StreamArchiveTemplate.GetString());
	stream.set_uri_custom_arc_template(m_CustomStreamArchiveTemplate.GetString());
	stream.set_dune_params(m_DuneParams.GetString());
	stream.cu_type = (CatchupType)m_wndCatchupType.GetCurSel();

	m_wndBtnStreamParseTest.EnableWindow(!m_ParseStream.IsEmpty());
	BOOL useMapTags = m_wndCheckMapTags.GetCheck() != 0;
	m_wndTags.EnableWindow(useMapTags);

	if (useMapTags)
	{
		CString tag;
		int tag_idx = m_wndTags.GetCurSel();
		if (tag_idx != CB_ERR)
		{
			m_wndTags.GetLBText(m_wndTags.GetCurSel(), tag);
			plugin->set_tag_id_match(idx, tag.GetString());
		}
	}

	AllowSave();
}

void CPluginConfigPageTV::OnCbnSelchangeComboStreamType()
{
	FillControlsStream();
}

void CPluginConfigPageTV::OnBnClickedButtonPlaylistShow()
{
	const auto& cred = GetPropertySheet()->m_selected_cred;
	TemplateParams params;
	params.token = cred.get_token();
	params.login = cred.get_login();
	params.password = cred.get_password();
	params.subdomain = cred.get_subdomain();
	params.server_idx = cred.server_id;
	params.device_idx = cred.device_id;
	params.profile_idx = cred.profile_id;
	params.quality_idx = cred.quality_id;
	params.playlist_idx = m_wndPlaylistTemplates.GetCurSel();

	CWaitCursor cur;
	const auto& url = GetPropertySheet()->m_plugin->get_playlist_url(params);
	std::stringstream data;
	if (GetPropertySheet()->m_plugin->download_url(url, data))
	{
		const auto& out_file = std::filesystem::temp_directory_path().wstring() + L"tmp.m3u8";

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

void CPluginConfigPageTV::OnBnClickedButtonStreamRegexTest()
{
	const auto& url = fmt::format(L"https://regex101.com/?regex={:s}", utils::string_replace<wchar_t>(m_ParseStream.GetString(), L"+", L"%2B"));
	ShellExecute(nullptr, _T("open"), url.c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
}

void CPluginConfigPageTV::OnCbnSelchangeComboPlaylistTemplate()
{
	FillPlaylistSettings(m_wndPlaylistTemplates.GetCurSel());
}

void CPluginConfigPageTV::OnBnClickedButtonEditTemplates()
{
	const auto& current_info = GetPropertySheet()->m_plugin->get_playlist_infos();

	std::vector<CFillParamsInfoDlg::variantInfo> info;
	for (const auto& item : current_info)
	{
		info.emplace_back(item);
	}

	CFillParamsInfoDlg dlg;
	dlg.m_type = DynamicParamsType::enPlaylistTV;
	dlg.m_paramsList = std::move(info);
	dlg.m_readonly = GetPropertySheet()->GetSelectedConfig().empty();

	if (dlg.DoModal() == IDOK)
	{
		std::vector<PlaylistTemplateInfo> playlists;
		for (const auto& item : dlg.m_paramsList)
		{
			playlists.emplace_back(std::get<PlaylistTemplateInfo>(item));
		}

		GetPropertySheet()->m_plugin->set_playlist_infos(playlists);
		FillControls();
		AllowSave();
	}
}
