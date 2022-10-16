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

IMPLEMENT_DYNAMIC(CPluginConfigPage, CMFCPropertyPage)

BEGIN_MESSAGE_MAP(CPluginConfigPage, CMFCPropertyPage)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, &CPluginConfigPage::OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, &CPluginConfigPage::OnToolTipText)
	ON_CBN_SELCHANGE(IDC_COMBO_PLUGIN_TYPE, &CPluginConfigPage::OnCbnSelchangeComboPluginType)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_CONFIG, &CPluginConfigPage::OnBnClickedButtonToggleEditConfig)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_CONFIG, &CPluginConfigPage::OnBnClickedButtonSaveConfig)
	ON_CBN_SELCHANGE(IDC_COMBO_STREAM_TYPE, &CPluginConfigPage::OnCbnSelchangeComboStreamType)
	ON_CBN_DROPDOWN(IDC_COMBO_STREAM_TYPE, &CPluginConfigPage::OnCbnDropdownComboStreamType)
	ON_CBN_SELCHANGE(IDC_COMBO_EPG_TYPE, &CPluginConfigPage::OnCbnSelchangeComboEpgType)
	ON_CBN_DROPDOWN(IDC_COMBO_EPG_TYPE, &CPluginConfigPage::OnCbnDropdownComboEpgType)
	ON_BN_CLICKED(IDC_BUTTON_EPG_SHOW, &CPluginConfigPage::OnBnClickedButtonEpgTest)
	ON_BN_CLICKED(IDC_BUTTON_PLAYLIST_SHOW, &CPluginConfigPage::OnBnClickedButtonPlaylistShow)
	ON_BN_CLICKED(IDC_BUTTON_STREAM_PARSE, &CPluginConfigPage::OnBnClickedButtonStreamParse)
	ON_BN_CLICKED(IDC_BUTTON_STREAM_ID_PARSE, &CPluginConfigPage::OnBnClickedButtonStreamIdParse)
	ON_EN_CHANGE(IDC_EDIT_PARSE_PATTERN, &CPluginConfigPage::OnEnChangeEditParsePattern)
	ON_EN_CHANGE(IDC_EDIT_PARSE_PATTERN_ID, &CPluginConfigPage::OnEnChangeEditParsePatternID)
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
END_MESSAGE_MAP()

CPluginConfigPage::CPluginConfigPage(std::vector<std::wstring>& configs)
	: CMFCPropertyPage(IDD_DIALOG_PLUGIN_CONFIG)
	, m_Date(COleDateTime::GetCurrentTime())
	, m_configs(configs)
{
}

