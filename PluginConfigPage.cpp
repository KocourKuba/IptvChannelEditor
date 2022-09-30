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
#include "IPTVChannelEditor.h"
#include "PluginConfigPage.h"
#include "StreamContainer.h"
#include "UtilsLib/inet_utils.h"
#include <iosfwd>

// CPluginConfigPage dialog

IMPLEMENT_DYNAMIC(CPluginConfigPage, CMFCPropertyPage)

BEGIN_MESSAGE_MAP(CPluginConfigPage, CMFCPropertyPage)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, &CPluginConfigPage::OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, &CPluginConfigPage::OnToolTipText)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_CONFIG, &CPluginConfigPage::OnBnClickedButtonEditConfig)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_CONFIG, &CPluginConfigPage::OnBnClickedButtonLoadConfig)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_CONFIG, &CPluginConfigPage::OnBnClickedButtonSaveConfig)
	ON_CBN_SELCHANGE(IDC_COMBO_STREAM_TYPE, &CPluginConfigPage::OnCbnSelchangeComboStreamSubType)
	ON_CBN_SELCHANGE(IDC_COMBO_EPG_TYPE, &CPluginConfigPage::OnCbnSelchangeComboEpgType)
	ON_BN_CLICKED(IDC_BUTTON_EPG_SHOW, &CPluginConfigPage::OnBnClickedButtonEpgTest)
	ON_CBN_SELCHANGE(IDC_COMBO_PLUGIN_TYPE, &CPluginConfigPage::OnCbnSelchangeComboPluginType)
	ON_BN_CLICKED(IDC_BUTTON_PLAYLIST_SHOW, &CPluginConfigPage::OnBnClickedButtonPlaylistShow)
	ON_BN_CLICKED(IDC_BUTTON_STREAM_PARSE, &CPluginConfigPage::OnBnClickedButtonStreamParse)
	ON_BN_CLICKED(IDC_BUTTON_STREAM_ID_PARSE, &CPluginConfigPage::OnBnClickedButtonStreamIdParse)
	ON_EN_CHANGE(IDC_EDIT_PARSE_PATTERN, &CPluginConfigPage::OnEnChangeEditParsePattern)
	ON_EN_CHANGE(IDC_EDIT_PARSE_PATTERN_ID, &CPluginConfigPage::OnEnChangeEditParsePatternID)
END_MESSAGE_MAP()

CPluginConfigPage::CPluginConfigPage() : CMFCPropertyPage(IDD_DIALOG_PLUGIN_CONFIG)
, m_Date(COleDateTime::GetCurrentTime())
{
}

