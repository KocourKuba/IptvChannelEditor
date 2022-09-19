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
#include <afxdialogex.h>
#include <tuple>

#include "IPTVChannelEditor.h"
#include "IPTVChannelEditorDlg.h"
#include "ResizedPropertySheet.h"
#include "AboutDlg.h"
#include "MainSettingsPage.h"
#include "PathsSettingsPage.h"
#include "UpdateSettingsPage.h"
#include "AccessInfoDlg.h"
#include "FilterDialog.h"
#include "CustomPlaylistDlg.h"
#include "NewChannelsListDlg.h"
#include "PlaylistParseM3U8Thread.h"
#include "PlaylistParseXMLThread.h"
#include "GetStreamInfoThread.h"
#include "IconCache.h"
#include "IconsListDlg.h"
#include "IconLinkDlg.h"
#include "UtilsLib\utils.h"
#include "Config.h"
#include "EpgListDlg.h"
#include "VodViewer.h"

#include "UtilsLib\inet_utils.h"
#include "UtilsLib\md5.h"
#include "UtilsLib\rapidxml_print.hpp"
#include "UtilsLib\rapidxml_value.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

constexpr auto ID_COPY_TO_START = 40000;
constexpr auto ID_COPY_TO_END = ID_COPY_TO_START + 512;

constexpr auto ID_MOVE_TO_START = ID_COPY_TO_END + 1;
constexpr auto ID_MOVE_TO_END = ID_MOVE_TO_START + 512;

constexpr auto ID_ADD_TO_START = ID_MOVE_TO_END + 1;
constexpr auto ID_ADD_TO_END = ID_ADD_TO_START + 512;

constexpr auto ID_ACCOUNT_TO_START = ID_ADD_TO_END + 1;
constexpr auto ID_ACCOUNT_TO_END = ID_ACCOUNT_TO_START + 512;

constexpr auto ID_UPDATE_EPG_TIMER = 1000;

constexpr auto MOD_TITLE   = 0x01;
constexpr auto MOD_ARCHIVE = 0x02;
constexpr auto MOD_EPG1    = 0x04;
constexpr auto MOD_EPG2    = 0x08;
constexpr auto MOD_ICON    = 0x10;

// Common
constexpr auto CHANNELS_LIST_VERSION = 4;

int CALLBACK CBCompareForSwap(LPARAM lParam1, LPARAM lParam2, LPARAM)
{
	return lParam1 < lParam2 ? -1 : lParam1 == lParam2 ? 0 : 1;
}

// CEdemChannelEditorDlg dialog

BEGIN_MESSAGE_MAP(CIPTVChannelEditorDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_TIMER()

	ON_BN_CLICKED(IDC_BUTTON_ABOUT, &CIPTVChannelEditorDlg::OnBnClickedButtonAbout)
	ON_BN_CLICKED(IDC_BUTTON_ACCOUNT_SETTINGS, &CIPTVChannelEditorDlg::OnBnClickedButtonAccountSettings)
	ON_BN_CLICKED(IDC_BUTTON_PL_SEARCH_NEXT, &CIPTVChannelEditorDlg::OnBnClickedButtonPlSearchNext)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_PL_SEARCH_NEXT, &CIPTVChannelEditorDlg::OnUpdateButtonPlSearchNext)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH_NEXT, &CIPTVChannelEditorDlg::OnBnClickedButtonSearchNext)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_SEARCH_NEXT, &CIPTVChannelEditorDlg::OnUpdateButtonSearchNext)
	ON_BN_CLICKED(IDC_CHECK_SHOW_UNKNOWN, &CIPTVChannelEditorDlg::OnBnClickedCheckShowUnknown)
	ON_BN_CLICKED(IDC_BUTTON_PL_FILTER, &CIPTVChannelEditorDlg::OnBnClickedButtonPlFilter)
	ON_BN_CLICKED(IDC_BUTTON_ADD_NEW_CHANNELS_LIST, &CIPTVChannelEditorDlg::OnBnClickedButtonCreateNewChannelsList)
	ON_BN_CLICKED(IDC_BUTTON_DOWNLOAD_PLAYLIST, &CIPTVChannelEditorDlg::OnBnClickedButtonDownloadPlaylist)
	ON_BN_CLICKED(IDC_BUTTON_TEST_EPG, &CIPTVChannelEditorDlg::OnBnClickedButtonViewEpg)
	ON_BN_CLICKED(IDC_CHECK_ADULT, &CIPTVChannelEditorDlg::OnBnClickedCheckAdult)
	ON_BN_CLICKED(IDC_CHECK_ARCHIVE, &CIPTVChannelEditorDlg::OnBnClickedCheckArchive)
	ON_BN_CLICKED(IDC_CHECK_CUSTOMIZE, &CIPTVChannelEditorDlg::OnBnClickedCheckCustomize)
	ON_BN_CLICKED(IDC_BUTTON_CHECK_ARCHIVE, &CIPTVChannelEditorDlg::OnBnClickCheckArchive)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CIPTVChannelEditorDlg::OnSave)
	ON_BN_CLICKED(IDC_SPLIT_BUTTON_PACK, &CIPTVChannelEditorDlg::OnBnClickedButtonPack)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CIPTVChannelEditorDlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_SETTINGS, &CIPTVChannelEditorDlg::OnBnClickedButtonSettings)
	ON_BN_CLICKED(IDC_BUTTON_CACHE_ICON, &CIPTVChannelEditorDlg::OnBnClickedButtonCacheIcon)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_CACHE_ICON, &CIPTVChannelEditorDlg::OnUpdateButtonCacheIcon)
	ON_BN_CLICKED(IDC_RADIO_EPG1, &CIPTVChannelEditorDlg::OnBnClickedButtonEpg)
	ON_BN_CLICKED(IDC_RADIO_EPG2, &CIPTVChannelEditorDlg::OnBnClickedButtonEpg)
	ON_BN_CLICKED(IDC_SPLIT_BUTTON_UPDATE_CHANGED, &CIPTVChannelEditorDlg::OnBnClickedButtonUpdateChanged)
	ON_BN_CLICKED(IDC_CHECK_SHOW_CHANGED, &CIPTVChannelEditorDlg::OnBnClickedCheckShowChanged)
	ON_BN_CLICKED(IDC_CHECK_SHOW_CHANGED_CH, &CIPTVChannelEditorDlg::OnBnClickedCheckShowChangedCh)

	ON_EN_CHANGE(IDC_EDIT_EPG1_ID, &CIPTVChannelEditorDlg::OnEnChangeEditEpg1ID)
	ON_EN_CHANGE(IDC_EDIT_EPG2_ID, &CIPTVChannelEditorDlg::OnEnChangeEditEpg2ID)
	ON_EN_CHANGE(IDC_EDIT_TIME_SHIFT, &CIPTVChannelEditorDlg::OnEnChangeEditTimeShiftHours)
	ON_EN_CHANGE(IDC_EDIT_ARCHIVE_CHECK_DAYS, &CIPTVChannelEditorDlg::OnEnChangeEditArchiveCheckDays)
	ON_EN_CHANGE(IDC_EDIT_ARCHIVE_CHECK_HOURS, &CIPTVChannelEditorDlg::OnEnChangeEditArchiveCheckHours)
	ON_EN_CHANGE(IDC_EDIT_STREAM_URL, &CIPTVChannelEditorDlg::OnEnChangeEditStreamUrl)
	ON_EN_CHANGE(IDC_EDIT_ARCHIVE_DAYS, &CIPTVChannelEditorDlg::OnEnChangeEditArchiveDays)
	ON_EN_CHANGE(IDC_EDIT_URL_ID, &CIPTVChannelEditorDlg::OnEnChangeEditUrlID)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_TIME_SHIFT, &CIPTVChannelEditorDlg::OnDeltaposSpinTimeShiftHours)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_ARCHIVE_CHECK_DAYS, &CIPTVChannelEditorDlg::OnDeltaposSpinArchiveCheckDay)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_ARCHIVE_CHECK_HOURS, &CIPTVChannelEditorDlg::OnDeltaposSpinArchiveCheckHour)
	ON_STN_CLICKED(IDC_STATIC_ICON, &CIPTVChannelEditorDlg::OnStnClickedStaticIcon)

	ON_CBN_SELCHANGE(IDC_COMBO_PLUGIN_TYPE, &CIPTVChannelEditorDlg::OnCbnSelchangeComboPluginType)
	ON_CBN_SELCHANGE(IDC_COMBO_CHANNELS, &CIPTVChannelEditorDlg::OnCbnSelchangeComboChannels)
	ON_CBN_SELCHANGE(IDC_COMBO_PLAYLIST, &CIPTVChannelEditorDlg::OnCbnSelchangeComboPlaylist)
	ON_CBN_SELCHANGE(IDC_COMBO_ICON_SOURCE, &CIPTVChannelEditorDlg::OnCbnSelchangeComboIconSource)
	ON_CBN_SELCHANGE(IDC_COMBO_STREAM_TYPE, &CIPTVChannelEditorDlg::OnCbnSelchangeComboStreamType)

	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_CHANNELS, &CIPTVChannelEditorDlg::OnTvnSelchangedTreeChannels)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_CHANNELS, &CIPTVChannelEditorDlg::OnNMDblclkTreeChannels)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_TREE_CHANNELS, &CIPTVChannelEditorDlg::OnTvnEndlabeleditTreeChannels)
	ON_NOTIFY(NM_RCLICK, IDC_TREE_CHANNELS, &CIPTVChannelEditorDlg::OnNMRclickTreeChannel)
	ON_NOTIFY(NM_SETFOCUS, IDC_TREE_CHANNELS, &CIPTVChannelEditorDlg::OnNMSetfocusTree)
	ON_NOTIFY(TVN_GETINFOTIP, IDC_TREE_CHANNELS, &CIPTVChannelEditorDlg::OnTvnChannelsGetInfoTip)
	ON_NOTIFY(BCN_DROPDOWN, IDC_SPLIT_BUTTON_UPDATE_CHANGED, &CIPTVChannelEditorDlg::OnBnDropDownSplitButtonUpdateChanged)

	ON_NOTIFY(NM_DBLCLK, IDC_TREE_PLAYLIST, &CIPTVChannelEditorDlg::OnNMDblclkTreePaylist)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_PLAYLIST, &CIPTVChannelEditorDlg::OnTvnSelchangedTreePaylist)
	ON_NOTIFY(NM_RCLICK, IDC_TREE_PLAYLIST, &CIPTVChannelEditorDlg::OnNMRclickTreePlaylist)
	ON_NOTIFY(NM_SETFOCUS, IDC_TREE_PLAYLIST, &CIPTVChannelEditorDlg::OnNMSetfocusTree)
	ON_NOTIFY(TVN_GETINFOTIP, IDC_TREE_PLAYLIST, &CIPTVChannelEditorDlg::OnTvnPlaylistGetInfoTip)

	ON_COMMAND(ID_SAVE, &CIPTVChannelEditorDlg::OnSave)
	ON_UPDATE_COMMAND_UI(ID_SAVE, &CIPTVChannelEditorDlg::OnUpdateSave)
	ON_COMMAND(ID_NEW_CHANNEL, &CIPTVChannelEditorDlg::OnNewChannel)
	ON_UPDATE_COMMAND_UI(ID_NEW_CHANNEL, &CIPTVChannelEditorDlg::OnUpdateNewChannel)
	ON_COMMAND(ID_EDIT_RENAME, &CIPTVChannelEditorDlg::OnRenameChannel)
	ON_UPDATE_COMMAND_UI(ID_EDIT_RENAME, &CIPTVChannelEditorDlg::OnUpdateRenameChannel)
	ON_COMMAND(ID_REMOVE, &CIPTVChannelEditorDlg::OnRemove)
	ON_UPDATE_COMMAND_UI(ID_REMOVE, &CIPTVChannelEditorDlg::OnUpdateRemove)
	ON_COMMAND(ID_UPDATE_CHANNEL, &CIPTVChannelEditorDlg::OnAddUpdateChannel)
	ON_UPDATE_COMMAND_UI(ID_UPDATE_CHANNEL, &CIPTVChannelEditorDlg::OnUpdateAddUpdateChannel)
	ON_COMMAND(ID_CHANNEL_UP, &CIPTVChannelEditorDlg::OnChannelUp)
	ON_UPDATE_COMMAND_UI(ID_CHANNEL_UP, &CIPTVChannelEditorDlg::OnUpdateChannelUp)
	ON_COMMAND(ID_CHANNEL_DOWN, &CIPTVChannelEditorDlg::OnChannelDown)
	ON_UPDATE_COMMAND_UI(ID_CHANNEL_DOWN, &CIPTVChannelEditorDlg::OnUpdateChannelDown)
	ON_COMMAND(ID_ADD_CATEGORY, &CIPTVChannelEditorDlg::OnAddCategory)
	ON_UPDATE_COMMAND_UI(ID_ADD_CATEGORY, &CIPTVChannelEditorDlg::OnUpdateAddCategory)
	ON_COMMAND(ID_NEW_CATEGORY, &CIPTVChannelEditorDlg::OnNewCategory)
	ON_COMMAND(ID_SORT_CATEGORY, &CIPTVChannelEditorDlg::OnSortCategory)
	ON_UPDATE_COMMAND_UI(ID_SORT_CATEGORY, &CIPTVChannelEditorDlg::OnUpdateSortCategory)
	ON_COMMAND(ID_TOGGLE_CHANNEL, &CIPTVChannelEditorDlg::OnToggleChannel)
	ON_UPDATE_COMMAND_UI(ID_TOGGLE_CHANNEL, &CIPTVChannelEditorDlg::OnUpdateToggleChannel)
	ON_COMMAND(ID_TOGGLE_CATEGORY, &CIPTVChannelEditorDlg::OnToggleCategory)
	ON_UPDATE_COMMAND_UI(ID_TOGGLE_CATEGORY, &CIPTVChannelEditorDlg::OnUpdateToggleCategory)
	ON_COMMAND(ID_GET_STREAM_INFO, &CIPTVChannelEditorDlg::OnGetStreamInfo)
	ON_UPDATE_COMMAND_UI(ID_GET_STREAM_INFO, &CIPTVChannelEditorDlg::OnUpdateGetStreamInfo)
	ON_COMMAND(ID_CLEAR_STREAM_INFO, &CIPTVChannelEditorDlg::OnClearStreamInfo)
	ON_UPDATE_COMMAND_UI(ID_CLEAR_STREAM_INFO, &CIPTVChannelEditorDlg::OnUpdateClearStreamInfo)
	ON_COMMAND(ID_PLAY_STREAM, &CIPTVChannelEditorDlg::OnPlayStream)
	ON_UPDATE_COMMAND_UI(ID_PLAY_STREAM, &CIPTVChannelEditorDlg::OnUpdatePlayStream)
	ON_COMMAND(ID_SYNC_TREE_ITEM, &CIPTVChannelEditorDlg::OnSyncTreeItem)
	ON_UPDATE_COMMAND_UI(ID_SYNC_TREE_ITEM, &CIPTVChannelEditorDlg::OnUpdateSyncTreeItem)
	ON_COMMAND(ID_ADD_TO_FAVORITE, &CIPTVChannelEditorDlg::OnAddToFavorite)
	ON_UPDATE_COMMAND_UI(ID_ADD_TO_FAVORITE, &CIPTVChannelEditorDlg::OnUpdateAddToFavorite)
	ON_COMMAND(ID_MAKE_ALL, &CIPTVChannelEditorDlg::OnMakeAll)
	ON_COMMAND(ID_MAKE_ALL_ACCOUNTS, &CIPTVChannelEditorDlg::OnMakeAllAccounts)
	ON_COMMAND(ID_REMOVE_UNKNOWN, &CIPTVChannelEditorDlg::OnRemoveUnknownChannels)

	ON_COMMAND(ID_RESTORE, &CIPTVChannelEditorDlg::OnRestore)
	ON_COMMAND(ID_APP_EXIT, &CIPTVChannelEditorDlg::OnAppExit)

	ON_MESSAGE_VOID(WM_KICKIDLE, OnKickIdle)
	ON_MESSAGE(WM_INIT_PROGRESS, &CIPTVChannelEditorDlg::OnInitProgress)
	ON_MESSAGE(WM_UPDATE_PROGRESS, &CIPTVChannelEditorDlg::OnUpdateProgress)
	ON_MESSAGE(WM_END_LOAD_PLAYLIST, &CIPTVChannelEditorDlg::OnEndLoadPlaylist)
	ON_MESSAGE(WM_UPDATE_PROGRESS_STREAM, &CIPTVChannelEditorDlg::OnUpdateProgressStream)
	ON_MESSAGE(WM_END_GET_STREAM_INFO, &CIPTVChannelEditorDlg::OnEndGetStreamInfo)
	ON_MESSAGE(WM_TRAYICON_NOTIFY, &CIPTVChannelEditorDlg::OnTrayIconNotify)

	ON_COMMAND_RANGE(ID_COPY_TO_START, ID_COPY_TO_END, &CIPTVChannelEditorDlg::OnCopyTo)
	ON_COMMAND_RANGE(ID_MOVE_TO_START, ID_MOVE_TO_END, &CIPTVChannelEditorDlg::OnMoveTo)
	ON_COMMAND_RANGE(ID_ADD_TO_START, ID_ADD_TO_END, &CIPTVChannelEditorDlg::OnAddTo)
	ON_COMMAND_RANGE(ID_ACCOUNT_TO_START, ID_ACCOUNT_TO_END, &CIPTVChannelEditorDlg::OnMakeAccount)

	ON_BN_CLICKED(IDC_CHECK_NOT_ADDED, &CIPTVChannelEditorDlg::OnBnClickedCheckNotAdded)
	ON_BN_CLICKED(IDC_CHECK_SHOW_UNKNOWN, &CIPTVChannelEditorDlg::OnBnClickedCheckNotAdded)
	ON_BN_CLICKED(IDC_CHECK_SHOW_URL, &CIPTVChannelEditorDlg::OnBnClickedCheckShowUrl)
	ON_BN_CLICKED(IDC_BUTTON_VOD, &CIPTVChannelEditorDlg::OnBnClickedButtonVod)

	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, &CIPTVChannelEditorDlg::OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, &CIPTVChannelEditorDlg::OnToolTipText)
END_MESSAGE_MAP()

CIPTVChannelEditorDlg::CIPTVChannelEditorDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EDEMCHANNELEDITOR_DIALOG, pParent)
	, m_evtStop(FALSE, TRUE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_normal = ::GetSysColor(COLOR_WINDOWTEXT);
	m_gray = ::GetSysColor(COLOR_GRAYTEXT);
	m_colorAdded = RGB(0, 200, 0);
	m_colorNotAdded = RGB(200, 0, 0);
	m_colorHEVC = RGB(158, 255, 250);
	m_colorChanged = RGB(226, 135, 67);
}

void CIPTVChannelEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_COMBO_PLUGIN_TYPE, m_wndPluginType);
	DDX_Check(pDX, IDC_CHECK_ARCHIVE, m_isArchive);
	DDX_Control(pDX, IDC_CHECK_ARCHIVE, m_wndArchive);
	DDX_Text(pDX, IDC_EDIT_ARCHIVE_DAYS, m_archiveDays);
	DDX_Control(pDX, IDC_EDIT_ARCHIVE_DAYS, m_wndArchiveDays);
	DDX_Check(pDX, IDC_CHECK_ADULT, m_isAdult);
	DDX_Control(pDX, IDC_CHECK_ADULT, m_wndAdult);
	DDX_Control(pDX, IDC_TREE_CHANNELS, m_wndChannelsTree);
	DDX_Control(pDX, IDC_CHECK_CUSTOMIZE, m_wndCustom);
	DDX_Control(pDX, IDC_CHECK_SHOW_URL, m_wndShowUrl);
	DDX_Text(pDX, IDC_EDIT_URL_ID, m_streamID);
	DDX_Control(pDX, IDC_EDIT_URL_ID, m_wndStreamID);
	DDX_Text(pDX, IDC_EDIT_EPG1_ID, m_epgID1);
	DDX_Control(pDX, IDC_EDIT_EPG1_ID, m_wndEpgID1);
	DDX_Control(pDX, IDC_BUTTON_TEST_EPG, m_wndViewEPG);
	DDX_Text(pDX, IDC_EDIT_EPG2_ID, m_epgID2);
	DDX_Control(pDX, IDC_EDIT_EPG2_ID, m_wndEpgID2);
	DDX_Control(pDX, IDC_EDIT_STREAM_URL, m_wndStreamUrl);
	DDX_Text(pDX, IDC_EDIT_STREAM_URL, m_streamUrl);
	DDX_Text(pDX, IDC_EDIT_TIME_SHIFT, m_timeShiftHours);
	DDX_Control(pDX, IDC_EDIT_TIME_SHIFT, m_wndTimeShift);
	DDX_Control(pDX, IDC_SPIN_TIME_SHIFT, m_wndSpinTimeShift);
	DDX_Text(pDX, IDC_EDIT_ARCHIVE_CHECK_DAYS, m_archiveCheckDays);
	DDX_Control(pDX, IDC_EDIT_ARCHIVE_CHECK_DAYS, m_wndArchiveCheckDays);
	DDX_Text(pDX, IDC_EDIT_ARCHIVE_CHECK_HOURS, m_archiveCheckHours);
	DDX_Control(pDX, IDC_EDIT_ARCHIVE_CHECK_HOURS, m_wndArchiveCheckHours);
	DDX_Control(pDX, IDC_STATIC_ICON, m_wndChannelIcon);
	DDX_Text(pDX, IDC_EDIT_SEARCH, m_search);
	DDX_Control(pDX, IDC_EDIT_SEARCH, m_wndSearch);
	DDX_Control(pDX, IDC_CHECK_SHOW_UNKNOWN, m_wndShowUnknown);
	DDX_Control(pDX, IDC_TREE_PLAYLIST, m_wndPlaylistTree);
	DDX_Control(pDX, IDC_BUTTON_PL_FILTER, m_wndFilter);
	DDX_Control(pDX, IDC_EDIT_PL_SEARCH, m_wndPlSearch);
	DDX_Control(pDX, IDC_CHECK_SHOW_CHANGED, m_wndShowChanged);
	DDX_Control(pDX, IDC_CHECK_SHOW_CHANGED_CH, m_wndShowChangedCh);
	DDX_Control(pDX, IDC_CHECK_NOT_ADDED, m_wndNotAdded);
	DDX_Text(pDX, IDC_EDIT_PL_SEARCH, m_plSearch);
	DDX_Control(pDX, IDC_EDIT_ICON_NAME, m_wndIconUrl);
	DDX_Text(pDX, IDC_EDIT_ICON_NAME, m_iconUrl);
	DDX_Control(pDX, IDC_STATIC_PL_ICON, m_wndPlIcon);
	DDX_Text(pDX, IDC_EDIT_PL_ICON_NAME, m_plIconName);
	DDX_Control(pDX, IDC_STATIC_PLAYLIST, m_wndPlInfo);
	DDX_Text(pDX, IDC_STATIC_PL_ID, m_plID);
	DDX_Text(pDX, IDC_STATIC_PL_EPG, m_plEPG);
	DDX_Control(pDX, IDC_CHECK_PL_ARCHIVE, m_wndPlArchive);
	DDX_Text(pDX, IDC_EDIT_PL_ARCHIVE_DAYS, m_archivePlDays);
	DDX_Text(pDX, IDC_EDIT_INFO_VIDEO, m_infoVideo);
	DDX_Control(pDX, IDC_EDIT_INFO_VIDEO, m_wndInfoVideo);
	DDX_Text(pDX, IDC_EDIT_INFO_AUDIO, m_infoAudio);
	DDX_Control(pDX, IDC_EDIT_INFO_AUDIO, m_wndInfoAudio);
	DDX_Control(pDX, IDC_STATIC_CHANNELS, m_wndChInfo);
	DDX_Control(pDX, IDC_BUTTON_CHECK_ARCHIVE, m_wndCheckArchive);
	DDX_Control(pDX, IDC_COMBO_STREAM_TYPE, m_wndStreamType);
	DDX_Control(pDX, IDC_COMBO_PLAYLIST, m_wndPlaylist);
	DDX_Control(pDX, IDC_COMBO_CHANNELS, m_wndChannels);
	DDX_Control(pDX, IDC_BUTTON_ACCOUNT_SETTINGS, m_wndAccountSetting);
	DDX_Control(pDX, IDC_BUTTON_DOWNLOAD_PLAYLIST, m_wndDownloadUrl);
	DDX_Control(pDX, IDC_PROGRESS_LOAD, m_wndProgress);
	DDX_Control(pDX, IDC_BUTTON_CACHE_ICON, m_wndCacheIcon);
	DDX_Control(pDX, IDC_BUTTON_SAVE, m_wndSave);
	DDX_Control(pDX, IDC_BUTTON_STOP, m_wndStop);
	DDX_Control(pDX, IDC_STATIC_PROGRESS_INFO, m_wndProgressInfo);
	DDX_Control(pDX, IDC_COMBO_ICON_SOURCE, m_wndIconSource);
	DDX_Control(pDX, IDC_RICHEDIT_EPG, m_wndEpg);
	DDX_Control(pDX, IDC_RADIO_EPG1, m_wndEpg1);
	DDX_Control(pDX, IDC_RADIO_EPG2, m_wndEpg2);
	DDX_Control(pDX, IDC_SPLIT_BUTTON_UPDATE_CHANGED, m_wndUpdateChanged);
	DDX_Control(pDX, IDC_SPLIT_BUTTON_PACK, m_wndPack);
	DDX_Control(pDX, IDC_BUTTON_SETTINGS, m_wndSettings);
	DDX_Control(pDX, IDC_PROGRESS_PROGRAM, m_wndProgressTime);
	DDX_Control(pDX, IDC_BUTTON_VOD, m_wndVod);
	DDX_Control(pDX, IDC_CHECK_MAKE_WEB_UPDATE, m_wndMakeWebUpdate);
}

