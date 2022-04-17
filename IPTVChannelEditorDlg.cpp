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

#include "UtilsLib\inet_utils.h"
#include "UtilsLib\md5.h"
#include "UtilsLib\rapidxml.hpp"
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

// Common
constexpr auto CHANNELS_LIST_VERSION = 3;

std::map<UINT, UINT> tooltips_info =
{
	{ IDC_COMBO_PLUGIN_TYPE, IDS_STRING_COMBO_PLUGIN_TYPE },
	{ IDC_COMBO_CHANNELS, IDS_STRING_COMBO_CHANNELS },
	{ IDC_COMBO_PLAYLIST, IDS_STRING_COMBO_PLAYLIST },
	{ IDC_BUTTON_ADD_NEW_CHANNELS_LIST, IDS_STRING_BUTTON_ADD_NEW_CHANNELS_LIST },
	{ IDC_EDIT_SEARCH, IDS_STRING_EDIT_SEARCH },
	{ IDC_BUTTON_SEARCH_NEXT, IDS_STRING_BUTTON_SEARCH_NEXT },
	{ IDC_CHECK_SHOW_UNKNOWN, IDS_STRING_SHOW_UNKNOWN },
	{ IDC_EDIT_URL_ID, IDS_STRING_EDIT_URL_ID },
	{ IDC_BUTTON_TEST_EPG, IDS_STRING_BUTTON_TEST_EPG },
	{ IDC_EDIT_EPG1_ID, IDS_STRING_EDIT_EPG1_ID },
	{ IDC_EDIT_EPG2_ID, IDS_STRING_EDIT_EPG2_ID },
	{ IDC_CHECK_CUSTOMIZE, IDS_STRING_CHECK_CUSTOMIZE },
	{ IDC_CHECK_ARCHIVE, IDS_STRING_CHECK_ARCHIVE },
	{ IDC_EDIT_ARCHIVE_DAYS, IDS_STRING_EDIT_ARCHIVE_DAYS },
	{ IDC_CHECK_ADULT, IDS_STRING_CHECK_ADULT },
	{ IDC_BUTTON_CACHE_ICON, IDS_STRING_BUTTON_CACHE_ICON },
	{ IDC_BUTTON_SAVE, IDS_STRING_BUTTON_SAVE },
	{ IDC_SPLIT_BUTTON_PACK, IDS_STRING_BUTTON_PACK },
	{ IDC_BUTTON_SETTINGS, IDS_STRING_BUTTON_SETTINGS },
	{ IDC_BUTTON_CHOOSE_PLAYLIST, IDS_STRING_ACCOUNT_SETTINGS },
	{ IDC_BUTTON_DOWNLOAD_PLAYLIST, IDS_STRING_BUTTON_DOWNLOAD_PLAYLIST },
	{ IDC_EDIT_PL_SEARCH, IDS_STRING_EDIT_PL_SEARCH },
	{ IDC_BUTTON_PL_SEARCH_NEXT, IDS_STRING_BUTTON_PL_SEARCH_NEXT },
	{ IDC_BUTTON_PL_FILTER, IDS_STRING_BUTTON_PL_FILTER },
	{ IDC_CHECK_SHOW_CHANGED, IDS_STRING_SHOW_CHANGED },
	{ IDC_CHECK_NOT_ADDED, IDS_STRING_NOT_ADDED },
	{ IDC_STATIC_ICON, IDS_STRING_STATIC_ICON },
	{ IDC_EDIT_ARCHIVE_CHECK_DAYS, IDS_STRING_EDIT_ARCHIVE_CHECK_DAYS },
	{ IDC_SPIN_ARCHIVE_CHECK_DAYS, IDS_STRING_EDIT_ARCHIVE_CHECK_DAYS },
	{ IDC_EDIT_ARCHIVE_CHECK_HOURS, IDS_STRING_EDIT_ARCHIVE_CHECK_HOURS },
	{ IDC_SPIN_ARCHIVE_CHECK_HOURS, IDS_STRING_EDIT_ARCHIVE_CHECK_HOURS },
	{ IDC_EDIT_TIME_SHIFT, IDS_STRING_EDIT_TIME_SHIFT },
	{ IDC_SPIN_TIME_SHIFT, IDS_STRING_EDIT_TIME_SHIFT },
	{ IDC_EDIT_INFO_VIDEO, IDS_STRING_EDIT_INFO_VIDEO },
	{ IDC_EDIT_INFO_AUDIO, IDS_STRING_EDIT_INFO_AUDIO },
	{ IDC_COMBO_STREAM_TYPE, IDS_STRING_COMBO_STREAM_TYPE },
	{ IDC_COMBO_ICON_SOURCE, IDS_STRING_COMBO_ICON_SOURCE },
	{ IDC_BUTTON_STOP, IDS_STRING_BUTTON_STOP },
	{ IDC_BUTTON_CHECK_ARCHIVE, IDS_STRING_BUTTON_CHECK_ARCHIVE },
	{ IDC_BUTTON_UPDATE_CHANGED, IDS_STRING_BUTTON_UPDATE_CHANGED },
};

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

	ON_BN_CLICKED(IDC_BUTTON_ABOUT, &CIPTVChannelEditorDlg::OnBnClickedButtonAbout)
	ON_BN_CLICKED(IDC_BUTTON_CHOOSE_PLAYLIST, &CIPTVChannelEditorDlg::OnBnClickedButtonAccountSettings)
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
	ON_BN_CLICKED(IDC_BUTTON_UPDATE_CHANGED, &CIPTVChannelEditorDlg::OnBnClickedButtonUpdateChanged)
	ON_BN_CLICKED(IDC_CHECK_SHOW_CHANGED, &CIPTVChannelEditorDlg::OnBnClickedCheckShowChanged)

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

	ON_COMMAND(ID_RESTORE, &CIPTVChannelEditorDlg::OnRestore)
	ON_COMMAND(ID_APP_EXIT, &CIPTVChannelEditorDlg::OnAppExit)

	ON_MESSAGE_VOID(WM_KICKIDLE, OnKickIdle)
	ON_MESSAGE(WM_UPDATE_PROGRESS, &CIPTVChannelEditorDlg::OnUpdateProgress)
	ON_MESSAGE(WM_END_LOAD_PLAYLIST, &CIPTVChannelEditorDlg::OnEndLoadPlaylist)
	ON_MESSAGE(WM_UPDATE_PROGRESS_STREAM, &CIPTVChannelEditorDlg::OnUpdateProgressStream)
	ON_MESSAGE(WM_END_GET_STREAM_INFO, &CIPTVChannelEditorDlg::OnEndGetStreamInfo)
	ON_MESSAGE(WM_TRAYICON_NOTIFY, &CIPTVChannelEditorDlg::OnTrayIconNotify)

	ON_COMMAND_RANGE(ID_COPY_TO_START, ID_COPY_TO_END, &CIPTVChannelEditorDlg::OnCopyTo)
	ON_COMMAND_RANGE(ID_MOVE_TO_START, ID_MOVE_TO_END, &CIPTVChannelEditorDlg::OnMoveTo)
	ON_COMMAND_RANGE(ID_ADD_TO_START, ID_ADD_TO_END, &CIPTVChannelEditorDlg::OnAddTo)

	ON_BN_CLICKED(IDC_CHECK_NOT_ADDED, &CIPTVChannelEditorDlg::OnBnClickedCheckNotAdded)
	ON_BN_CLICKED(IDC_CHECK_SHOW_UNKNOWN, &CIPTVChannelEditorDlg::OnBnClickedCheckNotAdded)
	ON_BN_CLICKED(IDC_CHECK_SHOW_URL, &CIPTVChannelEditorDlg::OnBnClickedCheckShowUrl)
