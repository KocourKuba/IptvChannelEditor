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
#include "PluginConfigPageTV.h"
#include "FillParamsInfoDlg.h"

#include "UtilsLib/inet_utils.h"

// CPluginConfigPageTV dialog

IMPLEMENT_DYNAMIC(CPluginConfigPageTV, CTooltipPropertyPage)

BEGIN_MESSAGE_MAP(CPluginConfigPageTV, CTooltipPropertyPage)
	ON_CBN_SELCHANGE(IDC_COMBO_STREAM_TYPE, &CPluginConfigPageTV::OnCbnSelchangeComboStreamType)
	ON_BN_CLICKED(IDC_BUTTON_PLAYLIST_SHOW, &CPluginConfigPageTV::OnBnClickedButtonPlaylistShow)
	ON_BN_CLICKED(IDC_BUTTON_STREAM_PARSE, &CPluginConfigPageTV::OnBnClickedButtonStreamRegexTest)
	ON_BN_CLICKED(IDC_CHECK_PER_CHANNEL_TOKEN, &CPluginConfigPageTV::OnBnClickedCheckPerChannelToken)
	ON_EN_CHANGE(IDC_EDIT_PARSE_PATTERN, &CPluginConfigPageTV::OnEnChangeEditParsePattern)
	ON_BN_CLICKED(IDC_CHECK_MAP_TAG_TO_ID, &CPluginConfigPageTV::OnBnClickedCheckMapTagToId)
	ON_CBN_SELCHANGE(IDC_COMBO_PLAYLIST_TEMPLATE, &CPluginConfigPageTV::OnCbnSelchangeComboPlaylistTemplate)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_TEMPLATES, &CPluginConfigPageTV::OnBnClickedButtonEditTemplates)
	ON_EN_CHANGE(IDC_EDIT_PLAYLIST_TEMPLATE, &CPluginConfigPageTV::OnEnChangeEditPlaylistTemplate)
	ON_EN_CHANGE(IDC_EDIT_SHIFT_SUBST, &CPluginConfigPageTV::OnEnChangeEditShiftSubst)
	ON_EN_CHANGE(IDC_EDIT_DURATION, &CPluginConfigPageTV::OnEnChangeEditDuration)
	ON_EN_CHANGE(IDC_EDIT_STREAM_TEMPLATE, &CPluginConfigPageTV::OnEnChangeEditStreamTemplate)
	ON_EN_CHANGE(IDC_EDIT_STREAM_ARC_TEMPLATE, &CPluginConfigPageTV::OnEnChangeEditStreamArcTemplate)
	ON_EN_CHANGE(IDC_EDIT_CUSTOM_STREAM_ARC_TEMPLATE, &CPluginConfigPageTV::OnEnChangeEditCustomStreamArcTemplate)
	ON_EN_CHANGE(IDC_EDIT_DUNE_PARAMS, &CPluginConfigPageTV::OnEnChangeEditDuneParams)
	ON_CBN_SELCHANGE(IDC_COMBO_TAGS, &CPluginConfigPageTV::OnCbnSelchangeComboTags)
	ON_CBN_SELCHANGE(IDC_COMBO_CATCHUP_TYPE, &CPluginConfigPageTV::OnCbnSelchangeComboCatchupType)
END_MESSAGE_MAP()

CPluginConfigPageTV::CPluginConfigPageTV() : CTooltipPropertyPage(IDD_DIALOG_PLUGIN_CONFIG_TV)
{
}

void CPluginConfigPageTV::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_EDIT_PARSE_PATTERN, m_wndParseStream);
	DDX_Text(pDX, IDC_EDIT_PARSE_PATTERN, m_ParseStream);
	DDX_Control(pDX, IDC_EDIT_SHIFT_SUBST, m_wndSubst);
	DDX_Text(pDX, IDC_EDIT_SHIFT_SUBST, m_Subst);
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
	DDX_Control(pDX, IDC_EDIT_PLAYLIST_TEMPLATE, m_wndPlaylistTemplate);
	DDX_Text(pDX, IDC_EDIT_PLAYLIST_TEMPLATE, m_PlaylistTemplate);
	DDX_Control(pDX, IDC_BUTTON_PLAYLIST_SHOW, m_wndBtnPlaylistTest);
	DDX_Control(pDX, IDC_BUTTON_STREAM_PARSE, m_wndBtnStreamParseTest);
	DDX_Control(pDX, IDC_COMBO_TAGS, m_wndTags);
	DDX_Control(pDX, IDC_CHECK_MAP_TAG_TO_ID, m_wndCheckMapTags);
	DDX_Control(pDX, IDC_CHECK_PER_CHANNEL_TOKEN, m_wndChkPerChannelToken);
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
	AddTooltip(IDC_EDIT_SHIFT_SUBST, IDS_STRING_EDIT_SHIFT_SUBST);
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
	AddTooltip(IDC_CHECK_MAP_TAG_TO_ID, IDS_CHECK_MAP_TAG_TO_ID);
	AddTooltip(IDC_EDIT_DUNE_PARAMS, IDS_STRING_EDIT_DUNE_PARAMS);

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
		L"(?<domain>)",
		L"(?<port>)",
		L"(?<login>)",
		L"(?<password>)",
		L"(?<subdomain>)",
		L"(?<token>)",
		L"(?<int_id>)",
		L"(?<quality>)",
		L"(?<host>)"
	};
	m_wndParseStream.SetTemplateParams(stream_params);

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

	m_wndPlaylistTemplate.SetTemplateParams(pl_params);

	std::vector<std::wstring> strm_params(std::move(pl_params));
	strm_params.insert(strm_params.end(),
					   {
						   L"{DOMAIN}",
						   L"{PORT}",
						   L"{ID}",
						   L"{INT_ID}",
						   L"{HOST}",
					   });

	m_wndStreamTemplate.SetTemplateParams(strm_params);

	std::vector<std::wstring> arc_params(std::move(strm_params));
	arc_params.insert(arc_params.end(),
					  {
						  L"{LIVE_URL}",
						  L"{CU_SUBST}",
						  L"{START}",
						  L"{NOW}",
						  L"{DURATION}",
						  L"{OFFSET}",
					  });

	m_wndStreamArchiveTemplate.SetTemplateParams(arc_params);
	m_wndCustomStreamArchiveTemplate.SetTemplateParams(arc_params);

	std::vector<std::wstring> dune_params =
	{
		L"{BUFFERING}",
	};
	m_wndDuneParams.SetTemplateParams(dune_params);
}