// CEdemChannelEditorDlg message handlers

BOOL CIPTVChannelEditorDlg::OnInitDialog()
{
	__super::OnInitDialog();

	RestoreWindowPos(GetSafeHwnd(), REG_WINDOW_POS);

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_hAccel = LoadAccelerators(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR_TABLE));

	if (!m_wndToolTipCtrl.Create(this, TTS_ALWAYSTIP))
	{
		TRACE(_T("Unable To create ToolTip\n"));
		return FALSE;
	}

	TRACE0("Run as Application, create tray icon\n");
	if (!m_wndTrayIcon.Create(
		_T("IPTV Channel Editor for Dune HD"),	// Tooltip text
		this,				// Parent window
		IDR_MAINFRAME,		// Icon resource ID
		IDR_POPUP_TRAY,		// Resource ID of pop-up menu
		ID_RESTORE))		// Default menu item for pop-up menu
	{
		TRACE("Failed to create tray icon\n");
		AfxMessageBox(IDS_STRING_ERR_TRAY_ICON, MB_ICONERROR);
	}

	m_wndTrayIcon.HideIcon();

	UpdateWindowTitle();

	CString ver;
	ver.Format(_T("for DUNE HD v%d.%d.%d"), MAJOR, MINOR, BUILD);
	GetDlgItem(IDC_STATIC_APP_TITLE)->SetWindowText(ver);
	LOGFONT lfDlg;
	GetFont()->GetLogFont(&lfDlg);
	CDC* dc = GetDC();
	lfDlg.lfHeight = -MulDiv(160, GetDeviceCaps(dc->m_hDC, LOGPIXELSY), 720);
	lfDlg.lfWeight = FW_BOLD;
	m_largeFont.CreateFontIndirect(&lfDlg);

	GetDlgItem(IDC_STATIC_TITLE)->SetFont(&m_largeFont);

	// Setup tooltips
	m_wndChannelsTree.GetToolTips()->SetDelayTime(TTDT_AUTOPOP, 10000);
	m_wndChannelsTree.GetToolTips()->SetDelayTime(TTDT_INITIAL, 500);
	m_wndChannelsTree.GetToolTips()->SetMaxTipWidth(100);

	m_wndPlaylistTree.GetToolTips()->SetDelayTime(TTDT_AUTOPOP, 10000);
	m_wndPlaylistTree.GetToolTips()->SetDelayTime(TTDT_INITIAL, 500);
	m_wndPlaylistTree.GetToolTips()->SetMaxTipWidth(100);

	m_wndToolTipCtrl.SetDelayTime(TTDT_AUTOPOP, 10000);
	m_wndToolTipCtrl.SetDelayTime(TTDT_INITIAL, 500);
	m_wndToolTipCtrl.SetMaxTipWidth(500);

	m_tooltips_info =
	{
		{ IDC_COMBO_PLUGIN_TYPE, load_string_resource(IDS_STRING_COMBO_PLUGIN_TYPE) },
		{ IDC_BUTTON_VOD, load_string_resource(IDS_STRING_BUTTON_VOD) },
		{ IDC_BUTTON_ABOUT, load_string_resource(IDS_STRING_BUTTON_ABOUT) },
		{ IDC_COMBO_CHANNELS, load_string_resource(IDS_STRING_COMBO_CHANNELS) },
		{ IDC_COMBO_PLAYLIST, load_string_resource(IDS_STRING_COMBO_PLAYLIST) },
		{ IDC_BUTTON_ADD_NEW_CHANNELS_LIST, load_string_resource(IDS_STRING_BUTTON_ADD_NEW_CHANNELS_LIST) },
		{ IDC_EDIT_SEARCH, load_string_resource(IDS_STRING_EDIT_SEARCH) },
		{ IDC_BUTTON_SEARCH_NEXT, load_string_resource(IDS_STRING_BUTTON_SEARCH_NEXT) },
		{ IDC_CHECK_SHOW_UNKNOWN, load_string_resource(IDS_STRING_SHOW_UNKNOWN) },
		{ IDC_EDIT_URL_ID, load_string_resource(IDS_STRING_EDIT_URL_ID) },
		{ IDC_BUTTON_TEST_EPG, load_string_resource(IDS_STRING_BUTTON_TEST_EPG) },
		{ IDC_EDIT_EPG1_ID, load_string_resource(IDS_STRING_EDIT_EPG1_ID) },
		{ IDC_EDIT_EPG2_ID, load_string_resource(IDS_STRING_EDIT_EPG2_ID) },
		{ IDC_CHECK_CUSTOMIZE, load_string_resource(IDS_STRING_CHECK_CUSTOMIZE) },
		{ IDC_CHECK_ARCHIVE, load_string_resource(IDS_STRING_CHECK_ARCHIVE) },
		{ IDC_EDIT_ARCHIVE_DAYS, load_string_resource(IDS_STRING_EDIT_ARCHIVE_DAYS) },
		{ IDC_CHECK_ADULT, load_string_resource(IDS_STRING_CHECK_ADULT) },
		{ IDC_BUTTON_CACHE_ICON, load_string_resource(IDS_STRING_BUTTON_CACHE_ICON) },
		{ IDC_BUTTON_SAVE, load_string_resource(IDS_STRING_BUTTON_SAVE) },
		{ IDC_SPLIT_BUTTON_PACK, load_string_resource(IDS_STRING_BUTTON_PACK) },
		{ IDC_BUTTON_SETTINGS, load_string_resource(IDS_STRING_BUTTON_SETTINGS) },
		{ IDC_BUTTON_ACCOUNT_SETTINGS, load_string_resource(IDS_STRING_ACCOUNT_SETTINGS) },
		{ IDC_BUTTON_DOWNLOAD_PLAYLIST, load_string_resource(IDS_STRING_BUTTON_DOWNLOAD_PLAYLIST) },
		{ IDC_EDIT_PL_SEARCH, load_string_resource(IDS_STRING_EDIT_PL_SEARCH) },
		{ IDC_BUTTON_PL_SEARCH_NEXT, load_string_resource(IDS_STRING_BUTTON_PL_SEARCH_NEXT) },
		{ IDC_BUTTON_PL_FILTER, load_string_resource(IDS_STRING_BUTTON_PL_FILTER) },
		{ IDC_CHECK_SHOW_CHANGED, load_string_resource(IDS_STRING_SHOW_CHANGED) },
		{ IDC_CHECK_NOT_ADDED, load_string_resource(IDS_STRING_NOT_ADDED) },
		{ IDC_STATIC_ICON, load_string_resource(IDS_STRING_STATIC_ICON) },
		{ IDC_EDIT_ARCHIVE_CHECK_DAYS, load_string_resource(IDS_STRING_EDIT_ARCHIVE_CHECK_DAYS) },
		{ IDC_SPIN_ARCHIVE_CHECK_DAYS, load_string_resource(IDS_STRING_EDIT_ARCHIVE_CHECK_DAYS) },
		{ IDC_EDIT_ARCHIVE_CHECK_HOURS, load_string_resource(IDS_STRING_EDIT_ARCHIVE_CHECK_HOURS) },
		{ IDC_SPIN_ARCHIVE_CHECK_HOURS, load_string_resource(IDS_STRING_EDIT_ARCHIVE_CHECK_HOURS) },
		{ IDC_EDIT_TIME_SHIFT, load_string_resource(IDS_STRING_EDIT_TIME_SHIFT) },
		{ IDC_SPIN_TIME_SHIFT, load_string_resource(IDS_STRING_EDIT_TIME_SHIFT) },
		{ IDC_EDIT_INFO_VIDEO, load_string_resource(IDS_STRING_EDIT_INFO_VIDEO) },
		{ IDC_EDIT_INFO_AUDIO, load_string_resource(IDS_STRING_EDIT_INFO_AUDIO) },
		{ IDC_COMBO_STREAM_TYPE, load_string_resource(IDS_STRING_COMBO_STREAM_TYPE) },
		{ IDC_COMBO_ICON_SOURCE, load_string_resource(IDS_STRING_COMBO_ICON_SOURCE) },
		{ IDC_BUTTON_STOP, load_string_resource(IDS_STRING_BUTTON_STOP) },
		{ IDC_BUTTON_CHECK_ARCHIVE, load_string_resource(IDS_STRING_BUTTON_CHECK_ARCHIVE) },
		{ IDC_SPLIT_BUTTON_UPDATE_CHANGED, load_string_resource(IDS_STRING_BUTTON_UPDATE_CHANGED) },
	};

	for (const auto& pair : m_tooltips_info)
	{
		m_wndToolTipCtrl.AddTool(GetDlgItem(pair.first), LPSTR_TEXTCALLBACK);
	}

	m_wndToolTipCtrl.Activate(TRUE);

	m_archiveCheckDays = GetConfig().get_int(true, REG_DAYS_BACK);
	m_archiveCheckHours = GetConfig().get_int(true, REG_HOURS_BACK);

	m_colorAdded = GetConfig().get_int(true, REG_COLOR_ADDED, RGB(0, 200, 0));
	m_colorNotAdded = GetConfig().get_int(true, REG_COLOR_NOT_ADDED, RGB(200, 0, 0));
	m_colorChanged = GetConfig().get_int(true, REG_COLOR_CHANGED, RGB(158, 255, 250));
	m_colorHEVC = GetConfig().get_int(true, REG_COLOR_HEVC, RGB(226, 135, 67));

	UpdateData(FALSE);

	// Fill available plugins
	for (const auto& item : GetConfig().get_plugins_info())
	{
		int idx = m_wndPluginType.AddString(item.title.c_str());
		m_wndPluginType.SetItemData(idx, (DWORD_PTR)item.type);
	}

	m_wndIconSource.AddString(load_string_resource(IDS_STRING_FILE).c_str());
	m_wndIconSource.AddString(load_string_resource(IDS_STRING_URL).c_str());
	m_wndIconSource.AddString(_T("it999.ru"));

	// Toggle controls state
	m_wndSearch.EnableWindow(FALSE);
	m_wndPlSearch.EnableWindow(FALSE);
	m_wndCustom.EnableWindow(FALSE);
	m_wndShowUrl.SetCheck(GetConfig().get_int(true, REG_SHOW_URL));
	m_wndArchive.EnableWindow(FALSE);
	m_wndArchiveDays.EnableWindow(FALSE);
	m_wndAdult.EnableWindow(FALSE);
	m_wndViewEPG.EnableWindow(FALSE);
	m_wndStreamID.EnableWindow(FALSE);
	m_wndStreamUrl.SetReadOnly(TRUE);
	m_wndCheckArchive.EnableWindow(FALSE);
	m_wndCheckArchive.EnableWindow(FALSE);
	m_wndTimeShift.EnableWindow(FALSE);
	m_wndSpinTimeShift.EnableWindow(FALSE);
	m_wndEpg1.SetCheck(TRUE);
	m_wndEpg1.EnableWindow(FALSE);
	m_wndEpg2.EnableWindow(FALSE);
	m_wndEpgID1.EnableWindow(FALSE);
	m_wndEpgID2.EnableWindow(FALSE);
	m_wndPluginType.SetCurSel(GetConfig().get_plugin_idx());
	m_wndIconSource.SetCurSel(GetConfig().get_int(true, REG_ICON_SOURCE));

	SwitchPlugin();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

BOOL CIPTVChannelEditorDlg::OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult)
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
		auto& pair = m_tooltips_info.find(nID);
		if (pair != m_tooltips_info.end())
		{
			pTTT->lpszText = pair->second.data();
			*pResult = 0;
			return TRUE;
		}
	}

	return FALSE;
}

void CIPTVChannelEditorDlg::UpdateWindowTitle()
{
	SetWindowText(load_string_resource(GetConfig().IsPortable() ? IDS_STRING_APP_TITLE_PORTABLE : IDS_STRING_APP_TITLE).c_str());
}

