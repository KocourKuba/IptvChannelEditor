/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2024): sharky72 (https://github.com/KocourKuba)

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
#include "PluginConfigPageEPG.h"
#include "AccountSettings.h"
#include "Constants.h"
#include "PluginFactory.h"

#include "UtilsLib/inet_utils.h"

// CPluginConfigPageEPG dialog

IMPLEMENT_DYNAMIC(CPluginConfigPageEPG, CTooltipPropertyPage)

BEGIN_MESSAGE_MAP(CPluginConfigPageEPG, CTooltipPropertyPage)
	ON_CBN_SELCHANGE(IDC_COMBO_EPG_TYPE, &CPluginConfigPageEPG::OnCbnSelchangeComboEpgType)
	ON_BN_CLICKED(IDC_BUTTON_EPG_SHOW, &CPluginConfigPageEPG::OnBnClickedButtonEpgTest)
	ON_EN_CHANGE(IDC_EDIT_EPG_DOMAIN, &CPluginConfigPageEPG::SaveParameters)
	ON_EN_CHANGE(IDC_EDIT_EPG_URL, &CPluginConfigPageEPG::SaveParameters)
	ON_CBN_SELCHANGE(IDC_COMBO_EPG_PARSER_PRESET, &CPluginConfigPageEPG::OnCbnSelchangeComboEpgParserPreset)
	ON_EN_CHANGE(IDC_EDIT_EPG_ROOT, &CPluginConfigPageEPG::SaveParameters)
	ON_EN_CHANGE(IDC_EDIT_EPG_NAME, &CPluginConfigPageEPG::SaveParameters)
	ON_EN_CHANGE(IDC_EDIT_EPG_DESC, &CPluginConfigPageEPG::SaveParameters)
	ON_EN_CHANGE(IDC_EDIT_EPG_START, &CPluginConfigPageEPG::SaveParameters)
	ON_EN_CHANGE(IDC_EDIT_EPG_END, &CPluginConfigPageEPG::SaveParameters)
	ON_EN_CHANGE(IDC_EDIT_EPG_TZ, &CPluginConfigPageEPG::SaveParameters)
	ON_EN_CHANGE(IDC_EDIT_EPG_FMT_DATE, &CPluginConfigPageEPG::SaveParameters)
	ON_EN_CHANGE(IDC_EDIT_EPG_FMT_TIME, &CPluginConfigPageEPG::SaveParameters)
	ON_BN_CLICKED(IDC_CHECK_USE_DURATION, &CPluginConfigPageEPG::SaveParameters)
	ON_EN_CHANGE(IDC_EDIT_UTC, &CPluginConfigPageEPG::OnEnChangeEditUtc)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER_DATE, &CPluginConfigPageEPG::OnDtnDatetimechangeDatetimepickerDate)
END_MESSAGE_MAP()

CPluginConfigPageEPG::CPluginConfigPageEPG()
	: CTooltipPropertyPage(IDD_DIALOG_PLUGIN_CONFIG_EPG)
	, m_Date(COleDateTime::GetCurrentTime())
{
}

void CPluginConfigPageEPG::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_EDIT_EPG_DOMAIN, m_wndEpgDomain);
	DDX_Text(pDX, IDC_EDIT_EPG_DOMAIN, m_EpgDomain);
	DDX_Control(pDX, IDC_EDIT_EPG_URL, m_wndEpgUrl);
	DDX_Text(pDX, IDC_EDIT_EPG_URL, m_EpgUrl);
	DDX_Control(pDX, IDC_EDIT_EPG_ROOT, m_wndEpgRoot);
	DDX_Control(pDX, IDC_COMBO_EPG_PARSER_PRESET, m_wndEpgPreset);
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
	DDX_Control(pDX, IDC_EDIT_EPG_FMT_TIME, m_wndEpgStartFormat);
	DDX_Text(pDX, IDC_EDIT_EPG_FMT_TIME, m_EpgStartTimeFormat);
	DDX_Control(pDX, IDC_EDIT_EPG_TZ, m_wndEpgTimezone);
	DDX_Text(pDX, IDC_EDIT_EPG_TZ, m_EpgTimezone);
	DDX_Control(pDX, IDC_COMBO_EPG_TYPE, m_wndEpgType);
	DDX_Control(pDX, IDC_BUTTON_EPG_SHOW, m_wndBtnEpgTest);
	DDX_Text(pDX, IDC_EDIT_SET_ID, m_SetID);
	DDX_Text(pDX, IDC_EDIT_SET_TOKEN, m_Token);
	DDX_Control(pDX, IDC_CHECK_USE_DURATION, m_wndChkUseDuration);
	DDX_Control(pDX, IDC_DATETIMEPICKER_DATE, m_wndDate);
	DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER_DATE, m_Date);
	DDX_Text(pDX, IDC_EDIT_UTC, m_UTC);
	DDX_Text(pDX, IDC_EDIT_SET_DUNE_IP, m_DuneIP);
}