void CPluginConfigPage::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_COMBO_PLUGIN_TYPE, m_wndPluginType);
	DDX_Control(pDX, IDC_COMBO_PLUGIN_CONFIG, m_wndPluginConfigs);
	DDX_Control(pDX, IDC_BUTTON_EDIT_CONFIG, m_wndBtnToggleEdit);
	DDX_Control(pDX, IDC_BUTTON_SAVE_CONFIG, m_wndBtnSaveConf);
	DDX_Control(pDX, IDC_BUTTON_SAVE_AS_CONFIG, m_wndBtnSaveAsConf);
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
	DDX_Control(pDX, IDC_EDIT_CUSTOM_STREAM_ARC_TEMPLATE, m_wndCustomStreamArchiveTemplate);
	DDX_Text(pDX, IDC_EDIT_CUSTOM_STREAM_ARC_TEMPLATE, m_CustomStreamArchiveTemplate);
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
	DDX_Control(pDX, IDC_EDIT_EPG_FMT_TIME, m_wndEpgStartFormat);
	DDX_Text(pDX, IDC_EDIT_EPG_FMT_TIME, m_EpgTimeFormat);
	DDX_Control(pDX, IDC_EDIT_EPG_TZ, m_wndEpgTimezone);
	DDX_Text(pDX, IDC_EDIT_EPG_TZ, m_EpgTimezone);
	DDX_Control(pDX, IDC_COMBO_ACCESS_TYPE, m_wndAccessType);
	DDX_Control(pDX, IDC_COMBO_STREAM_TYPE, m_wndStreamType);
	DDX_Control(pDX, IDC_COMBO_CATCHUP_TYPE, m_wndCatchupType);
	DDX_Control(pDX, IDC_COMBO_EPG_TYPE, m_wndEpgType);
	DDX_Control(pDX, IDC_BUTTON_EPG_SHOW, m_wndBtnEpgTest);
	DDX_Control(pDX, IDC_EDIT_SET_ID, m_wndSetID);
	DDX_Text(pDX, IDC_EDIT_SET_ID, m_SetID);
	DDX_Control(pDX, IDC_EDIT_SET_TOKEN, m_wndToken);
	DDX_Text(pDX, IDC_EDIT_SET_TOKEN, m_Token);
	DDX_Control(pDX, IDC_DATETIMEPICKER_DATE, m_wndDate);
	DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER_DATE, m_Date);
	DDX_Control(pDX, IDC_EDIT_PLAYLIST_TEMPLATE, m_wndPlaylistTemplate);
	DDX_Text(pDX, IDC_EDIT_PLAYLIST_TEMPLATE, m_PlaylistTemplate);
	DDX_Control(pDX, IDC_CHECK_SQUARE_ICONS, m_wndChkSquareIcons);
	DDX_Control(pDX, IDC_BUTTON_PLAYLIST_SHOW, m_wndBtnPlaylistTest);
	DDX_Control(pDX, IDC_BUTTON_STREAM_PARSE, m_wndBtnStreamParseTest);
	DDX_Control(pDX, IDC_BUTTON_STREAM_ID_PARSE, m_wndBtnStreamParseIdTest);
	DDX_Control(pDX, IDC_CHECK_STATIC_SERVERS, m_wndChkStaticServers);
	DDX_Control(pDX, IDC_BUTTON_EDIT_SERVERS, m_wndBtnServers);
	DDX_Control(pDX, IDC_CHECK_STATIC_DEVICES, m_wndChkStaticDevices);
	DDX_Control(pDX, IDC_BUTTON_EDIT_DEVICES, m_wndBtnDevices);
	DDX_Control(pDX, IDC_CHECK_STATIC_QUALITIES, m_wndChkStaticQualities);
	DDX_Control(pDX, IDC_BUTTON_EDIT_QUALITY, m_wndBtnQualities);
	DDX_Control(pDX, IDC_CHECK_STATIC_PROFILES, m_wndChkStaticProfiles);
	DDX_Control(pDX, IDC_BUTTON_EDIT_PROFILES, m_wndBtnProfiles);
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

	m_tooltips_info_account =
	{
		{ IDC_COMBO_PLUGIN_CONFIG, load_string_resource(IDS_STRING_COMBO_CONFIG) },
		{ IDC_BUTTON_EDIT_CONFIG, load_string_resource(IDS_STRING_BUTTON_EDIT_CONFIG) },
		{ IDC_BUTTON_SAVE_CONFIG, load_string_resource(IDS_STRING_BUTTON_SAVE_CONFIG) },
		{ IDC_BUTTON_SAVE_AS_CONFIG, load_string_resource(IDS_STRING_BUTTON_SAVE_AS_CONFIG) },
		{ IDC_EDIT_PLUGIN_NAME, load_string_resource(IDS_STRING_EDIT_PLUGIN_NAME) },
		{ IDC_EDIT_TITLE, load_string_resource(IDS_STRING_EDIT_TITLE) },
		{ IDC_EDIT_SHORT_NAME, load_string_resource(IDS_STRING_EDIT_SHORT_NAME) },
		{ IDC_EDIT_PROVIDER_URL, load_string_resource(IDS_STRING_EDIT_PROVIDER_URL) },
		{ IDC_CHECK_SQUARE_ICONS, load_string_resource(IDS_STRING_CHECK_SQUARE_ICONS) },
		{ IDC_EDIT_PLAYLIST_TEMPLATE, load_string_resource(IDS_STRING_EDIT_PLAYLIST_TEMPLATE) },
		{ IDC_EDIT_PARSE_PATTERN, load_string_resource(IDS_STRING_EDIT_PARSE_PATTERN) },
		{ IDC_EDIT_PARSE_PATTERN_ID, load_string_resource(IDS_STRING_EDIT_PARSE_PATTERN_ID) },
		{ IDC_BUTTON_PLAYLIST_SHOW, load_string_resource(IDS_STRING_BUTTON_PLAYLIST_SHOW) },
		{ IDC_BUTTON_STREAM_PARSE, load_string_resource(IDS_STRING_BUTTON_STREAM_PARSE) },
		{ IDC_BUTTON_STREAM_ID_PARSE, load_string_resource(IDS_STRING_BUTTON_STREAM_PARSE) },
		{ IDC_CHECK_STATIC_SERVERS, load_string_resource(IDS_STRING_CHECK_STATIC_SERVERS) },
		{ IDC_BUTTON_EDIT_SERVERS, load_string_resource(IDS_STRING_BUTTON_EDIT_SERVERS) },
		{ IDC_CHECK_STATIC_DEVICES, load_string_resource(IDS_STRING_CHECK_STATIC_DEVICES) },
		{ IDC_BUTTON_EDIT_DEVICES, load_string_resource(IDS_STRING_BUTTON_EDIT_DEVICES) },
		{ IDC_CHECK_STATIC_QUALITIES, load_string_resource(IDS_STRING_CHECK_STATIC_QUALITIES) },
		{ IDC_BUTTON_EDIT_QUALITY, load_string_resource(IDS_STRING_BUTTON_EDIT_QUALITY) },
		{ IDC_CHECK_STATIC_PROFILES, load_string_resource(IDS_STRING_CHECK_STATIC_PROFILES) },
		{ IDC_BUTTON_EDIT_PROFILES, load_string_resource(IDS_STRING_BUTTON_EDIT_PROFILES) },
		{ IDC_EDIT_SHIFT_SUBST, load_string_resource(IDS_STRING_EDIT_SHIFT_SUBST) },
		{ IDC_EDIT_DURATION, load_string_resource(IDS_STRING_EDIT_DURATION) },
		{ IDC_EDIT_STREAM_TEMPLATE, load_string_resource(IDS_STRING_EDIT_STREAM_TEMPLATE) },
		{ IDC_EDIT_STREAM_ARC_TEMPLATE, load_string_resource(IDS_STRING_EDIT_STREAM_ARC_TEMPLATE) },
		{ IDC_EDIT_CUSTOM_STREAM_ARC_TEMPLATE, load_string_resource(IDS_STRING_EDIT_STREAM_ARC_TEMPLATE) },
		{ IDC_EDIT_EPG_URL, load_string_resource(IDS_STRING_EDIT_EPG_URL) },
		{ IDC_EDIT_EPG_ROOT, load_string_resource(IDS_STRING_EDIT_EPG_ROOT) },
		{ IDC_EDIT_EPG_NAME, load_string_resource(IDS_STRING_EDIT_EPG_NAME) },
		{ IDC_EDIT_EPG_DESC, load_string_resource(IDS_STRING_EDIT_EPG_DESC) },
		{ IDC_EDIT_EPG_START, load_string_resource(IDS_STRING_EDIT_EPG_START) },
		{ IDC_EDIT_EPG_END, load_string_resource(IDS_STRING_EDIT_EPG_END) },
		{ IDC_EDIT_EPG_FMT_DATE, load_string_resource(IDS_STRING_EDIT_EPG_FMT_DATE) },
		{ IDC_EDIT_EPG_FMT_TIME, load_string_resource(IDS_STRING_EDIT_EPG_FMT_TIME) },
		{ IDC_EDIT_EPG_TZ, load_string_resource(IDS_STRING_EDIT_EPG_TZ) },
		{ IDC_COMBO_ACCESS_TYPE, load_string_resource(IDS_STRING_COMBO_ACCESS_TYPE) },
		{ IDC_COMBO_STREAM_TYPE, load_string_resource(IDS_STRING_COMBO_STREAM_TYPE) },
		{ IDC_COMBO_CATCHUP_TYPE, load_string_resource(IDS_STRING_COMBO_CATCHUP_TYPE) },
		{ IDC_COMBO_EPG_TYPE, load_string_resource(IDS_STRING_EPG_TYPE) },
		{ IDC_BUTTON_EPG_SHOW, load_string_resource(IDS_STRING_BUTTON_EPG_SHOW) },
		{ IDC_EDIT_SET_ID, load_string_resource(IDS_STRING_EDIT_SET_ID) },
		{ IDC_EDIT_SET_TOKEN, load_string_resource(IDS_STRING_EDIT_SET_TOKEN) },
		{ IDC_DATETIMEPICKER_DATE, load_string_resource(IDS_STRING_DATETIMEPICKER_DATE) },
	};

	m_wndToolTipCtrl.SetDelayTime(TTDT_AUTOPOP, 10000);
	m_wndToolTipCtrl.SetDelayTime(TTDT_INITIAL, 500);
	m_wndToolTipCtrl.SetMaxTipWidth(300);

	for (const auto& pair : m_tooltips_info_account)
	{
		m_wndToolTipCtrl.AddTool(GetDlgItem(pair.first), LPSTR_TEXTCALLBACK);
	}

	m_wndToolTipCtrl.Activate(TRUE);

	RestoreWindowPos(GetSafeHwnd(), REG_CONFIG_WINDOW_POS);

	SetButtonImage(IDB_PNG_EDIT, m_wndBtnToggleEdit);
	SetButtonImage(IDB_PNG_SAVE, m_wndBtnSaveConf);
	SetButtonImage(IDB_PNG_SAVE_AS, m_wndBtnSaveAsConf);

	//ASSERT(m_plugin_type != PluginType::enCustom);

	// Fill available plugins
	int sel_idx = 0;
	for (const auto& item : GetConfig().get_all_plugins())
	{
		auto plugin = StreamContainer::get_instance(item);
		if (!plugin) continue;

		int idx = m_wndPluginType.AddString(plugin->get_title().c_str());
		m_wndPluginType.SetItemData(idx, (DWORD_PTR)item);
		if (item == m_plugin->get_plugin_type())
		{
			sel_idx = idx;
		}
	}
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
				L"{CU_SUBST}",
				L"{START}",
				L"{NOW}",
				L"{DURATION}",
				L"{OFFSET}",
	});

	m_wndStreamArchiveTemplate.SetTemplateParams(arc_params);
	m_wndCustomStreamArchiveTemplate.SetTemplateParams(arc_params);

	std::vector<std::wstring> epg_params =
	{
				L"{DOMAIN}",
				L"{EPG_ID}",
				L"{TOKEN}",
				L"{TIMESTAMP}",
				L"{DATE}",
	};
	m_wndEpgUrl.SetTemplateParams(epg_params);

	std::vector<std::wstring> date_fmt_params =
	{
				L"{YEAR}",
				L"{MONTH}",
				L"{DAY}",
	};
	m_wndDateFormat.SetTemplateParams(date_fmt_params);

	std::vector<std::wstring> epg_start_time_params =
	{
				L"{YEAR}",
				L"{MONTH}",
				L"{DAY}",
				L"{HOUR}",
				L"{MINUTE}",
				L"{TIMESTAMP}",
	};
	m_wndEpgStartFormat.SetTemplateParams(epg_start_time_params);

	FillConfigs();

	if (m_initial_cred.config.empty())
	{
		m_wndPluginConfigs.SetCurSel(0);
	}
	else
	{
		m_wndPluginConfigs.SelectString(0, utils::utf8_to_utf16(m_initial_cred.config).c_str());
	}

	m_wndBtnToggleEdit.EnableWindow(!m_single);
	m_wndPluginType.SetCurSel(sel_idx);
	m_wndPluginType.ShowWindow(m_single ? SW_SHOW : SW_HIDE);

	if (m_pAccessPage)
	{
		m_Token = m_pAccessPage->GetCheckedAccount().get_token().c_str();
	}


	if (m_single)
	{
		OnCbnSelchangeComboPluginType();
	}
	else
	{
		m_plugin->load_plugin_parameters(m_initial_cred.get_config());
		FillControlsCommon();
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPluginConfigPage::FillConfigs()
{
	m_wndPluginConfigs.ResetContent();
	for (const auto& entry : m_configs)
	{
		m_wndPluginConfigs.AddString(entry.c_str());
	}
	m_wndPluginConfigs.SetCurSel(0);
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

std::wstring CPluginConfigPage::GetSelectedConfig()
{
	int idx = m_wndPluginConfigs.GetCurSel();
	if (idx < 1) return L"";

	CString name;
	m_wndPluginConfigs.GetLBText(idx, name);
	return name.GetString();
}

void CPluginConfigPage::EnableControls()
{
	UpdateData(TRUE);

	bool enable = m_allow_edit;
	if (m_single)
		enable = false;

	m_wndPluginConfigs.EnableWindow(!enable);

	// buttons
	m_wndBtnSaveConf.EnableWindow(enable && !GetSelectedConfig().empty());
	m_wndBtnSaveAsConf.EnableWindow(enable);

	// common
	m_wndName.EnableWindow(enable);
	m_wndTitle.EnableWindow(enable);
	m_wndShortName.EnableWindow(FALSE);
	m_wndProviderUrl.EnableWindow(enable);
	m_wndChkSquareIcons.EnableWindow(enable);
	m_wndAccessType.EnableWindow(enable);
	m_wndPlaylistTemplate.EnableWindow(enable);
	m_wndParseStream.EnableWindow(enable);
	m_wndParseStreamID.EnableWindow(enable);

	// test
	m_wndBtnPlaylistTest.EnableWindow(enable && m_pAccessPage != nullptr);
	m_wndBtnStreamParseTest.EnableWindow(enable && !m_ParseStream.IsEmpty());
	m_wndBtnStreamParseIdTest.EnableWindow(enable && !m_ParseStreamID.IsEmpty());

	// streams
	m_wndStreamType.EnableWindow(enable);
	m_wndCatchupType.EnableWindow(enable);
	m_wndSubst.EnableWindow(enable);
	m_wndDuration.EnableWindow(enable);
	m_wndStreamTemplate.EnableWindow(enable);
	m_wndStreamArchiveTemplate.EnableWindow(enable);
	m_wndCustomStreamArchiveTemplate.EnableWindow(enable);

	// epg
	m_wndEpgType.EnableWindow(enable);
	m_wndEpgUrl.EnableWindow(enable);
	m_wndEpgRoot.EnableWindow(enable);
	m_wndEpgName.EnableWindow(enable);
	m_wndEpgDesc.EnableWindow(enable);
	m_wndEpgStart.EnableWindow(enable);
	m_wndEpgEnd.EnableWindow(enable);
	m_wndDateFormat.EnableWindow(enable);
	m_wndEpgStartFormat.EnableWindow(enable);
	m_wndEpgTimezone.EnableWindow(enable);

	// servers
	m_wndChkStaticServers.SetCheck(m_plugin->get_static_servers());
	m_wndChkStaticServers.EnableWindow(enable);
	m_wndBtnServers.EnableWindow(enable && m_plugin->get_static_servers());

	// devices
	m_wndChkStaticDevices.SetCheck(m_plugin->get_static_devices());
	m_wndChkStaticDevices.EnableWindow(enable);
	m_wndBtnDevices.EnableWindow(enable && m_plugin->get_static_devices());

	// qualities
	m_wndChkStaticQualities.SetCheck(m_plugin->get_static_qualities());
	m_wndChkStaticQualities.EnableWindow(enable);
	m_wndBtnQualities.EnableWindow(enable && m_plugin->get_static_qualities());

	// profiles
	m_wndChkStaticProfiles.SetCheck(m_plugin->get_static_profiles());
	m_wndChkStaticProfiles.EnableWindow(enable);
	m_wndBtnProfiles.EnableWindow(enable && m_plugin->get_static_profiles());

	// epg test
	m_wndSetID.EnableWindow(enable);
	m_wndToken.EnableWindow(enable);
	m_wndDate.EnableWindow(enable);
	m_wndBtnEpgTest.EnableWindow(enable && !m_EpgUrl.IsEmpty());
}

void CPluginConfigPage::FillControlsCommon()
{
	if (!m_plugin) return;

	m_wndAccessType.SetCurSel((int)m_plugin->get_access_type());
	m_wndChkSquareIcons.SetCheck(m_plugin->get_square_icons() != false);

	m_Name = m_plugin->get_name().c_str();
	m_Title = m_plugin->get_title().c_str();
	m_ShortName = m_plugin->get_short_name_w().c_str();
	m_ProviderUrl = m_plugin->get_provider_url().c_str();
	m_PlaylistTemplate = m_plugin->get_playlist_template().c_str();
	m_ParseStream = m_plugin->get_uri_parse_pattern().c_str();
	m_ParseStreamID = m_plugin->get_uri_id_parse_pattern().c_str();

	m_supported_streams = m_plugin->get_supported_streams();
	m_epg_parameters = m_plugin->get_epg_parameters();

	m_wndStreamType.SetCurSel(0);
	m_wndEpgType.SetCurSel(0);

	UpdateData(FALSE);
	FillControlsStream();
	FillControlsEpg();
	EnableControls();
}

void CPluginConfigPage::SaveControlsCommon()
{
	if (!m_plugin) return;

	UpdateData(TRUE);

	m_plugin->set_square_icons(m_wndChkSquareIcons.GetCheck() != 0);

	m_plugin->set_name(m_Name.GetString());
	m_plugin->set_title(m_Title.GetString());
	m_plugin->set_short_name_w(m_ShortName.GetString());
	m_plugin->set_provider_url(m_ProviderUrl.GetString());
	m_plugin->set_playlist_template(m_PlaylistTemplate.GetString());
	m_plugin->set_uri_parse_pattern(m_ParseStream.GetString());
	m_plugin->set_uri_id_parse_pattern(m_ParseStreamID.GetString());

	SaveControlsStream();
	SaveControlsEpg();

	int i = 0;
	for (const auto& stream : m_supported_streams)
	{
		m_plugin->set_supported_stream(i++, stream);
	}

	i = 0;
	for (const auto& epg : m_epg_parameters)
	{
		m_plugin->set_epg_parameter(i++, epg);
	}
}

void CPluginConfigPage::FillControlsStream()
{
	if (!m_plugin) return;

	const auto& stream = m_supported_streams[m_wndStreamType.GetCurSel()];

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
	m_CustomStreamArchiveTemplate = stream.uri_custom_arc_template.c_str();

	UpdateData(FALSE);
}

void CPluginConfigPage::SaveControlsStream()
{
	if (!m_plugin) return;

	UpdateData(TRUE);

	auto& cur_stream = m_supported_streams[m_wndStreamType.GetCurSel()];

	cur_stream.cu_type = (CatchupType)m_wndCatchupType.GetCurSel();
	cur_stream.cu_duration = m_Duration;
	cur_stream.set_shift_replace(m_Subst.GetString());
	cur_stream.set_uri_template(m_StreamTemplate.GetString());
	cur_stream.set_uri_arc_template(m_StreamArchiveTemplate.GetString());
	cur_stream.set_uri_custom_arc_template(m_CustomStreamArchiveTemplate.GetString());
}

void CPluginConfigPage::FillControlsEpg()
{
	if (!m_plugin) return;

	const auto& epg = m_epg_parameters[m_wndEpgType.GetCurSel()];

	m_EpgUrl = epg.epg_url.c_str();
	m_EpgRoot = epg.epg_root.c_str();
	m_EpgName = epg.epg_name.c_str();
	m_EpgDesc = epg.epg_desc.c_str();
	m_EpgStart = epg.epg_start.c_str();
	m_EpgEnd = epg.epg_end.c_str();
	m_EpgDateFormat = epg.epg_date_format.c_str();
	m_EpgTimeFormat = epg.epg_time_format.c_str();
	m_EpgTimezone = epg.epg_timezone;

	UpdateData(FALSE);
}

void CPluginConfigPage::SaveControlsEpg()
{
	if (!m_plugin) return;

	UpdateData(TRUE);

	auto& epg = m_epg_parameters[m_wndEpgType.GetCurSel()];

	epg.set_epg_url(m_EpgUrl.GetString());
	epg.set_epg_root(m_EpgRoot.GetString());
	epg.set_epg_name(m_EpgName.GetString());
	epg.set_epg_desc(m_EpgDesc.GetString());
	epg.set_epg_start(m_EpgStart.GetString());
	epg.set_epg_end(m_EpgEnd.GetString());
	epg.set_epg_date_format(m_EpgDateFormat.GetString());
	epg.set_epg_time_format(m_EpgTimeFormat.GetString());
	epg.epg_timezone = m_EpgTimezone;
}

void CPluginConfigPage::OnCbnSelchangeComboPluginType()
{
	m_allow_edit = false;
	m_plugin = StreamContainer::get_instance((PluginType)m_wndPluginType.GetItemData(m_wndPluginType.GetCurSel()));
	m_plugin->load_plugin_parameters(m_initial_cred.get_config());
	FillControlsCommon();
}

void CPluginConfigPage::OnBnClickedButtonToggleEditConfig()
{
	m_allow_edit = !m_allow_edit;
	EnableControls();
}

void CPluginConfigPage::OnBnClickedButtonSaveConfig()
{
	SaveControlsCommon();

	auto name = GetSelectedConfig();
	if (name.empty()) return;

	if (!m_plugin->save_plugin_parameters(name))
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

	if (!m_plugin->save_plugin_parameters(new_conf))
	{
		AfxMessageBox(IDS_STRING_ERR_SAVE_CONFIG, MB_ICONERROR | MB_OK);
	}
	else
	{
		m_configs.emplace_back(new_conf);
		FillConfigs();
		m_wndPluginConfigs.SetCurSel((int)m_configs.size() - 1);
		UpdateData(FALSE);
	}
}

void CPluginConfigPage::OnCbnSelchangeComboStreamType()
{
	FillControlsStream();
}

void CPluginConfigPage::OnCbnDropdownComboStreamType()
{
	SaveControlsStream();
}

void CPluginConfigPage::OnCbnSelchangeComboEpgType()
{
	FillControlsEpg();
}

void CPluginConfigPage::OnCbnDropdownComboEpgType()
{
	SaveControlsEpg();
}

void CPluginConfigPage::OnBnClickedButtonEpgTest()
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

	utils::string_replace_inplace<wchar_t>(url, base_plugin::REPL_EPG_ID, m_SetID.GetString());
	utils::string_replace_inplace<wchar_t>(url, base_plugin::REPL_TOKEN, m_Token.GetString());
	utils::string_replace_inplace<wchar_t>(url, base_plugin::REPL_DATE, m_EpgDateFormat.GetString());
	utils::string_replace_inplace<wchar_t>(url, base_plugin::REPL_YEAR, std::to_wstring(m_Date.GetYear()));
	utils::string_replace_inplace<wchar_t>(url, base_plugin::REPL_MONTH, std::to_wstring(m_Date.GetMonth()));
	utils::string_replace_inplace<wchar_t>(url, base_plugin::REPL_DAY, std::to_wstring(m_Date.GetDay()));
	utils::string_replace_inplace<wchar_t>(url, base_plugin::REPL_TIMESTAMP, std::to_wstring(dayTime).c_str());

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
		ShellExecute(nullptr, _T("open"), L"https://regex101.com/", nullptr, nullptr, SW_SHOWDEFAULT);
}

void CPluginConfigPage::OnBnClickedButtonStreamIdParse()
{
	if (!m_ParseStreamID.IsEmpty())
		ShellExecute(nullptr, _T("open"), L"https://regex101.com/", nullptr, nullptr, SW_SHOWDEFAULT);
}

void CPluginConfigPage::OnEnChangeEditParsePattern()
{
	UpdateData(TRUE);
	m_wndBtnStreamParseTest.EnableWindow(!m_ParseStream.IsEmpty());
}

void CPluginConfigPage::OnEnChangeEditParsePatternID()
{
	UpdateData(TRUE);
	m_wndBtnStreamParseIdTest.EnableWindow(!m_ParseStreamID.IsEmpty());
}

void CPluginConfigPage::OnBnClickedButtonEditServers()
{
	CFillParamsInfoDlg dlg;
	dlg.m_type = 0;
	dlg.m_paramsList = m_plugin->get_servers_list();
	dlg.m_readonly = !m_allow_edit;

	if (dlg.DoModal() == IDOK)
		m_plugin->set_servers_list(dlg.m_paramsList);
}

void CPluginConfigPage::OnBnClickedButtonEditDevices()
{
	CFillParamsInfoDlg dlg;
	dlg.m_type = 1;
	dlg.m_paramsList = m_plugin->get_devices_list();
	dlg.m_readonly = !m_allow_edit;

	if (dlg.DoModal() == IDOK)
		m_plugin->set_devices_list(dlg.m_paramsList);
}

void CPluginConfigPage::OnBnClickedButtonEditQuality()
{
	CFillParamsInfoDlg dlg;
	dlg.m_type = 2;
	dlg.m_paramsList = m_plugin->get_qualities_list();
	dlg.m_readonly = !m_allow_edit;

	if (dlg.DoModal() == IDOK)
		m_plugin->set_qualities_list(dlg.m_paramsList);
}

void CPluginConfigPage::OnBnClickedButtonEditProfiles()
{
	CFillParamsInfoDlg dlg;
	dlg.m_type = 3;
	dlg.m_paramsList = m_plugin->get_profiles_list();
	dlg.m_readonly = !m_allow_edit;

	if (dlg.DoModal() == IDOK)
		m_plugin->set_profiles_list(dlg.m_paramsList);
}

void CPluginConfigPage::OnCbnSelchangeComboPluginConfig()
{
	m_plugin->load_plugin_parameters(GetSelectedConfig());
	FillControlsCommon();
}


void CPluginConfigPage::OnBnClickedCheckStaticServers()
{
	GetDlgItem(IDC_BUTTON_EDIT_SERVERS)->EnableWindow(m_wndChkStaticServers.GetCheck());
}


void CPluginConfigPage::OnBnClickedCheckStaticDevices()
{
	GetDlgItem(IDC_BUTTON_EDIT_DEVICES)->EnableWindow(m_wndChkStaticDevices.GetCheck());
}


void CPluginConfigPage::OnBnClickedCheckStaticQualities()
{
	GetDlgItem(IDC_BUTTON_EDIT_QUALITY)->EnableWindow(m_wndChkStaticQualities.GetCheck());
}


void CPluginConfigPage::OnBnClickedCheckStaticProfiles()
{
	GetDlgItem(IDC_BUTTON_EDIT_PROFILES)->EnableWindow(m_wndChkStaticProfiles.GetCheck());
}