void CIPTVChannelEditorDlg::SwitchPlugin()
{
	// Rebuild available playlist types and set current plugin parameters
	m_inSync = true;

	CollectCredentials();

	m_cur_account.Clear();

	int selected = GetConfig().get_int(false, REG_ACTIVE_ACCOUNT);
	if (selected == -1 || selected >= (int)m_all_credentials.size())
		selected = 0;

	if (selected < (int)m_all_credentials.size())
	{
		m_cur_account = m_all_credentials[selected];
	}

	m_wndPlaylist.ResetContent();

	int pl_idx = GetConfig().get_int(false, REG_PLAYLIST_TYPE);
	m_plugin_type = GetConfig().get_plugin_type();
	m_plugin = StreamContainer::get_instance(m_plugin_type);

	BOOL showWebUpdate = (!m_cur_account.update_url.empty() && !m_cur_account.update_package_url.empty());
	m_wndMakeWebUpdate.EnableWindow(showWebUpdate);
	if (!showWebUpdate)
		m_wndMakeWebUpdate.SetCheck(0);

	if (m_plugin_type == StreamType::enSharaclub)
	{
		if (GetConfig().get_string(false, REG_LIST_DOMAIN).empty() || GetConfig().get_string(false, REG_EPG_DOMAIN).empty())
		{
			const auto& provider_api_url = m_plugin->get_provider_api_url();
			std::vector<BYTE> data;
			if (utils::DownloadFile(provider_api_url, data) && !data.empty())
			{
				JSON_ALL_TRY;
				const auto& parsed_json = nlohmann::json::parse(data.begin(), data.end());
				GetConfig().set_string(false, REG_LIST_DOMAIN, utils::utf8_to_utf16(parsed_json["listdomain"].get<std::string>()));
				GetConfig().set_string(false, REG_EPG_DOMAIN, utils::utf8_to_utf16(parsed_json["jsonEpgDomain"].get<std::string>()));
				JSON_ALL_CATCH;
			}
		}
	}

	const auto& streams = m_plugin->get_supported_streams();

	TemplateParams params;
	m_wndVod.ShowWindow(m_plugin->get_vod_url(params).empty() ? SW_HIDE : SW_SHOW);

	int cur_sel = GetConfig().get_int(false, REG_STREAM_TYPE, 0);
	m_wndStreamType.ResetContent();
	for (const auto& stream : streams)
	{
		std::wstring name;
		switch (stream.stream_type)
		{
			case StreamSubType::enHLS:
				name = L"HLS";
				break;
			case StreamSubType::enMPEGTS:
				name = L"MPEG-TS";
				break;
			default:
				break;
		}

		if (!name.empty())
		{
			int idx = m_wndStreamType.AddString(name.c_str());
			m_wndStreamType.SetItemData(idx, (DWORD_PTR)stream.stream_type);
		}
	}

	if (cur_sel >= m_wndStreamType.GetCount())
		cur_sel = m_wndStreamType.GetCount() - 1;

	if (m_wndStreamType.GetCount() == 0)
		cur_sel = -1;

	m_wndStreamType.SetCurSel(cur_sel);
	m_wndStreamType.EnableWindow(m_wndStreamType.GetCount() > 1);

	// Set selected playlist
	m_playlist_info = m_plugin->get_playlists();
	for (const auto& playlist : m_playlist_info)
	{
		int idx = m_wndPlaylist.AddString(playlist.name.c_str());
		m_wndPlaylist.SetItemData(idx, (DWORD_PTR)&playlist);
	}

	if (pl_idx >= m_wndPlaylist.GetCount() || pl_idx < 0)
		pl_idx = 0;

	const auto& pl_info = ((PlaylistInfo*)m_wndPlaylist.GetItemData(pl_idx));
	m_wndPlaylist.SetCurSel(pl_idx);
	m_wndPlaylist.EnableWindow(TRUE);

	// Load channel lists
	const auto& plugin_name = GetConfig().GetCurrentPluginName();
	const auto& channelsPath = fmt::format(L"{:s}{:s}\\", GetConfig().get_string(true, REG_LISTS_PATH), plugin_name);
	const auto& default_tv_name = fmt::format(L"{:s}_channel_list.xml", plugin_name);
	const auto& default_vod_name = fmt::format(L"{:s}_mediateka_list.xml", plugin_name);

	m_all_channels_lists.clear();
	m_unknownChannels.clear();
	m_changedChannels.clear();
	m_wndChannels.ResetContent();

	std::error_code err;
	std::filesystem::directory_iterator dir_iter(channelsPath, err);
	for (auto const& dir_entry : dir_iter)
	{
		const auto& path = dir_entry.path();
		if (path.extension() != _T(".xml")) continue;

		if (path.filename() == default_tv_name)
			m_all_channels_lists.emplace(m_all_channels_lists.begin(), path.filename());
		else if (path.filename() == default_vod_name)
			m_all_channels_lists.emplace(m_all_channels_lists.end(), path.filename());
		else
			m_all_channels_lists.emplace_back(path.filename());
	}

	if (m_all_channels_lists.empty())
	{
		CString str;
		str.Format(IDS_STRING_NO_CHANNELS, channelsPath.c_str());
		AfxMessageBox(str, MB_ICONERROR | MB_OK);
	}

	for (const auto& item : m_all_channels_lists)
	{
		if (item == default_tv_name)
		{
			const auto& name = item + load_string_resource(IDS_STRING_STANDARD);
			m_wndChannels.AddString(name.c_str());
		}
		else
		{
			m_wndChannels.AddString(item.c_str());
		}
	}

	int idx = GetConfig().get_int(false, REG_CHANNELS_TYPE);
	if (idx < m_wndChannels.GetCount())
		m_wndChannels.SetCurSel(idx);
	else
		m_wndChannels.SetCurSel(0);

	// load stream info
	m_stream_infos.clear();
	std::ifstream is(channelsPath + _T("stream_info.bin"), std::istream::binary);
	if (is.good())
	{
		std::vector<char> dump((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
		m_stream_infos.deserialize(dump);
	}

	m_blockChecking = true;
	m_wndChannels.EnableWindow(m_all_channels_lists.size() > 1);

	// Reload selected channels list
	OnCbnSelchangeComboChannels();

	// Reload selected playlist
	OnCbnSelchangeComboPlaylist();
	m_lastTree = &m_wndChannelsTree;
	m_blockChecking = false;

	m_update_epg_timer = ID_UPDATE_EPG_TIMER;
	OnTimer(ID_UPDATE_EPG_TIMER);
}

void CIPTVChannelEditorDlg::CollectCredentials()
{
	m_all_credentials.clear();

	nlohmann::json creds;
	JSON_ALL_TRY;
	{
		creds = nlohmann::json::parse(GetConfig().get_string(false, REG_ACCOUNT_DATA));
	}
	JSON_ALL_CATCH;
	for (const auto& item : creds.items())
	{
		const auto& val = item.value();
		if (val.empty()) continue;

		Credentials cred;
		JSON_ALL_TRY;
		{
			cred = val.get<Credentials>();
		}
		JSON_ALL_CATCH;
		m_all_credentials.emplace_back(cred);
	}

	CMenu* pMenu = new CMenu;
	pMenu->CreatePopupMenu();
	pMenu->AppendMenu(MF_STRING, ID_MAKE_ALL, load_string_resource(ID_MAKE_ALL).c_str());
	pMenu->AppendMenu(MF_STRING, ID_MAKE_ALL_ACCOUNTS, load_string_resource(ID_MAKE_ALL_ACCOUNTS).c_str());

	if (m_all_credentials.size() > 1)
	{
		int i = 0;
		pMenu->AppendMenu(MF_STRING | MF_ENABLED, ID_SEPARATOR);
		for (const auto& cred : m_all_credentials)
		{
			std::string title;
			if (!cred.comment.empty())
				title = cred.comment;

			if (title.empty() && !cred.login.empty())
				title = cred.login;

			if (title.empty() && !cred.password.empty())
				title = cred.password;

			if (title.empty() && !cred.token.empty())
				title = cred.token;

			pMenu->AppendMenu(MF_STRING | MF_ENABLED, ID_ACCOUNT_TO_START + i++, utils::utf8_to_utf16(title).c_str());
		}
	}

	m_wndPack.SetDropDownMenu(pMenu);
}

void CIPTVChannelEditorDlg::LoadPlaylist(bool saveToFile /*= false*/)
{
	m_plFileName.Empty();
	std::wstring url;
	int idx = m_wndPlaylist.GetCurSel();
	const auto& pl_info = ((PlaylistInfo*)m_wndPlaylist.GetItemData(idx));

	bool isFile = pl_info->is_file;

	m_plFileName = fmt::format(_T("{:s}_Playlist.m3u8"), GetConfig().GetCurrentPluginName(true)).c_str();

	TemplateParams params;
	params.login = m_cur_account.get_login();
	params.password = m_cur_account.get_password();
	params.token = m_cur_account.get_token();
	params.server = m_cur_account.device_id;
	params.profile = m_cur_account.profile_id;
	params.quality = m_cur_account.quality_id;
	params.number = idx;

	if (m_plugin_type == StreamType::enSharaclub)
	{
		params.subdomain = GetConfig().get_string(false, REG_LIST_DOMAIN);
	}
	if (m_plugin_type == StreamType::enTVClub || m_plugin_type == StreamType::enVidok)
	{
		params.token = m_plugin->get_api_token(m_cur_account);
	}
	m_plugin->get_playlist_url(url, params);

	if (url.empty())
	{
		AfxMessageBox(IDS_STRING_ERR_SOURCE_NOT_SET, MB_OK | MB_ICONERROR);
		OnEndLoadPlaylist(0);
		return;
	}

	if (m_plFileName.IsEmpty())
	{
		CString slashed(url.c_str());
		slashed.Replace('\\', '/');
		auto pos = slashed.ReverseFind('/');
		if (pos != -1)
		{
			m_plFileName = slashed.Mid(++pos);
		}
		else
		{
			m_plFileName = slashed;
		}
	}

	auto data = std::make_unique<std::vector<BYTE>>();
	std::wstring host;
	std::wstring path;
	WORD port = 0;
	if (isFile)
	{
		std::ifstream stream(url);
		data->assign(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
	}
	else if (utils::CrackUrl(url, host, path, port))
	{
		if (!utils::DownloadFile(url, *data))
		{
			AfxMessageBox(IDS_STRING_ERR_CANT_DOWNLOAD_PLAYLIST, MB_OK | MB_ICONERROR);
			OnEndLoadPlaylist(0);
			return;
		}

		if (saveToFile)
		{
			std::ofstream os(m_plFileName);
			os.write((char*)data->data(), data->size());
			os.close();
			return;
		}
	}

	if (data->empty())
	{
		AfxMessageBox(IDS_STRING_ERR_EMPTY_PLAYLIST, MB_OK | MB_ICONERROR);
		OnEndLoadPlaylist(0);
		return;
	}

	m_wndStop.EnableWindow(TRUE);
	m_wndProgressInfo.ShowWindow(SW_SHOW);

	auto pThread = (CPlaylistParseM3U8Thread*)AfxBeginThread(RUNTIME_CLASS(CPlaylistParseM3U8Thread), THREAD_PRIORITY_HIGHEST, 0, CREATE_SUSPENDED);
	if (!pThread)
	{
		AfxMessageBox(IDS_STRING_ERR_THREAD_NOT_START, MB_OK | MB_ICONERROR);
		OnEndLoadPlaylist(0);
		return;
	}

	m_loading = true;

	m_wndPluginType.EnableWindow(FALSE);
	m_wndCheckArchive.EnableWindow(FALSE);
	m_wndPlaylist.EnableWindow(FALSE);
	m_wndPlaylistTree.EnableWindow(FALSE);
	m_wndChannels.EnableWindow(FALSE);
	m_wndFilter.EnableWindow(FALSE);
	m_wndDownloadUrl.EnableWindow(FALSE);
	m_wndAccountSetting.EnableWindow(FALSE);
	m_evtStop.ResetEvent();

	std::unique_ptr<Playlist> playlistEntriesOld = std::move(m_playlistEntries);

	FillTreePlaylist();

	ThreadConfig cfg;
	cfg.m_parent = this;
	cfg.m_data = data.release();
	cfg.m_hStop = m_evtStop;
	cfg.m_pluginType = m_plugin_type;
	cfg.m_rootPath = GetAppPath(utils::PLUGIN_ROOT);

	pThread->SetData(cfg);
	pThread->ResumeThread();
}

void CIPTVChannelEditorDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent != m_update_epg_timer )
	{
		__super::OnTimer(nIDEvent);
		return;
	}

	KillTimer(m_update_epg_timer);
	m_update_epg_timer = 0;
	time_t shifted = time(nullptr) - (time_t)m_archiveCheckDays * 24 * 3600 - (time_t)m_archiveCheckHours * 3600;
	COleDateTime now(shifted);
	GetDlgItem(IDC_STATIC_CUR_TIME)->SetWindowText(now.Format(_T("%H:%M:%S")));
	if (now.GetSecond() == 0)
	{
		UpdateEPG(&m_wndChannelsTree);
	}
	m_update_epg_timer = SetTimer(ID_UPDATE_EPG_TIMER, 1000, nullptr);
}

LRESULT CIPTVChannelEditorDlg::OnEndLoadPlaylist(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	m_evtStop.ResetEvent();
	m_playlistEntries.reset((Playlist*)wParam);

	m_inSync = false;
	m_wndPluginType.EnableWindow(TRUE);
	m_wndProgress.ShowWindow(SW_HIDE);
	m_wndProgressInfo.ShowWindow(SW_HIDE);
	m_wndFilter.EnableWindow(TRUE);
	m_wndPlSearch.EnableWindow(!m_channelsMap.empty());
	m_wndPlaylistTree.EnableWindow(TRUE);
	m_wndChannels.EnableWindow(m_all_channels_lists.size() > 1);
	m_wndStop.EnableWindow(FALSE);

	m_playlistMap.clear();
	if (m_playlistEntries)
	{
		bool bSet = false;
		for (const auto& entry : m_playlistEntries->m_entries)
		{
			const auto& parser = entry->get_uri_stream()->get_parser();
			switch (m_plugin_type)
			{
				case StreamType::enEdem: // domain/token
				{
					const auto& token = parser.token;
					const auto& domain = parser.domain;
					if (!token.empty() && token != L"00000000000000" && !domain.empty() && domain != L"localhost")
					{
						m_cur_account.set_token(parser.token);
						m_cur_account.set_domain(parser.domain);
						bSet = true;
					}
					break;
				}
				case StreamType::enGlanz: // login/token
					if (!parser.token.empty()
						&& !parser.login.empty()
						&& !parser.password.empty()
						&& !parser.int_id.empty()
						)
					{
						m_cur_account.set_token(parser.token);
						m_cur_account.set_login(parser.login);
						m_cur_account.set_password(parser.password);
						bSet = true;
					}
					break;
				case StreamType::enKineskop: // login/token
					bSet = true;
					break;
				default:
				{
					m_cur_account.set_token(parser.token);
					bSet = true;
					break;
				}
			}

			if (bSet) break;
		}

		for (auto& entry : m_playlistEntries->m_entries)
		{
			auto res = m_playlistMap.emplace(entry->stream_uri->get_parser().id, entry);
			if (!res.second)
			{
				TRACE(L"Duplicate channel: %s (%s)\n",
					  res.first->second->get_title().c_str(),
					  res.first->second->stream_uri->get_parser().id.c_str());
				continue;
			}
		}
	}

	UpdateChannelsTreeColors();
	FillTreePlaylist();
	UpdateControlsForItem();

	m_loading = false;
	m_wndAccountSetting.EnableWindow(TRUE);
	m_wndDownloadUrl.EnableWindow(TRUE);
	m_wndPlaylist.EnableWindow(TRUE);
	m_wndCheckArchive.EnableWindow(TRUE);

	AfxGetApp()->EndWaitCursor();

	return 0;
}

LRESULT CIPTVChannelEditorDlg::OnInitProgress(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	m_wndProgress.SetRange32(0, (int)wParam);
	m_wndProgress.SetPos((int)lParam);
	m_wndProgress.ShowWindow(SW_SHOW);

	return 0;
}

LRESULT CIPTVChannelEditorDlg::OnUpdateProgress(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	CString str;
	str.Format(IDS_STRING_FMT_CHANNELS_READED, wParam);
	m_wndProgressInfo.SetWindowText(str);
	m_wndProgress.SetPos((int)lParam);

	return 0;
}

LRESULT CIPTVChannelEditorDlg::OnUpdateProgressStream(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	if (wParam)
	{
		ULARGE_INTEGER* ul = (ULARGE_INTEGER*)wParam;

		m_wndProgress.SetPos(ul->LowPart);

		CString str;
		str.Format(IDS_STRING_FMT_STREAM_INFO, ul->LowPart, ul->HighPart);
		m_wndProgressInfo.SetWindowText(str);
	}

	if (lParam)
	{
		auto tuple = (std::tuple<int, std::string, std::string>*)lParam;
		int hash;
		std::string audio;
		std::string video;
		std::tie(hash, audio, video) = *tuple;
		m_stream_infos[hash] = {audio, video};
	}

	return 0;
}

LRESULT CIPTVChannelEditorDlg::OnEndGetStreamInfo(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	SaveStreamInfo();

	m_inStreamInfo = false;
	m_wndStop.EnableWindow(FALSE);
	m_wndProgress.ShowWindow(SW_HIDE);
	m_wndProgressInfo.ShowWindow(SW_HIDE);

	m_wndPluginType.EnableWindow(TRUE);
	m_wndChannels.EnableWindow(m_all_channels_lists.size() > 1);
	m_wndPlaylist.EnableWindow(TRUE);
	m_wndAccountSetting.EnableWindow(TRUE);
	m_wndDownloadUrl.EnableWindow(TRUE);
	m_wndSettings.EnableWindow(TRUE);

	UpdateControlsForItem();
	LoadPlayListInfo();

	return 0;
}

void CIPTVChannelEditorDlg::OnKickIdle()
{
	UpdateDialogControls(this, FALSE);
}

void CIPTVChannelEditorDlg::OnOK()
{
	// Prevent exit on Enter

	UpdateData(TRUE);

	HWND hFocus = ::GetFocus();
	if (hFocus == m_wndSearch.GetSafeHwnd() && !m_search.IsEmpty())
	{
		OnBnClickedButtonSearchNext();
	}
	else if (hFocus == m_wndPlSearch.GetSafeHwnd() && !m_plSearch.IsEmpty())
	{
		OnBnClickedButtonPlSearchNext();
	}
}

void CIPTVChannelEditorDlg::OnCancel()
{
	if (!CheckForSave())
	{
		return;
	}

	m_evtStop.SetEvent();

	GetConfig().UpdatePluginSettings();
	GetConfig().SaveSettings();

	EndDialog(IDCANCEL);
}

BOOL CIPTVChannelEditorDlg::PreTranslateMessage(MSG* pMsg)
{
	if (m_hAccel && ::TranslateAccelerator(m_hWnd, m_hAccel, pMsg))
		return(TRUE);

	if (pMsg->message == WM_LBUTTONDOWN
		|| pMsg->message == WM_LBUTTONUP
		|| pMsg->message == WM_MOUSEMOVE)
	{
		m_wndToolTipCtrl.RelayEvent(pMsg);
	}

	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
	{
		CEdit* edit = m_wndChannelsTree.GetEditControl();
		if (edit)
		{
			edit->SendMessage(WM_KEYDOWN, pMsg->wParam, pMsg->lParam);
			return TRUE;
		}
	}

	return __super::PreTranslateMessage(pMsg);
}

void CIPTVChannelEditorDlg::OnDestroy()
{
	VERIFY(DestroyAcceleratorTable(m_hAccel));

	__super::OnDestroy();
}

void CIPTVChannelEditorDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	CMFCDynamicLayout* layout = GetDynamicLayout();

	if (layout)
	{
		CSize size = layout->GetMinSize();
		CRect rect(0, 0, size.cx, size.cy);
		AdjustWindowRect(&rect, GetStyle(), FALSE);
		lpMMI->ptMinTrackSize.x = rect.Width();
		lpMMI->ptMinTrackSize.y = rect.Height();
	}

	__super::OnGetMinMaxInfo(lpMMI);
}

void CIPTVChannelEditorDlg::set_allow_save(BOOL val)
{
	m_allow_save = val;
	if (m_wndSave.GetSafeHwnd())
		m_wndSave.EnableWindow(m_allow_save);
}

void CIPTVChannelEditorDlg::FillTreeChannels(LPCWSTR select /*= nullptr*/)
{
	m_bInFillTree = true;

	BOOL bCheckUnknown = m_wndShowUnknown.GetCheck();
	if (bCheckUnknown && m_playlistMap.empty())
	{
		bCheckUnknown = FALSE;
		m_wndShowUnknown.SetCheck(FALSE);
	}

	BOOL bCheckChanged = m_wndShowChangedCh.GetCheck();
	if (bCheckChanged && m_playlistMap.empty())
	{
		bCheckChanged = FALSE;
		m_wndShowChangedCh.SetCheck(FALSE);
	}

	int flags = GetConfig().get_int(true, REG_CMP_FLAGS, CMP_FLAG_ALL);
	BOOL bCmpTitle = (flags & CMP_FLAG_TITLE) ? TRUE : FALSE;
	BOOL bCmpIcon = (flags & CMP_FLAG_ICON) ? TRUE : FALSE;
	BOOL bCmpArchive = (flags & CMP_FLAG_ARCHIVE) ? TRUE : FALSE;
	BOOL bCmpEpg1 = (flags & CMP_FLAG_EPG1) ? TRUE : FALSE;
	BOOL bCmpEpg2 = ((flags & CMP_FLAG_EPG2) && m_plugin->get_epg_parameters(1).epg_url.empty()) ? FALSE : TRUE;

	m_wndChannelsTree.LockWindowUpdate();
	m_wndChannelsTree.DeleteAllItems();

	m_channelsTreeMap.clear();
	m_categoriesTreeMap.clear();

	for (auto& pair : m_categoriesMap)
	{
		TVINSERTSTRUCTW tvCategory = { nullptr };
		tvCategory.hParent = TVI_ROOT;
		tvCategory.item.pszText = (LPWSTR)pair.second.category->get_title().c_str();
		tvCategory.item.mask = TVIF_TEXT | TVIF_PARAM;
		tvCategory.item.lParam = (DWORD_PTR)InfoType::enCategory;
		tvCategory.hInsertAfter = (pair.first == ID_ADD_TO_FAVORITE) ? TVI_FIRST : nullptr;
		auto hParent = m_wndChannelsTree.InsertItem(&tvCategory);
		m_wndChannelsTree.SetItemColor(hParent, pair.second.category->is_disabled() ? m_gray : m_normal);

		m_categoriesTreeMap.emplace(hParent, pair.first);
		pair.second.hItem = hParent;

		int cnt = 0;
		for (const auto& channel : pair.second.category->get_channels())
		{
			bool bUnknown = false;
			bool bChanged = false;

			const auto& id = channel->stream_uri->get_parser().id;
			if (const auto& found = m_playlistMap.find(id); found != m_playlistMap.end())
			{
				const auto& entry = found->second;
				bChanged = (bCmpTitle && channel->get_title() != entry->get_title()
							|| bCmpArchive && entry->get_archive_days() != 0 && channel->get_archive_days() != entry->get_archive_days()
							|| bCmpEpg1 && !entry->get_epg_id(0).empty() && channel->get_epg_id(0) != entry->get_epg_id(0)
							|| bCmpEpg2 && !entry->get_epg_id(1).empty() && channel->get_epg_id(1) != entry->get_epg_id(1)
							|| (bCmpIcon && !entry->get_icon_uri().get_uri().empty() && !channel->get_icon_uri().is_equal(entry->get_icon_uri(), false))
							);
			}
			else
			{
				bUnknown = true;
			}

			if (!bCheckChanged && !bCheckUnknown || bCheckChanged && bChanged || bCheckUnknown && bUnknown)
			{
				TVINSERTSTRUCTW tvChannel = { nullptr };
				tvChannel.hParent = hParent;
				tvChannel.item.pszText = (LPWSTR)channel->get_title().c_str();
				tvChannel.item.lParam = (DWORD_PTR)InfoType::enChannel;
				tvChannel.item.mask = TVIF_TEXT | TVIF_PARAM;

				HTREEITEM hItem = m_wndChannelsTree.InsertItem(&tvChannel);
				m_channelsTreeMap.emplace(hItem, channel);
				cnt++;
			}
		}

		if (!cnt && pair.first != ID_ADD_TO_FAVORITE)
		{
			m_wndChannelsTree.DeleteItem(hParent);
			m_categoriesTreeMap.erase(hParent);
		}
	}

	UpdateChannelsTreeColors();

	m_wndChannelsTree.UnlockWindowUpdate();

	m_bInFillTree = false;

	if (!m_channelsMap.empty())
	{
		SearchParams params;
		params.id = select ? select : m_categoriesMap.begin()->second.category->get_channels().front()->stream_uri->get_parser().id;
		SelectTreeItem(&m_wndChannelsTree, params);
	}
}

void CIPTVChannelEditorDlg::RemoveOrphanChannels()
{
	std::set<std::wstring> ids;
	for (const auto& pair : m_categoriesMap)
	{
		if (pair.first == ID_ADD_TO_FAVORITE) continue;

		for (const auto& ch : pair.second.category->get_channels())
		{
			ids.emplace(ch->stream_uri->get_parser().id);
		}
	}

	for (auto it = m_channelsMap.begin(); it != m_channelsMap.end(); )
	{
		if (ids.find(it->first) == ids.end())
		{
			m_channelsMap.erase(it++);
		}
		else
		{
			++it;
		}
	}
}

void CIPTVChannelEditorDlg::UpdateChannelsTreeColors(HTREEITEM root /*= nullptr*/)
{
	if (!m_blockChecking)
	{
		if (root == nullptr)
		{
			root = m_wndChannelsTree.GetRootItem();
			m_unknownChannels.clear();
			m_changedChannels.clear();
		}

		int flags = GetConfig().get_int(true, REG_CMP_FLAGS, CMP_FLAG_ALL);
		BOOL bCmpTitle = (flags & CMP_FLAG_TITLE) ? TRUE : FALSE;
		BOOL bCmpIcon = (flags & CMP_FLAG_ICON) ? TRUE : FALSE;
		BOOL bCmpArchive = (flags & CMP_FLAG_ARCHIVE) ? TRUE : FALSE;
		BOOL bCmpEpg1 = (flags & CMP_FLAG_EPG1) ? TRUE : FALSE;
		BOOL bCmpEpg2 = ((flags & CMP_FLAG_EPG2) && m_plugin->get_epg_parameters(1).epg_url.empty()) ? FALSE : TRUE;

		while (root != nullptr && !m_playlistMap.empty())
		{
			// iterate subitems
			for (HTREEITEM hItem = m_wndChannelsTree.GetChildItem(root); hItem != nullptr; hItem = m_wndChannelsTree.GetNextSiblingItem(hItem))
			{
				const auto& channel = FindChannel(hItem);
				if (!channel) continue;

				COLORREF color = m_normal;
				auto& ch_parser = channel->stream_uri->get_parser();
				const auto& id = ch_parser.id;
				if (const auto& found = m_playlistMap.find(id); found != m_playlistMap.end())
				{
					const auto& entry = found->second;
					const auto& entry_parser = entry->stream_uri->get_parser();
					if (ch_parser.domain.empty()
						&& !entry_parser.domain.empty()
						&& ch_parser.domain != entry_parser.domain)
					{
						ch_parser.domain = entry_parser.domain;
					}

					if (ch_parser.port.empty()
						&& !entry_parser.port.empty()
						&& ch_parser.port != entry_parser.port)
					{
						ch_parser.port = entry_parser.port;
					}

					if (ch_parser.host.empty()
						&& !entry_parser.host.empty()
						&& ch_parser.host != entry_parser.host)
					{
						ch_parser.host = entry_parser.host;
					}

					int changed_flag = 0;
					changed_flag |= (bCmpTitle && channel->get_title() != entry->get_title() ? MOD_TITLE : 0);
					changed_flag |= (bCmpArchive && entry->get_archive_days() != 0 && channel->get_archive_days() != entry->get_archive_days() ? MOD_ARCHIVE : 0);
					changed_flag |= (bCmpEpg1 && !entry->get_epg_id(0).empty() && channel->get_epg_id(0) != entry->get_epg_id(0) ? MOD_EPG1 : 0);
					changed_flag |= (bCmpEpg2 && !entry->get_epg_id(1).empty() && channel->get_epg_id(1) != entry->get_epg_id(1) ? MOD_EPG2 : 0);
					changed_flag |= (bCmpIcon && !entry->get_icon_uri().get_uri().empty() && !channel->get_icon_uri().is_equal(entry->get_icon_uri(), false) ? MOD_ICON : 0);

					if (changed_flag)
					{
						color = m_colorChanged;
						m_changedChannels.emplace(id, changed_flag);
					}
					else
					{
						color = m_colorAdded;
						m_changedChannels.erase(id);
					}
				}
				else
				{
					m_unknownChannels.emplace(id);
				}

				if (channel->is_disabled())
				{
					color = m_gray;
				}

				if (const auto& pair = m_stream_infos.find(channel->stream_uri->get_hash()); pair != m_stream_infos.end())
				{
					if (pair->second.second.find("HEVC") != std::string::npos)
					{
						m_wndChannelsTree.SetItemBackColor(hItem, m_colorHEVC);
					}
				}

				m_wndChannelsTree.SetItemColor(hItem, color);
			}

			root = m_wndChannelsTree.GetNextSiblingItem(root);
		}
	}

	m_wndChInfo.SetWindowText(fmt::format(load_string_resource(IDS_STRING_FMT_CHANNELS),
										  m_channelsMap.size(),
										  m_changedChannels.size(),
										  m_unknownChannels.size()).c_str());

	m_wndUpdateChanged.EnableWindow(!m_changedChannels.empty() || !m_unknownChannels.empty());
}

void CIPTVChannelEditorDlg::CheckForExistingPlaylist()
{
	HTREEITEM root = m_wndPlaylistTree.GetRootItem();
	while (root != nullptr)
	{
		// iterate subitems
		for (HTREEITEM hItem = m_wndPlaylistTree.GetChildItem(root); hItem != nullptr; hItem = m_wndPlaylistTree.GetNextSiblingItem(hItem))
		{
			const auto& entry = FindEntry(hItem);
			if (!entry) continue;

			COLORREF color = m_colorNotAdded;
			if (const auto& pair = m_channelsMap.find(entry->stream_uri->get_parser().id); pair != m_channelsMap.end())
			{
				color = m_normal;
				const auto& channel = pair->second;
				if (channel->get_title() != entry->get_title()
					|| (entry->get_archive_days() != 0 && channel->get_archive_days() != entry->get_archive_days())
					|| (!entry->get_epg_id(0).empty() && channel->get_epg_id(0) != entry->get_epg_id(0))
					|| (!entry->get_icon_uri().get_uri().empty() && !channel->get_icon_uri().is_equal(entry->get_icon_uri(), false))
					)
				{
					color = m_colorChanged;
				}
			}

			if (const auto& pair = m_stream_infos.find(entry->stream_uri->get_hash()); pair != m_stream_infos.end())
			{
				if (pair->second.second.find("HEVC") != std::string::npos)
				{
					m_wndPlaylistTree.SetItemBackColor(hItem, m_colorHEVC);
				}
			}

			m_wndPlaylistTree.SetItemColor(hItem, color);
		}

		root = m_wndPlaylistTree.GetNextSiblingItem(root);
	}
}

void CIPTVChannelEditorDlg::LoadChannelInfo(HTREEITEM hItem /*= nullptr*/)
{
	if (hItem == nullptr)
		hItem = m_wndChannelsTree.GetSelectedItem();

	m_infoAudio.Empty();
	m_infoVideo.Empty();
	m_epgID1.Empty();
	m_epgID2.Empty();
	m_timeShiftHours = 0;
	m_iconUrl.Empty();
	m_streamUrl.Empty();
	m_streamID.Empty();

	const auto& channel = FindChannel(hItem);
	if (channel)
	{
		m_epgID1 = channel->get_epg_id(0).c_str();
		m_epgID2 = m_plugin->get_epg_parameters(1).epg_url.empty() ? L"" : channel->get_epg_id(1).c_str();

		const auto& uri = channel->stream_uri;

		m_streamUrl = uri->get_uri().c_str();
		if (uri->is_template() && m_wndShowUrl.GetCheck())
		{
			TemplateParams params;
			params.subdomain = m_cur_account.get_domain();
			params.token = m_cur_account.get_token();
			params.login = m_cur_account.get_login();
			params.password = m_cur_account.get_password();
			params.streamSubtype = (StreamSubType)m_wndStreamType.GetItemData(m_wndStreamType.GetCurSel());
			if (uri->get_server_subst_type() == ServerSubstType::enStream)
			{
				params.server = m_cur_account.device_id;
			}

			UpdateExtToken(uri.get());
			m_streamUrl = uri->get_templated_stream(params).c_str();
		}

		m_streamID = uri->is_template() ? uri->get_parser().id.c_str() : L"";
		auto hash = uri->get_hash();
		if (auto pair = m_stream_infos.find(hash); pair != m_stream_infos.end())
		{
			m_infoAudio = pair->second.first.c_str();
			m_infoVideo = pair->second.second.c_str();
		}

		m_wndCustom.SetCheck(!uri->is_template());
		m_timeShiftHours = channel->get_time_shift_hours();
		m_isArchive = !!channel->is_archive();
		m_wndArchiveDays.EnableWindow(m_isArchive);
		m_archiveDays = channel->get_archive_days();
		m_isAdult = channel->get_adult();

		if (channel->get_icon_uri().get_uri().empty())
		{
			m_iconUrl.Empty();
			m_wndChannelIcon.SetBitmap(nullptr);
			GetDlgItem(IDC_STATIC_ICON_SIZE)->SetWindowText(_T(""));
		}
		else
		{
			m_iconUrl = channel->get_icon_uri().get_uri().c_str();
			const auto& img = GetIconCache().get_icon(channel->get_icon_absolute_path());
			CString str;
			if (img != nullptr)
			{
				str.Format(_T("%d x %d px"), img.GetWidth(), img.GetHeight());
			}
			GetDlgItem(IDC_STATIC_ICON_SIZE)->SetWindowText(str);
			SetImageControl(img, m_wndChannelIcon);
		}

		UpdateData(FALSE);
		UpdateEPG(&m_wndChannelsTree);
	}
	else
	{
		m_wndChannelIcon.SetBitmap(nullptr);
		m_wndCustom.SetCheck(FALSE);
		UpdateData(FALSE);
	}
}