void CPluginConfigPageTV::UpdateControls()
{
	UpdateData(TRUE);

	bool enable = !GetPropertySheet()->GetSelectedConfig().empty();

	// common
	m_wndPlaylistTemplates.EnableWindow(enable);
	m_wndBtnEditTemplates.EnableWindow(enable);
	m_wndPlaylistTemplate.EnableWindow(enable);
	m_wndChkPerChannelToken.EnableWindow(enable);
	m_wndParseStream.EnableWindow(enable);
	m_wndCheckMapTags.EnableWindow(enable);
	m_wndTags.EnableWindow(enable && m_wndCheckMapTags.GetCheck() != 0);

	// test
	m_wndBtnStreamParseTest.EnableWindow(!m_ParseStream.IsEmpty());

	// streams
	m_wndStreamType.EnableWindow(enable);
	m_wndCatchupType.EnableWindow(enable);
	m_wndSubst.EnableWindow(enable);
	m_wndDuration.EnableWindow(enable);
	m_wndDuneParams.EnableWindow(enable);
	m_wndStreamTemplate.EnableWindow(enable);
	m_wndStreamArchiveTemplate.EnableWindow(enable);
	m_wndCustomStreamArchiveTemplate.EnableWindow(enable);
}

void CPluginConfigPageTV::FillControls()
{
	const auto& plugin = GetPropertySheet()->m_plugin;
	if (!plugin) return;

	m_wndChkPerChannelToken.SetCheck(plugin->get_per_channel_token() != false);

	m_wndPlaylistTemplates.ResetContent();
	for (const auto& entry : plugin->get_playlist_infos())
	{
		m_wndPlaylistTemplates.AddString(entry.get_name().c_str());
	}

	m_wndPlaylistTemplates.SetCurSel(plugin->get_playlist_template_idx());
	FillPlaylistSettings();

	m_wndStreamType.SetCurSel(0);
	FillControlsStream();
	UpdateControls();
}