void CPluginConfigPage::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_BUTTON_EDIT_CONFIG, m_wndToggleEdit);
	DDX_Control(pDX, IDC_BUTTON_LOAD_CONFIG, m_wndLoadConf);
	DDX_Control(pDX, IDC_BUTTON_SAVE_CONFIG, m_wndSaveConf);
	DDX_Control(pDX, IDC_EDIT_PLUGIN_NAME, m_wndName);
	DDX_Text(pDX, IDC_EDIT_PLUGIN_NAME, m_Name);
	DDX_Control(pDX, IDC_EDIT_TITLE, m_wndTitle);
	DDX_Text(pDX, IDC_EDIT_TITLE, m_Title);
	DDX_Control(pDX, IDC_EDIT_SHORT_NAME, m_wndShortName);
	DDX_Text(pDX, IDC_EDIT_SHORT_NAME, m_ShortName);
	DDX_Control(pDX, IDC_EDIT_PROVIDER_URL, m_wndProviderUrl);
	DDX_Text(pDX, IDC_EDIT_PROVIDER_URL, m_ProviderUrl);
	DDX_Control(pDX, IDC_EDIT_PARSE_PATTERN, m_wndParseStream);
	DDX_Text(pDX, IDC_EDIT_PARSE_PATTERN, m_ParseStream);
	DDX_Control(pDX, IDC_EDIT_PARSE_PATTERN_ID, m_wndParseStreamID);
	DDX_Text(pDX, IDC_EDIT_PARSE_PATTERN_ID, m_ParseStreamID);
	DDX_Control(pDX, IDC_EDIT_SHIFT_SUBST, m_wndSubst);
	DDX_Text(pDX, IDC_EDIT_SHIFT_SUBST, m_Subst);
	DDX_Control(pDX, IDC_EDIT_DURATION, m_wndDuration);
	DDX_Text(pDX, IDC_EDIT_DURATION, m_Duration);
	DDX_Control(pDX, IDC_STATIC_DURATION, m_wndDurationCaption);
	DDX_Control(pDX, IDC_EDIT_STREAM_TEMPLATE, m_wndStreamTemplate);
	DDX_Text(pDX, IDC_EDIT_STREAM_TEMPLATE, m_StreamTemplate);
	DDX_Control(pDX, IDC_EDIT_STREAM_ARC_TEMPLATE, m_wndStreamArchiveTemplate);
	DDX_Text(pDX, IDC_EDIT_STREAM_ARC_TEMPLATE, m_StreamArchiveTemplate);
	DDX_Control(pDX, IDC_EDIT_EPG_URL, m_wndEpgUrl);
	DDX_Text(pDX, IDC_EDIT_EPG_URL, m_EpgUrl);
	DDX_Control(pDX, IDC_EDIT_EPG_ROOT, m_wndEpgRoot);
	DDX_Text(pDX, IDC_EDIT_EPG_ROOT, m_EpgRoot);
	DDX_Control(pDX, IDC_EDIT_EPG_NAME, m_wndEpgName);
	DDX_Text(pDX, IDC_EDIT_EPG_NAME, m_EpgName);
	DDX_Control(pDX, IDC_EDIT_EPG_DESC, m_wndEpgDesc);
	DDX_Text(pDX, IDC_EDIT_EPG_DESC, m_EpgDesc);
	DDX_Control(pDX, IDC_EDIT_EPG_START, m_wndEpgStart);
	DDX_Text(pDX, IDC_EDIT_EPG_START, m_EpgStart);
	DDX_Control(pDX, IDC_EDIT_EPG_END, m_wndEpgEnd);
	DDX_Text(pDX, IDC_EDIT_EPG_END, m_EpgEnd);
	DDX_Control(pDX, IDC_EDIT_EPG_FMT_DATE, m_wndDateFormat);
	DDX_Text(pDX, IDC_EDIT_EPG_FMT_DATE, m_EpgDateFormat);
	DDX_Control(pDX, IDC_EDIT_EPG_FMT_TIME, m_wndEpgTimeFormat);
	DDX_Text(pDX, IDC_EDIT_EPG_FMT_TIME, m_EpgTimeFormat);
	DDX_Control(pDX, IDC_EDIT_EPG_TZ, m_wndEpgTimezone);
	DDX_Text(pDX, IDC_EDIT_EPG_TZ, m_EpgTimezone);
	DDX_Control(pDX, IDC_COMBO_ACCESS_TYPE, m_wndAccessType);
	DDX_Control(pDX, IDC_COMBO_STREAM_TYPE, m_wndStreamSubType);
	DDX_Control(pDX, IDC_COMBO_CATCHUP_TYPE, m_wndCatchupType);
	DDX_Control(pDX, IDC_COMBO_EPG_TYPE, m_wndEpgType);
	DDX_Control(pDX, IDC_BUTTON_EPG_SHOW, m_wndEpgTest);
	DDX_Control(pDX, IDC_EDIT_SET_ID, m_wndSetID);
	DDX_Text(pDX, IDC_EDIT_SET_ID, m_SetID);
	DDX_Control(pDX, IDC_COMBO_PLUGIN_TYPE, m_wndPluginType);
	DDX_Text(pDX, IDC_EDIT_SET_TOKEN, m_Token);
	DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER_DATE, m_Date);
	DDX_Control(pDX, IDC_EDIT_PLAYLIST_TEMPLATE, m_wndPlaylistTemplate);
	DDX_Text(pDX, IDC_EDIT_PLAYLIST_TEMPLATE, m_PlaylistTemplate);
	DDX_Control(pDX, IDC_CHECK_SQUARE_ICONS, m_wndSquareIcons);
	DDX_Control(pDX, IDC_BUTTON_PLAYLIST_SHOW, m_wndPlaylistShow);
	DDX_Control(pDX, IDC_BUTTON_STREAM_PARSE, m_wndBtnStreamParse);
	DDX_Control(pDX, IDC_BUTTON_STREAM_ID_PARSE, m_wndBtnStreamParseID);
}