void CIPTVChannelEditorDlg::LoadPlayListInfo(HTREEITEM hItem /*= nullptr*/)
{
	if (hItem == nullptr)
		hItem = m_wndPlaylistTree.GetSelectedItem();

	if (!hItem)
		return;

	m_infoAudio.Empty();
	m_infoVideo.Empty();
	m_plIconName.Empty();
	m_plID.Empty();
	m_plEPG.Empty();
	m_archivePlDays = 0;
	m_wndPlArchive.SetCheck(0);

	const auto& entry = FindEntry(hItem);
	if (entry)
	{
		m_plIconName = entry->get_icon_uri().get_uri().c_str();
		m_plID = entry->stream_uri->get_parser().id.c_str();

		if (!entry->get_epg_id(0).empty())
			m_plEPG = entry->get_epg_id(0).c_str();

		m_wndPlArchive.SetCheck(!!entry->is_archive());
		m_archivePlDays = entry->get_archive_days();
		auto hash = entry->stream_uri->get_hash();
		if (auto pair = m_stream_infos.find(hash); pair != m_stream_infos.end())
		{
			m_infoAudio = pair->second.first.c_str();
			m_infoVideo = pair->second.second.c_str();
		}

		const auto& img = GetIconCache().get_icon(entry->get_icon_absolute_path());
		SetImageControl(img, m_wndPlIcon);

		UpdateEPG(&m_wndPlaylistTree);
	}
	else
	{
		m_wndPlIcon.SetBitmap(nullptr);
	}

	UpdateData(FALSE);
}

void CIPTVChannelEditorDlg::UpdateEPG(const CTreeCtrlEx* pTreeCtl)
{
	m_wndEpg.SetWindowText(L"");
	if (!pTreeCtl)
		return;

	const auto info = GetBaseInfo(pTreeCtl, pTreeCtl->GetSelectedItem());
	if (!info)
		return;

	int epg_idx = GetCheckedRadioButton(IDC_RADIO_EPG1, IDC_RADIO_EPG2) - IDC_RADIO_EPG1;
	int time_shift = m_timeShiftHours * 3600;

	time_t now = time(nullptr) - (time_t)m_archiveCheckDays * 24 * 3600 - (time_t)m_archiveCheckHours * 3600;
	if (pTreeCtl == &m_wndChannelsTree)
	{
		now += time_shift;
	}

#ifdef _DEBUG
	COleDateTime tnow(now);
	std::wstring snow = fmt::format(L"{:04d}-{:02d}-{:02d} {:02d}:{:02d}", tnow.GetYear(), tnow.GetMonth(), tnow.GetDay(), tnow.GetHour(), tnow.GetMinute());
	ATLTRACE(L"\n%s\n", snow.c_str());
#endif // _DEBUG

	auto& epg_id = info->get_epg_id(epg_idx);

	auto& allEpgMap = m_epg_cache[epg_idx];
	auto& epgChannelMap = allEpgMap[epg_id];

	UpdateExtToken(info->stream_uri.get());
	if (m_plugin_type == StreamType::enSharaclub)
	{
		auto& url = info->stream_uri->get_epg_parameters(epg_idx).epg_url;
		utils::string_replace_inplace<wchar_t>(url, L"{DOMAIN}", GetConfig().get_string(false, REG_EPG_DOMAIN));
	}

	// check end time
	EpgInfo epg_info{};
	bool need_load = true;
	while(need_load)
	{
		for (auto& epg_pair : epgChannelMap)
		{
			if (epg_pair.second.time_start <= now && now < epg_pair.second.time_end)
			{
				epg_info = epg_pair.second;
				need_load = false;
				break;
			}
		}

		if (need_load && !info->stream_uri->parse_epg(epg_idx, epg_id, epgChannelMap, now))
		{
			need_load = false;
		}
	}

	if (epg_info.time_start != 0)
	{
		COleDateTime time_n(now);
		COleDateTime time_s(epg_info.time_start - time_shift);
		COleDateTime time_e(epg_info.time_end - time_shift);
		CStringA text;
		text.Format(R"({\rtf1 %ls - %ls\par\b %s\b0\par %s})",
					time_s.Format(_T("%d.%m.%Y %H:%M:%S")).GetString(),
					time_e.Format(_T("%d.%m.%Y %H:%M:%S")).GetString(),
					epg_info.name.c_str(),
					epg_info.desc.c_str()
		);

		COleDateTimeSpan time_left = time_e - time_n;
		m_wndProgressTime.SetRange32(0, int(epg_info.time_end - epg_info.time_start));
		m_wndProgressTime.SetPos(int(now - epg_info.time_start));
		GetDlgItem(IDC_STATIC_TIME_LEFT)->SetWindowText(time_left.Format(_T("%H:%M")));

		SETTEXTEX set_text_ex = { ST_SELECTION, CP_UTF8 };
		m_wndEpg.SendMessage(EM_SETTEXTEX, (WPARAM)&set_text_ex, (LPARAM)text.GetString());
	}
}

std::shared_ptr<ChannelInfo> CIPTVChannelEditorDlg::FindChannel(HTREEITEM hItem) const
{
	auto pair = m_channelsTreeMap.find(hItem);
	return pair != m_channelsTreeMap.end() ? pair->second : nullptr;
}

std::shared_ptr<ChannelCategory> CIPTVChannelEditorDlg::FindCategory(HTREEITEM hItem) const
{
	if (auto pair = m_categoriesTreeMap.find(hItem); pair != m_categoriesTreeMap.end())
	{
		if (auto cat_pair = m_categoriesMap.find(pair->second); cat_pair != m_categoriesMap.end())
		{
			return  cat_pair->second.category;
		}
	}

	return nullptr;
}

std::shared_ptr<PlaylistEntry> CIPTVChannelEditorDlg::FindEntry(HTREEITEM hItem) const
{
	auto pair = m_playlistTreeMap.find(hItem);
	return pair != m_playlistTreeMap.end() ? pair->second : nullptr;
}

BaseInfo* CIPTVChannelEditorDlg::GetBaseInfo(const CTreeCtrlEx* pCtl, HTREEITEM hItem) const
{
	BaseInfo* baseInfo = nullptr;
	if (pCtl == &m_wndChannelsTree)
	{
		if (IsChannel(hItem))
		{
			const auto& channel = FindChannel(hItem);
			baseInfo = dynamic_cast<BaseInfo*>(channel.get());
		}
		else if (IsCategory(hItem))
		{
			const auto& category = FindCategory(hItem);
			baseInfo = dynamic_cast<BaseInfo*>(category.get());
		}
	}
	else if (pCtl == &m_wndPlaylistTree)
	{
		const auto& entry = FindEntry(hItem);
		baseInfo = dynamic_cast<BaseInfo*>(entry.get());
	}

	return baseInfo;
}

std::shared_ptr<ChannelCategory> CIPTVChannelEditorDlg::GetItemCategory(HTREEITEM hItem) const
{
	if (hItem != nullptr && m_wndChannelsTree.GetParentItem(hItem) != nullptr)
	{
		hItem = m_wndChannelsTree.GetParentItem(hItem);
	}

	return GetCategory(hItem);
}

std::shared_ptr<ChannelCategory> CIPTVChannelEditorDlg::GetCategory(HTREEITEM hItem) const
{
	// find category id by HTREE item
	if (const auto& pair = m_categoriesTreeMap.find(hItem); pair != m_categoriesTreeMap.end())
	{
		// find category by id
		auto found = m_categoriesMap.find(pair->second);
		return found != m_categoriesMap.end() ? found->second.category : nullptr;
	}

	return nullptr;
}

int CIPTVChannelEditorDlg::GetNewCategoryID() const
{
	int id = 0;
	if (!m_categoriesMap.empty())
	{
		for (auto pair = m_categoriesMap.crbegin(); pair != m_categoriesMap.crend(); ++pair)
		{
			if (pair->first != ID_ADD_TO_FAVORITE)
			{
				id = pair->first;
				break;
			}
		}
	}

	return ++id;
}

bool CIPTVChannelEditorDlg::LoadChannels()
{
	set_allow_save(FALSE);

	int lst_idx = m_wndChannels.GetCurSel();
	if (lst_idx == CB_ERR)
		return false;

	const auto& channelsPath = fmt::format(L"{:s}{:s}\\{:s}",
										   GetConfig().get_string(true, REG_LISTS_PATH),
										   GetConfig().GetCurrentPluginName(),
										   m_all_channels_lists[lst_idx]);

	std::ifstream is(channelsPath, std::istream::binary);
	if (!is.good())
		return false;

	// Read the xml file into a vector
	std::vector<char> buffer((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
	buffer.emplace_back('\0');

	// Parse the buffer using the xml file parsing library into doc
	auto doc = std::make_unique<rapidxml::xml_document<>>();

	try
	{
		doc->parse<0>(buffer.data());
	}
	catch (rapidxml::parse_error& ex)
	{
		ex;
		return false;
	}

	auto i_node = doc->first_node(utils::TV_INFO);
	auto info_node = i_node->first_node(utils::VERSION_INFO);
	if (!info_node || rapidxml::get_value_int(info_node->first_node(utils::LIST_VERSION)) != CHANNELS_LIST_VERSION)
	{
		set_allow_save(TRUE);
	}

	const auto& root_path = GetAppPath(utils::PLUGIN_ROOT);
	auto cat_node = i_node->first_node(utils::TV_CATEGORIES)->first_node(utils::TV_CATEGORY);
	// Iterate <tv_category> nodes
	while (cat_node)
	{
		auto category = std::make_shared<ChannelCategory>(cat_node, StreamType::enBase, root_path);
		CategoryInfo info = { nullptr, category };
		m_categoriesMap.emplace(category->get_key(), info);
		cat_node = cat_node->next_sibling();
	}

	auto fav_category = std::make_shared<ChannelCategory>(StreamType::enBase, root_path);
	fav_category->set_icon_uri(L"plugin_file://icons/fav.png");
	fav_category->set_title(L"Favorites");
	fav_category->set_key(ID_ADD_TO_FAVORITE);
	CategoryInfo info = { nullptr, fav_category };
	m_categoriesMap.emplace(ID_ADD_TO_FAVORITE, info);

	auto ch_node = i_node->first_node(utils::TV_CHANNELS)->first_node(utils::TV_CHANNEL);
	// Iterate <tv_channel> nodes
	while (ch_node)
	{
		auto channel = std::make_shared<ChannelInfo>(ch_node, StreamType::enChannels, root_path);
		ASSERT(!channel->stream_uri->get_parser().id.empty());

		auto ch_pair = m_channelsMap.find(channel->stream_uri->get_parser().id);
		if (ch_pair != m_channelsMap.end())
		{
			// Only one unique channel must used!
			// at first add parsed categories
			ch_pair->second->get_category_ids().insert(channel->get_category_ids().begin(), channel->get_category_ids().end());
			// now replace channel to unique reference
			channel = ch_pair->second;
		}
		else
		{
			channel->set_type(m_plugin_type);
			ch_pair = m_channelsMap.emplace(channel->stream_uri->get_parser().id, channel).first;
			if (channel->is_favorite())
				fav_category->add_channel(channel);

			ch_pair->second->get_category_ids().insert(channel->get_category_ids().begin(), channel->get_category_ids().end());
		}

		for (const auto& id : channel->get_category_ids())
		{
			auto cat_pair = m_categoriesMap.find(id);
			ASSERT(cat_pair != m_categoriesMap.end());
			cat_pair->second.category->add_channel(channel);
		}

		ch_node = ch_node->next_sibling();
	}

	m_wndSearch.EnableWindow(!m_channelsMap.empty());

	return true;
}

void CIPTVChannelEditorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
		return;
	}

	if (nID == SC_MINIMIZE && GetConfig().get_int(true, REG_AUTO_HIDE))
	{
		m_wndTrayIcon.MinToTray(this);
		return;
	}

	__super::OnSysCommand(nID, lParam);
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CIPTVChannelEditorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		__super::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CIPTVChannelEditorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CIPTVChannelEditorDlg::OnAddCategory()
{
	CWaitCursor cur;

	// disable autosync when add channels
	BOOL autoSyncOld = GetConfig().get_int(true, REG_AUTO_SYNC);
	GetConfig().set_int(true, REG_AUTO_SYNC, FALSE);

	bool needCheckExisting = false;
	for (const auto& hCategory : m_wndPlaylistTree.GetSelectedItems())
	{
		for (auto hIter = m_wndPlaylistTree.GetChildItem(hCategory); hIter != nullptr; hIter = m_wndPlaylistTree.GetNextSiblingItem(hIter))
		{
			needCheckExisting |= AddChannel(FindEntry(hIter));
		}
	}

	if (needCheckExisting)
	{
		UpdateChannelsTreeColors();
		CheckForExistingPlaylist();
	}

	GetConfig().set_int(true, REG_AUTO_SYNC, autoSyncOld);

	LoadChannelInfo();
	set_allow_save();
}

void CIPTVChannelEditorDlg::OnUpdateAddCategory(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsPlaylistCategory(m_wndPlaylistTree.GetFirstSelectedItem()) && IsSelectedTheSameType(&m_wndPlaylistTree));
}

void CIPTVChannelEditorDlg::OnNewChannel()
{
	auto hCategory = m_wndChannelsTree.GetSelectedItem();
	if (auto hRoot = m_wndChannelsTree.GetParentItem(hCategory); hRoot != nullptr)
	{
		hCategory = hRoot;
	}

	const auto& category = GetCategory(hCategory);
	if (!category)
		return;

	auto channel = std::make_shared<ChannelInfo>(m_plugin_type, GetAppPath(utils::PLUGIN_ROOT));
	channel->set_title(L"New Channel");
	channel->set_icon_uri(utils::ICON_TEMPLATE);

	SetImageControl(GetIconCache().get_icon(channel->get_icon_absolute_path()), m_wndChannelIcon);

	TVINSERTSTRUCTW tvInsert = { nullptr };
	tvInsert.hParent = hCategory;
	tvInsert.item.pszText = (LPWSTR)channel->get_title().c_str();
	tvInsert.item.lParam = (LPARAM)InfoType::enChannel;
	tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
	auto hNewItem = m_wndChannelsTree.InsertItem(&tvInsert);

	// templated url. set to -1 if empty
	int id = -1;
	while (m_channelsMap.find(std::to_wstring(id)) != m_channelsMap.end())
	{
		--id;
	}

	const auto& str_id = std::to_wstring(id);
	channel->stream_uri->set_template(true);
	channel->stream_uri->get_parser().id = str_id;

	category->add_channel(channel);
	m_channelsMap.emplace(channel->stream_uri->get_parser().id, channel);
	m_channelsTreeMap.emplace(hNewItem, channel);

	m_wndChannelsTree.SelectItem(hNewItem);
	m_wndChannelsTree.EditLabel(hNewItem);
}

void CIPTVChannelEditorDlg::OnUpdateNewChannel(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!!IsSelectedNotFavorite());
}

void CIPTVChannelEditorDlg::OnRemove()
{
	CWnd* pFocused = GetFocus();
	if (pFocused != &m_wndChannelsTree)
	{
		pFocused->SendMessage(WM_KEYDOWN, VK_DELETE);
		return;
	}

	if (!m_wndChannelsTree.GetSelectedCount() || AfxMessageBox(IDS_STRING_WARN_DELETE_ITEMS, MB_YESNO | MB_ICONWARNING) != IDYES)
		return;

	if (IsCategory(m_wndChannelsTree.GetFirstSelectedItem()))
	{
		for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
		{
			const auto& category = GetCategory(hItem);
			if (!category) continue;

			auto category_id = category->get_key();
			if (category_id == ID_ADD_TO_FAVORITE) continue;

			for (auto hChildItem = m_wndChannelsTree.GetChildItem(hItem); hChildItem != nullptr; hChildItem = m_wndChannelsTree.GetNextSiblingItem(hChildItem))
			{
				m_channelsTreeMap.erase(hChildItem);
			}
			m_categoriesMap.erase(category_id);
			m_categoriesTreeMap.erase(hItem);

			m_wndChannelsTree.DeleteItem(hItem);
		}
	}
	else if (const auto& category = GetItemCategory(m_wndChannelsTree.GetFirstSelectedItem()); category != nullptr)
	{
		std::vector<HTREEITEM> toDelete;
		for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
		{
			const auto& channel = FindChannel(hItem);
			if (channel == nullptr) continue;

			if (category->get_key() == ID_ADD_TO_FAVORITE)
				channel->set_favorite(false);

			toDelete.emplace_back(hItem);
			category->remove_channel(channel->stream_uri->get_parser().id);
		}

		for (const auto& hItem : toDelete)
		{
			m_channelsTreeMap.erase(hItem);
			m_wndChannelsTree.DeleteItem(hItem);
		}
	}

	RemoveOrphanChannels();
	CheckForExistingPlaylist();
	UpdateChannelsTreeColors();
	set_allow_save();
}

void CIPTVChannelEditorDlg::OnUpdateRemove(CCmdUI* pCmdUI)
{
	pCmdUI->Enable((IsSelectedChannelsOrEntries(true) && IsSelectedInTheSameCategory() || IsSelectedCategory()) && IsSelectedNotFavorite());
}

void CIPTVChannelEditorDlg::OnChannelUp()
{
	HTREEITEM hTop = m_wndChannelsTree.GetFirstSelectedItem();
	HTREEITEM hBottom = m_wndChannelsTree.GetLastSelectedItem();

	if (IsChannel(m_wndChannelsTree.GetPrevSiblingItem(hTop)))
	{
		MoveChannels(hTop, hBottom, false);
	}
	else if (IsCategory(hTop) && m_wndChannelsTree.GetSelectedCount() == 1)
	{
		SwapCategories(m_wndChannelsTree.GetPrevSiblingItem(hTop), hTop);
	}
}

void CIPTVChannelEditorDlg::OnUpdateChannelUp(CCmdUI* pCmdUI)
{
	BOOL enable = FALSE;
	HTREEITEM hCur = m_wndChannelsTree.GetFirstSelectedItem();
	HTREEITEM hPrev = m_wndChannelsTree.GetPrevSiblingItem(hCur);

	if (IsChannel(hCur))
	{
		enable = IsChannelSelectionConsistent()
			&& IsSelectedInTheSameCategory()
			&& IsSelectedNotInFavorite()
			&& hPrev != nullptr;
	}
	else if (IsCategory(hCur) && m_wndChannelsTree.GetSelectedCount() == 1)
	{
		auto prevCat = FindCategory(hPrev);

		enable = IsSelectedNotFavorite() && prevCat != nullptr && prevCat->get_key() != ID_ADD_TO_FAVORITE;
	}

	pCmdUI->Enable(enable);
}

void CIPTVChannelEditorDlg::OnChannelDown()
{
	HTREEITEM hTop = m_wndChannelsTree.GetFirstSelectedItem();
	HTREEITEM hBottom = m_wndChannelsTree.GetLastSelectedItem();

	if (IsChannel(m_wndChannelsTree.GetNextSiblingItem(hBottom)))
	{
		MoveChannels(hTop, hBottom, true);
	}
	else if (IsCategory(hTop) && m_wndChannelsTree.GetSelectedCount() == 1)
	{
		SwapCategories(m_wndChannelsTree.GetNextSiblingItem(hTop), hTop);
	}
}

void CIPTVChannelEditorDlg::OnUpdateChannelDown(CCmdUI* pCmdUI)
{
	BOOL enable = FALSE;
	HTREEITEM hCur = m_wndChannelsTree.GetFirstSelectedItem();

	if (IsChannel(hCur))
	{
		enable = IsChannelSelectionConsistent()
			&& IsSelectedInTheSameCategory()
			&& IsSelectedNotInFavorite()
			&& m_wndChannelsTree.GetNextSiblingItem(m_wndChannelsTree.GetLastSelectedItem()) != nullptr;
	}
	else if (IsCategory(hCur) && m_wndChannelsTree.GetSelectedCount() == 1)
	{
		enable = IsSelectedNotFavorite()
			&& m_wndChannelsTree.GetNextSiblingItem(m_wndChannelsTree.GetLastSelectedItem()) != nullptr;
	}

	pCmdUI->Enable(enable);
}

void CIPTVChannelEditorDlg::MoveChannels(HTREEITEM hBegin, HTREEITEM hEnd, bool down)
{
	const auto& category = GetItemCategory(hBegin);

	HTREEITEM hMoved = nullptr;
	HTREEITEM hAfter = nullptr;
	if (down)
	{
		hMoved = m_wndChannelsTree.GetNextSiblingItem(hEnd);
		hAfter = m_wndChannelsTree.GetPrevSiblingItem(hBegin);
		hAfter = hAfter ? hAfter : TVI_FIRST;
	}
	else
	{
		hMoved = m_wndChannelsTree.GetPrevSiblingItem(hBegin);
		hAfter = hEnd;
	}

	const auto& chBegin = FindChannel(hBegin);
	const auto& chEnd = FindChannel(hEnd);
	const auto& chMoved = FindChannel(hMoved);
	if (!chBegin || !chEnd || !chMoved)
		return;

	category->move_channels(chBegin, chEnd, down);

	TVINSERTSTRUCTW tvInsert = { nullptr };
	tvInsert.hParent = m_wndChannelsTree.GetParentItem(hBegin);
	tvInsert.item.pszText = (LPWSTR)chMoved->get_title().c_str();
	tvInsert.item.lParam = (LPARAM)InfoType::enChannel;
	tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
	tvInsert.hInsertAfter = hAfter;

	HTREEITEM newItem = m_wndChannelsTree.InsertItem(&tvInsert);
	m_wndChannelsTree.SetItemColor(newItem, m_wndChannelsTree.GetItemColor(hMoved));
	m_channelsTreeMap.emplace(newItem, chMoved);

	m_channelsTreeMap.erase(hMoved);
	m_wndChannelsTree.DeleteItem(hMoved);

	set_allow_save();
}