END_MESSAGE_MAP()

CIPTVChannelEditorDlg::CIPTVChannelEditorDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EDEMCHANNELEDITOR_DIALOG, pParent)
	, m_evtStop(FALSE, TRUE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_normal = ::GetSysColor(COLOR_WINDOWTEXT);
	m_gray = ::GetSysColor(COLOR_GRAYTEXT);
	m_red = RGB(200, 0, 0);
	m_green = RGB(0, 200, 0);
	m_hevc_color = RGB(158, 255, 250);
	m_brown = RGB(226, 135, 67);
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
	DDX_Control(pDX, IDC_CHECK_NOT_ADDED, m_wndNotAdded);
	DDX_Text(pDX, IDC_EDIT_PL_SEARCH, m_plSearch);
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
	DDX_Control(pDX, IDC_BUTTON_CHOOSE_PLAYLIST, m_wndChooseUrl);
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
	DDX_Control(pDX, IDC_BUTTON_UPDATE_CHANGED, m_wndUpdateChanged);
	DDX_Control(pDX, IDC_SPLIT_BUTTON_PACK, m_wndPack);
	DDX_Control(pDX, IDC_BUTTON_SETTINGS, m_wndSettings);
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

	for (const auto& pair : tooltips_info)
	{
		m_wndToolTipCtrl.AddTool(GetDlgItem(pair.first), pair.second);
	}

	m_wndToolTipCtrl.Activate(TRUE);

	m_archiveCheckDays = GetConfig().get_int(true, REG_DAYS_BACK);
	m_archiveCheckHours = GetConfig().get_int(true, REG_HOURS_BACK);

	UpdateData(FALSE);

	// Fill available plugins
	for (const auto& item : GetConfig().get_plugins_info())
	{
		int idx = m_wndPluginType.AddString(item.title.c_str());
		m_wndPluginType.SetItemData(idx, (DWORD_PTR)item.type);
	}

	CString res;
	res.LoadString(IDS_STRING_FILE);
	m_wndIconSource.AddString(res);
	res.LoadString(IDS_STRING_URL);
	m_wndIconSource.AddString(res);
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
	m_wndPack.SetDropDownMenu(IDR_MENU_SPLIT, 0);

	SwitchPlugin();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CIPTVChannelEditorDlg::UpdateWindowTitle()
{
	CString text;
	text.LoadString(GetConfig().IsPortable() ? IDS_STRING_APP_TITLE_PORTABLE : IDS_STRING_APP_TITLE);
	SetWindowText(text);
}

void CIPTVChannelEditorDlg::SwitchPlugin()
{
	// Rebuild available playlist types and set current plugin parameters
	m_inSync = true;

	m_host.clear();
	m_embedded_info = FALSE;

	m_domain = GetConfig().get_string(false, REG_DOMAIN);
	m_token = GetConfig().get_string(false, REG_TOKEN);
	m_login = GetConfig().get_string(false, REG_LOGIN);
	m_password = GetConfig().get_string(false, REG_PASSWORD);

	m_wndPlaylist.ResetContent();

	CString str;
	int pl_idx = GetConfig().get_int(false, REG_PLAYLIST_TYPE);
	const auto plugin_type = GetConfig().get_plugin_type();
	if (plugin_type == StreamType::enEdem)
	{
		str.LoadString(IDS_STRING_EDEM_STANDARD);
		m_wndPlaylist.AddString(str);
		str.LoadString(IDS_STRING_EDEM_THEMATIC);
		m_wndPlaylist.AddString(str);
		m_portal = GetConfig().get_string(false, REG_PORTAL);
		m_enableDownload = (pl_idx != 2);
	}
	else
	{
		m_portal.clear();
		str.LoadString(IDS_STRING_PLAYLIST);
		m_wndPlaylist.AddString(str);
		m_enableDownload = (pl_idx != 1);
	}

	const auto& plugin = StreamContainer::get_instance(plugin_type);
	const auto& streams = plugin->get_supported_stream_type();
	m_epg_mapper.swap(plugin->get_tvg_id_mapper());

	int cur_sel = GetConfig().get_int(false, REG_STREAM_TYPE, 0);
	m_wndStreamType.ResetContent();
	for (const auto& streamType : streams)
	{
		int idx = m_wndStreamType.AddString(std::get<1>(streamType).c_str());
		DWORD_PTR type = (DWORD_PTR)std::get<0>(streamType);
		m_wndStreamType.SetItemData(idx, type);
	}

	m_wndStreamType.SetCurSel(cur_sel);
	m_wndStreamType.EnableWindow(streams.size() > 1);

	str.LoadString(IDS_STRING_CUSTOM_PLAYLIST);

	m_wndPlaylist.EnableWindow(TRUE);
	int idx = m_wndPlaylist.AddString(str);
	m_wndPlaylist.SetItemData(idx, TRUE);

	// Set selected playlist
	if (pl_idx > m_wndPlaylist.GetCount() || pl_idx < 0)
		pl_idx = 0;

	m_wndPlaylist.SetCurSel(pl_idx);

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
			CString str;
			str.LoadString(IDS_STRING_STANDARD);
			const auto& name = item + str.GetString();
			m_wndChannels.AddString(name.c_str());
		}
		else
		{
			m_wndChannels.AddString(item.c_str());
		}
	}

	idx = GetConfig().get_int(false, REG_CHANNELS_TYPE);
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
}