void CPluginConfigPageTV::FillPlaylistSettings()
{
	const auto& plugin = GetPropertySheet()->m_plugin;
	if (!plugin) return;

	m_PlaylistTemplate = plugin->get_current_playlist_template().c_str();
	m_ParseStream = plugin->get_current_parse_pattern().c_str();

	if (plugin->get_current_tag_id_match().empty())
	{
		m_wndCheckMapTags.SetCheck(FALSE);
		m_wndTags.EnableWindow(FALSE);
	}
	else
	{
		int idx = m_wndTags.FindString(-1, plugin->get_current_tag_id_match().c_str());
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
	auto& plugin = GetPropertySheet()->m_plugin;
	if (!plugin) return;

	const auto& stream = GetSupportedStream();

	BOOL enableDuration = (stream.cu_type == CatchupType::cu_flussonic);
	if (enableDuration)
	{
		m_Duration = stream.cu_duration;
	}

	m_Subst = stream.cu_subst.c_str();

	m_wndCatchupType.SetCurSel((int)stream.cu_type);
	m_wndDuration.EnableWindow(enableDuration);

	m_DuneParams = stream.dune_params.c_str();
	m_StreamTemplate = stream.uri_template.c_str();
	m_StreamArchiveTemplate = stream.uri_arc_template.c_str();
	m_CustomStreamArchiveTemplate = stream.uri_custom_arc_template.c_str();

	UpdateData(FALSE);
}

StreamParameters& CPluginConfigPageTV::GetSupportedStream()
{
	return GetPropertySheet()->m_plugin->get_supported_stream(m_wndStreamType.GetCurSel());
}

void CPluginConfigPageTV::OnCbnSelchangeComboStreamType()
{
	FillControlsStream();
}

void CPluginConfigPageTV::OnCbnSelchangeComboCatchupType()
{
	UpdateData(TRUE);
	GetSupportedStream().cu_type = (CatchupType)m_wndCatchupType.GetCurSel();
	AllowSave();
}

void CPluginConfigPageTV::OnBnClickedButtonPlaylistShow()
{
	const auto& cred = GetPropertySheet()->m_initial_cred;
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
	if (utils::DownloadFile(url, data))
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
}

void CPluginConfigPageTV::OnBnClickedButtonStreamRegexTest()
{
	ShellExecute(nullptr, _T("open"), L"https://regex101.com/", nullptr, nullptr, SW_SHOWDEFAULT);
}

void CPluginConfigPageTV::OnEnChangeEditParsePattern()
{
	UpdateData(TRUE);
	m_wndBtnStreamParseTest.EnableWindow(!m_ParseStream.IsEmpty());
	GetPropertySheet()->m_plugin->set_uri_parse_pattern(m_wndPlaylistTemplates.GetCurSel(), m_ParseStream.GetString());
	AllowSave();
}

void CPluginConfigPageTV::OnEnChangeEditPlaylistTemplate()
{
	UpdateData(TRUE);
	GetPropertySheet()->m_plugin->set_playlist_template(m_wndPlaylistTemplates.GetCurSel(), m_PlaylistTemplate.GetString());
	AllowSave();
}

void CPluginConfigPageTV::OnEnChangeEditShiftSubst()
{
	UpdateData(TRUE);
	GetSupportedStream().set_shift_replace(m_Subst.GetString());
	AllowSave();
}

void CPluginConfigPageTV::OnEnChangeEditDuration()
{
	UpdateData(TRUE);
	GetSupportedStream().cu_duration = m_Duration;
	AllowSave();
}

void CPluginConfigPageTV::OnEnChangeEditStreamTemplate()
{
	UpdateData(TRUE);
	GetSupportedStream().set_uri_template(m_StreamTemplate.GetString());
	AllowSave();
}

void CPluginConfigPageTV::OnEnChangeEditStreamArcTemplate()
{
	UpdateData(TRUE);
	GetSupportedStream().set_uri_arc_template(m_StreamArchiveTemplate.GetString());
	AllowSave();
}

void CPluginConfigPageTV::OnEnChangeEditCustomStreamArcTemplate()
{
	UpdateData(TRUE);
	GetSupportedStream().set_uri_custom_arc_template(m_CustomStreamArchiveTemplate.GetString());
	AllowSave();
}

void CPluginConfigPageTV::OnEnChangeEditDuneParams()
{
	UpdateData(TRUE);
	GetSupportedStream().set_dune_params(m_DuneParams.GetString());
	AllowSave();
}

void CPluginConfigPageTV::OnCbnSelchangeComboTags()
{
	UpdateData(TRUE);
	CString tag;
	if (m_wndCheckMapTags.GetCheck() != 0)
	{
		m_wndTags.GetLBText(m_wndTags.GetCurSel(), tag);
	}

	int idx = m_wndPlaylistTemplates.GetCurSel();
	GetPropertySheet()->m_plugin->set_tag_id_match(idx, tag.GetString());

	AllowSave();
}

void CPluginConfigPageTV::OnCbnSelchangeComboPlaylistTemplate()
{
	int idx = m_wndPlaylistTemplates.GetCurSel();
	auto& plugin = GetPropertySheet()->m_plugin;

	plugin->set_playlist_template_idx(idx);
	FillPlaylistSettings();
}

void CPluginConfigPageTV::OnBnClickedButtonEditTemplates()
{
	std::vector<DynamicParamsInfo> info;
	for (const auto& item : GetPropertySheet()->m_plugin->get_playlist_infos())
	{
		info.emplace_back(item.name, item.pl_template);
	}

	CFillParamsInfoDlg dlg;
	dlg.m_type = 4;
	dlg.m_paramsList = std::move(info);
	dlg.m_readonly = GetPropertySheet()->GetSelectedConfig().empty();

	if (dlg.DoModal() == IDOK)
	{
		std::vector<PlaylistTemplateInfo> playlists;
		for (const auto& item : dlg.m_paramsList)
		{
			playlists.emplace_back(item.id, item.name);
		}
		GetPropertySheet()->m_plugin->set_playlist_infos(playlists);

		FillControls();
		AllowSave();
	}
}

void CPluginConfigPageTV::OnBnClickedCheckMapTagToId()
{
	m_wndTags.EnableWindow(m_wndCheckMapTags.GetCheck() != 0);
	AllowSave();
}

void CPluginConfigPageTV::OnBnClickedCheckPerChannelToken()
{
	GetPropertySheet()->m_plugin->set_per_channel_token(m_wndChkPerChannelToken.GetCheck() != 0);
	AllowSave();
}