void CIPTVChannelEditorDlg::SwapCategories(const HTREEITEM hLeft, const HTREEITEM hRight)
{
	// 1 2 3 4 5 - order left to right. Left has low value then right and position in the tree upper then right
	const auto& category_right = GetCategory(hRight);
	const auto& category_left = GetCategory(hLeft);
	if (!category_left || !category_right)
	{
		ASSERT(false);
		return;
	}

	const auto& lKey = GetCategory(hLeft)->get_key();
	const auto& rKey = GetCategory(hRight)->get_key();

	// swap struct in map

	// get copy struct
	CategoryInfo lStruct = m_categoriesMap[lKey];
	CategoryInfo rStruct = m_categoriesMap[rKey];

	lStruct.category->set_key(rKey);
	rStruct.category->set_key(lKey);

	// Set swapped struct
	m_categoriesMap[lKey] = rStruct;
	m_categoriesMap[rKey] = lStruct;

	// запоминаем ItemData для нод и подменяем на счетчик
	std::vector<HTREEITEM> itemData;
	for (HTREEITEM hItem = m_wndChannelsTree.GetChildItem(nullptr); hItem != nullptr; hItem = m_wndChannelsTree.GetNextSiblingItem(hItem))
	{
		itemData.emplace_back(hItem);
		int key = m_categoriesTreeMap[hItem];
		if (key == ID_ADD_TO_FAVORITE)
			key = -ID_ADD_TO_FAVORITE; // move to first position for sort
		m_wndChannelsTree.SetItemData(hItem, key);
	}

	// Меняем местами нужные ItemData для сортировки
	int idx = (int)m_wndChannelsTree.GetItemData(hLeft);
	m_wndChannelsTree.SetItemData(hLeft, m_wndChannelsTree.GetItemData(hRight));
	m_wndChannelsTree.SetItemData(hRight, idx);

	// сортируем. Пусть TreeCtrl сам переупорядочит внутренний список
	TVSORTCB sortInfo = { nullptr };
	sortInfo.lpfnCompare = &CBCompareForSwap;
	m_wndChannelsTree.SortChildrenCB(&sortInfo);

	for (HTREEITEM hItem = m_wndChannelsTree.GetChildItem(nullptr); hItem != nullptr; hItem = m_wndChannelsTree.GetNextSiblingItem(hItem))
	{
		int key = (int)m_wndChannelsTree.GetItemData(hItem);
		if (key == -ID_ADD_TO_FAVORITE)
			key = ID_ADD_TO_FAVORITE;
		m_categoriesTreeMap[hItem] = key;
		m_categoriesMap[key].hItem = hItem;
		m_wndChannelsTree.SetItemData(hItem, (DWORD_PTR)InfoType::enCategory);
	}

	m_wndChannelsTree.SelectItem(hRight);

	set_allow_save();
}

void CIPTVChannelEditorDlg::OnRenameChannel()
{
	if (::GetFocus() == m_wndChannelsTree.GetSafeHwnd())
	{
		m_wndChannelsTree.EditLabel(m_wndChannelsTree.GetSelectedItem());
	}
}

void CIPTVChannelEditorDlg::OnUpdateRenameChannel(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_wndChannelsTree.GetSelectedCount() == 1 && IsSelectedNotFavorite());
}

void CIPTVChannelEditorDlg::OnBnClickedCheckCustomize()
{
	BOOL not_checked = m_wndCustom.GetCheck() ? FALSE : TRUE;
	m_wndStreamUrl.SetReadOnly(not_checked);
	m_wndStreamID.EnableWindow(not_checked);

	if (m_wndChannelsTree.GetSelectedCount() == 1)
	{
		auto hItem = m_wndChannelsTree.GetSelectedItem();
		const auto& channel = FindChannel(hItem);
		if (channel)
		{
			const auto& category = GetItemCategory(hItem);
			const std::wstring old_id = channel->stream_uri->get_parser().id;

			channel->stream_uri->set_template(not_checked);
			if (!not_checked)
			{
				// custom url. clear id
				channel->stream_uri->get_parser().id.clear();
				m_streamID.Empty();
			}
			else if (channel->stream_uri->get_parser().id.empty())
			{
				// templated url. set to -1 if empty
				int id = -1;
				while (m_channelsMap.find(std::to_wstring(id)) != m_channelsMap.end())
				{
					--id;
				}

				const auto& str_id = std::to_wstring(id);
				channel->stream_uri->get_parser().id = str_id;
				m_streamID = str_id.c_str();
			}

			// recalculate hash
			channel->stream_uri->recalc_hash();

			category->remove_channel(old_id);
			m_channelsMap.erase(old_id);

			m_channelsMap.emplace(channel->stream_uri->get_parser().id, channel);
			category->add_channel(channel);

			UpdateData(FALSE);
		}
	}

	set_allow_save();
}

void CIPTVChannelEditorDlg::OnTvnSelchangedTreeChannels(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (m_bInFillTree)
		return;

	HTREEITEM hSelected = reinterpret_cast<LPNMTREEVIEW>(pNMHDR)->itemNew.hItem;
	UpdateControlsForItem(hSelected);

	OnSyncTreeItem();

	if (pResult)
		*pResult = 0;
}

void CIPTVChannelEditorDlg::UpdateControlsForItem(HTREEITEM hSelected /*= nullptr*/)
{
	if (hSelected == nullptr)
		hSelected = m_wndChannelsTree.GetSelectedItem();

	int state = 0; // none selected
	bool bSameType = IsSelectedTheSameType(&m_wndChannelsTree);
	int changed_flag = 0;
	if (bSameType)
	{
		const auto& channel = FindChannel(hSelected);
		if (channel != nullptr)
		{
			LoadChannelInfo(hSelected);
			state = 2; // multiple selected
			m_wndChannelIcon.EnableWindow(FALSE);
		}

		if (m_wndChannelsTree.GetSelectedCount() == 1)
		{
			if (channel != nullptr)
			{
				state = 1; // single selected
				if (!channel->stream_uri->is_template())
					m_streamID.Empty();

				m_wndChannelIcon.EnableWindow(TRUE);
				if (auto& pair = m_changedChannels.find(channel->stream_uri->get_parser().id); pair != m_changedChannels.end())
					changed_flag = pair->second;
			}
			else if (IsCategory(hSelected))
			{
				m_epgID1.Empty();
				m_epgID2.Empty();
				m_isArchive = 0;
				m_isAdult = 0;
				m_streamUrl.Empty();
				m_streamID.Empty();
				m_infoAudio.Empty();
				m_infoVideo.Empty();
				m_wndEpg.SetWindowText(L"");

				m_wndChannelIcon.EnableWindow(TRUE);
				const auto& category = GetCategory(hSelected);
				if (category)
				{
					m_iconUrl = category->get_icon_uri().get_uri().c_str();
					const auto& img = GetIconCache().get_icon(category->get_icon_absolute_path());
					if (img == nullptr)
					{
						GetDlgItem(IDC_STATIC_ICON_SIZE)->SetWindowText(_T(""));
					}
					else
					{
						GetDlgItem(IDC_STATIC_ICON_SIZE)->SetWindowText(fmt::format(L"{:d} x {:d} px", img.GetWidth(), img.GetHeight()).c_str());
					}
					SetImageControl(img, m_wndChannelIcon);
				}
			}
		}
	}

	bool single = (state == 1);
	bool bSameCategory = IsSelectedInTheSameCategory();
	bool hasProbe = !GetConfig().get_string(true, REG_FFPROBE).empty();

	bool firstEpg = GetCheckedRadioButton(IDC_RADIO_EPG1, IDC_RADIO_EPG2) == IDC_RADIO_EPG1;
	m_wndCustom.EnableWindow(single);
	m_wndArchive.EnableWindow(state != 0);
	m_wndAdult.EnableWindow(state != 0);
	m_wndViewEPG.EnableWindow(single && (firstEpg ? !m_epgID1.IsEmpty() : !m_epgID2.IsEmpty()));
	m_wndStreamID.EnableWindow(single && !m_streamID.IsEmpty());
	m_wndCheckArchive.EnableWindow(single && hasProbe && !m_loading);
	m_wndTimeShift.EnableWindow(state);
	m_wndSpinTimeShift.EnableWindow(state);
	m_wndSearch.EnableWindow(TRUE);
	m_wndEpg1.EnableWindow(single);
	m_wndEpg2.EnableWindow(single && !m_plugin->get_epg_parameters(1).epg_url.empty());

	m_wndArchiveDays.EnableWindow((state != 0) && (m_isArchive != 0));
	m_wndArchiveDays.SetTextColor(single && (changed_flag & MOD_ARCHIVE) ? m_colorChanged : m_normal);

	m_wndEpgID1.SetTextColor(single && (changed_flag & MOD_EPG1) ? m_colorChanged : m_normal);
	m_wndEpgID1.EnableWindow(single);

	m_wndEpgID2.SetTextColor(single && (changed_flag & MOD_EPG2) ? m_colorChanged : m_normal);
	m_wndEpgID2.EnableWindow(single && !m_plugin->get_epg_parameters(1).epg_url.empty());

	m_wndIconUrl.SetTextColor(single && (changed_flag & MOD_ICON) ? m_colorChanged : m_normal);

	if (state == 2)
	{
		bool bEnable = hasProbe;
		if (m_lastTree != nullptr)
		{
			bEnable &= (GetBaseInfo(m_lastTree, m_lastTree->GetFirstSelectedItem()) != nullptr);
		}
		else
		{
			bEnable = false;
		}

		bEnable = bEnable && bSameType;
	}

	UpdateData(FALSE);
}

void CIPTVChannelEditorDlg::OnTvnEndlabeleditTreeChannels(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);

	*pResult = FALSE;
	if (pTVDispInfo->item.pszText && pTVDispInfo->item.pszText[0])
	{
		if (const auto& channel = FindChannel(pTVDispInfo->item.hItem); channel != nullptr)
		{
			channel->set_title(pTVDispInfo->item.pszText);
			set_allow_save();
			*pResult = TRUE;
		}
		else if (const auto& category = FindCategory(pTVDispInfo->item.hItem); category != nullptr)
		{
			category->set_title(pTVDispInfo->item.pszText);
			set_allow_save();
			*pResult = TRUE;
		}
	}
}

void CIPTVChannelEditorDlg::OnNMDblclkTreeChannels(NMHDR* pNMHDR, LRESULT* pResult)
{
	CPoint pt(GetMessagePos());
	m_wndChannelsTree.ScreenToClient(&pt);

	*pResult = 0;
	UINT uFlags = 0;
	HTREEITEM hItem = m_wndChannelsTree.HitTest(pt, &uFlags);
	if (hItem && (uFlags & TVHT_ONITEM) && !IsCategory(hItem))
	{
		PlayItem(hItem);
	}
}

void CIPTVChannelEditorDlg::OnNMRclickTreeChannel(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;

	CPoint ptScreen;
	if (!GetCursorPos(&ptScreen))
		return;

	CPoint ptClient(ptScreen);
	m_wndChannelsTree.ScreenToClient(&ptClient);

	UINT uFlags = 0;
	HTREEITEM hItem = m_wndChannelsTree.HitTest(ptClient, &uFlags);

	if (!hItem || !(TVHT_ONITEM & uFlags))
		return;

	// The user hasn't clicked on any item.
	CMenu menu;
	VERIFY(menu.LoadMenu(IDR_MENU_CHANNEL));

	CMenu* pMenu = menu.GetSubMenu(0);
	if (!pMenu)
		return;

	if (m_wndChannelsTree.GetSelectedCount() == 1)
		pMenu->SetDefaultItem(ID_PLAY_STREAM);

	CCmdUI cmdUI;
	cmdUI.m_nIndexMax = pMenu->GetMenuItemCount();
	for (UINT i = 0; i < cmdUI.m_nIndexMax; ++i)
	{
		cmdUI.m_nIndex = i;
		cmdUI.m_nID = pMenu->GetMenuItemID(i);
		cmdUI.m_pMenu = pMenu;
		cmdUI.DoUpdate(this, FALSE);
	}

	if (!m_categoriesMap.empty() && IsSelectedChannelsOrEntries())
	{
		CMenu subMenuCopy;
		subMenuCopy.CreatePopupMenu();

		CMenu subMenuMove;
		subMenuMove.CreatePopupMenu();

		const auto& itemCategory = GetItemCategory(m_wndChannelsTree.GetFirstSelectedItem());
		if (itemCategory != nullptr && itemCategory->get_key() != ID_ADD_TO_FAVORITE)
		{
			for (const auto& category : m_categoriesMap)
			{
				if (ID_ADD_TO_FAVORITE == category.first || itemCategory->get_key() == category.first) continue;

				subMenuCopy.AppendMenu(MF_STRING | MF_ENABLED, ID_COPY_TO_START + category.first, category.second.category->get_title().c_str());
				subMenuMove.AppendMenu(MF_STRING | MF_ENABLED, ID_MOVE_TO_START + category.first, category.second.category->get_title().c_str());
			}

			pMenu->InsertMenu(ID_EDIT_RENAME, MF_BYCOMMAND | MF_POPUP, (UINT_PTR)subMenuCopy.Detach(), _T("Copy To"));
			pMenu->InsertMenu(ID_EDIT_RENAME, MF_BYCOMMAND | MF_POPUP, (UINT_PTR)subMenuMove.Detach(), _T("Move To"));
		}
	}

	CContextMenuManager* manager = theApp.GetContextMenuManager();
	//for CDialogEx:
	theApp.GetContextMenuManager()->ShowPopupMenu(pMenu->GetSafeHmenu(), ptScreen.x, ptScreen.y, this, TRUE, TRUE, FALSE);
}

void CIPTVChannelEditorDlg::OnNMDblclkTreePaylist(NMHDR* pNMHDR, LRESULT* pResult)
{
	CPoint pt(GetMessagePos());
	m_wndPlaylistTree.ScreenToClient(&pt);

	*pResult = 0;
	UINT uFlags = 0;
	HTREEITEM hItem = m_wndPlaylistTree.HitTest(pt, &uFlags);
	if (hItem && (uFlags & TVHT_ONITEM))
	{
		PlayItem(hItem);
	}
}

void CIPTVChannelEditorDlg::OnNMRclickTreePlaylist(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;

	CPoint ptScreen;
	if (!GetCursorPos(&ptScreen))
		return;

	CPoint ptClient(ptScreen);
	m_wndPlaylistTree.ScreenToClient(&ptClient);

	UINT uFlags = 0;
	HTREEITEM hItem = m_wndPlaylistTree.HitTest(ptClient, &uFlags);

	if (!hItem || !(TVHT_ONITEM & uFlags))
		return;

	// The user hasn't clicked on any item.
	CMenu menu;
	VERIFY(menu.LoadMenu(IDR_MENU_PLAYLIST));

	CMenu* pMenu = menu.GetSubMenu(0);
	if (!pMenu)
		return;

	if (m_wndPlaylistTree.GetSelectedCount() == 1)
		pMenu->SetDefaultItem(ID_PLAY_STREAM);

	CCmdUI cmdUI;
	cmdUI.m_nIndexMax = pMenu->GetMenuItemCount();
	for (UINT i = 0; i < cmdUI.m_nIndexMax; ++i)
	{
		cmdUI.m_nIndex = i;
		cmdUI.m_nID = pMenu->GetMenuItemID(i);
		cmdUI.m_pMenu = pMenu;
		cmdUI.DoUpdate(this, FALSE);
	}

	if (IsSelectedChannelsOrEntries())
	{
		CMenu subMenuCopy;
		subMenuCopy.CreatePopupMenu();

		const auto& entry = FindEntry(m_wndPlaylistTree.GetFirstSelectedItem());

		for (const auto& category : m_categoriesMap)
		{
			subMenuCopy.AppendMenu(MF_STRING | MF_ENABLED, ID_ADD_TO_START + category.first, category.second.category->get_title().c_str());
		}

		pMenu->InsertMenu(ID_PLAYLISTMENU_SUBMENU, MF_BYCOMMAND | MF_POPUP, (UINT_PTR)subMenuCopy.Detach(), _T("Add To"));
	}

	pMenu->DeleteMenu(ID_PLAYLISTMENU_SUBMENU, MF_BYCOMMAND);

	CContextMenuManager* manager = theApp.GetContextMenuManager();
	//for CDialogEx:
	theApp.GetContextMenuManager()->ShowPopupMenu(pMenu->GetSafeHmenu(), ptScreen.x, ptScreen.y, this, TRUE, TRUE, FALSE);
}

void CIPTVChannelEditorDlg::OnAddToFavorite()
{
	OnCopyTo(ID_COPY_TO_START + ID_ADD_TO_FAVORITE);
}

void CIPTVChannelEditorDlg::OnUpdateAddToFavorite(CCmdUI* pCmdUI)
{
	const auto& itemCategory = GetItemCategory(m_wndChannelsTree.GetFirstSelectedItem());

	pCmdUI->Enable(itemCategory != nullptr && itemCategory->get_key() != ID_ADD_TO_FAVORITE);
}

void CIPTVChannelEditorDlg::OnCopyTo(UINT id)
{
	CopyMoveChannelTo(id - ID_COPY_TO_START, false);
}

void CIPTVChannelEditorDlg::OnMoveTo(UINT id)
{
	CopyMoveChannelTo(id - ID_MOVE_TO_START, true);
}

void CIPTVChannelEditorDlg::OnAddTo(UINT id)
{
	UINT category_id = id - ID_ADD_TO_START;
	auto pair = m_categoriesMap.find(category_id);
	if (pair == m_categoriesMap.end())
		return;

	m_wndChannelsTree.LockWindowUpdate();

	HTREEITEM hTarget = pair->second.hItem;
	bool changed = false;
	for (const auto& hSelectedItem : m_wndPlaylistTree.GetSelectedItems())
	{
		changed |= AddChannel(FindEntry(hSelectedItem), category_id);
	}

	OnSyncTreeItem();

	if (changed)
	{
		UpdateChannelsTreeColors(hTarget);
		CheckForExistingPlaylist();
		set_allow_save();
	}

	m_wndChannelsTree.UnlockWindowUpdate();
}

void CIPTVChannelEditorDlg::OnBnClickedCheckAdult()
{
	UpdateData(TRUE);

	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		const auto& channel = FindChannel(hItem);
		if (channel)
			channel->set_adult(m_isAdult);
	}

	UpdateChannelsTreeColors(m_wndChannelsTree.GetParentItem(m_wndChannelsTree.GetSelectedItem()));
	set_allow_save();
}

void CIPTVChannelEditorDlg::OnBnClickedCheckArchive()
{
	UpdateData(TRUE);

	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		const auto& channel = FindChannel(hItem);
		if (!channel) continue;

		m_archiveDays = m_isArchive ? (m_archiveDays != 0 ? m_archiveDays : 7) : 0;
		channel->set_archive_days(m_archiveDays);
		m_wndArchiveDays.EnableWindow(m_isArchive);
		UpdateData(FALSE);
	}

	UpdateChannelsTreeColors(m_wndChannelsTree.GetParentItem(m_wndChannelsTree.GetSelectedItem()));
	set_allow_save();
}

void CIPTVChannelEditorDlg::OnEnChangeEditEpg1ID()
{
	UpdateData(TRUE);

	if (m_wndChannelsTree.GetSelectedCount() == 1)
	{
		auto hSelected = m_wndChannelsTree.GetSelectedItem();
		const auto& channel = FindChannel(hSelected);
		if (channel)
		{
			channel->set_epg_id(0, m_epgID1.GetString());
			UpdateChannelsTreeColors(m_wndChannelsTree.GetParentItem(hSelected));
			CheckForExistingPlaylist();
			set_allow_save();
		}
	}
}

void CIPTVChannelEditorDlg::OnEnChangeEditEpg2ID()
{
	UpdateData(TRUE);

	if (m_wndChannelsTree.GetSelectedCount() == 1)
	{
		const auto& channel = FindChannel(m_wndChannelsTree.GetSelectedItem());
		if (channel)
		{
			channel->set_epg_id(1, m_epgID2.GetString());
			set_allow_save();
		}
	}
}

void CIPTVChannelEditorDlg::OnEnChangeEditStreamUrl()
{
	UpdateData(TRUE);

	if (m_streamUrl.IsEmpty() || m_wndChannelsTree.GetSelectedCount() != 1 || !m_wndCustom.GetCheck())
		return;

	auto hItem = m_wndChannelsTree.GetSelectedItem();
	const auto& channel = FindChannel(hItem);
	if (!channel)
		return;

	const auto& category = GetItemCategory(hItem);
	const std::wstring old_id = channel->stream_uri->get_parser().id;

	auto newChannel = std::make_shared<ChannelInfo>(*channel);
	newChannel->stream_uri->set_uri(m_streamUrl.GetString());
	newChannel->stream_uri->recalc_hash();

	if (m_channelsMap.find(newChannel->stream_uri->get_parser().id) != m_channelsMap.end())
	{
		AfxMessageBox(IDS_STRING_WRN_CHANNEL_EXIST, MB_OK | MB_ICONWARNING);
		return;
	}

	category->remove_channel(old_id);
	m_channelsMap.erase(old_id);

	category->add_channel(newChannel);
	m_channelsMap.emplace(newChannel->stream_uri->get_parser().id, newChannel);
	m_channelsTreeMap[hItem] = newChannel;

	UpdateChannelsTreeColors(m_wndChannelsTree.GetParentItem(hItem));
	CheckForExistingPlaylist();

	set_allow_save();
}

void CIPTVChannelEditorDlg::OnEnChangeEditArchiveDays()
{
	UpdateData(TRUE);

	for (const auto& hSelectedItem : m_wndChannelsTree.GetSelectedItems())
	{
		const auto& channel = FindChannel(hSelectedItem);
		if (channel && m_isArchive)
		{
			channel->set_archive_days(m_archiveDays);
			set_allow_save();
		}
	}

	UpdateChannelsTreeColors(m_wndChannelsTree.GetParentItem(m_wndChannelsTree.GetSelectedItem()));
}

void CIPTVChannelEditorDlg::OnEnChangeEditUrlID()
{
	UpdateData(TRUE);

	if (m_streamID.IsEmpty() || m_wndChannelsTree.GetSelectedCount() != 1)
		return;

	auto hItem = m_wndChannelsTree.GetSelectedItem();
	const auto& channel = FindChannel(hItem);
	std::wstring new_id = m_streamID.GetString();
	if (!channel || channel->stream_uri->get_parser().id == new_id)
		return;

	if (m_channelsMap.find(new_id) != m_channelsMap.end())
	{
		INT_PTR res = AfxMessageBox(IDS_STRING_WRN_CHANNEL_ID_EXIST, MB_YESNO | MB_ICONWARNING);
		if (res != IDYES)
			return;
	}

	const auto& category = GetItemCategory(hItem);
	const std::wstring old_id = channel->stream_uri->get_parser().id;

	channel->stream_uri->get_parser().id = new_id;

	// recalculate hash
	channel->stream_uri->recalc_hash();


	category->remove_channel(old_id);
	m_channelsMap.erase(old_id);

	m_channelsMap.emplace(channel->stream_uri->get_parser().id, channel);
	category->add_channel(channel);

	UpdateChannelsTreeColors(m_wndChannelsTree.GetParentItem(m_wndChannelsTree.GetSelectedItem()));
	CheckForExistingPlaylist();
	set_allow_save();
}

void CIPTVChannelEditorDlg::OnEnChangeEditTimeShiftHours()
{
	if (m_timeShiftHours < -12)
		m_timeShiftHours = -12;

	if (m_timeShiftHours > 12)
		m_timeShiftHours = 12;

	UpdateData(FALSE);

	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		const auto& channel = FindChannel(hItem);
		if (channel)
		{
			channel->set_time_shift_hours(m_timeShiftHours);
		}
	}

	UpdateEPG(m_lastTree);
	set_allow_save();
}

void CIPTVChannelEditorDlg::OnEnChangeEditArchiveCheckDays()
{
	UpdateData(TRUE);

	if (m_archiveCheckDays < 0)
		m_archiveCheckDays = 0;

	if (m_archiveCheckDays > 31)
		m_archiveCheckDays = 31;

	UpdateData(FALSE);

	GetConfig().set_int(true, REG_DAYS_BACK, m_archiveCheckDays);

	UpdateEPG(m_lastTree);
}

void CIPTVChannelEditorDlg::OnEnChangeEditArchiveCheckHours()
{
	UpdateData(TRUE);

	if (m_archiveCheckHours < 0)
		m_archiveCheckHours = 0;

	if (m_archiveCheckHours > 23)
		m_archiveCheckHours = 23;

	UpdateData(FALSE);

	GetConfig().set_int(true, REG_HOURS_BACK, m_archiveCheckHours);

	UpdateEPG(m_lastTree);
}