BOOL CPluginConfigPageEPG::OnInitDialog()
{
	__super::OnInitDialog();

	AddTooltip(IDC_EDIT_EPG_URL, IDS_STRING_EDIT_EPG_URL);
	AddTooltip(IDC_EDIT_EPG_ROOT, IDS_STRING_EDIT_EPG_ROOT);
	AddTooltip(IDC_EDIT_EPG_NAME, IDS_STRING_EDIT_EPG_NAME);
	AddTooltip(IDC_EDIT_EPG_DESC, IDS_STRING_EDIT_EPG_DESC);
	AddTooltip(IDC_EDIT_EPG_START, IDS_STRING_EDIT_EPG_START);
	AddTooltip(IDC_EDIT_EPG_END, IDS_STRING_EDIT_EPG_END);
	AddTooltip(IDC_EDIT_EPG_FMT_DATE, IDS_STRING_EDIT_EPG_FMT_DATE);
	AddTooltip(IDC_EDIT_EPG_FMT_TIME, IDS_STRING_EDIT_EPG_FMT_TIME);
	AddTooltip(IDC_EDIT_EPG_TZ, IDS_STRING_EDIT_EPG_TZ);
	AddTooltip(IDC_COMBO_EPG_TYPE, IDS_STRING_EPG_TYPE);
	AddTooltip(IDC_BUTTON_EPG_SHOW, IDS_STRING_BUTTON_EPG_SHOW);
	AddTooltip(IDC_EDIT_SET_ID, IDS_STRING_EDIT_SET_ID);
	AddTooltip(IDC_EDIT_SET_TOKEN, IDS_STRING_EDIT_SET_TOKEN);
	AddTooltip(IDC_DATETIMEPICKER_DATE, IDS_STRING_DATETIMEPICKER_DATE);
	AddTooltip(IDC_EDIT_UTC, IDS_STRING_DATETIMEPICKER_DATE);
	AddTooltip(IDC_CHECK_USE_DURATION, IDS_STRING_CHECK_USE_DURATION);
	AddTooltip(IDC_EDIT_EPG_DOMAIN, IDS_STRING_EDIT_EPG_DOMAIN);

	AssignMacros();

	UpdateDateTimestamp(true);

	for (const auto& param : GetPropertySheet()->m_plugin->get_epg_parameters())
	{
		m_wndEpgType.AddString(utils::utf8_to_utf16(param.epg_param).c_str());
	}

	m_wndEpgType.SetCurSel(0);
	m_DuneIP = GetConfig().get_string(true, REG_DUNE_IP).c_str();
	m_Token = GetPropertySheet()->m_plugin->get_api_token(GetPropertySheet()->m_selected_cred).c_str();
	m_wndEpgType.SetCurSel(0);

	for(size_t it = (size_t)EpgPresets::enDRM; it != (size_t)EpgPresets::enLast; ((size_t&)it)++)
	{
		const auto& name = utils::utf8_to_utf16(GetPluginFactory().get_epg_preset((EpgPresets)it).epg_name);
		m_wndEpgPreset.AddString(name.c_str());
	}
	m_wndEpgPreset.SetCurSel((int)GetPropertySheet()->m_plugin->get_epg_preset_idx(0));


	FillControls();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPluginConfigPageEPG::OnSetActive()
{
	__super::OnSetActive();

	FillControls();

	return TRUE;
}

BOOL CPluginConfigPageEPG::OnApply()
{
	UpdateData(TRUE);

	GetConfig().set_string(true, REG_DUNE_IP, m_DuneIP.GetString());

	return TRUE;
}

void CPluginConfigPageEPG::AssignMacros()
{
	std::vector<std::wstring> epg_params =
	{
		REPL_API_URL,
		REPL_EPG_DOMAIN,
		REPL_ID,
		REPL_DOMAIN,
		REPL_EPG_ID,
		REPL_TOKEN,
		REPL_TIMESTAMP,
		REPL_DATE,
		REPL_DUNE_IP,
	};
	m_wndEpgUrl.SetTemplateParams(epg_params);

	std::vector<std::wstring> date_fmt_params =
	{
		REPL_YEAR,
		REPL_MONTH,
		REPL_DAY,
	};
	m_wndDateFormat.SetTemplateParams(date_fmt_params);

	std::vector<std::wstring> epg_start_time_params =
	{
		REPL_YEAR,
		REPL_MONTH,
		REPL_DAY,
		REPL_HOUR,
		REPL_MIN,
		REPL_TIMESTAMP,
	};
	m_wndEpgStartFormat.SetTemplateParams(epg_start_time_params);
}

void CPluginConfigPageEPG::FillControls()
{
	const auto& epg = GetEpgParameters();

	m_EpgDomain = epg.get_epg_domain().c_str();
	m_EpgUrl = epg.get_epg_url().c_str();
	m_EpgRoot = epg.get_epg_root().c_str();
	m_EpgName = epg.get_epg_name().c_str();
	m_EpgDesc = epg.get_epg_desc().c_str();
	m_EpgStart = epg.get_epg_start().c_str();
	m_EpgEnd = epg.get_epg_end().c_str();
	m_EpgDateFormat = epg.get_epg_date_format().c_str();
	m_EpgStartTimeFormat = epg.get_epg_time_format().c_str();
	m_EpgTimezone = (int)epg.epg_timezone;
	m_wndChkUseDuration.SetCheck(epg.epg_use_duration != false);

	int epg_type = m_wndEpgType.GetCurSel();
	m_SetID = GetPropertySheet()->m_CurrentStream->get_epg_id(epg_type).c_str();

	m_wndEpgPreset.SetCurSel((int)GetPropertySheet()->m_plugin->get_epg_preset_idx(epg_type));

	UpdateData(FALSE);

	UpdateControls();
}

void CPluginConfigPageEPG::UpdateControls()
{
	bool readOnly = GetPropertySheet()->GetSelectedConfig().empty();
	auto preset = (EpgPresets)m_wndEpgPreset.GetCurSel();
	bool not_custom_preset = preset != EpgPresets::enCustom;

	// epg
	m_wndEpgDomain.SetReadOnly(readOnly);
	m_wndEpgUrl.SetReadOnly(readOnly);
	m_wndDateFormat.SetReadOnly(readOnly);

	m_wndEpgRoot.SetReadOnly(readOnly || not_custom_preset);
	m_wndEpgName.SetReadOnly(readOnly || not_custom_preset);
	m_wndEpgDesc.SetReadOnly(readOnly || not_custom_preset);
	m_wndEpgStart.SetReadOnly(readOnly || not_custom_preset);
	m_wndEpgEnd.SetReadOnly(readOnly || not_custom_preset);
	m_wndEpgStartFormat.SetReadOnly(readOnly || not_custom_preset);
	m_wndEpgTimezone.SetReadOnly(readOnly || not_custom_preset);
	m_wndChkUseDuration.EnableWindow(!readOnly || not_custom_preset);

	m_wndBtnEpgTest.EnableWindow(!m_EpgUrl.IsEmpty());
}

void CPluginConfigPageEPG::UpdateDateTimestamp(bool dateToUtc)
{
	UpdateData(TRUE);
	if (dateToUtc)
	{
		CTime nt(m_Date.GetYear(), m_Date.GetMonth(), m_Date.GetDay(), m_Date.GetHour(), m_Date.GetMinute(), m_Date.GetSecond());

		time_t selTime = nt.GetTime();
		std::tm lt{};
		localtime_s(&lt, &selTime);
		lt.tm_hour = 0;
		lt.tm_min = 0;
		lt.tm_sec = 0;
		time_t dayTime = std::mktime(&lt);
		m_UTC = dayTime;
	}
	else
	{
		std::tm lt = fmt::localtime(m_UTC);
		m_Date.SetDate(lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday);
	}

	UpdateData(FALSE);
}

EpgParameters& CPluginConfigPageEPG::GetEpgParameters()
{
	return GetPropertySheet()->m_plugin->get_epg_parameters().at(m_wndEpgType.GetCurSel());
}

void CPluginConfigPageEPG::OnCbnSelchangeComboEpgType()
{
	FillControls();
}

void CPluginConfigPageEPG::OnBnClickedButtonEpgTest()
{
	UpdateData(TRUE);

	std::wstring url(m_EpgUrl.GetString());

	// set to begin of the day
	CTime nt(m_Date.GetYear(), m_Date.GetMonth(), m_Date.GetDay(), m_Date.GetHour(), m_Date.GetMinute(), m_Date.GetSecond());

	time_t selTime = nt.GetTime();
	std::tm lt{};
	localtime_s(&lt, &selTime);
	lt.tm_hour = 0;
	lt.tm_min = 0;
	lt.tm_sec = 0;
	time_t dayTime = std::mktime(&lt);

	utils::string_replace_inplace<wchar_t>(url, REPL_API_URL, GetPropertySheet()->m_plugin->get_provider_api_url());
	utils::string_replace_inplace<wchar_t>(url, REPL_EPG_DOMAIN, m_EpgDomain.GetString());
	utils::string_replace_inplace<wchar_t>(url, REPL_EPG_ID, m_SetID.GetString());
	utils::string_replace_inplace<wchar_t>(url, REPL_TOKEN, m_Token.GetString());
	utils::string_replace_inplace<wchar_t>(url, REPL_DATE, m_EpgDateFormat.GetString());
	utils::string_replace_inplace<wchar_t>(url, REPL_YEAR, std::to_wstring(m_Date.GetYear()));
	utils::string_replace_inplace<wchar_t>(url, REPL_MONTH, std::to_wstring(m_Date.GetMonth()));
	utils::string_replace_inplace<wchar_t>(url, REPL_DAY, std::to_wstring(m_Date.GetDay()));
	utils::string_replace_inplace<wchar_t>(url, REPL_TIMESTAMP, std::to_wstring(dayTime).c_str());
	utils::string_replace_inplace<wchar_t>(url, REPL_DUNE_IP, m_DuneIP.GetString());

	CWaitCursor cur;
	std::stringstream data;
	if (GetPropertySheet()->m_plugin->download_url(url, data))
	{
		nlohmann::json parsed_json;
		JSON_ALL_TRY;
		parsed_json = nlohmann::json::parse(data.str());
		JSON_ALL_CATCH;

		const auto& json_str = parsed_json.dump(2);

		const auto& out_file = std::filesystem::temp_directory_path().wstring() + L"tmp.json";

		std::ofstream out_stream(out_file, std::ofstream::binary);
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
	else
	{
		AfxMessageBox(GetPropertySheet()->m_plugin->get_download_error().c_str(), MB_ICONERROR | MB_OK);
	}
}

void CPluginConfigPageEPG::OnEnChangeEditUtc()
{
	UpdateDateTimestamp(false);
}

void CPluginConfigPageEPG::OnDtnDatetimechangeDatetimepickerDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateDateTimestamp(true);
	*pResult = 0;
}

void CPluginConfigPageEPG::OnCbnSelchangeComboEpgParserPreset()
{
	int idx = m_wndEpgPreset.GetCurSel();
	if (idx != CB_ERR)
	{
		EpgParameters epg = (idx == (int)EpgPresets::enCustom) ? GetEpgParameters() : GetPluginFactory().get_epg_preset((EpgPresets)idx);
		m_EpgRoot = epg.get_epg_root().c_str();
		m_EpgName = epg.get_epg_name().c_str();
		m_EpgDesc = epg.get_epg_desc().c_str();
		m_EpgStart = epg.get_epg_start().c_str();
		m_EpgEnd = epg.get_epg_end().c_str();
		m_EpgStartTimeFormat = epg.get_epg_time_format().c_str();
		m_EpgTimezone = (int)epg.epg_timezone;
		m_wndChkUseDuration.SetCheck(epg.epg_use_duration != false);

		UpdateData(FALSE);

		UpdateControls();

		SaveParameters();
	}
}

void CPluginConfigPageEPG::SaveParameters()
{
	UpdateData(TRUE);

	auto& parameters = GetEpgParameters();
	parameters.set_epg_domain(m_EpgDomain.GetString());
	parameters.set_epg_url(m_EpgUrl.GetString());
	parameters.set_epg_root(m_EpgRoot.GetString());
	parameters.set_epg_name(m_EpgName.GetString());
	parameters.set_epg_desc(m_EpgDesc.GetString());
	parameters.set_epg_start(m_EpgStart.GetString());
	parameters.set_epg_end(m_EpgEnd.GetString());
	parameters.set_epg_date_format(m_EpgDateFormat.GetString());
	parameters.set_epg_time_format(m_EpgStartTimeFormat.GetString());
	parameters.epg_timezone = m_EpgTimezone;
	parameters.epg_use_duration = m_wndChkUseDuration.GetCheck() != 0;

	AllowSave();
}
