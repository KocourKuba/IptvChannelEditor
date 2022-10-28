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
	ON_BN_CLICKED(IDC_BUTTON_EDIT_CONFIG, &CPluginConfigPage::OnBnClickedButtonToggleEditConfig)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_CONFIG, &CPluginConfigPage::OnBnClickedButtonSaveConfig)
	ON_CBN_SELCHANGE(IDC_COMBO_ACCESS_TYPE, &CPluginConfigPage::OnCbnSelchangeComboAccessType)
	ON_CBN_DROPDOWN(IDC_COMBO_ACCESS_TYPE, &CPluginConfigPage::OnCbnDropdownComboAccessType)
	ON_CBN_SELCHANGE(IDC_COMBO_STREAM_TYPE, &CPluginConfigPage::OnCbnSelchangeComboStreamType)
	ON_CBN_DROPDOWN(IDC_COMBO_STREAM_TYPE, &CPluginConfigPage::OnCbnDropdownComboStreamType)
	ON_CBN_SELCHANGE(IDC_COMBO_EPG_TYPE, &CPluginConfigPage::OnCbnSelchangeComboEpgType)
	ON_CBN_DROPDOWN(IDC_COMBO_EPG_TYPE, &CPluginConfigPage::OnCbnDropdownComboEpgType)
	ON_BN_CLICKED(IDC_BUTTON_EPG_SHOW, &CPluginConfigPage::OnBnClickedButtonEpgTest)
	ON_BN_CLICKED(IDC_BUTTON_PLAYLIST_SHOW, &CPluginConfigPage::OnBnClickedButtonPlaylistShow)
	ON_BN_CLICKED(IDC_BUTTON_STREAM_PARSE, &CPluginConfigPage::OnBnClickedButtonStreamRegexTest)
	ON_BN_CLICKED(IDC_BUTTON_VOD_PARSE, &CPluginConfigPage::OnBnClickedButtonStreamRegexTest)
	ON_EN_CHANGE(IDC_EDIT_PARSE_PATTERN, &CPluginConfigPage::OnEnChangeEditParsePattern)
	ON_BN_CLICKED(IDC_CHECK_MAP_TAG_TO_ID, &CPluginConfigPage::OnBnClickedCheckMapTagToId)
	ON_CBN_SELCHANGE(IDC_COMBO_VOD_TEMPLATE, &CPluginConfigPage::OnCbnSelchangeComboVodTemplate)
	ON_CBN_DROPDOWN(IDC_COMBO_VOD_TEMPLATE, &CPluginConfigPage::OnCbnDropdownComboVodTemplate)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_VOD_TEMPLATES, &CPluginConfigPage::OnBnClickedButtonEditVodTemplates)
	ON_EN_CHANGE(IDC_EDIT_PROVIDER_VOD_URL, &CPluginConfigPage::OnEnChangeEditProviderVodUrl)
	ON_BN_CLICKED(IDC_BUTTON_VOD_TEMPLATE, &CPluginConfigPage::OnBnClickedButtonVodTemplate)
	ON_EN_CHANGE(IDC_EDIT_VOD_REGEX, &CPluginConfigPage::OnEnChangeEditVodRegex)
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
	ON_CBN_SELCHANGE(IDC_COMBO_PLAYLIST_TEMPLATE, &CPluginConfigPage::OnCbnSelchangeComboPlaylistTemplate)
	ON_CBN_SELCHANGE(IDC_COMBO_PLAYLIST_TEMPLATE, &CPluginConfigPage::OnCbnDropdownComboPlaylistTemplate)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_TEMPLATES, &CPluginConfigPage::OnBnClickedButtonEditTemplates)
	ON_BN_CLICKED(IDC_CHECK_VOD_SUPPORT, &CPluginConfigPage::OnChanges)
	ON_BN_CLICKED(IDC_CHECK_VOD_M3U, &CPluginConfigPage::OnChanges)
	ON_BN_CLICKED(IDC_CHECK_SQUARE_ICONS, &CPluginConfigPage::OnChanges)
	ON_EN_CHANGE(IDC_EDIT_PLUGIN_NAME, &CPluginConfigPage::OnChanges)
	ON_EN_CHANGE(IDC_EDIT_TITLE, &CPluginConfigPage::OnChanges)
	ON_EN_CHANGE(IDC_EDIT_PROVIDER_URL, &CPluginConfigPage::OnChanges)
	ON_EN_CHANGE(IDC_EDIT_PLAYLIST_TEMPLATE, &CPluginConfigPage::OnChanges)
	ON_CBN_SELCHANGE(IDC_COMBO_CATCHUP_TYPE, &CPluginConfigPage::OnChanges)
	ON_EN_CHANGE(IDC_EDIT_SHIFT_SUBST, &CPluginConfigPage::OnChanges)
	ON_EN_CHANGE(IDC_EDIT_DURATION, &CPluginConfigPage::OnChanges)
	ON_EN_CHANGE(IDC_EDIT_STREAM_TEMPLATE, &CPluginConfigPage::OnChanges)
	ON_EN_CHANGE(IDC_EDIT_STREAM_ARC_TEMPLATE, &CPluginConfigPage::OnChanges)
	ON_EN_CHANGE(IDC_EDIT_CUSTOM_STREAM_ARC_TEMPLATE, &CPluginConfigPage::OnChanges)
	ON_EN_CHANGE(IDC_EDIT_EPG_URL, &CPluginConfigPage::OnChanges)
	ON_EN_CHANGE(IDC_EDIT_EPG_FMT_DATE, &CPluginConfigPage::OnChanges)
	ON_EN_CHANGE(IDC_EDIT_EPG_ROOT, &CPluginConfigPage::OnChanges)
	ON_EN_CHANGE(IDC_EDIT_EPG_NAME, &CPluginConfigPage::OnChanges)
	ON_EN_CHANGE(IDC_EDIT_EPG_START, &CPluginConfigPage::OnChanges)
	ON_EN_CHANGE(IDC_EDIT_EPG_END, &CPluginConfigPage::OnChanges)
	ON_EN_CHANGE(IDC_EDIT_EPG_TZ, &CPluginConfigPage::OnChanges)
	ON_EN_CHANGE(IDC_EDIT_EPG_DESC, &CPluginConfigPage::OnChanges)
	ON_EN_CHANGE(IDC_EDIT_EPG_FMT_TIME, &CPluginConfigPage::OnChanges)
	ON_CBN_SELCHANGE(IDC_COMBO_TAGS, &CPluginConfigPage::OnChanges)
	ON_EN_CHANGE(IDC_EDIT_UTC, &CPluginConfigPage::OnEnChangeEditUtc)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER_DATE, &CPluginConfigPage::OnDtnDatetimechangeDatetimepickerDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER_DATE, &CPluginConfigPage::OnDtnDatetimechangeDatetimepickerDate)