void CIPTVChannelEditorDlg::OnDeltaposSpinTimeShiftHours(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateData(TRUE);
	m_timeShiftHours -= reinterpret_cast<LPNMUPDOWN>(pNMHDR)->iDelta;
	UpdateData(FALSE);
	OnEnChangeEditTimeShiftHours();
	*pResult = 0;
}

void CIPTVChannelEditorDlg::OnDeltaposSpinArchiveCheckDay(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateData(TRUE);
	m_archiveCheckDays -= reinterpret_cast<LPNMUPDOWN>(pNMHDR)->iDelta;
	UpdateData(FALSE);
	OnEnChangeEditArchiveCheckDays();
	*pResult = 0;
}

void CIPTVChannelEditorDlg::OnDeltaposSpinArchiveCheckHour(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateData(TRUE);
	m_archiveCheckHours -= reinterpret_cast<LPNMUPDOWN>(pNMHDR)->iDelta;
	UpdateData(FALSE);
	OnEnChangeEditArchiveCheckHours();
	*pResult = 0;
}

void CIPTVChannelEditorDlg::OnBnClickedButtonViewEpg()
{
	auto info = GetBaseInfo(&m_wndChannelsTree, m_wndChannelsTree.GetSelectedItem());
	if (info)
	{
		CEpgListDlg dlg;
		dlg.m_epg_idx = GetCheckedRadioButton(IDC_RADIO_EPG1, IDC_RADIO_EPG2) - IDC_RADIO_EPG1;
		dlg.m_epg_cache = &m_epg_cache;
		dlg.m_params.subdomain = m_cur_account.get_domain();
		dlg.m_params.token = m_cur_account.get_token();
		dlg.m_params.login = m_cur_account.get_login();
		dlg.m_params.password = m_cur_account.get_password();
		dlg.m_params.streamSubtype = (StreamSubType)m_wndStreamType.GetItemData(m_wndStreamType.GetCurSel());

		if (info->stream_uri->get_server_subst_type() == ServerSubstType::enStream)
		{
			dlg.m_params.server = m_cur_account.device_id;
		}

		UpdateExtToken(info->stream_uri.get());
		dlg.m_info = info;


		dlg.DoModal();
	}
}

void CIPTVChannelEditorDlg::OnBnClickedButtonEpg()
{
	UpdateEPG(m_lastTree);
	bool firstEpg = GetCheckedRadioButton(IDC_RADIO_EPG1, IDC_RADIO_EPG2) == IDC_RADIO_EPG1;
	m_wndViewEPG.EnableWindow(firstEpg ? !m_epgID1.IsEmpty() : !m_epgID2.IsEmpty());
}

void CIPTVChannelEditorDlg::OnBnClickedButtonUpdateChanged()
{
	CWaitCursor cur;

	bool changed = false;
	for (const auto& item : m_changedChannels)
	{
		changed |= AddChannel(m_playlistMap[item.first]);
	}

	if (changed)
	{
		UpdateChannelsTreeColors();
		FillTreePlaylist();
		UpdateControlsForItem();
	}

	set_allow_save(changed);
}

void CIPTVChannelEditorDlg::OnBnDropDownSplitButtonUpdateChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMBCDROPDOWN pDropDown = reinterpret_cast<LPNMBCDROPDOWN>(pNMHDR);

	CMenu menu;
	VERIFY(menu.LoadMenu(IDR_MENU_CHANGED));

	CMenu* pMenu = menu.GetSubMenu(0);
	if (!pMenu)
		return;

	pMenu->EnableMenuItem(ID_REMOVE_UNKNOWN, m_unknownChannels.empty() ? MF_DISABLED : MF_ENABLED);

	CRect rectButton;
	m_wndUpdateChanged.GetWindowRect(&rectButton);

	TPMPARAMS tpmParams{};
	tpmParams.cbSize = sizeof(TPMPARAMS);
	tpmParams.rcExclude = rectButton;

	pMenu->TrackPopupMenuEx(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, rectButton.left, rectButton.bottom, this, &tpmParams);

	*pResult = 0;
}

void CIPTVChannelEditorDlg::OnRemoveUnknownChannels()
{
	CWaitCursor cur;

	bool changed = false;
	for (const auto& item : m_unknownChannels)
	{
		for (const auto& pair : m_categoriesMap)
		{
			changed |= pair.second.category->remove_channel(item);
		}
	}

	if (changed)
	{
		RemoveOrphanChannels();
		FillTreeChannels();
		set_allow_save();
	}
}

void CIPTVChannelEditorDlg::PlayItem(HTREEITEM hItem, int archive_hour /*= 0*/, int archive_day /*= 0*/) const
{
	if (auto info = GetBaseInfo(m_lastTree, hItem); info != nullptr)
	{
		TemplateParams params;
		params.subdomain = m_cur_account.get_domain();
		params.token = m_cur_account.get_token();
		params.login = m_cur_account.get_login();
		params.password = m_cur_account.get_password();
		params.streamSubtype = (StreamSubType)m_wndStreamType.GetItemData(m_wndStreamType.GetCurSel());

		if (info->stream_uri->get_server_subst_type() == ServerSubstType::enStream)
		{
			params.server = m_cur_account.device_id;
		}

		int sec_back = 86400 * archive_day + 3600 * archive_hour;
		params.shift_back = sec_back ? _time32(nullptr) - sec_back : sec_back;

		UpdateExtToken(info->stream_uri.get());
		const auto& url = info->stream_uri->get_templated_stream(params);

		TRACE(L"\nTest URL: %s\n", url.c_str());

		ShellExecuteW(nullptr,
					  L"open",
					  GetConfig().get_string(true, REG_PLAYER).c_str(),
					  url.c_str(),
					  nullptr,
					  SW_SHOWNORMAL);
	}
}

void CIPTVChannelEditorDlg::OnBnClickedButtonAccountSettings()
{
	const auto& pl_info = ((PlaylistInfo*)m_wndPlaylist.GetItemData(m_wndPlaylist.GetCurSel()));

	if (SetupAccount())
	{
		LoadPlaylist();
	}
}

bool CIPTVChannelEditorDlg::SetupAccount()
{
	auto pSheet = std::make_unique<CResizedPropertySheet>(IDS_STRING_ACCOUNT_SETTINGS, REG_ACC_WINDOW_POS);
	pSheet->m_psh.dwFlags |= PSH_NOAPPLYNOW;
	pSheet->m_psh.dwFlags &= ~PSH_HASHELP;

	CAccessInfoDlg dlgInfo;
	dlgInfo.m_psp.dwFlags &= ~PSP_HASHELP;
	dlgInfo.m_initial_cred = m_cur_account;
	dlgInfo.m_all_channels_lists = m_all_channels_lists;

	pSheet->AddPage(&dlgInfo);
	auto res = (pSheet->DoModal() == IDOK);
	if (res)
	{
		m_cur_account = dlgInfo.m_initial_cred;

		GetConfig().UpdatePluginSettings();
		CollectCredentials();
	}

	return res;
}

void CIPTVChannelEditorDlg::FillTreePlaylist()
{
	m_bInFillTree = true;

	m_wndPlaylistTree.DeleteAllItems();

	m_playlistIds.clear();
	m_pl_categoriesTreeMap.clear();
	m_pl_categoriesMap.clear();
	m_playlistTreeMap.clear();

	// fill playlist tree
	const auto& pl_categories = FilterPlaylist();
	if (!m_playlistIds.empty())
	{
		for (auto& category : pl_categories)
		{
			TVINSERTSTRUCTW tvInsert = { nullptr };
			tvInsert.hParent = TVI_ROOT;
			tvInsert.item.pszText = (LPWSTR)category.c_str();
			tvInsert.item.lParam = (LPARAM)InfoType::enPlCategory;
			tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
			HTREEITEM hItem = m_wndPlaylistTree.InsertItem(&tvInsert);
			m_pl_categoriesTreeMap[category] = hItem;
			m_pl_categoriesMap[hItem] = category;
		}

		int step = 0;
		for (const auto& item : m_playlistIds)
		{
			auto pair = m_playlistMap.find(item);
			ASSERT(pair != m_playlistMap.end());
			if (pair == m_playlistMap.end()) continue;

			const auto& plEntry = pair->second;
			auto found = m_pl_categoriesTreeMap.find(plEntry->get_category());
			ASSERT(found != m_pl_categoriesTreeMap.end());
			if (found == m_pl_categoriesTreeMap.end()) continue;

			TVINSERTSTRUCTW tvInsert = { nullptr };
			tvInsert.hParent = found->second;
			tvInsert.item.pszText = (LPWSTR)plEntry->get_title().c_str();
			tvInsert.item.lParam = (LPARAM)InfoType::enPlEntry;
			tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
			HTREEITEM hItem = m_wndPlaylistTree.InsertItem(&tvInsert);
			m_playlistTreeMap.emplace(hItem, plEntry);
		}
	}

	UpdateChannelsTreeColors();
	CheckForExistingPlaylist();

	if (m_playlistIds.size() != m_playlistMap.size())
	{
		m_wndPlInfo.SetWindowText(fmt::format(load_string_resource(IDS_STRING_FMT_PLAYLIST_FLT),
											  m_plFileName.GetString(),
											  m_playlistIds.size(),
											  m_playlistMap.size()).c_str());
	}
	else
	{
		m_wndPlInfo.SetWindowText(fmt::format(load_string_resource(IDS_STRING_FMT_CHANNELS_ALL),
											  m_plFileName.GetString(),
											  m_playlistIds.size()).c_str());
	}

	m_bInFillTree = false;

	UpdateData(FALSE);
}

std::vector<std::wstring> CIPTVChannelEditorDlg::FilterPlaylist()
{
	// Filter out playlist
	BOOL bChanged = m_wndShowChanged.GetCheck();
	BOOL bNotAdded = m_wndNotAdded.GetCheck();

	std::array<BOOL, 2> bState         = { GetConfig().get_int(false, REG_FILTER_STATE_S), GetConfig().get_int(false, REG_FILTER_STATE_H) };
	std::array<BOOL, 2> bRegex         = { GetConfig().get_int(false, REG_FILTER_REGEX_S), GetConfig().get_int(false, REG_FILTER_REGEX_H) };
	std::array<BOOL, 2> bCase          = { GetConfig().get_int(false, REG_FILTER_CASE_S), GetConfig().get_int(false, REG_FILTER_CASE_H) };
	std::array<std::wstring, 2> filter = { GetConfig().get_string(false, REG_FILTER_STRING_S), GetConfig().get_string(false, REG_FILTER_STRING_H) };

	std::array<std::wregex, 2> re;
	for (int i = 0; i < 2; i++)
	{
		if (!bRegex[i]) continue;

		try
		{
			re[i] = filter[i];
		}
		catch (std::regex_error& ex)
		{
			ex;
			bRegex[i] = FALSE;
			filter[i].clear();
		}
	}

	// list of playlist categories in the same order as in the playlist
	// Must not contains duplicates!
	std::vector<std::wstring> pl_categories;
	if (m_playlistEntries)
	{
		// for fast search categories
		std::set<std::wstring> categories;

		std::vector<std::shared_ptr<PlaylistEntry>> entries;
		if (bChanged)
		{
			entries.reserve(m_changedChannels.size());
			for (const auto& entry : m_changedChannels)
			{
				entries.emplace_back(m_playlistMap[entry.first]);
			}
		}
		else
		{
			entries = m_playlistEntries->m_entries;
		}

		for (auto& entry : entries)
		{
			std::array<bool, 2> matched = { true, true };
			for (int i = 0; i < 2; i++)
			{
				if (!bState[i]) continue;

				if (bRegex[i])
				{
					try
					{
						matched[i] = std::regex_search(entry->get_title(), re[i]);
					}
					catch (std::regex_error& ex)
					{
						ex;
						matched[i] = true;
					}

					// second loop is not show. We must invert flag
					if (i != 0)
						matched[i] = !matched[i];
				}
				else if (!filter[i].empty())
				{
					if (bCase[i])
					{
						matched[i] = (entry->get_title().find(filter[i]) != std::wstring::npos);
					}
					else
					{
						matched[i] = (StrStrI(entry->get_title().c_str(), filter[i].c_str()) != nullptr);
					}

					// second loop is not show. We must invert flag
					if (i != 0)
						matched[i] = !matched[i];
				}
			}

			bool show = matched[0] && matched[1];
			if (bNotAdded)
			{
				show &= m_channelsMap.find(entry->stream_uri->get_parser().id) == m_channelsMap.end();
			}

			if (!show) continue;

			m_playlistIds.emplace_back(entry->stream_uri->get_parser().id);
			const auto& category = entry->get_category();
			if (categories.find(category) == categories.end())
			{
				pl_categories.emplace_back(category);
				categories.emplace(category);
			}
		}
	}

	return pl_categories;
}

void CIPTVChannelEditorDlg::OnSave()
{
	// Категория должна содержать хотя бы один канал. Иначе плагин падает с ошибкой
	// [plugin] error: invalid plugin TV info: wrong num_channels(0) for group id '' in num_channels_by_group_id.

	int lst_idx = GetConfig().get_int(false, REG_CHANNELS_TYPE);
	if (lst_idx == -1 || m_all_channels_lists.empty())
	{
		const auto& plugin_name = GetConfig().GetCurrentPluginName();
		const auto& list_name = fmt::format(L"{:s}_channel_list.xml", plugin_name);
		const auto& list_path = fmt::format(L"{:s}{:s}\\", GetConfig().get_string(true, REG_LISTS_PATH), plugin_name);
		std::error_code err;
		std::filesystem::create_directory(list_path, err);

		m_all_channels_lists.emplace_back(list_name);
		lst_idx = m_wndChannels.AddString(list_name.c_str());
		m_wndChannels.SetCurSel(lst_idx);
		GetConfig().set_int(false, REG_CHANNELS_TYPE, lst_idx);
	}

	// renumber categories id
	LPCWSTR old_selected = nullptr;
	const auto& channel = FindChannel(m_wndChannelsTree.GetSelectedItem());
	if (channel)
		old_selected = channel->stream_uri->get_parser().id.c_str();

	try
	{
		// create document;
		auto doc = std::make_unique<rapidxml::xml_document<>>();
		auto decl = doc->allocate_node(rapidxml::node_declaration);

		// adding attributes at the top of our xml
		decl->append_attribute(doc->allocate_attribute("version", "1.0"));
		decl->append_attribute(doc->allocate_attribute("encoding", "UTF-8"));
		doc->append_node(decl);

		// create <tv_info> root node
		auto tv_info = doc->allocate_node(rapidxml::node_element, utils::TV_INFO);

		auto info_node = doc->allocate_node(rapidxml::node_element, utils::VERSION_INFO);
		info_node->append_node(rapidxml::alloc_node(*doc, utils::LIST_VERSION, std::to_string(CHANNELS_LIST_VERSION).c_str()));
		tv_info->append_node(info_node);

		// append <tv_category> to <tv_categories> node
		auto cat_node = doc->allocate_node(rapidxml::node_element, utils::TV_CATEGORIES);
		for (auto& category : m_categoriesMap)
		{
			if (!category.second.category->get_channels().empty())
			{
				cat_node->append_node(category.second.category->GetNode(*doc));
			}
		}
		// append <tv_categories> to <tv_info> node
		tv_info->append_node(cat_node);

		// create <tv_channels> node
		auto ch_node = doc->allocate_node(rapidxml::node_element, utils::TV_CHANNELS);
		// append <tv_channel> to <v_channels> node
		for (const auto& pair : m_categoriesMap)
		{
			for (auto& channel : pair.second.category->get_channels())
			{
				channel->get_category_ids().clear();
				channel->get_category_ids().emplace(pair.first);
				ch_node->append_node(channel->GetNode(*doc));
			}
		}

		// append <tv_channel> to <tv_info> node
		tv_info->append_node(ch_node);

		doc->append_node(tv_info);
		// write document
		auto& channelsPath = fmt::format(L"{:s}{:s}\\", GetConfig().get_string(true, REG_LISTS_PATH), GetConfig().GetCurrentPluginName());
		std::error_code err;
		std::filesystem::create_directories(channelsPath, err);

		channelsPath += m_all_channels_lists[lst_idx];

		std::ofstream os(channelsPath, std::istream::binary);
		os << *doc;

		set_allow_save(FALSE);
		FillTreeChannels(old_selected);
	}
	catch (const rapidxml::parse_error& ex)
	{
		CString error(ex.what());
		AfxMessageBox(error, MB_OK | MB_ICONERROR);
	}
	catch (const std::exception& ex)
	{
		CString error(ex.what());
		AfxMessageBox(error, MB_OK | MB_ICONERROR);
	}
}

void CIPTVChannelEditorDlg::OnUpdateSave(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(is_allow_save());
}

void CIPTVChannelEditorDlg::OnNewCategory()
{
	auto categoryId = GetNewCategoryID();
	auto newCategory = std::make_shared<ChannelCategory>(StreamType::enBase, GetAppPath(utils::PLUGIN_ROOT));
	newCategory->set_key(categoryId);
	newCategory->set_title(L"New Category");
	newCategory->set_icon_uri(utils::ICON_TEMPLATE);

	SetImageControl(GetIconCache().get_icon(newCategory->get_icon_absolute_path()), m_wndChannelIcon);

	TVINSERTSTRUCTW tvInsert = { nullptr };
	tvInsert.hParent = TVI_ROOT;
	tvInsert.item.pszText = (LPWSTR)newCategory->get_title().c_str();
	tvInsert.item.lParam = (LPARAM)InfoType::enCategory;
	tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
	auto hNewItem = m_wndChannelsTree.InsertItem(&tvInsert);

	CategoryInfo info = { hNewItem, newCategory };
	m_categoriesMap.emplace(categoryId, info);
	m_categoriesTreeMap.emplace(hNewItem, categoryId);

	m_wndChannelsTree.SelectItem(hNewItem);
	m_wndChannelsTree.EditLabel(hNewItem);
}

void CIPTVChannelEditorDlg::OnUpdateNewCategory(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CIPTVChannelEditorDlg::OnSortCategory()
{
	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		const auto& category = GetCategory(hItem);
		if (!category) continue;

		category->sort_channels();

		const auto& channels = category->get_channels();
		auto it = channels.begin();
		for (auto hChildItem = m_wndChannelsTree.GetChildItem(hItem); hChildItem != nullptr; hChildItem = m_wndChannelsTree.GetNextSiblingItem(hChildItem))
		{
			m_wndChannelsTree.SetItemText(hChildItem, (*it)->get_title().c_str());
			m_channelsTreeMap[hChildItem] = *it;
			++it;
		}
	}

	UpdateChannelsTreeColors();
	set_allow_save();
}

void CIPTVChannelEditorDlg::OnUpdateSortCategory(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsSelectedCategory() && IsSelectedNotFavorite());
}

void CIPTVChannelEditorDlg::OnStnClickedStaticIcon()
{
	UpdateData(TRUE);

	auto hCur = m_wndChannelsTree.GetSelectedItem();
	auto info = GetBaseInfo(&m_wndChannelsTree, hCur);
	if (!info || info->get_key() == ID_ADD_TO_FAVORITE)
		return;

	bool save = false;
	int idx = m_wndIconSource.GetCurSel();
	if (idx == 0)
	{
		CFileDialog dlg(TRUE);
		CString curPath = GetAppPath(IsChannel(hCur) ? utils::CHANNELS_LOGO_PATH : utils::CATEGORIES_LOGO_PATH).c_str();
		CString file(curPath);
		file.Replace('/', '\\');

		CString filter;
		filter.LoadString(IDS_STRING_LOAD_ICON);
		filter.Replace('|', '\0');

		CString title;
		title.LoadString(IDS_STRING_LOAD_ICONS_TITLE);

		OPENFILENAME& oFN = dlg.GetOFN();
		oFN.lpstrFilter = filter.GetString();
		oFN.nMaxFile = MAX_PATH;
		oFN.nFilterIndex = 0;
		oFN.lpstrFile = file.GetBuffer(MAX_PATH);
		oFN.lpstrTitle = title.GetString();
		oFN.lpstrInitialDir = curPath.GetString();
		oFN.Flags |= OFN_EXPLORER | OFN_NOREADONLYRETURN | OFN_ENABLESIZING | OFN_LONGNAMES | OFN_PATHMUSTEXIST;
		oFN.Flags |= OFN_FILEMUSTEXIST | OFN_NONETWORKBUTTON | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT | OFN_NODEREFERENCELINKS;

		dlg.ApplyOFNToShellDialog();
		INT_PTR nResult = dlg.DoModal();
		file.ReleaseBuffer();

		if (nResult == IDOK)
		{
			if (!utils::is_ascii(oFN.lpstrFileTitle))
			{
				AfxMessageBox(IDS_STRING_WRN_NON_ASCII, MB_ICONERROR | MB_OK);
				return;
			}

			std::filesystem::path newPath(file.GetString());
			if (curPath.CompareNoCase(newPath.parent_path().wstring().c_str()) != 0)
			{
				curPath += oFN.lpstrFileTitle;
				std::error_code err;
				std::filesystem::copy_file(file.GetString(), curPath.GetString(), std::filesystem::copy_options::overwrite_existing, err);
				SetImageControl(GetIconCache().get_icon(curPath.GetString()), m_wndChannelIcon);
			}

			m_iconUrl = uri_base::PLUGIN_SCHEME;
			m_iconUrl += IsChannel(hCur) ? utils::CHANNELS_LOGO_URL : utils::CATEGORIES_LOGO_URL;
			m_iconUrl += oFN.lpstrFileTitle;

			if (m_iconUrl != info->get_icon_uri().get_uri().c_str())
			{
				info->set_icon_uri(m_iconUrl.GetString());
				save = true;
			}
		}
	}
	else if (idx == 1)
	{
		CIconLinkDlg dlg;
		dlg.m_url = info->get_icon_absolute_path().c_str();
		if (dlg.DoModal() == IDOK)
		{
			if (dlg.m_url != info->get_icon_uri().get_uri().c_str())
			{
				info->set_icon_uri(dlg.m_url.GetString());
				save = true;
			}
		}
	}
	else if (idx == 2)
	{
		CIconsListDlg dlg(m_Icons, L"http://epg.it999.ru/edem_epg_ico2.m3u8");
		dlg.m_selected = m_lastIconSelected;
		dlg.m_search = info->get_title().c_str();

		if (dlg.DoModal() == IDOK)
		{
			const auto& choosed = m_Icons->m_entries[dlg.m_selected];
			if (m_iconUrl != choosed->get_icon_uri().get_uri().c_str())
			{
				info->set_icon_uri(choosed->get_icon_uri());
				m_lastIconSelected = dlg.m_selected;
				save = true;
			}
		}
	}

	if (save)
	{
		const auto& img = GetIconCache().get_icon(info->get_icon_absolute_path());
		SetImageControl(img, m_wndChannelIcon);

		UpdateChannelsTreeColors(m_wndChannelsTree.GetParentItem(m_wndChannelsTree.GetSelectedItem()));
		CheckForExistingPlaylist();
		set_allow_save();
	}

	UpdateData(FALSE);
}

void CIPTVChannelEditorDlg::OnBnClickedCheckShowUrl()
{
	GetConfig().set_int(true, REG_SHOW_URL, m_wndShowUrl.GetCheck());
	LoadChannelInfo();
}

void CIPTVChannelEditorDlg::OnBnClickedButtonAbout()
{
	CAboutDlg dlg;
	dlg.DoModal();
}

void CIPTVChannelEditorDlg::OnBnClickedButtonPack()
{
	if (CheckForSave())
	{
		PackPlugin(m_plugin_type, true, m_wndMakeWebUpdate.GetCheck());
	}
}