void CIPTVChannelEditorDlg::LoadPlaylist(bool saveToFile /*= false*/)
{
	m_plFileName.Empty();
	std::wstring url;
	int idx = m_wndPlaylist.GetCurSel();
	BOOL isFile = m_wndPlaylist.GetItemData(idx) != 0;
	const auto plugin_type = GetConfig().get_plugin_type();

	m_plFileName = fmt::format(_T("{:s}_Playlist.m3u8"), GetConfig().GetCurrentPluginName(true)).c_str();

	PlaylistTemplateParams params;
	params.login = m_login;
	params.password = m_password;
	params.token = m_token;
	params.device = GetConfig().get_int(false, REG_DEVICE_ID, 1);
	params.number = idx;

	if (plugin_type == StreamType::enEdem && idx == 2)
	{
		url = GetConfig().get_string(false, REG_CUSTOM_PLAYLIST);
		m_plFileName.Empty();
	}
	else if (plugin_type != StreamType::enEdem && idx == 1)
	{
		url = GetConfig().get_string(false, REG_CUSTOM_PLAYLIST);
	}
	else
	{
		url = StreamContainer::get_instance(plugin_type)->get_playlist_template(params);
	}

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
	m_wndProgress.SetRange32(0, (int)std::count(data->begin(), data->end(), '\n'));
	m_wndProgress.SetPos(0);
	m_wndProgress.ShowWindow(SW_SHOW);
	m_wndProgressInfo.ShowWindow(SW_SHOW);

	ThreadConfig cfg;
	cfg.m_parent = this;
	cfg.m_data = data.release();
	cfg.m_hStop = m_evtStop;
	cfg.m_pluginType = plugin_type;
	cfg.m_rootPath = GetAppPath(utils::PLUGIN_ROOT);

	auto pThread = (CPlaylistParseM3U8Thread*)AfxBeginThread(RUNTIME_CLASS(CPlaylistParseM3U8Thread), THREAD_PRIORITY_HIGHEST, 0, CREATE_SUSPENDED);
	if (!pThread)
	{
		AfxMessageBox(IDS_STRING_ERR_THREAD_NOT_START, MB_OK | MB_ICONERROR);
		OnEndLoadPlaylist(0);
		return;
	}

	pThread->SetData(cfg);
	m_loading = true;

	m_wndPluginType.EnableWindow(FALSE);
	m_wndCheckArchive.EnableWindow(FALSE);
	m_wndPlaylist.EnableWindow(FALSE);
	m_wndPlaylistTree.EnableWindow(FALSE);
	m_wndChannels.EnableWindow(FALSE);
	m_wndFilter.EnableWindow(FALSE);
	m_wndDownloadUrl.EnableWindow(FALSE);
	m_wndChooseUrl.EnableWindow(FALSE);
	m_evtStop.ResetEvent();

	std::unique_ptr<Playlist> playlistEntriesOld = std::move(m_playlistEntries);

	FillTreePlaylist();

	pThread->ResumeThread();
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

	const auto plugin_type = GetConfig().get_plugin_type();

	m_playlistMap.clear();
	if (m_playlistEntries)
	{
		bool bSet = false;
		for (const auto& entry : m_playlistEntries->m_entries)
		{
			const auto& stream = entry->get_uri_stream();
			switch (plugin_type)
			{
				case StreamType::enGlanz: // login/token
					if (!stream->get_token().empty()
						&& !stream->get_domain().empty()
						&& !stream->get_login().empty()
						&& !stream->get_password().empty()
						&& !stream->get_int_id().empty()
						&& !stream->get_host().empty()
						)
					{
						m_token = stream->get_token();
						m_domain = stream->get_domain();
						m_login = stream->get_login();
						m_password = stream->get_password();
						m_host = stream->get_host();
						bSet = true;
					}
					break;
				default:
				{
					const auto& token = stream->get_token();
					const auto& domain = stream->get_domain();
					if (!token.empty() && token != L"00000000000000" && !domain.empty() && domain != L"localhost")
					{
						m_token = stream->get_token();
						m_domain = stream->get_domain();
						bSet = true;
					}
					break;
				}
			}

			if (bSet) break;
		}

		for (auto& entry : m_playlistEntries->m_entries)
		{
			auto res = m_playlistMap.emplace(entry->stream_uri->get_id(), entry);
			if (!res.second)
			{
				TRACE(L"Duplicate channel: %s (%s)\n",
					  res.first->second->get_title().c_str(),
					  res.first->second->stream_uri->get_id().c_str());
				continue;
			}
		}
	}

	UpdateChannelsTreeColors();
	FillTreePlaylist();
	LoadChannelInfo();

	m_loading = false;
	m_wndChooseUrl.EnableWindow(TRUE);
	m_wndDownloadUrl.EnableWindow(m_enableDownload);
	m_wndPlaylist.EnableWindow(TRUE);
	m_wndCheckArchive.EnableWindow(TRUE);

	AfxGetApp()->EndWaitCursor();

	return 0;
}