BOOL CPluginConfigPage::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_LBUTTONDOWN
		|| pMsg->message == WM_LBUTTONUP
		|| pMsg->message == WM_MOUSEMOVE)
	{
		m_wndToolTipCtrl.RelayEvent(pMsg);
	}

	return __super::PreTranslateMessage(pMsg);
}

BOOL CPluginConfigPage::OnInitDialog()
{
	__super::OnInitDialog();

	if (!m_wndToolTipCtrl.Create(this, TTS_ALWAYSTIP))
	{
		TRACE(_T("Unable To create ToolTip\n"));
		return FALSE;
	}

	RestoreWindowPos(GetSafeHwnd(), REG_CONFIG_WINDOW_POS);

	// Fill available plugins
	ASSERT(m_plugin_type != PluginType::enCustom);

	int sel_idx = 0;
	for (const auto& item : GetConfig().get_all_plugins())
	{
		auto& plugin = StreamContainer::get_instance(item);
		if (!plugin) continue;

		int idx = m_wndPluginType.AddString(plugin->get_title().c_str());
		m_wndPluginType.SetItemData(idx, (DWORD_PTR)item);
		if (item == m_plugin_type)
		{
			sel_idx = idx;
		}
	}

	m_wndToggleEdit.EnableWindow(!m_single && !m_readonly);
	m_wndPluginType.SetCurSel(sel_idx);
	m_wndPluginType.EnableWindow(m_single);
	m_wndLoadConf.EnableWindow(!m_single && !m_readonly);

	if (m_pAccessPage)
	{
		m_Token = m_pAccessPage->GetCheckedAccount().get_token().c_str();
	}

	UpdateData(FALSE);

	OnCbnSelchangeComboPluginType();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPluginConfigPage::OnApply()
{
	return TRUE;
}

BOOL CPluginConfigPage::OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult)
{
	// if there is a top level routing frame then let it handle the message
	if (GetRoutingFrame() != nullptr)
		return FALSE;

	// to be thorough we will need to handle UNICODE versions of the message also !!

	UINT nID = pNMHDR->idFrom;
	TOOLTIPTEXT* pTTT = (TOOLTIPTEXT*)pNMHDR;

	if (pNMHDR->code == TTN_NEEDTEXT && (pTTT->uFlags & TTF_IDISHWND))
	{
		// idFrom is actually the HWND of the tool
		nID = ::GetDlgCtrlID((HWND)nID);
	}

	if (nID != 0) // will be zero on a separator
	{
		auto& pair = m_tooltips_info_account.find(nID);
		if (pair != m_tooltips_info_account.end())
		{
			pTTT->lpszText = pair->second.data();
			*pResult = 0;
			return TRUE;
		}
	}

	return FALSE;
}