void CIPTVChannelEditorDlg::OnMakeAll()
{
	if (!CheckForSave())
		return;

	CWaitCursor cur;
	bool success = true;
	int i = 0;
	m_wndProgress.ShowWindow(SW_SHOW);
	m_wndProgressInfo.ShowWindow(SW_SHOW);
	m_wndProgress.SetRange32(0, (int)GetConfig().get_plugins_info().size());
	m_wndProgress.SetPos(i);

	for (const auto& item : GetConfig().get_plugins_info())
	{
		m_wndProgressInfo.SetWindowText(item.title.c_str());
		m_wndProgress.SetPos(++i);

		if (!PackPlugin(item.type, false, m_wndMakeWebUpdate.GetCheck()))
		{
			success = false;
			CString str;
			str.Format(IDS_STRING_ERR_FAILED_PACK_PLUGIN, item.title.c_str());
			if (IDNO == AfxMessageBox(str, MB_YESNO)) break;
		}
	}

	m_wndProgress.ShowWindow(SW_HIDE);
	m_wndProgressInfo.ShowWindow(SW_HIDE);

	if (success)
		AfxMessageBox(IDS_STRING_INFO_CREATE_ALL_SUCCESS, MB_OK);
}

void CIPTVChannelEditorDlg::OnMakeAllAccounts()
{
	if (!CheckForSave())
		return;

	std::set<std::string> suffixes;
	for (const auto& cred : m_all_credentials)
	{
		if (!suffixes.emplace(cred.suffix).second)
		{
			if (IDNO == AfxMessageBox(IDS_STRING_ERR_SAME_SUFFIX, MB_ICONEXCLAMATION|MB_YESNO)) return;

			break;
		}
	}

	CWaitCursor cur;

	const auto& old_selected = GetConfig().get_int(false, REG_ACTIVE_ACCOUNT);

	m_wndProgress.ShowWindow(SW_SHOW);
	m_wndProgressInfo.ShowWindow(SW_SHOW);
	m_wndProgress.SetRange32(0, m_all_credentials.size());
	m_wndProgress.SetPos(0);
	bool success = true;
	int i = 0;
	for (const auto& cred : m_all_credentials)
	{
		GetConfig().set_int(false, REG_ACTIVE_ACCOUNT, i);
		m_wndProgress.SetPos(++i);
		const auto& title = fmt::format(L"#{:d}", i);
		m_wndProgressInfo.SetWindowText(title.c_str());

		if (!PackPlugin(m_plugin_type, false, m_wndMakeWebUpdate.GetCheck()))
		{
			success = false;
			CString str;
			str.Format(IDS_STRING_ERR_FAILED_PACK_PLUGIN, title.c_str());
			if (IDNO == AfxMessageBox(str, MB_ICONEXCLAMATION|MB_YESNO)) break;
		}
	}

	GetConfig().set_int(false, REG_ACTIVE_ACCOUNT, old_selected);
	m_wndProgress.ShowWindow(SW_HIDE);
	m_wndProgressInfo.ShowWindow(SW_HIDE);

	if (success)
		AfxMessageBox(IDS_STRING_INFO_CREATE_ALL_SUCCESS, MB_OK);
}

void CIPTVChannelEditorDlg::OnMakeAccount(UINT id)
{
	if (CheckForSave())
	{
		const auto& old_selected = GetConfig().get_int(false, REG_ACTIVE_ACCOUNT);

		GetConfig().set_int(false, REG_ACTIVE_ACCOUNT, id - ID_ACCOUNT_TO_START);
		PackPlugin(m_plugin_type, true, m_wndMakeWebUpdate.GetCheck());
		GetConfig().set_int(false, REG_ACTIVE_ACCOUNT, old_selected);
	}
}

void CIPTVChannelEditorDlg::OnRestore()
{
	m_wndTrayIcon.MaxFromTray(this);
}

void CIPTVChannelEditorDlg::OnAppExit()
{
	// post a message to ourself to close
	m_wndTrayIcon.RemoveIcon();
	m_wndTrayIcon.DestroyWindow();
	::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
}

void CIPTVChannelEditorDlg::OnBnClickedButtonStop()
{
	m_evtStop.SetEvent();
	m_wndStop.EnableWindow(FALSE);
}

void CIPTVChannelEditorDlg::OnUpdateButtonSearchNext(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_search.IsEmpty());
}

void CIPTVChannelEditorDlg::OnBnClickedButtonSearchNext()
{
	SearchTreeItem(InfoType::enChannel, true);
}

void CIPTVChannelEditorDlg::OnUpdateButtonPlSearchNext(CCmdUI* pCmdUI)
{
	UpdateData(TRUE);

	pCmdUI->Enable(!m_plSearch.IsEmpty());
}

void CIPTVChannelEditorDlg::OnBnClickedButtonPlSearchNext()
{
	SearchTreeItem(InfoType::enPlEntry, true);
}

void CIPTVChannelEditorDlg::OnBnClickedCheckShowChanged()
{
	m_wndShowChanged.SetWindowText(m_wndShowChanged.GetCheck() ? _T("\u2260") : _T("="));
	FillTreePlaylist();
}

void CIPTVChannelEditorDlg::OnBnClickedCheckShowChangedCh()
{
	m_wndShowChangedCh.SetWindowText(m_wndShowChangedCh.GetCheck() ? _T("\u2260") : _T("="));
	FillTreeChannels();
}

void CIPTVChannelEditorDlg::OnBnClickedCheckNotAdded()
{
	m_wndNotAdded.SetWindowText(m_wndNotAdded.GetCheck() ? _T("\u2179") : _T("\u221A"));
	FillTreePlaylist();
}

void CIPTVChannelEditorDlg::OnBnClickedCheckShowUnknown()
{
	FillTreeChannels();
}

bool CIPTVChannelEditorDlg::IsChannel(HTREEITEM hItem) const
{
	return hItem && m_wndChannelsTree.GetItemData(hItem) == (DWORD_PTR)InfoType::enChannel;
}

bool CIPTVChannelEditorDlg::IsCategory(HTREEITEM hItem) const
{
	return hItem && m_wndChannelsTree.GetItemData(hItem) == (DWORD_PTR)InfoType::enCategory;
}

bool CIPTVChannelEditorDlg::IsPlaylistEntry(HTREEITEM hItem) const
{
	return hItem && m_wndPlaylistTree.GetItemData(hItem) == (DWORD_PTR)InfoType::enPlEntry;
}

bool CIPTVChannelEditorDlg::IsPlaylistCategory(HTREEITEM hItem) const
{
	return hItem && m_wndPlaylistTree.GetItemData(hItem) == (DWORD_PTR)InfoType::enPlCategory;
}

void CIPTVChannelEditorDlg::OnAddUpdateChannel()
{
	CWaitCursor cur;

	bool needCheckExisting = false;
	if (m_lastTree == &m_wndPlaylistTree)
	{
		for (const auto& hSelectedItem : m_wndPlaylistTree.GetSelectedItems())
		{
			needCheckExisting |= AddChannel(FindEntry(hSelectedItem));
		}
	}
	else
	{
		for (const auto& hSelectedItem : m_wndChannelsTree.GetSelectedItems())
		{
			auto info = GetBaseInfo(&m_wndChannelsTree, hSelectedItem);
			if (!info) continue;

			SearchParams params;
			params.id = info->stream_uri->get_parser().id;
			params.type = InfoType::enPlEntry;
			params.select = false;
			needCheckExisting |= AddChannel(FindEntry(SelectTreeItem(&m_wndPlaylistTree, params)));
		}
	}

	OnSyncTreeItem();

	if (needCheckExisting)
	{
		UpdateChannelsTreeColors();
		CheckForExistingPlaylist();
		UpdateControlsForItem();
	}

	set_allow_save();
}

void CIPTVChannelEditorDlg::OnUpdateAddUpdateChannel(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsSelectedChannelsOrEntries());
}

void CIPTVChannelEditorDlg::OnBnClickedButtonSettings()
{
	CPropertySheet sheet;

	CMainSettingsPage dlg1;
	CPathsSettingsPage dlg2;
	CUpdateSettingsPage dlg3;
	sheet.AddPage(&dlg1);
	sheet.AddPage(&dlg2);
	sheet.AddPage(&dlg3);

	std::wstring old_list = GetConfig().get_string(true, REG_LISTS_PATH);
	int old_flags = GetConfig().get_int(true, REG_CMP_FLAGS);
	int old_update = GetConfig().get_int(true, REG_UPDATE_FREQ);
	int old_portable = GetConfig().IsPortable();

	if (sheet.DoModal() == IDOK)
	{
		if (old_portable != GetConfig().IsPortable())
		{
			if (GetConfig().IsPortable())
			{
				GetConfig().SaveSettingsToJson();
			}
			else
			{
				GetConfig().SaveSettingsToRegistry();
				GetConfig().RemovePortableSettings();
			}
			UpdateWindowTitle();
		}
		else
		{
			GetConfig().SaveSettings();
		}

		if (old_list != GetConfig().get_string(true, REG_LISTS_PATH))
		{
			SwitchPlugin();
		}

		if (old_flags != GetConfig().get_int(true, REG_CMP_FLAGS))
		{
			UpdateChannelsTreeColors();
		}

		if (old_update != GetConfig().get_int(true, REG_NEXT_UPDATE))
		{
			GetConfig().set_int64(true, REG_NEXT_UPDATE, 0);
		}

		m_colorAdded = GetConfig().get_int(true, REG_COLOR_ADDED);
		m_colorNotAdded = GetConfig().get_int(true, REG_COLOR_NOT_ADDED);
		m_colorChanged = GetConfig().get_int(true, REG_COLOR_CHANGED);
		m_colorHEVC = GetConfig().get_int(true, REG_COLOR_HEVC);
		UpdateChannelsTreeColors();
		CheckForExistingPlaylist();
	}
}

void CIPTVChannelEditorDlg::OnBnClickedButtonCacheIcon()
{
	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		const auto& channel = FindChannel(hItem);
		if (!channel) continue;

		auto fname = channel->get_icon_uri().get_path();
		auto pos = fname.rfind('/');
		if (pos == std::string::npos) continue;

		fname = fname.substr(pos + 1);
		std::wstring path = utils::CHANNELS_LOGO_URL;
		path += fname;

		uri_base icon_uri;
		icon_uri.set_uri(utils::ICON_TEMPLATE);
		icon_uri.set_path(path);

		std::vector<BYTE> image;
		if (!utils::DownloadFile(channel->get_icon_uri().get_uri(), image)) continue;

		channel->set_icon_uri(icon_uri.get_uri());

		const auto& fullPath = icon_uri.get_filesystem_path(GetAppPath(utils::PLUGIN_ROOT));
		std::ofstream os(fullPath.c_str(), std::ios::out | std::ios::binary);
		os.write((char*)&image[0], image.size());
		os.close();

		LoadChannelInfo(hItem);
		set_allow_save();
	}
}

void CIPTVChannelEditorDlg::OnUpdateButtonCacheIcon(CCmdUI* pCmdUI)
{
	BOOL enable = FALSE;
	const auto& channel = FindChannel(m_wndChannelsTree.GetSelectedItem());
	pCmdUI->Enable(channel && !channel->is_icon_local());
}

void CIPTVChannelEditorDlg::OnTvnSelchangedTreePaylist(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (m_bInFillTree)
		return;

	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	*pResult = 0;

	BOOL enable = FALSE;
	if (m_wndPlaylistTree.GetSelectedCount() == 1)
	{
		const auto& channel = FindChannel(m_wndChannelsTree.GetSelectedItem());
		const auto& entry = FindEntry(m_wndPlaylistTree.GetSelectedItem());
		if (channel && entry)
		{
			enable = !channel->get_icon_uri().is_equal(entry->get_icon_uri());
		}
	}

	LoadPlayListInfo(pNMTreeView->itemNew.hItem);

	OnSyncTreeItem();
}

void CIPTVChannelEditorDlg::OnBnClickedButtonCreateNewChannelsList()
{
	if (!CheckForSave())
		return;

	const auto& pluginName = GetConfig().GetCurrentPluginName();
	int cnt = 2;
	std::wstring newListName;
	for(;;)
	{
		bool found = false;
		newListName = fmt::format(L"{:s}_channel_list{:d}.xml", pluginName, cnt++);
		for (const auto& item : m_all_channels_lists)
		{
			if (StrCmpI(item.c_str(), newListName.c_str()) == 0)
			{
				found = true;
				break;
			}
		}

		if (!found) break;
	}

	CNewChannelsListDlg dlg;
	dlg.m_name = newListName.c_str();

	if (dlg.DoModal() != IDOK)
		return;

	int idx = 0;
	for (const auto& item : m_all_channels_lists)
	{
		if (StrCmpI(newListName.c_str(), item.c_str()) == 0)
		{
			m_wndChannels.SetCurSel(idx);
			OnCbnSelchangeComboChannels();
			return;
		}
		idx++;
	}

	auto& newList = fmt::format(L"{:s}{:s}\\", GetConfig().get_string(true, REG_LISTS_PATH), pluginName);
	std::filesystem::create_directory(newList);

	m_channelsMap.clear();
	m_categoriesMap.clear();

	m_all_channels_lists.emplace_back(dlg.m_name.GetString());
	idx = m_wndChannels.AddString(dlg.m_name);
	m_wndChannels.SetCurSel(idx);
	GetConfig().set_int(false, REG_CHANNELS_TYPE, idx);
	m_wndChannels.EnableWindow(m_all_channels_lists.size() > 1);

	OnSave();
	FillTreeChannels();
}

void CIPTVChannelEditorDlg::OnGetStreamInfo()
{
	if (!m_lastTree)
		return;

	bool isChannelsTree = (m_lastTree == &m_wndChannelsTree);
	auto container = std::make_unique<std::vector<uri_stream*>>();
	for (auto hItem = m_lastTree->GetFirstSelectedItem(); hItem != nullptr; hItem = m_lastTree->GetNextSelectedItem(hItem))
	{
		if (isChannelsTree)
		{
			if (IsChannel(hItem))
			{
				auto info = FindChannel(hItem);
				if (info)
					container->emplace_back(info->stream_uri.get());
			}
			else
			{
				for (const auto& item : GetItemCategory(hItem)->get_channels())
				{
					if (item)
						container->emplace_back(item->stream_uri.get());
				}
			}
		}
		else
		{
			if (IsPlaylistEntry(hItem))
			{
				auto info = FindEntry(hItem);
				if (info)
					container->emplace_back(info->stream_uri.get());
			}
			else
			{
				const auto& pair = m_pl_categoriesMap.find(hItem);
				if (pair == m_pl_categoriesMap.end()) continue;

				const auto& category = pair->second;
				for (auto hChildItem = m_lastTree->GetChildItem(hItem); hChildItem != nullptr; hChildItem = m_lastTree->GetNextSiblingItem(hChildItem))
				{
					const auto& pairEntry = m_playlistTreeMap.find(hChildItem);
					if (pairEntry == m_playlistTreeMap.end() || category != pairEntry->second->get_category()) continue;

					auto info = dynamic_cast<BaseInfo*>(pairEntry->second.get());
					if (info)
						container->emplace_back(info->stream_uri.get());
				}
			}
		}
	}

	if (container->empty())
		return;

	container->erase(std::unique(container->begin(), container->end()), container->end());
	if (container->empty())
		return;

	for (auto& item : *container)
	{
		UpdateExtToken(item);
	}

	m_wndPluginType.EnableWindow(FALSE);
	m_wndChannels.EnableWindow(FALSE);
	m_wndPlaylist.EnableWindow(FALSE);
	m_wndAccountSetting.EnableWindow(FALSE);
	m_wndDownloadUrl.EnableWindow(FALSE);
	m_wndSettings.EnableWindow(FALSE);

	m_evtStop.ResetEvent();
	m_wndStop.EnableWindow(TRUE);
	m_wndProgressInfo.ShowWindow(SW_SHOW);

	m_inStreamInfo = true;

	CGetStreamInfoThread::ThreadConfig cfg;
	cfg.m_parent = this;
	cfg.m_container = container.release();
	cfg.m_hStop = m_evtStop;
	cfg.m_probe = GetConfig().get_string(true, REG_FFPROBE);
	cfg.m_max_threads = GetConfig().get_int(true, REG_MAX_THREADS, 3);
	cfg.m_params.subdomain = m_cur_account.get_domain();
	cfg.m_params.token = m_cur_account.get_token();
	cfg.m_params.login = m_cur_account.get_login();
	cfg.m_params.password = m_cur_account.get_password();
	cfg.m_params.shift_back = 0;
	if (cfg.m_container->front()->get_server_subst_type() == ServerSubstType::enStream)
	{
		cfg.m_params.server = m_cur_account.device_id;
	}

	auto* pThread = (CGetStreamInfoThread*)AfxBeginThread(RUNTIME_CLASS(CGetStreamInfoThread), THREAD_PRIORITY_HIGHEST, 0, CREATE_SUSPENDED);
	if (!pThread)
	{
		AfxMessageBox(IDS_STRING_ERR_THREAD_NOT_START, MB_OK | MB_ICONERROR);
		OnEndGetStreamInfo();
		return;
	}

	pThread->SetData(cfg);
	pThread->ResumeThread();
}

void CIPTVChannelEditorDlg::OnUpdateGetStreamInfo(CCmdUI* pCmdUI)
{
	bool enable = !GetConfig().get_string(true, REG_FFPROBE).empty()
		&& !m_loading
		&& !m_inStreamInfo;

	if (m_lastTree)
	{
		HTREEITEM first = m_lastTree->GetFirstSelectedItem();
		enable = enable && IsSelectedTheSameType(m_lastTree);
	}
	else
	{
		enable = false;
	}

	pCmdUI->Enable(enable);
}

void CIPTVChannelEditorDlg::OnClearStreamInfo()
{
	if (!m_lastTree)
		return;

	bool isChannelsTree = (m_lastTree == &m_wndChannelsTree);
	std::set<int> to_erase;
	for (auto hItem = m_lastTree->GetFirstSelectedItem(); hItem != nullptr; hItem = m_lastTree->GetNextSelectedItem(hItem))
	{
		if (isChannelsTree)
		{
			if (!IsChannel(hItem))
			{
				for (const auto& item : GetItemCategory(hItem)->get_channels())
				{
					if (item != nullptr)
					{
						to_erase.emplace(item->stream_uri->get_hash());
					}
				}
			}
			else if (auto info = FindChannel(hItem); info != nullptr)
			{
				to_erase.emplace(info->stream_uri->get_hash());
			}
		}
		else if (IsPlaylistEntry(hItem))
		{
			if (auto info = FindEntry(hItem); info != nullptr)
			{
				to_erase.emplace(info->stream_uri->get_hash());
			}
		}
		else
		{
			const auto& pair = m_pl_categoriesMap.find(hItem);
			if (pair == m_pl_categoriesMap.end()) continue;

			const auto& category = pair->second;
			for (auto hChildItem = m_lastTree->GetChildItem(hItem); hChildItem != nullptr; hChildItem = m_lastTree->GetNextSiblingItem(hChildItem))
			{
				const auto& pairEntry = m_playlistTreeMap.find(hChildItem);
				if (pairEntry == m_playlistTreeMap.end() || category != pairEntry->second->get_category()) continue;

				if (auto info = dynamic_cast<BaseInfo*>(pairEntry->second.get()); info != nullptr)
				{
					to_erase.emplace(info->stream_uri->get_hash());
				}
			}
		}
	}

	for (const auto& item : to_erase)
	{
		m_stream_infos.erase(item);
	}

	SaveStreamInfo();

	LoadChannelInfo();
	LoadPlayListInfo();
}

void CIPTVChannelEditorDlg::OnUpdateClearStreamInfo(CCmdUI* pCmdUI)
{
	bool enable = !m_loading;
	if (m_lastTree)
	{
		enable = enable && IsSelectedTheSameType(m_lastTree);
	}
	else
	{
		enable = false;
	}

	pCmdUI->Enable(enable);
}

LRESULT CIPTVChannelEditorDlg::OnTrayIconNotify(WPARAM /*wParam*/, LPARAM lParam)
{
	//UINT uID       = (UINT)wParam; // resource ID of the tray icon.
	UINT uMouseMsg = (UINT)lParam; // mouse message that was sent.

	// We can let the tray icon control handle our context menu and
	// mouse double click events, but we want handle our balloon tip
	// notifications, so we will return 1 to let the tray icon control
	// know that we have handled these messages already...

	switch (uMouseMsg)
	{
		// Sent when the balloon is shown (balloons are queued).
		case NIN_BALLOONSHOW:
		return 1;

		// Sent when the balloon disappears-for example, when the
		// icon is deleted. This message is not sent if the balloon
		// is dismissed because of a timeout or a mouse click.
		case NIN_BALLOONHIDE:
		return 1;

		// Sent when the balloon is dismissed because of a timeout.
		case NIN_BALLOONTIMEOUT:
		return 1;

		// Sent when the balloon is dismissed because of a mouse click.
		case NIN_BALLOONUSERCLICK:
		return 1;

		case WM_RBUTTONUP:
		{
			CMenu menu;
			if (!menu.LoadMenu(IDR_POPUP_TRAY))
			{
				return 0;
			}

			CMenu* pSubMenu = menu.GetSubMenu(0);
			if (pSubMenu == nullptr)
			{
				return 0;
			}

			::SetMenuDefaultItem(pSubMenu->m_hMenu, ID_RESTORE, FALSE);

			CPoint pos;
			GetCursorPos(&pos);
			::SetForegroundWindow(m_hWnd);

			::TrackPopupMenu(pSubMenu->m_hMenu, 0, pos.x, pos.y, 0, GetSafeHwnd(), nullptr);

			::PostMessage(m_hWnd, WM_NULL, 0, 0);

			menu.DestroyMenu();
		}
		return 1;
	}

	return 0;
}

void CIPTVChannelEditorDlg::OnPlayStream()
{
	if (m_lastTree)
	{
		PlayItem(m_lastTree->GetSelectedItem());
	}
}

void CIPTVChannelEditorDlg::OnUpdatePlayStream(CCmdUI* pCmdUI)
{
	bool enable = (m_wndChannelsTree.GetSelectedCount() == 1 && IsChannel(m_wndChannelsTree.GetSelectedItem()))
		|| (m_wndPlaylistTree.GetSelectedCount() == 1 && IsPlaylistEntry(m_wndPlaylistTree.GetSelectedItem()));

	pCmdUI->Enable(!GetConfig().get_string(true, REG_PLAYER).empty() && enable);
}

void CIPTVChannelEditorDlg::OnBnClickCheckArchive()
{
	UpdateData(TRUE);

	if (m_lastTree)
	{
		PlayItem(m_lastTree->GetSelectedItem(), m_archiveCheckHours, m_archiveCheckDays);
	}
}

void CIPTVChannelEditorDlg::OnSyncTreeItem()
{
	if (m_loading || !m_lastTree || m_inSync || !GetConfig().get_int(true, REG_AUTO_SYNC))
		return;

	m_inSync = true;

	CWaitCursor cur;
	SearchParams params;

	auto info = GetBaseInfo(m_lastTree, m_lastTree->GetSelectedItem());
	if (info)
	{
		params.id = info->stream_uri->get_parser().id;
		params.next = true;
		if (m_lastTree == &m_wndPlaylistTree && m_channelsMap.find(params.id) != m_channelsMap.end() ||
			m_lastTree == &m_wndChannelsTree && m_playlistMap.find(params.id) != m_playlistMap.end())
		{
			CTreeCtrlEx* opposite = nullptr;
			if (m_lastTree == &m_wndPlaylistTree)
			{
				opposite = &m_wndChannelsTree;
			}
			else
			{
				opposite = &m_wndPlaylistTree;
				params.type = InfoType::enPlEntry;
			}

			SelectTreeItem(opposite, params);
		}
	}

	m_inSync = false;
}