LRESULT CIPTVChannelEditorDlg::OnUpdateProgress(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	CString str;
	str.Format(IDS_STRING_FMT_CHANNELS_READED, wParam);
	m_wndProgressInfo.SetWindowText(str);
	m_wndProgress.SetPos(lParam);

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
	m_wndChooseUrl.EnableWindow(TRUE);
	m_wndDownloadUrl.EnableWindow(m_enableDownload);
	m_wndSettings.EnableWindow(TRUE);

	LoadChannelInfo();
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

	BOOL bUnknown = m_wndShowUnknown.GetCheck();
	if (bUnknown && m_playlistMap.empty())
	{
		bUnknown = FALSE;
		m_wndShowUnknown.SetCheck(FALSE);
	}

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
			TVINSERTSTRUCTW tvChannel = { nullptr };
			tvChannel.hParent = hParent;
			tvChannel.item.pszText = (LPWSTR)channel->get_title().c_str();
			tvChannel.item.lParam = (DWORD_PTR)InfoType::enChannel;
			tvChannel.item.mask = TVIF_TEXT | TVIF_PARAM;
			if (bUnknown && m_playlistMap.find(channel->stream_uri->get_id()) != m_playlistMap.end()) continue;

			HTREEITEM hItem = m_wndChannelsTree.InsertItem(&tvChannel);
			m_channelsTreeMap.emplace(hItem, channel);
			cnt++;
		}

		if (!cnt)
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
		params.id = select ? select : m_categoriesMap.begin()->second.category->get_channels().front()->stream_uri->get_id();
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
			ids.emplace(ch->stream_uri->get_id());
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
		BOOL bCmpEpg2 = ((flags & CMP_FLAG_EPG2) && StreamContainer::get_instance(GetConfig().get_plugin_type())->has_epg2()) ? TRUE : FALSE;

		while (root != nullptr && !m_playlistMap.empty())
		{
			// iterate subitems
			for (HTREEITEM hItem = m_wndChannelsTree.GetChildItem(root); hItem != nullptr; hItem = m_wndChannelsTree.GetNextSiblingItem(hItem))
			{
				const auto& channel = FindChannel(hItem);
				if (!channel) continue;

				COLORREF color = m_normal;
				const auto& id = channel->stream_uri->get_id();
				if (const auto& found = m_playlistMap.find(id); found != m_playlistMap.end())
				{
					const auto& entry = found->second;
					if ((bCmpTitle && channel->get_title() != entry->get_title())
						|| (bCmpArchive && entry->get_archive_days() != 0 && channel->get_archive_days() != entry->get_archive_days())
						|| (bCmpEpg1 && !entry->get_epg1_id().empty() && channel->get_epg1_id() != entry->get_epg1_id())
						|| (bCmpEpg2 && !entry->get_epg2_id().empty() && channel->get_epg2_id() != entry->get_epg2_id())
						|| (bCmpIcon && !entry->get_icon_uri().get_uri().empty() && !channel->get_icon_uri().is_equal(entry->get_icon_uri(), false))
						)
					{
						color = m_brown;
						m_changedChannels.emplace(id, entry);
					}
					else
					{
						color = m_green;
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

				m_wndChannelsTree.SetItemColor(hItem, color);
			}

			root = m_wndChannelsTree.GetNextSiblingItem(root);
		}
	}

	CString fmt;
	fmt.LoadString(IDS_STRING_FMT_CHANNELS);
	m_wndChInfo.SetWindowText(fmt::format(fmt.GetString(), m_channelsMap.size(), m_changedChannels.size(), m_unknownChannels.size()).c_str());
	m_wndUpdateChanged.EnableWindow(!m_changedChannels.empty());
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

			COLORREF color = m_red;
			if (const auto& pair = m_channelsMap.find(entry->stream_uri->get_id()); pair != m_channelsMap.end())
			{
				color = m_normal;
				const auto& channel = pair->second;
				if (channel->get_title() != entry->get_title()
					|| (entry->get_archive_days() != 0 && channel->get_archive_days() != entry->get_archive_days())
					|| (!entry->get_epg1_id().empty() && channel->get_epg1_id() != entry->get_epg1_id())
					|| (!entry->get_icon_uri().get_uri().empty() && !channel->get_icon_uri().is_equal(entry->get_icon_uri(), false))
					)
				{
					color = m_brown;
				}
			}

			if (const auto& pair = m_stream_infos.find(entry->stream_uri->get_hash()); pair != m_stream_infos.end())
			{
				if (pair->second.second.find("HEVC") != std::string::npos)
				{
					m_wndPlaylistTree.SetItemBackColor(hItem, m_hevc_color);
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
		m_epgID1 = channel->get_epg1_id().c_str();
		m_epgID2 = StreamContainer::get_instance(GetConfig().get_plugin_type())->has_epg2() ? channel->get_epg2_id().c_str() : L"";

		const auto& uri = channel->stream_uri;

		m_streamUrl = uri->get_uri().c_str();
		if (uri->is_template() && m_wndShowUrl.GetCheck())
		{
			TemplateParams params;
			params.token = m_token;
			params.domain = m_domain;
			params.login = m_login;
			params.password = m_password;
			params.host = m_host;
			UpdateExtToken(uri.get(), m_token);
			m_streamUrl = uri->get_templated_stream((StreamSubType)m_wndStreamType.GetItemData(m_wndStreamType.GetCurSel()), params).c_str();
		}

		m_streamID = uri->is_template() ? uri->get_id().c_str() : L"";
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
			SetImage(img, m_wndChannelIcon);
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
		m_plID.Format(_T("%s"), entry->stream_uri->get_id().c_str());

		if (!entry->get_epg1_id().empty())
			m_plEPG.Format(_T("%s"), entry->get_epg1_id().c_str());

		m_wndPlArchive.SetCheck(!!entry->is_archive());
		m_archivePlDays = entry->get_archive_days();
		auto hash = entry->stream_uri->get_hash();
		if (auto pair = m_stream_infos.find(hash); pair != m_stream_infos.end())
		{
			m_infoAudio = pair->second.first.c_str();
			m_infoVideo = pair->second.second.c_str();
		}

		const auto& img = GetIconCache().get_icon(entry->get_icon_absolute_path());
		SetImage(img, m_wndPlIcon);

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

	time_t now = time(nullptr) - (time_t)m_archiveCheckDays * 24 * 3600 - (time_t)m_archiveCheckHours * 3600;
	if (pTreeCtl == &m_wndChannelsTree)
	{
		now += m_timeShiftHours;
	}

	auto& epg_id = epg_idx == 0 ? info->get_epg1_id() : info->get_epg2_id();
	const auto& mapper = m_epg_mapper[epg_idx];
	if (!mapper.empty())
	{
		if (const auto& pair = mapper.find(epg_id); pair != mapper.end())
		{
			epg_id = pair->second;
		}
	}

	auto& allEpgMap = m_epg_cache[epg_idx];
	auto& epgChannelMap = allEpgMap[epg_id];

	UpdateExtToken(info->stream_uri.get(), m_token);

	// check end time
	EpgInfo epg_info{};
	bool need_load = true;
	while(need_load)
	{
		for (auto& epg_pair : epgChannelMap)
		{
			if (epg_pair.second.time_start <= now && now <= epg_pair.second.time_end)
			{
				epg_info = epg_pair.second;
				need_load = false;
				break;
			}
		}

		if (need_load && !info->stream_uri->parse_epg(epg_idx, epg_id, epgChannelMap, now, GetConfig().get_int(true, REG_USE_EPG_PROXY)))
		{
			need_load = false;
		}
	}

	if (epg_info.time_start != 0)
	{
		COleDateTime time_s(epg_info.time_start);
		COleDateTime time_e(epg_info.time_end);
		CStringA text;
		text.Format(R"({\rtf1 %ls - %ls\par\b %s\b0\par %s})",
					time_s.Format(_T("%d.%m.%Y %H:%M:%S")).GetString(),
					time_e.Format(_T("%d.%m.%Y %H:%M:%S")).GetString(),
					epg_info.name.c_str(),
					epg_info.desc.c_str()
		);

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

	const auto& channelsPath = fmt::format(L"{:s}{:s}\\{:s}", GetConfig().get_string(true, REG_LISTS_PATH), GetConfig().GetCurrentPluginName(), m_all_channels_lists[lst_idx]);

	std::ifstream is(channelsPath, std::istream::binary);
	if (!is.good())
		return false;

	// Read the xml file into a vector
	std::vector<char> buffer((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
	buffer.emplace_back('\0');

	// Parse the buffer using the xml file parsing library into doc
	rapidxml::xml_document<> doc;

	try
	{
		doc.parse<0>(buffer.data());
	}
	catch (rapidxml::parse_error& ex)
	{
		ex;
		return false;
	}

	m_embedded_info = FALSE;
	auto i_node = doc.first_node(utils::TV_INFO);
	auto info_node = i_node->first_node(utils::VERSION_INFO);
	if (!info_node || rapidxml::get_value_int(info_node->first_node(utils::LIST_VERSION)) != CHANNELS_LIST_VERSION)
	{
		set_allow_save(TRUE);
	}

	const auto plugin_type = GetConfig().get_plugin_type();
	auto setup_node = i_node->first_node(utils::CHANNELS_SETUP);
	if (setup_node)
	{
		if (setup_node->first_node(utils::ACCESS_TOKEN))
		{
			m_embedded_info |= EmbedToken;
			m_token = rapidxml::get_value_wstring(setup_node->first_node(utils::ACCESS_TOKEN));
		}

		if (setup_node->first_node(utils::ACCESS_DOMAIN))
		{
			m_embedded_info |= EmbedToken;
			m_domain = rapidxml::get_value_wstring(setup_node->first_node(utils::ACCESS_DOMAIN));
		}

		if (setup_node->first_node(utils::ACCESS_LOGIN))
		{
			m_embedded_info |= EmbedToken;
			m_login = rapidxml::get_value_wstring(setup_node->first_node(utils::ACCESS_LOGIN));
		}

		if (setup_node->first_node(utils::ACCESS_PASSWORD))
		{
			m_embedded_info |= EmbedToken;
			m_password = rapidxml::get_value_wstring(setup_node->first_node(utils::ACCESS_PASSWORD));
		}
	}

	auto portal_node = i_node->first_node(utils::PORTAL_SETUP);
	if (portal_node)
	{
		m_embedded_info |= EmbedPortal;
		m_portal = rapidxml::get_value_wstring(portal_node->first_node(utils::PORTAL_KEY));
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
		ASSERT(!channel->stream_uri->get_id().empty());

		auto ch_pair = m_channelsMap.find(channel->stream_uri->get_id());
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
			channel->set_type(plugin_type);
			ch_pair = m_channelsMap.emplace(channel->stream_uri->get_id(), channel).first;
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

	auto channel = std::make_shared<ChannelInfo>(GetConfig().get_plugin_type(), GetAppPath(utils::PLUGIN_ROOT));
	channel->set_title(L"New Channel");
	channel->set_icon_uri(utils::ICON_TEMPLATE);

	CImage img;
	if (LoadImage(channel->get_icon_absolute_path(), img))
	{
		SetImage(img, m_wndChannelIcon);
	}

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
	channel->stream_uri->set_id(str_id);

	category->add_channel(channel);
	m_channelsMap.emplace(channel->stream_uri->get_id(), channel);
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
			auto category_id = GetCategory(hItem)->get_key();
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
			category->remove_channel(channel->stream_uri->get_id());
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

	auto lKey = GetCategory(hLeft)->get_key();
	auto rKey = GetCategory(hRight)->get_key();

	// swap struct in map

	// get copy struct
	CategoryInfo lStruct = m_categoriesMap[lKey];
	CategoryInfo rStruct = m_categoriesMap[rKey];

	lStruct.category->set_key(rKey);
	rStruct.category->set_key(lKey);

	// Set swapped struct
	m_categoriesMap[lKey] = rStruct;
	m_categoriesMap[rKey] = lStruct;

	// ���������� ItemData ��� ��� � ��������� �� �������
	std::vector<HTREEITEM> itemData;
	for (HTREEITEM hItem = m_wndChannelsTree.GetChildItem(nullptr); hItem != nullptr; hItem = m_wndChannelsTree.GetNextSiblingItem(hItem))
	{
		itemData.emplace_back(hItem);
		int key = m_categoriesTreeMap[hItem];
		if (key == ID_ADD_TO_FAVORITE)
			key = -ID_ADD_TO_FAVORITE; // move to first position for sort
		m_wndChannelsTree.SetItemData(hItem, key);
	}

	// ������ ������� ������ ItemData ��� ����������
	int idx = m_wndChannelsTree.GetItemData(hLeft);
	m_wndChannelsTree.SetItemData(hLeft, m_wndChannelsTree.GetItemData(hRight));
	m_wndChannelsTree.SetItemData(hRight, idx);

	// ���������. ����� TreeCtrl ��� �������������� ���������� ������
	TVSORTCB sortInfo = { nullptr };
	sortInfo.lpfnCompare = &CBCompareForSwap;
	m_wndChannelsTree.SortChildrenCB(&sortInfo);

	for (HTREEITEM hItem = m_wndChannelsTree.GetChildItem(nullptr); hItem != nullptr; hItem = m_wndChannelsTree.GetNextSiblingItem(hItem))
	{
		int key = m_wndChannelsTree.GetItemData(hItem);
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
			const std::wstring old_id = channel->stream_uri->get_id();

			channel->stream_uri->set_template(not_checked);
			if (!not_checked)
			{
				// custom url. clear id
				channel->stream_uri->set_id(L"");
				m_streamID.Empty();
			}
			else if (channel->stream_uri->get_id().empty())
			{
				// templated url. set to -1 if empty
				int id = -1;
				while (m_channelsMap.find(std::to_wstring(id)) != m_channelsMap.end())
				{
					--id;
				}

				const auto& str_id = std::to_wstring(id);
				channel->stream_uri->set_id(str_id);
				m_streamID = CString(str_id.c_str());
			}

			// recalculate hash
			channel->stream_uri->recalc_hash();

			category->remove_channel(old_id);
			m_channelsMap.erase(old_id);

			m_channelsMap.emplace(channel->stream_uri->get_id(), channel);
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
	int state = 0; // none selected
	bool bSameType = IsSelectedTheSameType(&m_wndChannelsTree);
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
						CString str;
						str.Format(_T("%d x %d px"), img.GetWidth(), img.GetHeight());
						GetDlgItem(IDC_STATIC_ICON_SIZE)->SetWindowText(str);
					}
					SetImage(img, m_wndChannelIcon);
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
	m_wndArchiveDays.EnableWindow((state != 0) && (m_isArchive != 0));
	m_wndCheckArchive.EnableWindow(single && hasProbe && !m_loading);
	m_wndTimeShift.EnableWindow(state);
	m_wndSpinTimeShift.EnableWindow(state);
	m_wndSearch.EnableWindow(TRUE);
	m_wndEpg1.EnableWindow(single);
	m_wndEpg2.EnableWindow(single && StreamContainer::get_instance(GetConfig().get_plugin_type())->has_epg2());
	m_wndEpgID1.EnableWindow(single);
	m_wndEpgID2.EnableWindow(single && StreamContainer::get_instance(GetConfig().get_plugin_type())->has_epg2());

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

	if (GetConfig().get_int(true, REG_AUTO_SYNC))
		OnSyncTreeItem();

	UpdateData(FALSE);

	if (pResult)
		*pResult = 0;
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

void CIPTVChannelEditorDlg::OnEnChangeEditEpg2ID()
{
	UpdateData(TRUE);

	if (m_wndChannelsTree.GetSelectedCount() == 1)
	{
		const auto& channel = FindChannel(m_wndChannelsTree.GetSelectedItem());
		if (channel)
		{
			channel->set_epg2_id(m_epgID2.GetString());
			set_allow_save();
		}
	}
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
			channel->set_epg1_id(m_epgID1.GetString());
			UpdateChannelsTreeColors(m_wndChannelsTree.GetParentItem(hSelected));
			CheckForExistingPlaylist();
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
	const std::wstring old_id = channel->stream_uri->get_id();

	auto newChannel = std::make_shared<ChannelInfo>(*channel);
	newChannel->stream_uri->set_uri(m_streamUrl.GetString());
	newChannel->stream_uri->recalc_hash();

	if (m_channelsMap.find(newChannel->stream_uri->get_id()) != m_channelsMap.end())
	{
		AfxMessageBox(IDS_STRING_WRN_CHANNEL_EXIST, MB_OK | MB_ICONWARNING);
		return;
	}

	category->remove_channel(old_id);
	m_channelsMap.erase(old_id);

	category->add_channel(newChannel);
	m_channelsMap.emplace(newChannel->stream_uri->get_id(), newChannel);
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
	if (!channel || channel->stream_uri->get_id() == new_id)
		return;

	if (m_channelsMap.find(new_id) != m_channelsMap.end())
	{
		INT_PTR res = AfxMessageBox(IDS_STRING_WRN_CHANNEL_ID_EXIST, MB_YESNO | MB_ICONWARNING);
		if (res != IDYES)
			return;
	}

	const auto& category = GetItemCategory(hItem);
	const std::wstring old_id = channel->stream_uri->get_id();

	channel->stream_uri->set_id(new_id);

	// recalculate hash
	channel->stream_uri->recalc_hash();


	category->remove_channel(old_id);
	m_channelsMap.erase(old_id);

	m_channelsMap.emplace(channel->stream_uri->get_id(), channel);
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
		dlg.m_info = info;
		dlg.m_epg_cache= &m_epg_cache;
		dlg.m_epg_mapper = &m_epg_mapper[dlg.m_epg_idx];
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
		changed |= AddChannel(item.second);
	}

	UpdateChannelsTreeColors();
	FillTreePlaylist();
	set_allow_save(changed);
}

void CIPTVChannelEditorDlg::PlayItem(HTREEITEM hItem, int archive_hour /*= 0*/, int archive_day /*= 0*/) const
{
	if (auto info = GetBaseInfo(m_lastTree, hItem); info != nullptr)
	{
		TemplateParams params;
		params.token = m_token;
		params.domain = m_domain;
		params.login = m_login;
		params.password = m_password;
		params.host = m_host;

		int sec_back = 86400 * archive_day + 3600 * archive_hour;
		params.shift_back = sec_back ? _time32(nullptr) - sec_back : sec_back;

		UpdateExtToken(info->stream_uri.get(), m_token);
		const auto& url = info->stream_uri->get_templated_stream((StreamSubType)m_wndStreamType.GetItemData(m_wndStreamType.GetCurSel()), params);

		TRACE(L"Test URL: %s\n", url.c_str());

		ShellExecuteW(nullptr, L"open", GetConfig().get_string(true, REG_PLAYER).c_str(), url.c_str(), nullptr, SW_SHOWNORMAL);
	}
}

void CIPTVChannelEditorDlg::OnBnClickedButtonAccountSettings()
{
	bool loaded = false;
	int playlist_idx = m_wndPlaylist.GetCurSel();
	int dialogType = 0; // none

	auto account = GetConfig().get_plugin_account_type();
	if (playlist_idx == 2 || (account != AccountType::enOtt && playlist_idx == 1))
	{
		loaded = LoadCustomPlaylist();
	}
	else
	{
		loaded = SetupAccount();
	}

	if (loaded)
	{
		LoadPlaylist();
	}
}

bool CIPTVChannelEditorDlg::SetupAccount()
{
	CPropertySheet sheet(IDS_STRING_ACCOUNT_SETTINGS);
	sheet.m_psh.dwFlags |= PSH_NOAPPLYNOW;
	sheet.m_psh.dwFlags &= ~PSH_HASHELP;

	CAccessInfoDlg dlgInfo;
	dlgInfo.m_psp.dwFlags &= ~PSP_HASHELP;
	dlgInfo.m_bEmbed = m_embedded_info;
	dlgInfo.m_bEmbed_vp = (m_embedded_info & EmbedPortal) ? TRUE : FALSE;
	dlgInfo.m_token = m_token;
	dlgInfo.m_domain = m_domain;
	dlgInfo.m_login = m_login;
	dlgInfo.m_password = m_password;
	dlgInfo.m_host = m_host;
	dlgInfo.m_portal = m_portal;

	sheet.AddPage(&dlgInfo);

	if (sheet.DoModal() == IDOK)
	{
		BOOL embed = ((dlgInfo.m_bEmbed ? EmbedToken : 0) | (dlgInfo.m_bEmbed_vp ? EmbedPortal : 0));
		if (m_embedded_info != embed)
		{
			m_embedded_info = embed;
			set_allow_save(TRUE);
		}
		else if (embed
			&& (   dlgInfo.m_token != m_token
				|| dlgInfo.m_domain != m_domain
				|| dlgInfo.m_login != m_login
				|| dlgInfo.m_password != m_password
				|| dlgInfo.m_portal != m_portal)
			)
		{
			set_allow_save(TRUE);
		}

		m_domain = dlgInfo.m_domain;
		m_token = dlgInfo.m_token;
		m_login = dlgInfo.m_login;
		m_password = dlgInfo.m_password;
		m_host = dlgInfo.m_host;
		m_portal = dlgInfo.m_portal;

		GetConfig().UpdatePluginSettings();
		return true;
	}

	return false;
}

bool CIPTVChannelEditorDlg::LoadCustomPlaylist()
{
	CCustomPlaylistDlg dlg;
	dlg.m_url = GetConfig().get_string(false, REG_CUSTOM_PLAYLIST).c_str();

	if (dlg.DoModal() == IDOK)
	{
		GetConfig().set_string(false, REG_CUSTOM_PLAYLIST, dlg.m_url.GetString());
		return true;
	}

	return false;
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

	CString fmt;
	if (m_playlistIds.size() != m_playlistMap.size())
	{
		fmt.LoadString(IDS_STRING_FMT_PLAYLIST_FLT);
		m_wndPlInfo.SetWindowText(fmt::format(fmt.GetString(), m_plFileName.GetString(), m_playlistIds.size(), m_playlistMap.size()).c_str());
	}
	else
	{
		fmt.LoadString(IDS_STRING_FMT_CHANNELS);
		m_wndPlInfo.SetWindowText(fmt::format(_T("{:s}, Channels: {:d}"), m_plFileName.GetString(), m_playlistIds.size()).c_str());
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
				entries.emplace_back(entry.second);
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
				show &= m_channelsMap.find(entry->stream_uri->get_id()) == m_channelsMap.end();
			}

			if (!show) continue;

			m_playlistIds.emplace_back(entry->stream_uri->get_id());
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
	// ��������� ������ ��������� ���� �� ���� �����. ����� ������ ������ � �������
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
		old_selected = channel->stream_uri->get_id().c_str();

	try
	{
		// create document;
		rapidxml::xml_document<> doc;
		auto decl = doc.allocate_node(rapidxml::node_declaration);

		// adding attributes at the top of our xml
		decl->append_attribute(doc.allocate_attribute("version", "1.0"));
		decl->append_attribute(doc.allocate_attribute("encoding", "UTF-8"));
		doc.append_node(decl);

		// create <tv_info> root node
		auto tv_info = doc.allocate_node(rapidxml::node_element, utils::TV_INFO);

		auto info_node = doc.allocate_node(rapidxml::node_element, utils::VERSION_INFO);
		info_node->append_node(rapidxml::alloc_node(doc, utils::LIST_VERSION, std::to_string(CHANNELS_LIST_VERSION).c_str()));
		tv_info->append_node(info_node);

		auto setup_node = doc.allocate_node(rapidxml::node_element, utils::CHANNELS_SETUP);

		if (m_channelsMap.begin()->second->stream_uri->has_epg2())
		{
			setup_node->append_node(rapidxml::alloc_node(doc, utils::HAS_SECONDARY_EPG, "true"));
		}

		if ((m_embedded_info & EmbedToken) == EmbedToken)
		{
			switch (GetConfig().get_plugin_type())
			{
				case StreamType::enEdem: // ott_key
					setup_node->append_node(rapidxml::alloc_node(doc, utils::ACCESS_TOKEN, utils::utf16_to_utf8(m_token).c_str()));
					setup_node->append_node(rapidxml::alloc_node(doc, utils::ACCESS_DOMAIN, utils::utf16_to_utf8(m_domain).c_str()));
					break;
				case StreamType::enFox: // login/pass
				case StreamType::enGlanz:
				case StreamType::enSharaclub:
				case StreamType::enSharaTV:
				case StreamType::enOneOtt:
				case StreamType::enVidok:
				case StreamType::enTVClub:
					setup_node->append_node(rapidxml::alloc_node(doc, utils::ACCESS_LOGIN, utils::utf16_to_utf8(m_login).c_str()));
					setup_node->append_node(rapidxml::alloc_node(doc, utils::ACCESS_PASSWORD, utils::utf16_to_utf8(m_password).c_str()));
					break;
				case StreamType::enAntifriz: // pin
				case StreamType::enItv:
				case StreamType::enOneCent:
				case StreamType::enOneUsd:
				case StreamType::enSharavoz:
				case StreamType::enVipLime:
				case StreamType::enTvTeam:
				case StreamType::enLightIptv:
				case StreamType::enCbilling:
				case StreamType::enOttclub:
				case StreamType::enIptvOnline:
				case StreamType::enShuraTV:
					setup_node->append_node(rapidxml::alloc_node(doc, utils::ACCESS_PASSWORD, utils::utf16_to_utf8(m_password).c_str()));
					break;
				default:
					break;
			}
		}

		if (setup_node->first_node())
		{
			tv_info->append_node(setup_node);
		}

		if ((m_embedded_info & EmbedPortal) == EmbedPortal)
		{
			// Currently only for Edem/iLook
			auto portal_node = doc.allocate_node(rapidxml::node_element, utils::PORTAL_SETUP);
			switch (GetConfig().get_plugin_type())
			{
				case StreamType::enEdem: // ott_key
					portal_node->append_node(rapidxml::alloc_node(doc, utils::PORTAL_KEY, utils::utf16_to_utf8(m_portal).c_str()));
					break;
				default:
					break;
			}

			tv_info->append_node(portal_node);
		}

		// create <tv_categories> node
		auto cat_node = doc.allocate_node(rapidxml::node_element, utils::TV_CATEGORIES);

		// append <tv_category> to <tv_categories> node
		for (auto& category : m_categoriesMap)
		{
			if (category.first != ID_ADD_TO_FAVORITE && !category.second.category->get_channels().empty())
			{
				cat_node->append_node(category.second.category->GetNode(doc));
			}
		}
		// append <tv_categories> to <tv_info> node
		tv_info->append_node(cat_node);

		// create <tv_channels> node
		auto ch_node = doc.allocate_node(rapidxml::node_element, utils::TV_CHANNELS);
		// append <tv_channel> to <v_channels> node
		for (const auto& pair : m_categoriesMap)
		{
			if (pair.first == ID_ADD_TO_FAVORITE) continue;

			for (auto& channel : pair.second.category->get_channels())
			{
				channel->get_category_ids().clear();
				channel->get_category_ids().emplace(pair.first);
				ch_node->append_node(channel->GetNode(doc));
			}
		}

		// append <tv_channel> to <tv_info> node
		tv_info->append_node(ch_node);

		doc.append_node(tv_info);
		// write document
		auto& channelsPath = fmt::format(L"{:s}{:s}\\", GetConfig().get_string(true, REG_LISTS_PATH), GetConfig().GetCurrentPluginName());
		std::error_code err;
		std::filesystem::create_directories(channelsPath, err);

		channelsPath += m_all_channels_lists[lst_idx];

		std::ofstream os(channelsPath, std::istream::binary);
		os << doc;

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

	CImage img;
	if (LoadImage(newCategory->get_icon_absolute_path(), img))
	{
		SetImage(img, m_wndChannelIcon);
	}

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
		CString path = GetAppPath(IsChannel(hCur) ? utils::CHANNELS_LOGO_PATH : utils::CATEGORIES_LOGO_PATH).c_str();
		CString file(path);
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
		oFN.lpstrInitialDir = path.GetString();
		oFN.Flags |= OFN_EXPLORER | OFN_NOREADONLYRETURN | OFN_ENABLESIZING | OFN_LONGNAMES | OFN_PATHMUSTEXIST;
		oFN.Flags |= OFN_FILEMUSTEXIST | OFN_NONETWORKBUTTON | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT | OFN_NODEREFERENCELINKS;

		dlg.ApplyOFNToShellDialog();
		INT_PTR nResult = dlg.DoModal();
		file.ReleaseBuffer();

		if (nResult == IDOK)
		{
			size_t len = _tcslen(oFN.lpstrFileTitle);
			for (size_t i = 0; i < len; i++)
			{
				if (oFN.lpstrFileTitle[i] > 127)
				{
					AfxMessageBox(IDS_STRING_WRN_NON_ASCII, MB_ICONERROR | MB_OK);
					return;
				}
			}

			CString newPath = file.Left(file.GetLength() - len);
			if (path.CompareNoCase(newPath) != 0)
			{
				path += oFN.lpstrFileTitle;
				CopyFile(file, path, FALSE);
				CImage img;
				if (LoadImage(path.GetString(), img))
				{
					SetImage(img, m_wndChannelIcon);
				}
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
		SetImage(img, m_wndChannelIcon);

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
		PackPlugin(GetConfig().get_plugin_type(), GetConfig().get_string(true, REG_OUTPUT_PATH), GetConfig().get_string(true, REG_LISTS_PATH), true);
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
		CString str;
		str.Format(_T("%s"), item.title.c_str());
		m_wndProgressInfo.SetWindowText(str);
		m_wndProgress.SetPos(++i);

		if (!PackPlugin(item.type, GetConfig().get_string(true, REG_OUTPUT_PATH), GetConfig().get_string(true, REG_LISTS_PATH), false))
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
			params.id = info->stream_uri->get_id();
			params.type = InfoType::enPlEntry;
			params.select = false;
			needCheckExisting |= AddChannel(FindEntry(SelectTreeItem(&m_wndPlaylistTree, params)));
		}
		LoadChannelInfo();
	}

	OnSyncTreeItem();

	if (needCheckExisting)
	{
		UpdateChannelsTreeColors();
		CheckForExistingPlaylist();
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

	if (GetConfig().get_int(true, REG_AUTO_SYNC))
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

	for (auto& item : *container)
	{
		UpdateExtToken(item, m_token);
	}

	m_wndPluginType.EnableWindow(FALSE);
	m_wndChannels.EnableWindow(FALSE);
	m_wndPlaylist.EnableWindow(FALSE);
	m_wndChooseUrl.EnableWindow(FALSE);
	m_wndDownloadUrl.EnableWindow(FALSE);
	m_wndSettings.EnableWindow(FALSE);

	m_evtStop.ResetEvent();
	m_wndStop.EnableWindow(TRUE);
	m_wndProgress.ShowWindow(SW_SHOW);
	m_wndProgressInfo.ShowWindow(SW_SHOW);
	m_wndProgress.SetRange32(0, (int)container->size());
	m_wndProgress.SetPos(0);

	auto* pThread = (CGetStreamInfoThread*)AfxBeginThread(RUNTIME_CLASS(CGetStreamInfoThread), THREAD_PRIORITY_HIGHEST, 0, CREATE_SUSPENDED);
	if (!pThread)
	{
		AfxMessageBox(IDS_STRING_ERR_THREAD_NOT_START, MB_OK | MB_ICONERROR);
		OnEndGetStreamInfo();
		return;
	}

	m_inStreamInfo = true;

	CGetStreamInfoThread::ThreadConfig cfg;
	cfg.m_parent = this;
	cfg.m_container = container.release();
	cfg.m_hStop = m_evtStop;
	cfg.m_probe = GetConfig().get_string(true, REG_FFPROBE);
	cfg.m_max_threads = GetConfig().get_int(true, REG_MAX_THREADS, 3);
	cfg.m_params.token = m_token;
	cfg.m_params.domain = m_domain;
	cfg.m_params.login = m_login;
	cfg.m_params.password = m_password;
	cfg.m_params.host = m_host;
	cfg.m_params.shift_back = 0;

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
		PlayItem(m_lastTree->GetSelectedItem(), m_archiveCheckDays, m_archiveCheckHours);
	}
}

void CIPTVChannelEditorDlg::OnSyncTreeItem()
{
	if (m_loading || !m_lastTree || m_inSync)
		return;

	m_inSync = true;

	CWaitCursor cur;
	SearchParams params;

	auto info = GetBaseInfo(m_lastTree, m_lastTree->GetSelectedItem());
	if (info)
	{
		params.id = info->stream_uri->get_id();
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
	CString text;
	text.LoadString(IDS_STRING_ENABLE_CHANNEL);
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
		pCmdUI->SetText(text);
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
	CString text;
	text.LoadString(IDS_STRING_ENABLE_CATEGORY);
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
		pCmdUI->SetText(text);
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

	LoadPlaylist();
}

void CIPTVChannelEditorDlg::OnCbnSelchangeComboStreamType()
{
	GetConfig().set_int(false, REG_STREAM_TYPE, (int)m_wndStreamType.GetItemData(m_wndStreamType.GetCurSel()));
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
			if (entry->stream_uri->get_id() == searchParams.id)
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
	auto pair = m_channelsMap.find(entry->stream_uri->get_id());
	if (pair != m_channelsMap.end())
	{
		// Channel already exist
		// add to first exists category if not set
		// channel data need to be updated later
		if (categoryId == -1)
		{
			for (const auto& pair : m_categoriesMap)
			{
				if (pair.second.category->find_channel(entry->stream_uri->get_id()) != nullptr)
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
		auto newChannel = std::make_shared<ChannelInfo>(GetConfig().get_plugin_type(), root_path);
		newChannel->stream_uri->copy(entry->stream_uri);
		// Add to channel array
		pair = m_channelsMap.emplace(newChannel->stream_uri->get_id(), newChannel).first;

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
	if (info.category->find_channel(channel->stream_uri->get_id()) == nullptr)
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
			if (channel->stream_uri->get_id() == pair.second->stream_uri->get_id())
				m_wndChannelsTree.SetItemText(pair.first, channel->get_title().c_str());
		}
	}

	// is tvg_id changed?
	if (bCmpEpg1 && !entry->get_epg1_id().empty() && channel->get_epg1_id() != entry->get_epg1_id())
	{
		channel->set_epg1_id(entry->get_epg1_id());
		needCheckExisting = true;
	}

	if (bCmpEpg2 && StreamContainer::get_instance(GetConfig().get_plugin_type())->has_epg2() && !entry->get_epg2_id().empty() && channel->get_epg2_id() != entry->get_epg2_id())
	{
		channel->set_epg2_id(entry->get_epg2_id());
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
			category->remove_channel(channel->stream_uri->get_id());
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

	CString custom;
	custom.LoadString(IDS_STRING_CUSTOM);

	auto entry = GetBaseInfo(&m_wndChannelsTree, pGetInfoTip->hItem);
	if (entry && entry->is_type(InfoType::enChannel))
	{
		const std::wstring ch_id = entry->stream_uri->get_id();
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
							 entry->stream_uri->is_template() ? ch_id.c_str() : custom.GetString(),
							 entry->get_epg1_id().c_str());

		if (!entry->get_epg2_id().empty())
		{
			m_toolTipText.AppendFormat(IDS_STRING_FMT_CHANNELS_TOOLTIP2, entry->get_epg2_id().c_str());
		}

		CString adult;
		adult.LoadString(entry->get_adult() ? IDS_STRING_YES : IDS_STRING_NO);

		m_toolTipText.AppendFormat(IDS_STRING_FMT_CHANNELS_TOOLTIP3,
								   entry->get_archive_days(),
								   adult.GetString(),
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

		CString custom;
		custom.LoadString(IDS_STRING_CUSTOM);

		CString adult;
		adult.LoadString(entry->get_adult() ? IDS_STRING_YES : IDS_STRING_NO);
		m_toolTipText.Format(IDS_STRING_FMT_PLAYLIST_TOOLTIPS,
							 entry->get_title().c_str(),
							 entry->stream_uri->is_template() ? entry->stream_uri->get_id().c_str() : custom.GetString(),
							 entry->get_epg1_id().c_str(),
							 entry->get_archive_days(),
							 adult.GetString());

		pGetInfoTip->pszText = m_toolTipText.GetBuffer();
	}

	*pResult = 0;
}

BOOL CIPTVChannelEditorDlg::DestroyWindow()
{
	SaveWindowPos(GetSafeHwnd(), REG_WINDOW_POS);

	return __super::DestroyWindow();
}

void CIPTVChannelEditorDlg::UpdateExtToken(uri_stream* uri, const std::wstring& token) const
{
	auto pluginType = GetConfig().get_plugin_type();

	if (   pluginType != StreamType::enFox
		&& pluginType != StreamType::enItv
		&& pluginType != StreamType::enOneUsd
		&& pluginType != StreamType::enTvTeam
		&& pluginType != StreamType::enVidok
		&& pluginType != StreamType::enTVClub
		)
	{
		uri->set_token(token);
		return;
	}
	if (pluginType == StreamType::enVidok || pluginType == StreamType::enTVClub)
	{
		uri->set_token(uri->get_api_token(m_login, m_password));
		return;
	}

	// fox, itv, 1usd, tvteam uses a unique token for each channel depends on user credentials
	// this token can't be saved to the playlist and the only way is to map channel id to playlist entry id

	const auto& pair = m_playlistMap.find(uri->get_id());
	if (pair != m_playlistMap.end())
		uri->set_token(pair->second->stream_uri->get_token());
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