void CPluginConfigPage::EnableControls(BOOL enable)
{
	UpdateData(TRUE);
	if (m_readonly)
		enable = FALSE;

	m_wndName.EnableWindow(enable);
	m_wndTitle.EnableWindow(enable);
	m_wndShortName.EnableWindow(enable);
	m_wndProviderUrl.EnableWindow(enable);
	m_wndSquareIcons.EnableWindow(enable);
	m_wndPlaylistTemplate.SetReadOnly(!enable);
	m_wndParseStream.SetReadOnly(!enable);
	m_wndParseStreamID.SetReadOnly(!enable);
	m_wndSubst.EnableWindow(enable);
	m_wndDuration.EnableWindow(enable);
	m_wndStreamTemplate.EnableWindow(enable);
	m_wndStreamArchiveTemplate.EnableWindow(enable);
	m_wndEpgUrl.SetReadOnly(!enable);
	m_wndEpgRoot.EnableWindow(enable);
	m_wndEpgName.EnableWindow(enable);
	m_wndEpgDesc.EnableWindow(enable);
	m_wndEpgStart.EnableWindow(enable);
	m_wndEpgEnd.EnableWindow(enable);
	m_wndDateFormat.EnableWindow(enable);
	m_wndEpgTimeFormat.EnableWindow(enable);
	m_wndEpgTimezone.EnableWindow(enable);
	m_wndAccessType.EnableWindow(enable);
	m_wndCatchupType.EnableWindow(enable);
	m_wndLoadConf.EnableWindow(enable);
	m_wndPlaylistShow.EnableWindow(!m_single && m_pAccessPage != nullptr);
	m_wndBtnStreamParse.EnableWindow(!m_single && !m_ParseStream.IsEmpty());
	m_wndBtnStreamParseID.EnableWindow(!m_single && !m_ParseStreamID.IsEmpty());
}

void CPluginConfigPage::FillControlsCommon()
{
	m_plugin = StreamContainer::get_instance(m_plugin_type);
	if (!m_plugin) return;

	m_plugin->save_plugin_parameters();

	m_wndAccessType.SetCurSel((int)m_plugin->get_access_type());
	m_wndSquareIcons.SetCheck(m_plugin->is_square_icons() != false);

	m_Name = m_plugin->get_name().c_str();
	m_Title = m_plugin->get_title().c_str();
	m_ShortName = m_plugin->get_short_name().c_str();
	m_ProviderUrl = m_plugin->get_provider_url().c_str();
	m_PlaylistTemplate = m_plugin->get_playlist_template().c_str();
	m_ParseStream = m_plugin->get_uri_parse_pattern().c_str();
	m_ParseStreamID = m_plugin->get_uri_id_parse_pattern().c_str();

	UpdateData(FALSE);

	m_wndStreamSubType.SetCurSel(0);
	m_wndEpgType.SetCurSel(0);

	FillControlsStream();
	FillControlsEpg();
}

void CPluginConfigPage::FillControlsStream()
{
	if (!m_plugin) return;

	const auto& stream = m_plugin->get_supported_stream(m_wndStreamSubType.GetCurSel());

	BOOL enableDuration = (stream.cu_type == CatchupType::cu_flussonic);
	if (enableDuration)
	{
		m_Duration = stream.cu_duration;
	}

	m_Subst = stream.cu_subst.c_str();

	m_wndCatchupType.SetCurSel((int)stream.cu_type);
	m_wndDuration.ShowWindow(enableDuration);
	m_wndDurationCaption.ShowWindow(enableDuration);

	m_StreamTemplate = stream.uri_template.c_str();
	m_StreamArchiveTemplate = stream.uri_arc_template.c_str();

	UpdateData(FALSE);
}

void CPluginConfigPage::FillControlsEpg()
{
	if (!m_plugin) return;

	const auto& epg = m_plugin->get_epg_parameters(m_wndEpgType.GetCurSel());

	m_EpgUrl = epg.epg_url.c_str();
	m_EpgRoot = epg.epg_root.c_str();
	m_EpgName = epg.epg_name.c_str();
	m_EpgDesc = epg.epg_desc.c_str();
	m_EpgStart = epg.epg_start.c_str();
	m_EpgEnd = epg.epg_end.c_str();
	m_EpgDateFormat = epg.epg_date_format.c_str();
	m_EpgTimeFormat = epg.epg_time_format.c_str();
	m_EpgTimezone = epg.epg_timezone;

	m_wndEpgTest.EnableWindow(!m_EpgUrl.IsEmpty());

	UpdateData(FALSE);
}

void CPluginConfigPage::OnBnClickedButtonEditConfig()
{
	allowEdit = !allowEdit;
	EnableControls(allowEdit);
}

void CPluginConfigPage::OnBnClickedButtonLoadConfig()
{
	// TODO: Add your control notification handler code here
}

void CPluginConfigPage::OnBnClickedButtonSaveConfig()
{
	m_plugin->save_plugin_parameters();
}