END_MESSAGE_MAP()

CPluginConfigPage::CPluginConfigPage(std::vector<std::wstring>& configs)
	: CMFCPropertyPage(IDD_DIALOG_PLUGIN_CONFIG)
	, m_Date(COleDateTime::GetCurrentTime())
	, m_configs(configs)
	, m_VodPlaylistTemplate(_T(""))
	, m_VodParseRegex(_T(""))
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
	DDX_Control(pDX, IDC_EDIT_SHORT_NAME, m_wndShortName);
	DDX_Text(pDX, IDC_EDIT_SHORT_NAME, m_ShortName);
	DDX_Control(pDX, IDC_EDIT_PROVIDER_URL, m_wndProviderUrl);
	DDX_Text(pDX, IDC_EDIT_PROVIDER_URL, m_ProviderUrl);
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
	DDX_Control(pDX, IDC_COMBO_PLAYLIST_TEMPLATE, m_wndPlaylistTemplates);
	DDX_Control(pDX, IDC_BUTTON_EDIT_TEMPLATES, m_wndBtnEditTemplates);
	DDX_Control(pDX, IDC_EDIT_PLAYLIST_TEMPLATE, m_wndPlaylistTemplate);
	DDX_Text(pDX, IDC_EDIT_PLAYLIST_TEMPLATE, m_PlaylistTemplate);
	DDX_Control(pDX, IDC_CHECK_SQUARE_ICONS, m_wndChkSquareIcons);
	DDX_Control(pDX, IDC_BUTTON_PLAYLIST_SHOW, m_wndBtnPlaylistTest);
	DDX_Control(pDX, IDC_BUTTON_STREAM_PARSE, m_wndBtnStreamParseTest);
	DDX_Control(pDX, IDC_CHECK_STATIC_SERVERS, m_wndChkStaticServers);
	DDX_Control(pDX, IDC_BUTTON_EDIT_SERVERS, m_wndBtnServers);
	DDX_Control(pDX, IDC_CHECK_STATIC_DEVICES, m_wndChkStaticDevices);
	DDX_Control(pDX, IDC_BUTTON_EDIT_DEVICES, m_wndBtnDevices);
	DDX_Control(pDX, IDC_CHECK_STATIC_QUALITIES, m_wndChkStaticQualities);
	DDX_Control(pDX, IDC_BUTTON_EDIT_QUALITY, m_wndBtnQualities);
	DDX_Control(pDX, IDC_CHECK_STATIC_PROFILES, m_wndChkStaticProfiles);
	DDX_Control(pDX, IDC_BUTTON_EDIT_PROFILES, m_wndBtnProfiles);
	DDX_Text(pDX, IDC_EDIT_UTC, m_UTC);
	DDX_Control(pDX, IDC_CHECK_VOD_SUPPORT, m_wndChkEnableVOD);
	DDX_Control(pDX, IDC_CHECK_VOD_M3U, m_wndChkVodM3U);
	DDX_Control(pDX, IDC_COMBO_VOD_TEMPLATE, m_wndVodTemplates);
	DDX_Control(pDX, IDC_EDIT_PROVIDER_VOD_URL, m_wndVodUrlTemplate);
	DDX_Control(pDX, IDC_EDIT_VOD_REGEX, m_wndVodRegex);
	DDX_Text(pDX, IDC_EDIT_PROVIDER_VOD_URL, m_VodPlaylistTemplate);
	DDX_Text(pDX, IDC_EDIT_VOD_REGEX, m_VodParseRegex);
	DDX_Control(pDX, IDC_BUTTON_VOD_PARSE, m_wndBtnVodParseTest);
	DDX_Control(pDX, IDC_BUTTON_VOD_TEMPLATE, m_wndBtnVodTemplateTest);
	DDX_Control(pDX, IDC_CHECK_USE_DURATION, m_wndChkUseDuration);
	DDX_Control(pDX, IDC_COMBO_TAGS, m_wndTags);
	DDX_Control(pDX, IDC_CHECK_MAP_TAG_TO_ID, m_wndCheckMapTags);
}