void CIPTVChannelEditorDlg::OnUpdateSyncTreeItem(CCmdUI* pCmdUI)
{
	BOOL enable = (!m_loading
				   && m_lastTree
				   && m_lastTree->GetSelectedCount() == 1
				   && GetBaseInfo(m_lastTree, m_lastTree->GetSelectedItem()) != nullptr
				   && !GetConfig().get_int(true, REG_AUTO_SYNC));

	pCmdUI->Enable(enable);
}

void CIPTVChannelEditorDlg::OnToggleChannel()
{
	const auto& selected = m_wndChannelsTree.GetSelectedItems();
	for (const auto& hItem : selected)
	{
		if (const auto& channel = FindChannel(hItem); channel != nullptr)
		{
			channel->set_disabled(!m_menu_enable_channel);
			m_wndChannelsTree.SetItemColor(hItem, channel->is_disabled() ? m_gray : m_normal);
			set_allow_save();
		}
	}

	UpdateChannelsTreeColors(m_wndChannelsTree.GetParentItem(selected.front()));
}

void CIPTVChannelEditorDlg::OnUpdateToggleChannel(CCmdUI* pCmdUI)
{
	BOOL enable = FALSE;
	if (IsSelectedChannelsOrEntries() && IsSelectedNotFavorite())
	{
		m_menu_enable_channel = false;
		for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
		{
			if (const auto& channel = FindChannel(hItem); channel != nullptr)
			{
				m_menu_enable_channel |= channel->is_disabled();
				enable = TRUE;
			}
		}
	}

	if (m_menu_enable_channel)
	{
		pCmdUI->SetText(load_string_resource(IDS_STRING_ENABLE_CHANNEL).c_str());
	}

	pCmdUI->Enable(enable);
}

void CIPTVChannelEditorDlg::OnToggleCategory()
{
	const auto& selected = m_wndChannelsTree.GetSelectedItems();
	for (const auto& hItem : selected)
	{
		if (const auto& category = FindCategory(hItem); category != nullptr)
		{
			category->set_disabled(!m_menu_enable_category);
			m_wndChannelsTree.SetItemColor(hItem, category->is_disabled() ? m_gray : m_normal);
			set_allow_save();
		}
	}

	UpdateChannelsTreeColors(m_wndChannelsTree.GetParentItem(selected.front()));
}

void CIPTVChannelEditorDlg::OnUpdateToggleCategory(CCmdUI* pCmdUI)
{
	BOOL enable = FALSE;
	if (IsSelectedCategory() && IsSelectedNotFavorite())
	{
		m_menu_enable_category = false;
		for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
		{
			if (const auto& category = FindCategory(hItem); category != nullptr)
			{
				m_menu_enable_category |= category->is_disabled();
				enable = TRUE;
			}
		}
	}

	if (m_menu_enable_category)
	{
		pCmdUI->SetText(load_string_resource(IDS_STRING_ENABLE_CATEGORY).c_str());
	}
	pCmdUI->Enable(enable);
}

void CIPTVChannelEditorDlg::OnBnClickedButtonDownloadPlaylist()
{
	LoadPlaylist(true);
}

void CIPTVChannelEditorDlg::OnCbnSelchangeComboIconSource()
{
	GetConfig().set_int(true, REG_ICON_SOURCE, m_wndIconSource.GetCurSel());
}

void CIPTVChannelEditorDlg::OnCbnSelchangeComboPluginType()
{
	if (!CheckForSave())
	{
		m_wndPluginType.SetCurSel(GetConfig().get_plugin_idx());
		return;
	}

	GetConfig().UpdatePluginSettings();

	GetConfig().set_plugin_idx(m_wndPluginType.GetCurSel());

	set_allow_save(FALSE);
	SwitchPlugin();
}

void CIPTVChannelEditorDlg::OnCbnSelchangeComboPlaylist()
{
	GetConfig().set_int(false, REG_PLAYLIST_TYPE, m_wndPlaylist.GetCurSel());

	const auto& pl_info = ((PlaylistInfo*)m_wndPlaylist.GetItemData(m_wndPlaylist.GetCurSel()));
	LoadPlaylist();
}

void CIPTVChannelEditorDlg::OnCbnSelchangeComboStreamType()
{
	GetConfig().set_int(false, REG_STREAM_TYPE, (int)m_wndStreamType.GetItemData(m_wndStreamType.GetCurSel()));
	LoadChannelInfo();
}

void CIPTVChannelEditorDlg::OnCbnSelchangeComboChannels()
{
	if (!CheckForSave())
	{
		m_wndChannels.SetCurSel(GetConfig().get_int(false, REG_CHANNELS_TYPE));
		return;
	}

	m_categoriesMap.clear();
	m_channelsMap.clear();
	int idx = m_wndChannels.GetCurSel();
	if (idx != -1)
	{
		GetConfig().set_int(false, REG_CHANNELS_TYPE, idx);
		if (!LoadChannels())
		{
			CString str;
			str.Format(IDS_STRING_ERR_LOAD_CHANNELS_LIST, m_all_channels_lists[idx].c_str());
			AfxMessageBox(str, MB_ICONERROR | MB_OK);
		}
	}

	FillTreeChannels();
	CheckForExistingPlaylist();
}

HTREEITEM CIPTVChannelEditorDlg::SelectTreeItem(CTreeCtrlEx* pTreeCtl, const SearchParams& searchParams)
{
	if (!pTreeCtl)
		return nullptr;

	std::vector<HTREEITEM> all_items;
	for (auto hItem = pTreeCtl->GetChildItem(nullptr); hItem != nullptr; hItem = pTreeCtl->GetNextSiblingItem(hItem))
	{
		for (auto hChildItem = pTreeCtl->GetChildItem(hItem); hChildItem != nullptr; hChildItem = pTreeCtl->GetNextSiblingItem(hChildItem))
		{
			all_items.emplace_back(hChildItem);
		}
	}

	if (all_items.empty())
		return nullptr;

	HTREEITEM hFound = nullptr;
	HTREEITEM hFirst = pTreeCtl->GetSelectedItem();
	auto& pos = std::find(all_items.begin(), all_items.end(), hFirst);
	auto& start = (pos != all_items.end()) ? pos : all_items.begin();
	std::vector<HTREEITEM>::iterator cur = start;

	if (searchParams.next && ++cur == all_items.end())
		cur = all_items.begin();

	bool bFound = false;
	do
	{
		BaseInfo* entry = nullptr;
		switch (searchParams.type)
		{
			case InfoType::enChannel:
				if (auto pair = m_channelsTreeMap.find(*cur); pair != m_channelsTreeMap.end())
				{
					entry = dynamic_cast<BaseInfo*>(pair->second.get());
				}
				break;
			case InfoType::enPlEntry:
				if (auto pair = m_playlistTreeMap.find(*cur); pair != m_playlistTreeMap.end())
				{
					entry = dynamic_cast<BaseInfo*>(pair->second.get());
				}
				break;
			default:
				break;
		}

		if (!entry) continue;

		if (!searchParams.id.empty())
		{
			if (entry->stream_uri->get_parser().id == searchParams.id)
			{
				bFound = true;
				break;
			}
		}
		else if (searchParams.hash)
		{
			if (entry->stream_uri->get_hash() == searchParams.hash)
			{
				bFound = true;
				break;
			}
		}
		else if (!searchParams.searchString.IsEmpty())
		{
			if (StrStrI(entry->get_title().c_str(), searchParams.searchString.GetString()) != nullptr)
			{
				bFound = true;
				break;
			}
		}

		if (++cur == all_items.end())
			cur = all_items.begin();
	} while (cur != start);

	if (bFound)
	{
		hFound = *cur;
		if (searchParams.select)
		{
			pTreeCtl->SelectItem(hFound);
			pTreeCtl->Expand(pTreeCtl->GetParentItem(hFound), TVE_EXPAND);
			pTreeCtl->EnsureVisible(hFound);
		}
	}

	return hFound;
}

void CIPTVChannelEditorDlg::SearchTreeItem(InfoType type, bool next /*= false*/)
{
	CTreeCtrlEx* pTreeCtl = nullptr;
	SearchParams params;
	params.next = next;
	params.type = type;

	switch (type)
	{
		case InfoType::enChannel:
		{
			if (m_search.IsEmpty())
				return;

			pTreeCtl = &m_wndChannelsTree;
			if (m_search.GetLength() > 1 && m_search.GetAt(0) == '\\')
			{
				params.id = m_search.Mid(1).GetString();
				if (m_channelsMap.find(params.id) == m_channelsMap.end())
					return;
			}
			else
			{
				params.searchString = m_search;
			}
			break;
		}
		case InfoType::enPlEntry:
		{
			if (m_plSearch.IsEmpty())
				return;

			pTreeCtl = &m_wndPlaylistTree;
			if (m_plSearch.GetLength() > 1 && m_plSearch.GetAt(0) == '\\')
			{
				params.id = m_plSearch.Mid(1).GetString();
				if (m_playlistMap.find(params.id) == m_playlistMap.end())
					return;
			}
			else
			{
				params.searchString = m_plSearch;
			}

			break;
		}
		default:
			return;
	}

	if (!SelectTreeItem(pTreeCtl, params))
	{
		AfxMessageBox(IDS_STRING_INFO_NOT_FOUND, MB_OK | MB_ICONINFORMATION);
	}
}

bool CIPTVChannelEditorDlg::AddChannel(const std::shared_ptr<PlaylistEntry>& entry, int categoryId /*= -1*/)
{
	if (!entry)
		return false;

	bool needCheckExisting = false;
	bool add = false;
	const auto& root_path = GetAppPath(utils::PLUGIN_ROOT);

	HTREEITEM hFoundItem = nullptr;
	auto pair = m_channelsMap.find(entry->stream_uri->get_parser().id);
	if (pair != m_channelsMap.end())
	{
		// Channel already exist
		// add to first exists category if not set
		// channel data need to be updated later
		if (categoryId == -1)
		{
			for (const auto& pair : m_categoriesMap)
			{
				if (pair.second.category->find_channel(entry->stream_uri->get_parser().id) != nullptr)
				{
					categoryId = pair.second.category->get_key();
					break;
				}
			}
		}
	}
	else
	{
		// Create new channel
		add = true;
		auto newChannel = std::make_shared<ChannelInfo>(m_plugin_type, root_path);
		newChannel->stream_uri->copy(entry->stream_uri);
		// Add to channel array
		pair = m_channelsMap.emplace(newChannel->stream_uri->get_parser().id, newChannel).first;

		// is add to category?
		if (categoryId == -1)
		{
			// Search for existing category
			for (const auto& category : m_categoriesMap)
			{
				if (category.second.category->get_title() == entry->get_category())
				{
					categoryId = category.first;
					break;
				}
			}
		}
	}

	CategoryInfo info;
	if (categoryId != -1)
	{
		info = m_categoriesMap[categoryId];
	}
	else
	{
		// Category not exist, create new
		auto category = std::make_shared<ChannelCategory>(StreamType::enBase, root_path);
		categoryId = GetNewCategoryID();
		category->set_key(categoryId);
		category->set_title(entry->get_category());

		TVINSERTSTRUCTW tvCategory = { nullptr };
		tvCategory.hParent = TVI_ROOT;
		tvCategory.item.pszText = (LPWSTR)category->get_title().c_str();
		tvCategory.item.lParam = (DWORD_PTR)InfoType::enCategory;
		tvCategory.item.mask = TVIF_TEXT | TVIF_PARAM;
		auto hParent = m_wndChannelsTree.InsertItem(&tvCategory);

		info = { hParent, category };
		m_categoriesMap.emplace(categoryId, info);
		m_categoriesTreeMap.emplace(hParent, categoryId);
		needCheckExisting = true;
	}

	auto& channel = pair->second;
	// is channel in this category?
	if (info.category->find_channel(channel->stream_uri->get_parser().id) == nullptr)
	{
		// add channel to category tree leaf
		TVINSERTSTRUCTW tvChannel = { nullptr };
		tvChannel.hParent = info.hItem;
		tvChannel.item.pszText = (LPWSTR)channel->get_title().c_str();
		tvChannel.item.lParam = (DWORD_PTR)InfoType::enChannel;
		tvChannel.item.mask = TVIF_TEXT | TVIF_PARAM;
		hFoundItem = m_wndChannelsTree.InsertItem(&tvChannel);

		info.category->add_channel(channel);
		m_channelsTreeMap.emplace(hFoundItem, channel);
		needCheckExisting = true;
	}

	int flags = GetConfig().get_int(true, REG_CMP_FLAGS, CMP_FLAG_ALL);
	if (add)
		flags = CMP_FLAG_ALL;

	BOOL bCmpTitle = (flags & CMP_FLAG_TITLE) ? TRUE : FALSE;
	BOOL bCmpIcon = (flags & CMP_FLAG_ICON) ? TRUE : FALSE;
	BOOL bCmpArchive = (flags & CMP_FLAG_ARCHIVE) ? TRUE : FALSE;
	BOOL bCmpEpg1 = (flags & CMP_FLAG_EPG1) ? TRUE : FALSE;
	BOOL bCmpEpg2 = (flags & CMP_FLAG_EPG2) ? TRUE : FALSE;

	// Is title changed?
	if (bCmpTitle && channel->get_title() != entry->get_title())
	{
		needCheckExisting = true;
		channel->set_title(entry->get_title());
		// Search and update tree items present in other leafs
		for (const auto& pair : m_channelsTreeMap)
		{
			if (channel->stream_uri->get_parser().id == pair.second->stream_uri->get_parser().id)
				m_wndChannelsTree.SetItemText(pair.first, channel->get_title().c_str());
		}
	}

	// is tvg_id changed?
	if (bCmpEpg1 && !entry->get_epg_id(0).empty() && channel->get_epg_id(0) != entry->get_epg_id(0))
	{
		channel->set_epg_id(0, entry->get_epg_id(0));
		needCheckExisting = true;
	}

	if (bCmpEpg2 && !m_plugin->get_epg_parameters(1).epg_url.empty() && !entry->get_epg_id(1).empty() && channel->get_epg_id(1) != entry->get_epg_id(1))
	{
		channel->set_epg_id(1, entry->get_epg_id(1));
		needCheckExisting = true;
	}

	if (bCmpArchive && entry->get_archive_days() && channel->get_archive_days() != entry->get_archive_days())
	{
		channel->set_archive_days(entry->get_archive_days());
		needCheckExisting = true;
	}

	if (!channel->stream_uri->compare(entry->stream_uri))
	{
		channel->stream_uri->copy(entry->stream_uri);
		needCheckExisting = true;
	}

	if (bCmpIcon && !entry->get_icon_uri().get_uri().empty() && !entry->get_icon_uri().is_equal(channel->get_icon_uri(), false))
	{
		channel->set_icon_uri(entry->get_icon_uri());
		needCheckExisting = true;
	}

	// Channel for adult
	if (channel->get_adult() != entry->get_adult())
	{
		channel->set_adult(entry->get_adult());
		needCheckExisting = true;
	}

	return needCheckExisting;
}

void CIPTVChannelEditorDlg::CopyMoveChannelTo(int category_id, bool move)
{
	auto pair = m_categoriesMap.find(category_id);
	if (pair == m_categoriesMap.end())
		return;

	bool changed = false;
	HTREEITEM hNewItem = nullptr;
	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		const auto& category = GetItemCategory(hItem);
		const auto& channel = FindChannel(hItem);
		if (!channel || !category) continue;

		changed = true;
		if (!pair->second.category->add_channel(channel))
			continue;

		if (move)
		{
			category->remove_channel(channel->stream_uri->get_parser().id);
			m_wndChannelsTree.DeleteItem(hItem);
		}

		if (pair->first == ID_ADD_TO_FAVORITE)
		{
			channel->set_favorite(true);
		}

		TVINSERTSTRUCTW tvInsert = { nullptr };
		tvInsert.hParent = pair->second.hItem;
		tvInsert.item.pszText = (LPWSTR)channel->get_title().c_str();
		tvInsert.item.lParam = (LPARAM)InfoType::enChannel;
		tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
		hNewItem = m_wndChannelsTree.InsertItem(&tvInsert);
		m_channelsTreeMap.emplace(hNewItem, channel);
	}

	if (changed)
	{
		UpdateChannelsTreeColors(pair->second.hItem);
		m_wndChannelsTree.SelectItem(hNewItem);
		set_allow_save();
	}
}

void CIPTVChannelEditorDlg::OnBnClickedButtonPlFilter()
{
	CFilterDialog dlg;

	if (dlg.DoModal() == IDOK)
	{
		FillTreePlaylist();
	}
}

bool CIPTVChannelEditorDlg::IsSelectedTheSameType(const CTreeCtrlEx* pTreeCtl) const
{
	if (!pTreeCtl)
		return false;

	auto selected = pTreeCtl->GetSelectedItems();
	if (selected.empty())
		return false;

	bool isEntry = (GetBaseInfo(pTreeCtl, selected[0]) != nullptr);
	for (const auto& hItem : selected)
	{
		if (isEntry != (GetBaseInfo(pTreeCtl, hItem) != nullptr))
			return false;
	}

	return true;
}

bool CIPTVChannelEditorDlg::IsSelectedChannelsOrEntries(bool onlyChannel /*= false*/) const
{
	if (!m_lastTree || (m_lastTree == &m_wndPlaylistTree && onlyChannel))
		return false;

	auto selected = m_lastTree->GetSelectedItems();
	if (selected.empty())
		return false;

	for (const auto& hItem : selected)
	{
		if (!GetBaseInfo(m_lastTree, hItem))
			return false;
	}

	return true;
}

bool CIPTVChannelEditorDlg::IsSelectedCategory() const
{
	return (m_lastTree == &m_wndChannelsTree && GetCategory(m_wndChannelsTree.GetFirstSelectedItem()) != nullptr && IsSelectedTheSameType(&m_wndChannelsTree));
}

bool CIPTVChannelEditorDlg::IsSelectedNotFavorite() const
{
	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		const auto& category = GetCategory(hItem);
		if (category && category->get_key() == ID_ADD_TO_FAVORITE)
			return false;
	}

	return true;
}

bool CIPTVChannelEditorDlg::IsSelectedNotInFavorite() const
{
	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		const auto& category = GetItemCategory(hItem);
		if (category && category->get_key() == ID_ADD_TO_FAVORITE)
			return false;
	}

	return true;
}

bool CIPTVChannelEditorDlg::IsSelectedInTheSameCategory() const
{
	if (m_lastTree != &m_wndChannelsTree)
		return false;

	const auto& selected = m_wndChannelsTree.GetSelectedItems();
	if (selected.empty())
		return false;

	const auto& category = GetItemCategory(selected[0]);
	for (const auto& hItem : selected)
	{
		if (GetItemCategory(hItem) != category)
			return false;
	}

	return true;
}

bool CIPTVChannelEditorDlg::IsChannelSelectionConsistent() const
{
	bool continues = false;
	auto hItem = m_wndChannelsTree.GetFirstSelectedItem();
	auto hNext = hItem;
	for (; hItem != nullptr; hItem = m_wndChannelsTree.GetNextSelectedItem(hItem))
	{
		if (hItem != hNext)
		{
			continues = false;
			break;
		}

		hNext = m_wndChannelsTree.GetNextSiblingItem(hNext);
		continues = true;
	}

	return continues;
}

void CIPTVChannelEditorDlg::OnNMSetfocusTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	m_lastTree = DYNAMIC_DOWNCAST(CTreeCtrlEx, CWnd::FromHandle(pNMHDR->hwndFrom));
	*pResult = 0;
}

void CIPTVChannelEditorDlg::OnTvnChannelsGetInfoTip(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMTVGETINFOTIP>(pNMHDR);

	auto entry = GetBaseInfo(&m_wndChannelsTree, pGetInfoTip->hItem);
	if (entry && entry->is_type(InfoType::enChannel))
	{
		const std::wstring ch_id = entry->stream_uri->get_parser().id;
		CString categories;
		for (const auto& pair : m_categoriesMap)
		{
			if (pair.second.category->find_channel(ch_id))
			{
				if (!categories.IsEmpty())
					categories += _T(", ");
				categories.Append(pair.second.category->get_title().c_str());
			}
		}

		m_toolTipText.Format(IDS_STRING_FMT_CHANNELS_TOOLTIP1,
							 entry->get_title().c_str(),
							 entry->stream_uri->is_template() ? ch_id.c_str() : load_string_resource(IDS_STRING_CUSTOM).c_str(),
							 entry->get_epg_id(0).c_str());

		if (!entry->get_epg_id(1).empty())
		{
			m_toolTipText.AppendFormat(IDS_STRING_FMT_CHANNELS_TOOLTIP2, entry->get_epg_id(1).c_str());
		}

		m_toolTipText.AppendFormat(IDS_STRING_FMT_CHANNELS_TOOLTIP3,
								   entry->get_archive_days(),
								   load_string_resource(entry->get_adult() ? IDS_STRING_YES : IDS_STRING_NO).c_str(),
								   categories.GetString());

		pGetInfoTip->pszText = m_toolTipText.GetBuffer();
	}

	*pResult = 0;
}

void CIPTVChannelEditorDlg::OnTvnPlaylistGetInfoTip(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMTVGETINFOTIP>(pNMHDR);

	const auto& entry = FindEntry(pGetInfoTip->hItem);
	if (entry)
	{
		m_toolTipText.Format(IDS_STRING_FMT_PLAYLIST_TOOLTIPS,
							 entry->get_title().c_str(),
							 entry->stream_uri->is_template() ? entry->stream_uri->get_parser().id.c_str() : load_string_resource(IDS_STRING_CUSTOM).c_str(),
							 entry->get_epg_id(0).c_str(),
							 entry->get_archive_days(),
							 load_string_resource(entry->get_adult() ? IDS_STRING_YES : IDS_STRING_NO).c_str());

		pGetInfoTip->pszText = m_toolTipText.GetBuffer();
	}

	*pResult = 0;
}

BOOL CIPTVChannelEditorDlg::DestroyWindow()
{
	SaveWindowPos(GetSafeHwnd(), REG_WINDOW_POS);

	return __super::DestroyWindow();
}

void CIPTVChannelEditorDlg::UpdateExtToken(uri_stream* uri) const
{
	auto& parser = uri->get_parser();
	if (!parser.per_channel_token)
	{
		if (m_plugin_type == StreamType::enVidok || m_plugin_type == StreamType::enTVClub)
		{
			parser.token = uri->get_api_token(m_cur_account);
		}
		else
		{
			parser.token = m_cur_account.get_token();
		}

		return;
	}

	// all other uses a unique token for each channel depends on user credentials
	// this token can't be saved to the playlist and the only way is to map channel id to playlist entry id

	const auto& pair = m_playlistMap.find(parser.id);
	if (pair != m_playlistMap.end())
	{
		parser.token = pair->second->stream_uri->get_parser().token;
	}
}

bool CIPTVChannelEditorDlg::CheckForSave()
{
	if (!is_allow_save())
		return true;

	int res = AfxMessageBox(IDS_STRING_WRN_NOT_SAVED, MB_YESNOCANCEL | MB_ICONWARNING);
	if (res == IDYES)
	{
		OnSave();
		return true;
	}

	return res == IDNO ? true : false;
}

void CIPTVChannelEditorDlg::SaveStreamInfo()
{
	const auto& dump = m_stream_infos.serialize();
	// write document
	const auto& streamInfoFile = fmt::format(L"{:s}{:s}\\stream_info.bin", GetConfig().get_string(true, REG_LISTS_PATH), GetConfig().GetCurrentPluginName());
	std::ofstream os(streamInfoFile, std::istream::binary);
	os.write(dump.data(), dump.size());
	os.close();
}


void CIPTVChannelEditorDlg::OnBnClickedButtonVod()
{
	CVodViewer dlg(&m_vod_categories[(size_t)m_plugin_type]);
	dlg.m_plugin_type = m_plugin_type;
	dlg.m_account = m_cur_account;
	dlg.DoModal();
}