void CPluginConfigPage::OnCbnSelchangeComboStreamSubType()
{
	FillControlsStream();
}

void CPluginConfigPage::OnCbnSelchangeComboEpgType()
{
	FillControlsEpg();
}

void CPluginConfigPage::OnBnClickedButtonEpgTest()
{
	UpdateData(TRUE);

	std::wstring url(m_EpgUrl.GetString());
	utils::string_replace_inplace<wchar_t>(url, uri_stream::REPL_EPG_ID, m_SetID.GetString());
	utils::string_replace_inplace<wchar_t>(url, uri_stream::REPL_TOKEN, m_Token.GetString());
	utils::string_replace_inplace<wchar_t>(url, uri_stream::REPL_DATE, m_Date.Format(m_EpgDateFormat).GetString());

	// set to begin of the day
	CTime nt(m_Date.GetYear(), m_Date.GetMonth(), m_Date.GetDay(), m_Date.GetHour(), m_Date.GetMinute(), m_Date.GetSecond());

	time_t selTime = nt.GetTime();
	std::tm lt{};
	localtime_s(&lt, &selTime);
	lt.tm_hour = 0;
	lt.tm_min = 0;
	lt.tm_sec = 0;
	time_t dayTime = std::mktime(&lt);

	utils::string_replace_inplace<wchar_t>(url, uri_stream::REPL_TIMESTAMP, std::to_wstring(dayTime).c_str());

	std::vector<BYTE> data;
	if (utils::DownloadFile(url, data) && !data.empty())
	{
		nlohmann::json parsed_json;
		JSON_ALL_TRY;
		parsed_json = nlohmann::json::parse(data.begin(), data.end());
		JSON_ALL_CATCH;

		const auto& json_str = parsed_json.dump(2);

		const auto& out_file = std::filesystem::temp_directory_path().wstring() + L"tmp.json";

		std::ofstream out_stream(out_file);
		out_stream << json_str << std::endl;
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

void CPluginConfigPage::OnCbnSelchangeComboPluginType()
{
	allowEdit = FALSE;
	m_plugin_type = (PluginType)m_wndPluginType.GetItemData(m_wndPluginType.GetCurSel());
	FillControlsCommon();
	EnableControls(allowEdit);
}


void CPluginConfigPage::OnBnClickedButtonPlaylistShow()
{
	const auto& selected = m_pAccessPage->GetCheckedAccount();
	TemplateParams params;
	params.token = selected.get_token();
	params.login = selected.get_login();
	params.password = selected.get_password();
	params.subdomain = selected.get_subdomain();
	params.server_idx = selected.server_id;
	params.device_idx = selected.device_id;
	params.profile_idx = selected.profile_id;
	params.quality_idx = selected.quality_id;

	const auto& url = m_plugin->get_playlist_url(params);
	std::vector<BYTE> data;
	if (utils::DownloadFile(url, data) && !data.empty())
	{
		const auto& out_file = std::filesystem::temp_directory_path().wstring() + L"tmp.m3u8";

		std::ofstream out_stream(out_file);
		out_stream.write((const char*)data.data(), data.size());
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

void CPluginConfigPage::OnBnClickedButtonStreamParse()
{
	if (!m_ParseStream.IsEmpty())
		ShellExecute(nullptr, _T("open"), L"https://regexr.com/", nullptr, nullptr, SW_SHOWDEFAULT);
}

void CPluginConfigPage::OnBnClickedButtonStreamIdParse()
{
	if (!m_ParseStreamID.IsEmpty())
		ShellExecute(nullptr, _T("open"), L"https://regexr.com/", nullptr, nullptr, SW_SHOWDEFAULT);
}

void CPluginConfigPage::OnEnChangeEditParsePattern()
{
	UpdateData(TRUE);
	m_wndBtnStreamParse.EnableWindow(!m_ParseStream.IsEmpty());
}

void CPluginConfigPage::OnEnChangeEditParsePatternID()
{
	UpdateData(TRUE);
	m_wndBtnStreamParseID.EnableWindow(!m_ParseStreamID.IsEmpty());
}