BOOL CPluginConfigPage::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_LBUTTONDOWN
		|| pMsg->message == WM_LBUTTONUP
		|| pMsg->message == WM_MOUSEMOVE)
	{
		HWND hWnd = pMsg->hwnd;
		LPARAM lParam = pMsg->lParam;

		POINT pt{};
		pt.x = LOWORD(pMsg->lParam);  // horizontal position of cursor
		pt.y = HIWORD(pMsg->lParam);  // vertical position of cursor

		for (auto& pair : m_tooltips_info)
		{
			auto& wnd = pair.first;
			CRect rect;
			wnd->GetWindowRect(&rect);
			ScreenToClient(&rect);

			if (rect.PtInRect(pt)) {
				pMsg->hwnd = wnd->m_hWnd;

				ClientToScreen(&pt);
				wnd->ScreenToClient(&pt);
				pMsg->lParam = MAKELPARAM(pt.x, pt.y);
				break;
			}
		}

		m_wndToolTipCtrl.RelayEvent(pMsg);
		m_wndToolTipCtrl.Activate(TRUE);

		pMsg->hwnd = hWnd;
		pMsg->lParam = lParam;
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

	AddTooltip(IDC_COMBO_PLUGIN_CONFIG, IDS_STRING_COMBO_CONFIG);
	AddTooltip(IDC_BUTTON_EDIT_CONFIG, IDS_STRING_BUTTON_EDIT_CONFIG);
	AddTooltip(IDC_BUTTON_SAVE_CONFIG, IDS_STRING_BUTTON_SAVE_CONFIG);
	AddTooltip(IDC_BUTTON_SAVE_AS_CONFIG, IDS_STRING_BUTTON_SAVE_AS_CONFIG);
	AddTooltip(IDC_EDIT_PLUGIN_NAME, IDS_STRING_EDIT_PLUGIN_NAME);
	AddTooltip(IDC_EDIT_TITLE, IDS_STRING_EDIT_TITLE);
	AddTooltip(IDC_EDIT_SHORT_NAME, IDS_STRING_EDIT_SHORT_NAME);
	AddTooltip(IDC_EDIT_PROVIDER_URL, IDS_STRING_EDIT_PROVIDER_URL);
	AddTooltip(IDC_CHECK_SQUARE_ICONS, IDS_STRING_CHECK_SQUARE_ICONS);
	AddTooltip(IDC_EDIT_PLAYLIST_TEMPLATE, IDS_STRING_EDIT_PLAYLIST_TEMPLATE);
	AddTooltip(IDC_EDIT_PARSE_PATTERN, IDS_STRING_EDIT_PARSE_PATTERN);
	AddTooltip(IDC_BUTTON_PLAYLIST_SHOW, IDS_STRING_BUTTON_PLAYLIST_SHOW);
	AddTooltip(IDC_BUTTON_STREAM_PARSE, IDS_STRING_BUTTON_STREAM_PARSE);
	AddTooltip(IDC_CHECK_STATIC_SERVERS, IDS_STRING_CHECK_STATIC_SERVERS);
	AddTooltip(IDC_BUTTON_EDIT_SERVERS, IDS_STRING_BUTTON_EDIT_SERVERS);
	AddTooltip(IDC_CHECK_STATIC_DEVICES, IDS_STRING_CHECK_STATIC_DEVICES);
	AddTooltip(IDC_BUTTON_EDIT_DEVICES, IDS_STRING_BUTTON_EDIT_DEVICES);
	AddTooltip(IDC_CHECK_STATIC_QUALITIES, IDS_STRING_CHECK_STATIC_QUALITIES);
	AddTooltip(IDC_BUTTON_EDIT_QUALITY, IDS_STRING_BUTTON_EDIT_QUALITY);
	AddTooltip(IDC_CHECK_STATIC_PROFILES, IDS_STRING_CHECK_STATIC_PROFILES);
	AddTooltip(IDC_BUTTON_EDIT_PROFILES, IDS_STRING_BUTTON_EDIT_PROFILES);
	AddTooltip(IDC_EDIT_SHIFT_SUBST, IDS_STRING_EDIT_SHIFT_SUBST);
	AddTooltip(IDC_EDIT_DURATION, IDS_STRING_EDIT_DURATION);
	AddTooltip(IDC_EDIT_STREAM_TEMPLATE, IDS_STRING_EDIT_STREAM_TEMPLATE);
	AddTooltip(IDC_EDIT_STREAM_ARC_TEMPLATE, IDS_STRING_EDIT_STREAM_ARC_TEMPLATE);
	AddTooltip(IDC_EDIT_CUSTOM_STREAM_ARC_TEMPLATE, IDS_STRING_EDIT_STREAM_ARC_TEMPLATE);
	AddTooltip(IDC_EDIT_EPG_URL, IDS_STRING_EDIT_EPG_URL);
	AddTooltip(IDC_EDIT_EPG_ROOT, IDS_STRING_EDIT_EPG_ROOT);
	AddTooltip(IDC_EDIT_EPG_NAME, IDS_STRING_EDIT_EPG_NAME);
	AddTooltip(IDC_EDIT_EPG_DESC, IDS_STRING_EDIT_EPG_DESC);
	AddTooltip(IDC_EDIT_EPG_START, IDS_STRING_EDIT_EPG_START);
	AddTooltip(IDC_EDIT_EPG_END, IDS_STRING_EDIT_EPG_END);
	AddTooltip(IDC_EDIT_EPG_FMT_DATE, IDS_STRING_EDIT_EPG_FMT_DATE);
	AddTooltip(IDC_EDIT_EPG_FMT_TIME, IDS_STRING_EDIT_EPG_FMT_TIME);
	AddTooltip(IDC_EDIT_EPG_TZ, IDS_STRING_EDIT_EPG_TZ);
	AddTooltip(IDC_COMBO_ACCESS_TYPE, IDS_STRING_COMBO_ACCESS_TYPE);
	AddTooltip(IDC_COMBO_STREAM_TYPE, IDS_STRING_COMBO_STREAM_TYPE);
	AddTooltip(IDC_COMBO_CATCHUP_TYPE, IDS_STRING_COMBO_CATCHUP_TYPE);
	AddTooltip(IDC_COMBO_EPG_TYPE, IDS_STRING_EPG_TYPE);
	AddTooltip(IDC_BUTTON_EPG_SHOW, IDS_STRING_BUTTON_EPG_SHOW);
	AddTooltip(IDC_EDIT_SET_ID, IDS_STRING_EDIT_SET_ID);
	AddTooltip(IDC_EDIT_SET_TOKEN, IDS_STRING_EDIT_SET_TOKEN);
	AddTooltip(IDC_DATETIMEPICKER_DATE, IDS_STRING_DATETIMEPICKER_DATE);
	AddTooltip(IDC_EDIT_UTC, IDS_STRING_DATETIMEPICKER_DATE);
	AddTooltip(IDC_CHECK_USE_DURATION, IDS_STRING_CHECK_USE_DURATION);

	m_wndToolTipCtrl.SetDelayTime(TTDT_AUTOPOP, 10000);
	m_wndToolTipCtrl.SetDelayTime(TTDT_INITIAL, 500);
	m_wndToolTipCtrl.SetMaxTipWidth(300);
	m_wndToolTipCtrl.Activate(TRUE);

	SetButtonImage(IDB_PNG_EDIT, m_wndBtnToggleEdit);
	SetButtonImage(IDB_PNG_SAVE, m_wndBtnSaveConf);
	SetButtonImage(IDB_PNG_SAVE_AS, m_wndBtnSaveAsConf);

	SetButtonImage(IDB_PNG_EDIT, m_wndBtnEditTemplates);
	SetButtonImage(IDB_PNG_EDIT, m_wndBtnServers);
	SetButtonImage(IDB_PNG_EDIT, m_wndBtnDevices);
	SetButtonImage(IDB_PNG_EDIT, m_wndBtnQualities);
	SetButtonImage(IDB_PNG_EDIT, m_wndBtnProfiles);

	m_wndBtnToggleEdit.EnableWindow(TRUE);
	m_plugin->load_plugin_parameters(m_initial_cred.get_config());

	AssignMacros();
	FillConfigs();

	UpdateDateTimestamp(true);
	m_Token = m_initial_cred.get_token().c_str();
	if (m_CurrentStream)
	{
		m_SetID = m_CurrentStream->get_epg_id(0).c_str();
	}

	FillControlsCommon();

	RestoreWindowPos(GetSafeHwnd(), REG_CONFIG_WINDOW_POS);
	AllowSave(false);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPluginConfigPage::AddTooltip(UINT ctrlID, UINT textID)
{
	CWnd* wnd = GetDlgItem(ctrlID);
	m_tooltips_info.emplace(wnd, load_string_resource(textID));
	m_wndToolTipCtrl.AddTool(wnd, LPSTR_TEXTCALLBACK);
}

void CPluginConfigPage::AssignMacros()
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
	m_wndVodUrlTemplate.SetTemplateParams(pl_params);

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

	std::vector<std::wstring> epg_params =
	{
		L"{ID}",
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
}

void CPluginConfigPage::FillConfigs()
{
	m_wndPluginConfigs.ResetContent();
	int cur_idx = 0;
	for (const auto& entry : m_configs)
	{
		int idx = m_wndPluginConfigs.AddString(entry.c_str());
		if (!m_initial_cred.config.empty() && entry == m_initial_cred.get_config())
			cur_idx = idx;
	}

	m_wndPluginConfigs.SetCurSel(cur_idx);
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
		const auto& pair = std::find_if(m_tooltips_info.begin(), m_tooltips_info.end(), [nID](auto& pair)
										{
											return pair.first->GetDlgCtrlID() == nID;
										});

		if (pair != m_tooltips_info.end())
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

void CPluginConfigPage::AllowSave(bool val /*= true*/)
{
	m_allow_save = val;
	m_wndBtnSaveConf.EnableWindow(m_allow_save && !GetSelectedConfig().empty());

	CPropertySheet* psheet = (CPropertySheet*)GetParent();
	//get the button and disable it modifying it's style
	//psheet->GetDlgItem(IDOK)->ModifyStyle(0, WS_DISABLED);
	//force the window (button) to be redrawn
	psheet->GetDlgItem(IDOK)->EnableWindow(!m_allow_save);

}

void CPluginConfigPage::EnableControls()
{
	UpdateData(TRUE);

	bool custom = m_plugin->get_plugin_type() == PluginType::enCustom;
	bool enable = m_allow_edit;

	m_wndPluginConfigs.EnableWindow(!enable);

	// buttons
	m_wndBtnSaveConf.EnableWindow(enable && m_allow_save && !GetSelectedConfig().empty());

	// common
	m_wndName.EnableWindow(enable);
	m_wndTitle.EnableWindow(enable);
	m_wndShortName.EnableWindow(FALSE);
	m_wndProviderUrl.EnableWindow(enable);
	m_wndChkSquareIcons.EnableWindow(enable);
	m_wndAccessType.EnableWindow(enable && custom);
	m_wndPlaylistTemplates.EnableWindow(enable);
	m_wndBtnEditTemplates.EnableWindow(enable);
	m_wndPlaylistTemplate.EnableWindow(enable);
	m_wndParseStream.EnableWindow(enable);
	m_wndCheckMapTags.EnableWindow(enable);
	m_wndTags.EnableWindow(enable && m_wndCheckMapTags.GetCheck() != 0);
	m_wndChkEnableVOD.EnableWindow(enable);
	m_wndChkVodM3U.EnableWindow(enable);
	m_wndVodUrlTemplate.EnableWindow(enable);
	m_wndVodRegex.EnableWindow(enable);

	// test
	m_wndBtnStreamParseTest.EnableWindow(!m_ParseStream.IsEmpty());

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
	m_wndChkUseDuration.EnableWindow(enable);

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
	m_wndBtnEpgTest.EnableWindow(!m_EpgUrl.IsEmpty());
}

void CPluginConfigPage::FillControlsCommon()
{
	if (!m_plugin) return;

	m_wndAccessType.SetCurSel((int)m_plugin->get_access_type());
	m_wndChkSquareIcons.SetCheck(m_plugin->get_square_icons() != false);
	m_wndChkEnableVOD.SetCheck(m_plugin->get_vod_support() != false);
	m_wndChkVodM3U.SetCheck(m_plugin->get_vod_m3u() != false);

	m_Name = m_plugin->get_name().c_str();
	m_Title = m_plugin->get_title().c_str();
	m_ShortName = m_plugin->get_short_name_w().c_str();
	m_ProviderUrl = m_plugin->get_provider_url().c_str();

	m_wndPlaylistTemplates.ResetContent();
	for (const auto& entry : m_plugin->get_playlist_templates())
	{
		m_wndPlaylistTemplates.AddString(entry.get_name().c_str());
	}

	int pl_idx = m_plugin->get_playlist_template_idx();
	m_wndPlaylistTemplates.SetCurSel(pl_idx);
	m_PlaylistTemplate = m_plugin->get_playlist_template(pl_idx).c_str();

	m_ParseStream = m_plugin->get_uri_parse_pattern().c_str();
	if (m_plugin->get_tag_id_match().empty())
	{
		m_wndCheckMapTags.SetCheck(FALSE);
		m_wndTags.EnableWindow(FALSE);
	}
	else
	{
		int idx = m_wndTags.FindString(-1, m_plugin->get_tag_id_match().c_str());
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

	m_wndVodTemplates.ResetContent();
	for (const auto& entry : m_plugin->get_vod_templates())
	{
		m_wndVodTemplates.AddString(entry.get_name().c_str());
	}

	int vod_idx = m_plugin->get_vod_template_idx();
	m_wndVodTemplates.SetCurSel(vod_idx);
	m_VodPlaylistTemplate = m_plugin->get_vod_template(vod_idx).c_str();

	m_VodParseRegex = m_plugin->get_vod_parse_pattern().c_str();

	m_supported_streams = m_plugin->get_supported_streams();
	m_epg_parameters = m_plugin->get_epg_parameters();

	m_wndStreamType.SetCurSel(0);
	m_wndEpgType.SetCurSel(0);

	FillControlsStream();
	FillControlsEpg();
	EnableControls();
}

void CPluginConfigPage::SaveControlsCommon()
{
	if (!m_plugin) return;

	UpdateData(TRUE);

	m_plugin->set_access_type((AccountAccessType)m_wndAccessType.GetCurSel());
	m_plugin->set_square_icons(m_wndChkSquareIcons.GetCheck() != 0);
	m_plugin->set_vod_support(m_wndChkEnableVOD.GetCheck() != 0);
	m_plugin->set_vod_m3u(m_wndChkVodM3U.GetCheck() != 0);

	m_plugin->set_name(m_Name.GetString());
	m_plugin->set_title(m_Title.GetString());
	m_plugin->set_short_name_w(m_ShortName.GetString());
	m_plugin->set_provider_url(m_ProviderUrl.GetString());
	m_plugin->set_playlist_template(m_plugin->get_playlist_template_idx(), m_PlaylistTemplate.GetString());
	m_plugin->set_uri_parse_pattern(m_ParseStream.GetString());
	CString tag;
	if (m_wndCheckMapTags.GetCheck() != 0)
	{
		m_wndTags.GetLBText(m_wndTags.GetCurSel(), tag);
	}
	m_plugin->set_tag_id_match(tag.GetString());
	m_plugin->set_vod_template(m_plugin->get_vod_template_idx(), m_VodPlaylistTemplate.GetString());
	m_plugin->set_vod_parse_regex(m_VodParseRegex.GetString());

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
	m_wndDuration.EnableWindow(enableDuration);

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
	m_wndChkUseDuration.SetCheck(epg.epg_use_duration != false);

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
	epg.epg_use_duration = m_wndChkUseDuration.GetCheck() != 0;
}

void CPluginConfigPage::UpdateDateTimestamp(bool dateToUtc)
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

void CPluginConfigPage::OnBnClickedButtonToggleEditConfig()
{
	if (m_allow_edit && m_allow_save)
	{
		if (IDNO == AfxMessageBox(L"Changes not saved. Continue?", MB_ICONWARNING | MB_YESNO))
			return;

		AllowSave(false);
	}

	m_allow_edit = !m_allow_edit;
	AllowSave(false);
	EnableControls();
}

void CPluginConfigPage::OnBnClickedButtonSaveConfig()
{
	SaveControlsCommon();

	auto name = GetSelectedConfig();
	if (name.empty()) return;

	if (m_plugin->save_plugin_parameters(name))
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

	SaveControlsCommon();
	std::filesystem::path new_conf = dlg.m_name.GetString();
	if (new_conf.extension().empty())
		new_conf += (L".json");

	if (!m_plugin->save_plugin_parameters(new_conf))
	{
		AfxMessageBox(IDS_STRING_ERR_SAVE_CONFIG, MB_ICONERROR | MB_OK);
	}
	else
	{
		AllowSave(false);
		m_configs.emplace_back(new_conf);
		FillConfigs();
		m_wndPluginConfigs.SetCurSel((int)m_configs.size() - 1);
		UpdateData(FALSE);
	}
}

void CPluginConfigPage::OnCbnSelchangeComboAccessType()
{
	m_plugin->set_access_type((AccountAccessType)m_wndAccessType.GetCurSel());
	OnChanges();
	FillControlsCommon();
}

void CPluginConfigPage::OnCbnDropdownComboAccessType()
{
	SaveControlsCommon();
}

void CPluginConfigPage::OnCbnSelchangeComboStreamType()
{
	OnChanges();
	FillControlsStream();
}

void CPluginConfigPage::OnCbnDropdownComboStreamType()
{
	OnChanges();
	SaveControlsStream();
}

void CPluginConfigPage::OnCbnSelchangeComboEpgType()
{
	OnChanges();
	FillControlsEpg();
}

void CPluginConfigPage::OnCbnDropdownComboEpgType()
{
	OnChanges();
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

	CWaitCursor cur;
	std::stringstream data;
	if (utils::DownloadFile(url, data))
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
}

void CPluginConfigPage::OnBnClickedButtonPlaylistShow()
{
	TemplateParams params;
	params.token = m_initial_cred.get_token();
	params.login = m_initial_cred.get_login();
	params.password = m_initial_cred.get_password();
	params.subdomain = m_initial_cred.get_subdomain();
	params.server_idx = m_initial_cred.server_id;
	params.device_idx = m_initial_cred.device_id;
	params.profile_idx = m_initial_cred.profile_id;
	params.quality_idx = m_initial_cred.quality_id;
	params.playlist_idx = m_wndPlaylistTemplates.GetCurSel();

	CWaitCursor cur;
	const auto& url = m_plugin->get_playlist_url(params);
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

void CPluginConfigPage::OnBnClickedButtonVodTemplate()
{
	TemplateParams params;
	params.token = m_initial_cred.get_token();
	params.login = m_initial_cred.get_login();
	params.password = m_initial_cred.get_password();
	params.subdomain = m_initial_cred.get_subdomain();
	params.server_idx = m_initial_cred.server_id;
	params.device_idx = m_initial_cred.device_id;
	params.profile_idx = m_initial_cred.profile_id;
	params.quality_idx = m_initial_cred.quality_id;
	params.playlist_idx = m_wndVodTemplates.GetCurSel();

	CWaitCursor cur;
	const auto& url = m_plugin->get_vod_url(params);
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
}

void CPluginConfigPage::OnBnClickedButtonStreamRegexTest()
{
	ShellExecute(nullptr, _T("open"), L"https://regex101.com/", nullptr, nullptr, SW_SHOWDEFAULT);
}

void CPluginConfigPage::OnEnChangeEditParsePattern()
{
	OnChanges();
	UpdateData(TRUE);
	m_wndBtnStreamParseTest.EnableWindow(!m_ParseStream.IsEmpty());
}

void CPluginConfigPage::OnEnChangeEditProviderVodUrl()
{
	OnChanges();
	UpdateData(TRUE);
	m_wndBtnVodTemplateTest.EnableWindow(!m_VodPlaylistTemplate.IsEmpty());
}

void CPluginConfigPage::OnEnChangeEditVodRegex()
{
	OnChanges();
	UpdateData(TRUE);
	m_wndBtnVodParseTest.EnableWindow(!m_VodParseRegex.IsEmpty());
}

void CPluginConfigPage::OnBnClickedButtonEditServers()
{
	CFillParamsInfoDlg dlg;
	dlg.m_type = 0;
	dlg.m_paramsList = m_plugin->get_servers_list();
	dlg.m_readonly = !m_allow_edit;

	if (dlg.DoModal() == IDOK)
	{
		OnChanges();
		m_plugin->set_servers_list(dlg.m_paramsList);
	}
}

void CPluginConfigPage::OnBnClickedButtonEditDevices()
{
	CFillParamsInfoDlg dlg;
	dlg.m_type = 1;
	dlg.m_paramsList = m_plugin->get_devices_list();
	dlg.m_readonly = !m_allow_edit;

	if (dlg.DoModal() == IDOK)
	{
		OnChanges();
		m_plugin->set_devices_list(dlg.m_paramsList);
	}
}

void CPluginConfigPage::OnBnClickedButtonEditQuality()
{
	CFillParamsInfoDlg dlg;
	dlg.m_type = 2;
	dlg.m_paramsList = m_plugin->get_qualities_list();
	dlg.m_readonly = !m_allow_edit;

	if (dlg.DoModal() == IDOK)
	{
		OnChanges();
		m_plugin->set_qualities_list(dlg.m_paramsList);
	}
}

void CPluginConfigPage::OnBnClickedButtonEditProfiles()
{
	CFillParamsInfoDlg dlg;
	dlg.m_type = 3;
	dlg.m_paramsList = m_plugin->get_profiles_list();
	dlg.m_readonly = !m_allow_edit;

	if (dlg.DoModal() == IDOK)
	{
		OnChanges();
		m_plugin->set_profiles_list(dlg.m_paramsList);
	}
}

void CPluginConfigPage::OnCbnSelchangeComboPluginConfig()
{
	AllowSave(false);
	m_plugin->load_plugin_parameters(GetSelectedConfig());
	FillControlsCommon();
}

void CPluginConfigPage::OnBnClickedCheckStaticServers()
{
	OnChanges();
	m_wndBtnServers.EnableWindow(m_wndChkStaticServers.GetCheck());
}


void CPluginConfigPage::OnBnClickedCheckStaticDevices()
{
	OnChanges();
	m_wndBtnDevices.EnableWindow(m_wndChkStaticDevices.GetCheck());
}


void CPluginConfigPage::OnBnClickedCheckStaticQualities()
{
	OnChanges();
	m_wndBtnQualities.EnableWindow(m_wndChkStaticQualities.GetCheck());
}


void CPluginConfigPage::OnBnClickedCheckStaticProfiles()
{
	OnChanges();
	m_wndBtnProfiles.EnableWindow(m_wndChkStaticProfiles.GetCheck());
}

void CPluginConfigPage::OnChanges()
{
	AllowSave();
}

void CPluginConfigPage::OnEnChangeEditUtc()
{
	UpdateDateTimestamp(false);
}

void CPluginConfigPage::OnDtnDatetimechangeDatetimepickerDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateDateTimestamp(true);
	*pResult = 0;
}

void CPluginConfigPage::OnCbnSelchangeComboPlaylistTemplate()
{
	int idx = m_wndPlaylistTemplates.GetCurSel();
	m_plugin->set_playlist_template_idx(idx);
	m_PlaylistTemplate = m_plugin->get_playlist_template(idx).c_str();
	UpdateData(FALSE);
}

void CPluginConfigPage::OnCbnDropdownComboPlaylistTemplate()
{
	UpdateData(TRUE);

	m_plugin->set_playlist_template(m_wndPlaylistTemplates.GetCurSel(), m_PlaylistTemplate.GetString());
}

void CPluginConfigPage::OnBnClickedButtonEditTemplates()
{
	std::vector<DynamicParamsInfo> info;
	for (const auto& item : m_plugin->get_playlist_templates())
	{
		info.emplace_back(item.name, item.pl_template);
	}

	CFillParamsInfoDlg dlg;
	dlg.m_type = 4;
	dlg.m_paramsList = std::move(info);
	dlg.m_readonly = !m_allow_edit;

	if (dlg.DoModal() == IDOK)
	{
		OnChanges();

		std::vector<PlaylistTemplateInfo> playlists;
		for (const auto& item : dlg.m_paramsList)
		{
			playlists.emplace_back(item.id, item.name);
		}
		m_plugin->set_playlist_templates(playlists);

		FillControlsCommon();
	}
}

void CPluginConfigPage::OnCbnSelchangeComboVodTemplate()
{
	int idx = m_wndVodTemplates.GetCurSel();
	m_plugin->set_vod_template_idx(idx);
	m_VodPlaylistTemplate = m_plugin->get_vod_template(idx).c_str();
	UpdateData(FALSE);
}

void CPluginConfigPage::OnCbnDropdownComboVodTemplate()
{
	UpdateData(TRUE);

	m_plugin->set_vod_template(m_wndVodTemplates.GetCurSel(), m_VodPlaylistTemplate.GetString());
}

void CPluginConfigPage::OnBnClickedButtonEditVodTemplates()
{
	std::vector<DynamicParamsInfo> info;
	for (const auto& item : m_plugin->get_vod_templates())
	{
		info.emplace_back(item.name, item.pl_template);
	}

	CFillParamsInfoDlg dlg;
	dlg.m_type = 4;
	dlg.m_paramsList = std::move(info);
	dlg.m_readonly = !m_allow_edit;

	if (dlg.DoModal() == IDOK)
	{
		OnChanges();

		std::vector<PlaylistTemplateInfo> playlists;
		for (const auto& item : dlg.m_paramsList)
		{
			playlists.emplace_back(item.id, item.name);
		}
		m_plugin->set_vod_templates(playlists);

		FillControlsCommon();
	}
}

void CPluginConfigPage::OnBnClickedCheckMapTagToId()
{
	m_wndTags.EnableWindow(m_wndCheckMapTags.GetCheck() != 0);
	OnChanges();
}
