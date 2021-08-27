
// EdemChannelEditorDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "framework.h"
#include <afxdialogex.h>
#include <array>
#include <thread>
#include <filesystem>

#include "IPTVChannelEditor.h"
#include "IPTVChannelEditorDlg.h"
#include "AboutDlg.h"
#include "SettingsDlg.h"
#include "AccessDlg.h"
#include "FilterDialog.h"
#include "CustomPlaylistDlg.h"
#include "PlaylistParseThread.h"
#include "IconCache.h"
#include "utils.h"

#include "rapidxml.hpp"
#include "rapidxml_print.hpp"
#include "fmt\format.h"

#include "SevenZip/7zip/SevenZipWrapper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr auto ID_COPY_TO_START = 40000;
constexpr auto ID_COPY_TO_END = ID_COPY_TO_START + 512;

constexpr auto ID_MOVE_TO_START = ID_COPY_TO_END + 1;
constexpr auto ID_MOVE_TO_END = ID_MOVE_TO_START + 512;

constexpr auto ID_ADD_TO_START = ID_MOVE_TO_END + 1;
constexpr auto ID_ADD_TO_END = ID_ADD_TO_START + 512;

// Common
constexpr auto REG_SETTINGS = _T("Settings");
constexpr auto REG_PLAYER = _T("Player");
constexpr auto REG_FFPROBE = _T("FFProbe");
constexpr auto REG_HOURS_BACK = _T("HoursBack");
constexpr auto REG_AUTOSYNC = _T("AutoSyncChannel");
constexpr auto REG_PLUGIN = _T("PluginType");
constexpr auto REG_FILTER_STRING = _T("FilterString");
constexpr auto REG_FILTER_REGEX = _T("FilterUseRegex");
constexpr auto REG_FILTER_CASE = _T("FilterUseCase");

// Plugin dependent
constexpr auto REG_ACCESS_KEY = _T("AccessKey");
constexpr auto REG_DOMAIN = _T("Domain");
constexpr auto REG_ACCESS_URL = _T("AccessUrl");
constexpr auto REG_CHANNELS_TYPE = _T("ChannelsType");
constexpr auto REG_PLAYLIST_TYPE = _T("PlaylistType");
constexpr auto REG_CUSTOM_URL = _T("CustomUrl");
constexpr auto REG_CUSTOM_FILE = _T("CustomPlaylist");

BOOL CIPTVChannelEditorDlg::m_embedded_info = FALSE;
CString CIPTVChannelEditorDlg::m_probe;
std::string CIPTVChannelEditorDlg::m_gl_domain;
std::string CIPTVChannelEditorDlg::m_gl_access_key;
std::string CIPTVChannelEditorDlg::m_ch_domain;
std::string CIPTVChannelEditorDlg::m_ch_access_key;

static constexpr auto URI_TEMPLATE_EDEM = "http://{SUBDOMAIN}/iptv/{UID}/{ID}/index.m3u8";
static constexpr auto URI_TEMPLATE_SHARAVOZ = "http://{SUBDOMAIN}/{ID}/index.m3u8?token={UID}";


// Возвращает разницу между заданным и текущим значением времени в тиках
inline DWORD GetTimeDiff(DWORD dwStartTime)
{
	DWORD dwCurrent = ::GetTickCount();

	if (dwStartTime > dwCurrent)
		return (0xffffffff - dwCurrent - dwStartTime);

	return (dwCurrent - dwStartTime);
}

// Требует начальное время и таймаут в миллисекундах
// Возвращает TRUE, если таймаут вышел
inline BOOL CheckForTimeOut(DWORD dwStartTime, DWORD dwTimeOut)
{
	return (GetTimeDiff(dwStartTime) > dwTimeOut);
}

int CALLBACK CBCompareForSwap(LPARAM lParam1, LPARAM lParam2, LPARAM)
{
	return lParam1 > lParam2;
}

using namespace SevenZip;

// CEdemChannelEditorDlg dialog

BEGIN_MESSAGE_MAP(CIPTVChannelEditorDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()

	ON_CBN_SELCHANGE(IDC_COMBO_PLUGIN_TYPE, &CIPTVChannelEditorDlg::OnCbnSelchangeComboPluginType)
	ON_BN_CLICKED(IDC_BUTTON_ABOUT, &CIPTVChannelEditorDlg::OnBnClickedButtonAbout)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_PLAYLIST, &CIPTVChannelEditorDlg::OnBnClickedButtonCustomPlaylist)
	ON_BN_CLICKED(IDC_BUTTON_PL_SEARCH_NEXT, &CIPTVChannelEditorDlg::OnBnClickedButtonPlSearchNext)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_PL_SEARCH_NEXT, &CIPTVChannelEditorDlg::OnUpdateButtonPlSearchNext)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH_NEXT, &CIPTVChannelEditorDlg::OnBnClickedButtonSearchNext)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_SEARCH_NEXT, &CIPTVChannelEditorDlg::OnUpdateButtonSearchNext)
	ON_BN_CLICKED(IDC_BUTTON_PL_FILTER, &CIPTVChannelEditorDlg::OnBnClickedButtonPlFilter)
	ON_BN_CLICKED(IDC_BUTTON_GET_INFO, &CIPTVChannelEditorDlg::OnGetStreamInfo)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_GET_INFO, &CIPTVChannelEditorDlg::OnUpdateGetStreamInfo)
	ON_BN_CLICKED(IDC_BUTTON_ADD_NEW_CHANNELS_LIST, &CIPTVChannelEditorDlg::OnBnClickedButtonAddNewChannelsList)
	ON_BN_CLICKED(IDC_BUTTON_ACCESS_INFO, &CIPTVChannelEditorDlg::OnBnClickedButtonAccessInfo)
	ON_BN_CLICKED(IDC_BUTTON_DOWNLOAD_PLAYLIST, &CIPTVChannelEditorDlg::OnBnClickedButtonDownloadPlaylist)
	ON_BN_CLICKED(IDC_BUTTON_TEST_TVG, &CIPTVChannelEditorDlg::OnBnClickedButtonTestEpg2)
	ON_BN_CLICKED(IDC_BUTTON_TEST_EPG, &CIPTVChannelEditorDlg::OnBnClickedButtonTestEpg1)
	ON_BN_CLICKED(IDC_CHECK_ADULT, &CIPTVChannelEditorDlg::OnBnClickedCheckAdult)
	ON_BN_CLICKED(IDC_CHECK_ARCHIVE, &CIPTVChannelEditorDlg::OnBnClickedCheckArchive)
	ON_BN_CLICKED(IDC_CHECK_CUSTOMIZE, &CIPTVChannelEditorDlg::OnBnClickedCheckCustomize)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CIPTVChannelEditorDlg::OnSave)
	ON_BN_CLICKED(IDC_BUTTON_PACK, &CIPTVChannelEditorDlg::OnBnClickedButtonPack)
	ON_BN_CLICKED(IDC_BUTTON_SETTINGS, &CIPTVChannelEditorDlg::OnBnClickedButtonSettings)
	ON_BN_CLICKED(IDC_BUTTON_CACHE_ICON, &CIPTVChannelEditorDlg::OnBnClickedButtonCacheIcon)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_CACHE_ICON, &CIPTVChannelEditorDlg::OnUpdateButtonCacheIcon)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE_ICON, &CIPTVChannelEditorDlg::OnUpdateIcon)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_UPDATE_ICON, &CIPTVChannelEditorDlg::OnUpdateUpdateIcon)
	ON_BN_CLICKED(IDC_BUTTON_CHECK_ARCHIVE, &CIPTVChannelEditorDlg::OnPlayChannelStreamArchive)

	ON_EN_CHANGE(IDC_EDIT_TVG_ID, &CIPTVChannelEditorDlg::OnEnChangeEditTvgID)
	ON_EN_CHANGE(IDC_EDIT_EPG_ID, &CIPTVChannelEditorDlg::OnEnChangeEditEpgID)
	ON_EN_CHANGE(IDC_EDIT_TIME_SHIFT, &CIPTVChannelEditorDlg::OnEnChangeEditTimeShiftHours)
	ON_EN_CHANGE(IDC_EDIT_ARCHIVE_CHECK, &CIPTVChannelEditorDlg::OnEnChangeEditArchiveCheck)
	ON_EN_CHANGE(IDC_EDIT_STREAM_URL, &CIPTVChannelEditorDlg::OnEnChangeEditStreamUrl)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_TIME_SHIFT, &CIPTVChannelEditorDlg::OnDeltaposSpinTimeShiftHours)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_ARCHIVE_CHECK, &CIPTVChannelEditorDlg::OnDeltaposSpinArchiveCheck)
	ON_EN_CHANGE(IDC_EDIT_URL_ID, &CIPTVChannelEditorDlg::OnEnChangeEditUrlID)
	ON_STN_CLICKED(IDC_STATIC_ICON, &CIPTVChannelEditorDlg::OnStnClickedStaticIcon)
	ON_CBN_SELCHANGE(IDC_COMBO_CHANNELS, &CIPTVChannelEditorDlg::OnCbnSelchangeComboChannels)
	ON_CBN_SELCHANGE(IDC_COMBO_PLAYLIST, &CIPTVChannelEditorDlg::OnCbnSelchangeComboPlaylist)

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
	ON_COMMAND(ID_REMOVE_CHANNEL, &CIPTVChannelEditorDlg::OnRemoveChannel)
	ON_UPDATE_COMMAND_UI(ID_REMOVE_CHANNEL, &CIPTVChannelEditorDlg::OnUpdateRemoveChannel)
	ON_COMMAND(ID_UPDATE_CHANNEL, &CIPTVChannelEditorDlg::OnAddUpdateChannel)
	ON_UPDATE_COMMAND_UI(ID_UPDATE_CHANNEL, &CIPTVChannelEditorDlg::OnUpdateAddUpdateChannel)
	ON_COMMAND(ID_UPDATE_ICON, &CIPTVChannelEditorDlg::OnUpdateIcon)
	ON_UPDATE_COMMAND_UI(ID_UPDATE_ICON, &CIPTVChannelEditorDlg::OnUpdateUpdateIcon)
	ON_COMMAND(ID_CHANNEL_UP, &CIPTVChannelEditorDlg::OnChannelUp)
	ON_UPDATE_COMMAND_UI(ID_CHANNEL_UP, &CIPTVChannelEditorDlg::OnUpdateChannelUp)
	ON_COMMAND(ID_CHANNEL_DOWN, &CIPTVChannelEditorDlg::OnChannelDown)
	ON_UPDATE_COMMAND_UI(ID_CHANNEL_DOWN, &CIPTVChannelEditorDlg::OnUpdateChannelDown)
	ON_COMMAND(ID_ADD_CATEGORY, &CIPTVChannelEditorDlg::OnAddCategory)
	ON_UPDATE_COMMAND_UI(ID_ADD_CATEGORY, &CIPTVChannelEditorDlg::OnUpdateAddCategory)
	ON_COMMAND(ID_NEW_CATEGORY, &CIPTVChannelEditorDlg::OnNewCategory)
	ON_COMMAND(ID_REMOVE_CATEGORY, &CIPTVChannelEditorDlg::OnRemoveCategory)
	ON_UPDATE_COMMAND_UI(ID_REMOVE_CATEGORY, &CIPTVChannelEditorDlg::OnUpdateRemoveCategory)
	ON_COMMAND(ID_SORT_CATEGORY, &CIPTVChannelEditorDlg::OnSortCategory)
	ON_UPDATE_COMMAND_UI(ID_SORT_CATEGORY, &CIPTVChannelEditorDlg::OnUpdateSortCategory)
	ON_COMMAND(ID_TOGGLE_CHANNEL, &CIPTVChannelEditorDlg::OnToggleChannel)
	ON_UPDATE_COMMAND_UI(ID_TOGGLE_CHANNEL, &CIPTVChannelEditorDlg::OnUpdateToggleChannel)
	ON_COMMAND(ID_GET_STREAM_INFO, &CIPTVChannelEditorDlg::OnGetStreamInfo)
	ON_UPDATE_COMMAND_UI(ID_GET_STREAM_INFO, &CIPTVChannelEditorDlg::OnUpdateGetStreamInfo)
	ON_COMMAND(ID_GET_STREAM_INFO_ALL, &CIPTVChannelEditorDlg::OnGetStreamInfoAll)
	ON_UPDATE_COMMAND_UI(ID_GET_STREAM_INFO_ALL, &CIPTVChannelEditorDlg::OnUpdateGetStreamInfoAll)
	ON_COMMAND(ID_PLAY_STREAM, &CIPTVChannelEditorDlg::OnPlayStream)
	ON_UPDATE_COMMAND_UI(ID_PLAY_STREAM, &CIPTVChannelEditorDlg::OnUpdatePlayStream)
	ON_COMMAND(ID_SYNC_TREE_ITEM, &CIPTVChannelEditorDlg::OnSyncTreeItem)
	ON_UPDATE_COMMAND_UI(ID_SYNC_TREE_ITEM, &CIPTVChannelEditorDlg::OnUpdateSyncTreeItem)
	ON_COMMAND(ID_ADD_TO_FAVORITE, &CIPTVChannelEditorDlg::OnAddToFavorite)
	ON_UPDATE_COMMAND_UI(ID_ADD_TO_FAVORITE, &CIPTVChannelEditorDlg::OnUpdateAddToFavorite)

	ON_MESSAGE_VOID(WM_KICKIDLE, OnKickIdle)
	ON_MESSAGE(WM_END_LOAD_PLAYLIST, &CIPTVChannelEditorDlg::OnEndLoadPlaylist)
	ON_MESSAGE(WM_UPDATE_PROGRESS, &CIPTVChannelEditorDlg::OnUpdateProgress)

	ON_COMMAND_RANGE(ID_COPY_TO_START, ID_COPY_TO_END, &CIPTVChannelEditorDlg::OnCopyTo)
	ON_COMMAND_RANGE(ID_MOVE_TO_START, ID_MOVE_TO_END, &CIPTVChannelEditorDlg::OnMoveTo)
	ON_COMMAND_RANGE(ID_ADD_TO_START, ID_ADD_TO_END, &CIPTVChannelEditorDlg::OnAddTo)
END_MESSAGE_MAP()

CIPTVChannelEditorDlg::CIPTVChannelEditorDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EDEMCHANNELEDITOR_DIALOG, pParent)
	, m_evtStop(FALSE, TRUE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_plID = _T("ID:");
	m_plEPG = _T("EPG:");
	m_normal = ::GetSysColor(COLOR_WINDOWTEXT);
	m_gray = ::GetSysColor(COLOR_GRAYTEXT);
	m_red = RGB(200, 0, 0);
	m_green = RGB(0, 200, 0);
}

void CIPTVChannelEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_COMBO_PLUGIN_TYPE, m_wndPluginType);
	DDX_Check(pDX, IDC_CHECK_ARCHIVE, m_hasArchive);
	DDX_Control(pDX, IDC_CHECK_ARCHIVE, m_wndArchive);
	DDX_Check(pDX, IDC_CHECK_ADULT, m_isAdult);
	DDX_Control(pDX, IDC_CHECK_ADULT, m_wndAdult);
	DDX_Control(pDX, IDC_TREE_CHANNELS, m_wndChannelsTree);
	DDX_Control(pDX, IDC_CHECK_CUSTOMIZE, m_wndCustom);
	DDX_Text(pDX, IDC_EDIT_URL_ID, m_streamID);
	DDX_Control(pDX, IDC_EDIT_URL_ID, m_wndStreamID);
	DDX_Text(pDX, IDC_EDIT_TVG_ID, m_epgID2);
	DDX_Control(pDX, IDC_EDIT_TVG_ID, m_wndTvgID);
	DDX_Control(pDX, IDC_BUTTON_TEST_TVG, m_wndTestTVG);
	DDX_Text(pDX, IDC_EDIT_EPG_ID, m_epgID1);
	DDX_Control(pDX, IDC_EDIT_EPG_ID, m_wndEpgID);
	DDX_Control(pDX, IDC_BUTTON_TEST_EPG, m_wndTestEPG);
	DDX_Control(pDX, IDC_EDIT_STREAM_URL, m_wndStreamUrl);
	DDX_Text(pDX, IDC_EDIT_STREAM_URL, m_streamUrl);
	DDX_Text(pDX, IDC_EDIT_TIME_SHIFT, m_timeShiftHours);
	DDX_Control(pDX, IDC_EDIT_TIME_SHIFT, m_wndTimeShift);
	DDX_Text(pDX, IDC_EDIT_ARCHIVE_CHECK, m_archiveCheck);
	DDX_Control(pDX, IDC_STATIC_ICON, m_wndIcon);
	DDX_Text(pDX, IDC_EDIT_SEARCH, m_search);
	DDX_Control(pDX, IDC_EDIT_SEARCH, m_wndSearch);
	DDX_Control(pDX, IDC_TREE_PLAYLIST, m_wndPlaylistTree);
	DDX_Control(pDX, IDC_BUTTON_PL_FILTER, m_wndFilter);
	DDX_Control(pDX, IDC_EDIT_PL_SEARCH, m_wndPlSearch);
	DDX_Text(pDX, IDC_EDIT_PL_SEARCH, m_plSearch);
	DDX_Text(pDX, IDC_STATIC_ICON_NAME, m_iconUrl);
	DDX_Control(pDX, IDC_STATIC_PL_ICON, m_wndPlIcon);
	DDX_Text(pDX, IDC_STATIC_PL_ICON_NAME, m_plIconName);
	DDX_Control(pDX, IDC_STATIC_PLAYLIST, m_wndPlInfo);
	DDX_Text(pDX, IDC_STATIC_PL_ID, m_plID);
	DDX_Text(pDX, IDC_STATIC_PL_TVG, m_plEPG);
	DDX_Control(pDX, IDC_CHECK_PL_ARCHIVE, m_wndPlArchive);
	DDX_Text(pDX, IDC_EDIT_INFO_VIDEO, m_infoVideo);
	DDX_Control(pDX, IDC_EDIT_INFO_VIDEO, m_wndInfoVideo);
	DDX_Text(pDX, IDC_EDIT_INFO_AUDIO, m_infoAudio);
	DDX_Control(pDX, IDC_EDIT_ARCHIVE_DAYS, m_wndArchiveDays);
	DDX_Text(pDX, IDC_EDIT_ARCHIVE_DAYS, m_archiveDays);
	DDX_Control(pDX, IDC_EDIT_INFO_AUDIO, m_wndInfoAudio);
	DDX_Control(pDX, IDC_STATIC_CHANNELS, m_wndChInfo);
	DDX_Control(pDX, IDC_BUTTON_GET_INFO, m_wndGetInfo);
	DDX_Control(pDX, IDC_BUTTON_CHECK_ARCHIVE, m_wndCheckArchive);
	DDX_Control(pDX, IDC_COMBO_PLAYLIST, m_wndPlaylist);
	DDX_Control(pDX, IDC_COMBO_CHANNELS, m_wndChannels);
	DDX_Control(pDX, IDC_BUTTON_LOAD_PLAYLIST, m_wndChooseUrl);
	DDX_Control(pDX, IDC_BUTTON_DOWNLOAD_PLAYLIST, m_wndDownloadUrl);
	DDX_Control(pDX, IDC_PROGRESS_LOAD, m_wndProgress);
	DDX_Control(pDX, IDC_BUTTON_CACHE_ICON, m_wndCacheIcon);
	DDX_Control(pDX, IDC_BUTTON_UPDATE_ICON, m_wndUpdateIcon);
	DDX_Control(pDX, IDC_BUTTON_SAVE, m_wndSave);
	DDX_Control(pDX, IDC_SPIN_TIME_SHIFT, m_wndSpinTimeShift);
}

// CEdemChannelEditorDlg message handlers

BOOL CIPTVChannelEditorDlg::OnInitDialog()
{
	__super::OnInitDialog();

	RestoreWindowPos();

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

	if (!m_pToolTipCtrl.Create(this, TTS_ALWAYSTIP))
	{
		TRACE(_T("Unable To create ToolTip\n"));
		return FALSE;
	}

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

	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_COMBO_CHANNELS), _T("Choose channel list to edit"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_COMBO_PLAYLIST), _T("Choose a playlist to import. Standard and Thematic downloaded from it999.ru"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_BUTTON_ADD_NEW_CHANNELS_LIST), _T("Add custom playlist"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_EDIT_SEARCH), _T("Search in channels. Use \\ prefix to find by ID"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_BUTTON_SEARCH_NEXT), _T("Search next"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_EDIT_TVG_ID), _T("EPG ID from teleguide.info"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_BUTTON_TEST_TVG), _T("Test EPG teleguide.info URL"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_EDIT_EPG_ID), _T("EPG ID from it999.ru"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_BUTTON_TEST_EPG), _T("Test EPG it999.ru URL"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_CHECK_CUSTOMIZE), _T("Use custom stream URL for the channel"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_CHECK_ARCHIVE), _T("Channel archive is supported"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_CHECK_ADULT), _T("Channel contents for adults"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_BUTTON_GET_INFO), _T("Get info about selected channel or playlist entry"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_BUTTON_CACHE_ICON), _T("Store icon to the local folder instead of downloading it from internet"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_BUTTON_SAVE), _T("Save channels list"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_BUTTON_ACCESS_INFO), _T("Provider access parameters"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_BUTTON_PACK), _T("Make a plugin to install on player"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_BUTTON_UPDATE_ICON), _T("Set channel icon from original playlist"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_BUTTON_LOAD_PLAYLIST), _T("Load playlist from file"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_BUTTON_DOWNLOAD_PLAYLIST), _T("Save downloaded playlist to disk"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_EDIT_PL_SEARCH), _T("Search in the playlist. Use \\ prefix to find by ID"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_BUTTON_PL_SEARCH_NEXT), _T("Search next"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_BUTTON_PL_FILTER), _T("Filter the playlist"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_STATIC_ICON), _T("Click to change the icon"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_EDIT_ARCHIVE_CHECK), _T("Hours in the past to test archive play"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_SPIN_ARCHIVE_CHECK), _T("Hours in the past to test archive play"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_EDIT_TIME_SHIFT), _T("EPG Time shift for channel, hours"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_SPIN_TIME_SHIFT), _T("EPG Time shift for channel, hours"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_EDIT_INFO_VIDEO), _T("Video stream info"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_EDIT_INFO_AUDIO), _T("Audio stream info"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_COMBO_PLUGIN_TYPE), _T("Plugin type"));

	m_player = theApp.GetProfileString(REG_SETTINGS, REG_PLAYER);
	m_probe = theApp.GetProfileString(REG_SETTINGS, REG_FFPROBE);
	m_archiveCheck = theApp.GetProfileInt(REG_SETTINGS, REG_HOURS_BACK, 0);
	m_bAutoSync = theApp.GetProfileInt(REG_SETTINGS, REG_AUTOSYNC, FALSE);
	m_wndPluginType.SetCurSel(theApp.GetProfileInt(REG_SETTINGS, REG_PLUGIN, 0));

	const auto& regPath = GetPluginRegPath();
	SetAccessKey(theApp.GetProfileString(regPath.c_str(), REG_ACCESS_KEY));
	SetDomain(theApp.GetProfileString(regPath.c_str(), REG_DOMAIN));

	SwitchPlugin();

	UpdateData(FALSE);

	set_allow_save(FALSE);

	m_wndSearch.EnableWindow(FALSE);
	m_wndPlSearch.EnableWindow(FALSE);
	m_wndCustom.EnableWindow(FALSE);
	m_wndTvgID.EnableWindow(FALSE);
	m_wndEpgID.EnableWindow(FALSE);
	m_wndArchive.EnableWindow(FALSE);
	m_wndAdult.EnableWindow(FALSE);
	m_wndTestTVG.EnableWindow(FALSE);
	m_wndTestEPG.EnableWindow(FALSE);
	m_wndStreamID.EnableWindow(FALSE);
	m_wndStreamUrl.EnableWindow(FALSE);
	m_wndCheckArchive.EnableWindow(FALSE);
	m_wndTimeShift.EnableWindow(FALSE);
	m_wndSpinTimeShift.EnableWindow(FALSE);
	m_wndInfoVideo.EnableWindow(FALSE);
	m_wndInfoAudio.EnableWindow(FALSE);
	m_wndChannelsTree.EnableToolTips(TRUE);

	if (m_wndChannels.GetCount() == 0)
	{
		AfxMessageBox(_T("No channels list found!!"), MB_ICONERROR | MB_OK);
		m_wndChannels.EnableWindow(FALSE);
	}
	else
	{
		bool changed = false;
		if (LoadChannels((LPCTSTR)m_wndChannels.GetItemData(m_wndChannels.GetCurSel()), changed))
		{
			FillTreeChannels();
			set_allow_save(changed != false);
		}
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CIPTVChannelEditorDlg::SwitchPlugin()
{
	// Rebuld available playlist types and set current plugin parameters
	switch (m_wndPluginType.GetCurSel())
	{
		case 0: // Edem
		{
			m_pluginType = StreamType::enEdem;
			m_pluginName = _T("Edem");

			m_wndPlaylist.ResetContent();
			m_wndPlaylist.AddString(_T("Edem Standard"));
			m_wndPlaylist.AddString(_T("Edem Thematic"));
			m_wndPlaylist.AddString(_T("Custom URL"));
			int idx = m_wndPlaylist.AddString(_T("Custom File"));
			m_wndPlaylist.SetItemData(idx, TRUE);
			break;
		}
		case 1: // Sharavoz
		{
			m_pluginType = StreamType::enSharovoz;
			m_pluginName = _T("Sharavoz");

			m_wndPlaylist.ResetContent();
			m_wndPlaylist.AddString(_T("Custom URL"));
			int idx = m_wndPlaylist.AddString(_T("Custom File"));
			m_wndPlaylist.SetItemData(idx, TRUE);
			break;
		}
		default:
			ASSERT(false);
			break;
	}

	// Set selected playlist
	int pl_idx = theApp.GetProfileInt(GetPluginRegPath().c_str(), REG_PLAYLIST_TYPE, 0);
	if (pl_idx > m_wndPlaylist.GetCount() || pl_idx < 0)
		pl_idx = 0;

	m_wndPlaylist.SetCurSel(pl_idx);

	const auto& regPath = GetPluginRegPath();

	// Load access credentials
	SetAccessKey(theApp.GetProfileString(regPath.c_str(), REG_ACCESS_KEY));
	SetDomain(theApp.GetProfileString(regPath.c_str(), REG_DOMAIN));

	// Load channel lists
	m_all_channels_lists.clear();

	const auto& channelsPath = fmt::format(theApp.GetAppPath(utils::PLAYLISTS_ROOT).c_str(), GetPluginName().c_str());
	std::error_code err;
	std::filesystem::directory_iterator dir_iter(channelsPath, err);
	for (auto const& dir_entry : dir_iter)
	{
		const auto& path = dir_entry.path();
		if (path.extension() == _T(".xml"))
		{
			m_all_channels_lists.emplace_back(path.filename(), path);
		}
	}

	m_wndChannels.ResetContent();
	for (const auto& playlist : m_all_channels_lists)
	{
		int idx = m_wndChannels.AddString(playlist.first.c_str());
		m_wndChannels.SetItemData(idx, (DWORD_PTR)playlist.second.c_str());
	}

	int idx = theApp.GetProfileInt(regPath.c_str(), REG_CHANNELS_TYPE, 0);
	if (idx < m_wndChannels.GetCount())
		m_wndChannels.SetCurSel(idx);

	// load stream info
	m_stream_infos.clear();
	const auto& path = channelsPath + _T("stream_info.bin");
	std::ifstream is(path, std::istream::binary);
	if (is.good())
	{
		std::vector<char> dump((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
		m_stream_infos.deserialize(dump);
	}

	// Reload selected channels list
	OnCbnSelchangeComboChannels();

	// Reload selected playlist
	OnCbnSelchangeComboPlaylist();
}

std::wstring CIPTVChannelEditorDlg::GetPluginName() const
{
	switch (m_wndPluginType.GetCurSel())
	{
		case 0: // Edem
			return L"edem";
		case 1: // Sharavoz
			return L"sharavoz";
	}

	return L"";
}

std::wstring CIPTVChannelEditorDlg::GetPluginRegPath() const
{
	return fmt::format(_T("{:s}\\{:s}"), REG_SETTINGS, GetPluginName().c_str());
}

void CIPTVChannelEditorDlg::LoadPlaylist(bool saveToFile /*= false*/)
{
	// #EXTM3U <--- header
	// #EXTINF:0 tvg-rec="3",Первый FHD <-- caption
	// #EXTGRP:Общие <-- Category
	// http://6646b6bc.akadatel.com/iptv/PWXQ2KD5G2VNSK/2402/index.m3u8

	CString url;
	int idx = m_wndPlaylist.GetCurSel();
	BOOL isFile = (BOOL)m_wndPlaylist.GetItemData(idx);

	const auto& regPath = GetPluginRegPath();

	switch (m_wndPluginType.GetCurSel())
	{
		case 0: // Edem
		{
			switch (idx)
			{
				case 0:
					url = _T("http://epg.it999.ru/edem_epg_ico.m3u8");
					break;
				case 1:
					url = _T("http://epg.it999.ru/edem_epg_ico2.m3u8");
					break;
				case 2:
					url = theApp.GetProfileString(regPath.c_str(), REG_CUSTOM_URL);
					break;
				case 3:
					url = theApp.GetProfileString(regPath.c_str(), REG_CUSTOM_FILE);
					break;
				default:
					break;
			}
			break;
		}
		case 1: // Sharavoz
		{
			switch (idx)
			{
				case 0:
					url = theApp.GetProfileString(regPath.c_str(), REG_CUSTOM_URL);
					break;
				case 1:
					url = theApp.GetProfileString(regPath.c_str(), REG_CUSTOM_FILE);
					break;
				default:
					break;
			}
			break;
		}
	}

	if (url.IsEmpty())
	{
		AfxMessageBox(_T("Playlist source not set!"), MB_OK | MB_ICONERROR);
		return;
	}

	CString slashed(url);
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

	// #EXTINF:<DURATION> [<KEY>="<VALUE>"]*,<TITLE>
	// Full playlist format
	// #EXTM3U tvg-shift="1"
	// #EXTINF:-1 channel-id="204" group-title="Общие" tvg-id="983" tvg-logo="http://epg.it999.ru/img/983.png" tvg-name="Первый HD" tvg-shift="0",Первый HD
	// http://aaaaaa.akadatel.com/iptv/xxxxxxxxxxxxxx/204/index.m3u8
	//
	// Short (OTTPplay.es) format
	// #EXTM3U
	// #EXTINF:0 tvg-rec="3",Первый HD
	// #EXTGRP:Общие
	// http://6646b6bc.akadatel.com/iptv/PWXQ2KD5G2VNSK/204/index.m3u8

	auto data = std::make_unique<std::vector<BYTE>>();
	if (isFile)
	{
		std::ifstream stream(url.GetString());
		data->assign((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
	}
	else if (utils::CrackUrl(utils::utf16_to_utf8(url.GetString())))
	{
		if (utils::DownloadFile(utils::utf16_to_utf8(url.GetString()), *data))
		{
			if(saveToFile)
			{
				std::ofstream os(m_plFileName);
				os.write((char*)data->data(), data->size());
				os.close();
				return;
			}
		}
		else
		{
			AfxMessageBox(_T("Unable to download playlist!"), MB_OK | MB_ICONERROR);
			return;
		}
	}

	if (data->empty())
	{
		AfxMessageBox(_T("Empty playlist!"), MB_OK | MB_ICONERROR);
		return;
	}

	m_wndProgress.SetRange32(0, (int)std::count(data->begin(), data->end(), '\n'));
	m_wndProgress.SetPos(0);
	m_wndProgress.ShowWindow(SW_SHOW);

	auto* pThread = (CPlaylistParseThread*)AfxBeginThread(RUNTIME_CLASS(CPlaylistParseThread), THREAD_PRIORITY_HIGHEST, 0, CREATE_SUSPENDED);
	if (!pThread)
	{
		AfxMessageBox(_T("Problem with starting load playlist thread!"), MB_OK | MB_ICONERROR);
	}

	m_loading = TRUE;

	m_wndPluginType.EnableWindow(FALSE);
	m_wndGetInfo.EnableWindow(FALSE);
	m_wndCheckArchive.EnableWindow(FALSE);
	m_wndPlaylist.EnableWindow(FALSE);
	m_wndPlaylistTree.EnableWindow(FALSE);
	m_wndChannels.EnableWindow(FALSE);
	m_wndChannelsTree.EnableWindow(FALSE);
	m_wndFilter.EnableWindow(FALSE);
	m_wndDownloadUrl.EnableWindow(FALSE);
	m_evtStop.ResetEvent();

	CPlaylistParseThread::ThreadConfig cfg;
	cfg.m_parent = this;
	cfg.m_data = data.release();
	cfg.m_hStop = m_evtStop;
	cfg.m_pluginType = m_pluginType;
	cfg.m_rootPath = theApp.GetAppPath(utils::PLUGIN_ROOT);

	pThread->SetData(cfg);
	pThread->ResumeThread();
}

LRESULT CIPTVChannelEditorDlg::OnEndLoadPlaylist(WPARAM wParam, LPARAM lParam /*= 0*/)
{
	m_playlistEntries.reset((std::vector<std::shared_ptr<PlaylistEntry>>*)wParam);

	m_wndPluginType.EnableWindow(TRUE);
	m_wndProgress.ShowWindow(SW_HIDE);
	m_wndDownloadUrl.EnableWindow(TRUE);
	m_wndFilter.EnableWindow(TRUE);
	m_wndPlSearch.EnableWindow(!m_channelsMap.empty());
	m_wndPlaylistTree.EnableWindow(TRUE);
	m_wndChannels.EnableWindow(TRUE);
	m_wndChannelsTree.EnableWindow(TRUE);

	FillTreePlaylist();

	m_loading = FALSE;
	m_wndPlaylist.EnableWindow(TRUE);
	m_wndGetInfo.EnableWindow(TRUE);
	m_wndCheckArchive.EnableWindow(TRUE);

	AfxGetApp()->EndWaitCursor();

	return 0;
}

LRESULT CIPTVChannelEditorDlg::OnUpdateProgress(WPARAM wParam, LPARAM lParam /*= 0*/)
{
	m_wndProgress.SetPos(wParam);

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
	UpdateData(FALSE);

	if (is_allow_save() && AfxMessageBox(_T("You have unsaved changes.\nAre you sure?"), MB_YESNO | MB_ICONWARNING) != IDYES)
	{
		return;
	}

	m_evtStop.SetEvent();

	theApp.WriteProfileInt(REG_SETTINGS, REG_PLUGIN, m_wndPluginType.GetCurSel());
	const auto& dump = m_stream_infos.serialize();
	// write document
	const auto& playlistPath = fmt::format(theApp.GetAppPath(utils::PLAYLISTS_ROOT).c_str(), GetPluginName().c_str());
	const auto& path = playlistPath + _T("stream_info.bin");
	std::ofstream os(path, std::istream::binary);
	os.write(dump.data(), dump.size());
	os.close();

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
		m_pToolTipCtrl.RelayEvent(pMsg);
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

void CIPTVChannelEditorDlg::FillTreeChannels()
{
	m_bInFillTree = true;

	m_wndChannelsTree.LockWindowUpdate();

	m_wndChannelsTree.DeleteAllItems();

	m_categoriesTreeMap.clear();

	for (auto& pair : m_categoriesMap)
	{
		TVINSERTSTRUCTW tvCategory = { nullptr };
		tvCategory.hParent = TVI_ROOT;
		tvCategory.item.pszText = (LPWSTR)pair.second->get_title().c_str();
		tvCategory.item.mask = TVIF_TEXT | TVIF_PARAM;
		tvCategory.item.lParam = (DWORD_PTR)pair.second.get();
		tvCategory.hInsertAfter = (pair.first == ID_ADD_TO_FAVORITE) ? TVI_FIRST : nullptr;
		auto hParent = m_wndChannelsTree.InsertItem(&tvCategory);

		m_categoriesTreeMap.emplace(pair.first, hParent);

		for (const auto& channel : pair.second->get_channels())
		{
			TVINSERTSTRUCTW tvChannel = { nullptr };
			tvChannel.hParent = hParent;
			tvChannel.item.pszText = (LPWSTR)channel->get_title().c_str();
			tvChannel.item.lParam = (LPARAM)channel;
			tvChannel.item.mask = TVIF_TEXT | TVIF_PARAM;
			m_wndChannelsTree.InsertItem(&tvChannel);
		}
	}

	UpdateChannelsCount();
	CheckForExistingChannels();

	m_wndChannelsTree.UnlockWindowUpdate();

	m_bInFillTree = false;

	if (!m_channelsMap.empty())
	{
		SearchParams params;
		params.id = m_categoriesMap.begin()->second->get_channels().front()->get_id();
		SelectTreeItem(m_wndChannelsTree, params);
	}
}

void CIPTVChannelEditorDlg::UpdateChannelsCount()
{
	CString str;
	str.Format(_T("Channels: %s - %d"), m_chFileName.GetString(), m_channelsMap.size());
	m_wndChInfo.SetWindowText(str);

	UpdateData(FALSE);
}

void CIPTVChannelEditorDlg::UpdatePlaylistCount()
{
	CString str;
	if (m_playlistIds.size() != m_playlistMap.size())
		str.Format(_T("Playlist: %s - %d (%d)"), m_plFileName.GetString(), m_playlistIds.size(), m_playlistMap.size());
	else
		str.Format(_T("Playlist: %s - %d"), m_plFileName.GetString(), m_playlistIds.size());

	m_wndPlInfo.SetWindowText(str);
	UpdateData(FALSE);
}

std::string CIPTVChannelEditorDlg::GetPlayableURL(const uri_stream* stream_uri,
												  const std::string& access_domain,
												  const std::string& access_key) const
{
	// templated url changed, custom is unchanged
	// edem
	// http://ts://{SUBDOMAIN}/iptv/{UID}/{ID}/index.m3u8 -> http://ts://{SUBDOMAIN}/iptv/{UID}/204/index.m3u8
	// http://ts://{SUBDOMAIN}/iptv/{UID}/204/index.m3u8 -> http://{SUBDOMAIN}/iptv/{UID}/204/index.m3u8
	// http://ts://rtmp.api.rt.com/hls/rtdru.m3u8 -> http://rtmp.api.rt.com/hls/rtdru.m3u8
	// http://ts://{SUBDOMAIN}/iptv/{UID}/205/index.m3u8 -> http://ts://domain.com/iptv/000000000000/205/index.m3u8
	//
	// sharavoz
	// http://{SUBDOMAIN}/{ID}/index.m3u8?token={UID} -> http://{SUBDOMAIN}/204/index.m3u8?token={UID}
	// http://{SUBDOMAIN}/204/index.m3u8?token={UID} -> http://domain.com/204/index.m3u8?token=adsdaSDFJKHKJd

	std::string uri_template;
	switch (m_pluginType)
	{
		case StreamType::enEdem:
			uri_template = URI_TEMPLATE_EDEM;
			break;
		case StreamType::enSharovoz:
			uri_template = URI_TEMPLATE_SHARAVOZ;
			break;
		case StreamType::enGlanz:
		case StreamType::enSharaclub:
		default:
			break;
	}

	auto& uri = stream_uri->is_template() ? utils::string_replace(uri_template, "{ID}", stream_uri->get_id()) : stream_uri->get_uri();
	utils::string_replace_inplace(uri, "{SUBDOMAIN}", access_domain);
	utils::string_replace_inplace(uri, "{UID}", access_key);

	return uri;
}

std::string CIPTVChannelEditorDlg::GetEpg1Template() const
{
	switch (m_pluginType)
	{
		case StreamType::enEdem:
			return "http://epg.ott-play.com/php/show_prog.php?f=edem/epg/{:d}.json";
		case StreamType::enSharovoz:
			return "http://epg.arlekino.tv/api/program?epg={:d}&date={:4d}-{:02d}-{:02d}";
		case StreamType::enBase:
		case StreamType::enChannels:
		case StreamType::enGlanz:
		case StreamType::enSharaclub:
		default:
			break;
	}

	return "";
}

std::string CIPTVChannelEditorDlg::GetEpg2Template() const
{
	switch (m_pluginType)
	{
		case StreamType::enEdem:
			return "http://www.teleguide.info/kanal{:d}_{:4d}{:02d}{:02d}.html";
		case StreamType::enSharovoz:
			return "http://api.program.spr24.net/api/program?epg={:d}&date={:4d}-{:02d}-{:02d}";
		case StreamType::enBase:
		case StreamType::enChannels:
		case StreamType::enGlanz:
		case StreamType::enSharaclub:
		default:
			break;
	}

	return "";
}

void CIPTVChannelEditorDlg::RemoveOrphanChannels()
{
	std::set<std::string> ids;
	for (const auto& pair : m_categoriesMap)
	{
		if (pair.first == ID_ADD_TO_FAVORITE) continue;

		for (const auto& ch : pair.second->get_channels())
		{
			ids.emplace(ch->get_id());
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

void CIPTVChannelEditorDlg::CheckForExistingChannels(HTREEITEM root /*= nullptr*/)
{
	TRACE("Start Check for existing\n");

	if (root == nullptr)
		root = m_wndChannelsTree.GetRootItem();

	while (root != nullptr && !m_playlistMap.empty())
	{
		// iterate subitems
		HTREEITEM hItem = m_wndChannelsTree.GetChildItem(root);
		while (hItem)
		{
			auto channel = (ChannelInfo*)m_wndChannelsTree.GetItemData(hItem);
			if (channel)
			{
				bool bFound = m_playlistMap.find(channel->get_id()) != m_playlistMap.end();
				COLORREF color = channel->is_disabled() ? m_gray : (bFound ? m_green : m_normal);
				m_wndChannelsTree.SetItemColor(hItem, color);
			}
			// get the next sibling item
			hItem = m_wndChannelsTree.GetNextSiblingItem(hItem);
		}

		root = m_wndChannelsTree.GetNextSiblingItem(root);
	}

	TRACE("End Check channels for existing\n");
}

void CIPTVChannelEditorDlg::CheckForExistingPlaylist()
{
	TRACE("Start Check for existing\n");
	HTREEITEM root = m_wndPlaylistTree.GetRootItem();
	while (root != nullptr)
	{
		// iterate subitems
		HTREEITEM hItem = m_wndPlaylistTree.GetChildItem(root);
		while (hItem)
		{
			auto item = (PlaylistEntry*)m_wndPlaylistTree.GetItemData(hItem);
			if (item)
			{
				COLORREF color = (m_channelsMap.find(item->get_id()) == m_channelsMap.end()) ? m_red : m_normal;
				m_wndPlaylistTree.SetItemColor(hItem, color);
			}

			// get the next sibling item
			hItem = m_wndPlaylistTree.GetNextSiblingItem(hItem);
		}

		root = m_wndPlaylistTree.GetNextSiblingItem(root);
	}

	TRACE("End Check playlist for existing\n");
}

void CIPTVChannelEditorDlg::LoadChannelInfo(HTREEITEM hItem)
{
	m_infoAudio.Empty();
	m_infoVideo.Empty();

	auto channel = GetChannel(hItem);
	if (channel)
	{
		m_epgID1 = channel->get_epg1_id();
		m_epgID2 = channel->get_epg2_id();

		m_streamUrl = channel->get_stream_uri()->get_uri().c_str();
		m_streamID = channel->get_id().c_str();
		auto hash = channel->get_stream_uri()->get_hash();
		if (auto pair = m_stream_infos.find(hash); pair != m_stream_infos.end())
		{
			m_infoAudio = pair->second.first.c_str();
			m_infoVideo = pair->second.second.c_str();
		}
		m_wndCustom.SetCheck(!channel->get_stream_uri()->is_template());
		m_timeShiftHours = channel->get_time_shift_hours();
		m_hasArchive = channel->get_archive();
		m_isAdult = channel->get_adult();

		if(m_iconUrl != channel->get_icon_uri().get_uri().c_str())
		{
			m_iconUrl = channel->get_icon_uri().get_uri().c_str();
			const auto& img = GetIconCache().get_icon(channel->get_title(), channel->get_icon_absolute_path());
			utils::SetImage(img, m_wndIcon);
		}
	}
	else
	{
		m_epgID2 = 0;
		m_epgID1 = 0;
		m_timeShiftHours = 0;
		m_iconUrl.Empty();
		m_streamUrl.Empty();
		m_streamID.Empty();
		m_infoAudio.Empty();
		m_infoVideo.Empty();
		m_wndIcon.SetBitmap(nullptr);
		m_wndCustom.SetCheck(FALSE);
	}

	UpdateData(FALSE);
}

void CIPTVChannelEditorDlg::LoadPlayListInfo(HTREEITEM hItem)
{
	UpdateData(TRUE);

	m_infoAudio.Empty();
	m_infoVideo.Empty();

	auto entry = GetPlaylistEntry(hItem);
	if (entry)
	{
		m_plIconName = entry->get_icon_uri().get_uri().c_str();
		m_plID.Format(_T("ID: %hs"), entry->get_id().c_str());

		if (entry->get_epg2_id() > 0)
			m_plEPG.Format(_T("EPG: %d"), entry->get_epg2_id());

		m_wndPlArchive.SetCheck(entry->get_archive() != 0);
		m_archiveDays = entry->get_archive();
		auto hash = entry->get_stream_uri()->get_hash();
		if (auto pair = m_stream_infos.find(hash); pair != m_stream_infos.end())
		{
			m_infoAudio = pair->second.first.c_str();
			m_infoVideo = pair->second.second.c_str();
		}

		const auto& img = GetIconCache().get_icon(entry->get_title(), entry->get_icon_absolute_path());
		utils::SetImage(img, m_wndPlIcon);

		if (m_bAutoSync)
		{
			OnSyncTreeItem();
		}

		UpdateData(FALSE);
	}
}

ChannelInfo* CIPTVChannelEditorDlg::GetChannel(HTREEITEM hItem) const
{
	return IsChannel(hItem) ? (ChannelInfo*)m_wndChannelsTree.GetItemData(hItem) : nullptr;
}

std::shared_ptr<ChannelInfo> CIPTVChannelEditorDlg::FindChannel(HTREEITEM hItem) const
{
	auto info = GetBaseInfo(&m_wndChannelsTree, hItem);
	if (info == nullptr || !info->is_type(InfoType::enChannel))
		return nullptr;

	auto pair = m_channelsMap.find(info->get_id());
	return pair != m_channelsMap.end() ? pair->second : nullptr;
}

ChannelCategory* CIPTVChannelEditorDlg::GetItemCategory(HTREEITEM hItem) const
{
	if (hItem != nullptr && m_wndChannelsTree.GetParentItem(hItem) != nullptr)
	{
		hItem = m_wndChannelsTree.GetParentItem(hItem);
	}

	return GetCategory(hItem);
}

ChannelCategory* CIPTVChannelEditorDlg::GetCategory(HTREEITEM hItem) const
{
	auto info = GetBaseInfo(&m_wndChannelsTree, hItem);
	if (info == nullptr || !info->is_type(InfoType::enCategory))
		return nullptr;

	auto found = m_categoriesMap.find(info->get_key());

	return found != m_categoriesMap.end() ? found->second.get() : nullptr;
}

HTREEITEM CIPTVChannelEditorDlg::GetCategoryTreeItemById(int id) const
{
	HTREEITEM hRoot = m_wndChannelsTree.GetRootItem();
	while (hRoot != nullptr)
	{
		if ((int)m_wndChannelsTree.GetItemData(hRoot) == id)
			return hRoot;

		hRoot = m_wndChannelsTree.GetNextSiblingItem(hRoot);
	}

	return nullptr;
}

PlaylistEntry* CIPTVChannelEditorDlg::GetPlaylistEntry(HTREEITEM hItem) const
{
	if (hItem == nullptr || m_wndPlaylistTree.GetParentItem(hItem) == nullptr)
		return nullptr;

	return (PlaylistEntry*)m_wndPlaylistTree.GetItemData(hItem);
}

int CIPTVChannelEditorDlg::GetNewCategoryID() const
{
	int id = 0;
	if (!m_categoriesMap.empty())
	{
		for(auto pair = m_categoriesMap.crbegin(); pair != m_categoriesMap.crend(); ++pair)
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

int CIPTVChannelEditorDlg::GetCategoryIdByName(const std::wstring& categoryName)
{
	// Search for existing category
	for (const auto& category : m_categoriesMap)
	{
		if (category.second->get_title() == categoryName)
		{
			return category.first;
		}
	}

	return -1;
}

bool CIPTVChannelEditorDlg::LoadChannels(const CString& path, bool& changed)
{
	set_allow_save(FALSE);

	m_categoriesMap.clear();
	m_channelsMap.clear();

	std::ifstream is(path.GetString(), std::istream::binary);
	if (!is.good())
		return false;

	auto pos = path.ReverseFind('\\');
	if (pos != -1)
	{
		m_chFileName = path.Mid(++pos);
	}
	else
	{
		m_chFileName = path;
	}

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


	auto i_node = doc.first_node(utils::TV_INFO);

	m_embedded_info = FALSE;
	m_ch_access_key.clear();
	m_ch_domain.clear();
	auto setup_node = i_node->first_node(utils::CHANNELS_SETUP);
	if (setup_node)
	{
		m_ch_access_key = utils::get_value_string(setup_node->first_node(utils::ACCESS_KEY));
		m_ch_domain = utils::get_value_string(setup_node->first_node(utils::ACCESS_DOMAIN));
		m_embedded_info = TRUE;
	}

	const auto& root_path = theApp.GetAppPath(utils::PLUGIN_ROOT);
	auto cat_node = i_node->first_node(utils::TV_CATEGORIES)->first_node(ChannelCategory::TV_CATEGORY);
	// Iterate <tv_category> nodes
	while (cat_node)
	{
		auto category = std::make_unique<ChannelCategory>(cat_node, StreamType::enBase, root_path);
		m_categoriesMap.emplace(category->get_key(), std::move(category));
		cat_node = cat_node->next_sibling();
	}

	auto fav_category = std::make_unique<ChannelCategory>(StreamType::enBase, root_path);
	fav_category->set_icon_uri("plugin_file://icons/fav.png");
	fav_category->set_title(L"Favorites");
	fav_category->set_key(ID_ADD_TO_FAVORITE);

	auto ch_node = i_node->first_node(utils::TV_CHANNELS)->first_node(ChannelInfo::TV_CHANNEL);
	// Iterate <tv_channel> nodes
	while (ch_node)
	{
		auto channel = std::make_shared<ChannelInfo>(ch_node, StreamType::enChannels, root_path);
		changed = channel->is_changed();
		auto ch_pair = m_channelsMap.find(channel->get_id());
		if (ch_pair == m_channelsMap.end())
		{
			m_channelsMap.emplace(channel->get_id(), channel);
			if (channel->is_favorite())
				fav_category->add_channel(channel);
		}

		for (const auto& id : channel->get_category_ids())
		{
			auto cat_pair = m_categoriesMap.find(id);
			ASSERT(cat_pair != m_categoriesMap.end());
			cat_pair->second->add_channel(channel);
		}

		if (ch_pair != m_channelsMap.end())
		{
			ch_pair->second->get_category_ids().insert(channel->get_category_ids().begin(), channel->get_category_ids().end());
		}

		ch_node = ch_node->next_sibling();
	}

	m_categoriesMap.emplace(ID_ADD_TO_FAVORITE, std::move(fav_category));

	m_wndSearch.EnableWindow(!m_channelsMap.empty());

	return true;
}

void CIPTVChannelEditorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		__super::OnSysCommand(nID, lParam);
	}
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
	BOOL autoSyncOld = m_bAutoSync;
	m_bAutoSync = FALSE;

	bool needCheckExisting = false;
	for (const auto& hCategory : m_wndPlaylistTree.GetSelectedItems())
	{
		for (auto hIter = m_wndPlaylistTree.GetChildItem(hCategory); hIter != nullptr; hIter = m_wndPlaylistTree.GetNextSiblingItem(hIter))
		{
			needCheckExisting |= AddChannel(hIter);
		}
	}

	if (needCheckExisting)
	{
		CheckForExistingChannels();
		CheckForExistingPlaylist();
	}

	m_bAutoSync = autoSyncOld;

	UpdateChannelsCount();
	LoadChannelInfo(m_wndChannelsTree.GetSelectedItem());
	set_allow_save();
}

void CIPTVChannelEditorDlg::OnUpdateAddCategory(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsPlaylistCategory(m_wndPlaylistTree.GetFirstSelectedItem()) && IsSelectedTheSameType());
}

void CIPTVChannelEditorDlg::OnNewChannel()
{
	auto hCategory = m_wndChannelsTree.GetSelectedItem();
	if (auto hRoot = m_wndChannelsTree.GetParentItem(hCategory); hRoot != nullptr)
	{
		hCategory = hRoot;
	}

	auto category = GetCategory(hCategory);
	if (!category)
		return;

	auto channel = std::make_shared<ChannelInfo>(StreamType::enChannels, theApp.GetAppPath(utils::PLUGIN_ROOT));
	channel->set_title(L"New Channel");
	channel->set_icon_uri(utils::ICON_TEMPLATE);

	CImage img;
	if (utils::LoadImage(channel->get_icon_absolute_path(), img))
	{
		utils::SetImage(img, m_wndIcon);
	}

	TVINSERTSTRUCTW tvInsert = { nullptr };
	tvInsert.hParent = hCategory;
	tvInsert.item.pszText = (LPWSTR)channel->get_title().c_str();
	tvInsert.item.lParam = (LPARAM)channel.get();
	tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
	auto hNewItem = m_wndChannelsTree.InsertItem(&tvInsert);

	category->add_channel(channel);

	m_wndChannelsTree.SelectItem(hNewItem);
	m_wndChannelsTree.EditLabel(hNewItem);
}

void CIPTVChannelEditorDlg::OnUpdateNewChannel(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!!IsSelectedNotFavorite());
}

void CIPTVChannelEditorDlg::OnRemoveChannel()
{
	CWnd* pFocused = GetFocus();
	if (pFocused != &m_wndChannelsTree)
	{
		pFocused->SendMessage(WM_KEYDOWN, VK_DELETE);
		return;
	}

	if (!m_wndChannelsTree.GetSelectedCount() || AfxMessageBox(_T("Delete channel(s). Are your sure?"), MB_YESNO | MB_ICONWARNING) != IDYES)
		return;

	auto category = GetItemCategory(m_wndChannelsTree.GetFirstSelectedItem());
	if (!category) return;

	std::vector<HTREEITEM> toDelete;
	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		auto channel = GetChannel(hItem);
		if (channel == nullptr) continue;

		if (category->get_key() == ID_ADD_TO_FAVORITE)
			channel->set_favorite(false);

		toDelete.emplace_back(hItem);
		category->remove_channel(channel->get_id());
	}

	for (const auto& hItem : toDelete)
		m_wndChannelsTree.DeleteItem(hItem);

	RemoveOrphanChannels();
	CheckForExistingPlaylist();
	set_allow_save();
	UpdateChannelsCount();
}

void CIPTVChannelEditorDlg::OnUpdateRemoveChannel(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!!IsSelectedChannelsOrEntries(true) && IsSelectedTheSameCategory());
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
		SwapCategories(hTop, m_wndChannelsTree.GetPrevSiblingItem(hTop));
	}
}

void CIPTVChannelEditorDlg::OnUpdateChannelUp(CCmdUI* pCmdUI)
{
	HTREEITEM hCur = m_wndChannelsTree.GetFirstSelectedItem();
	BOOL enable = (hCur != nullptr)
		&& IsChannelSelectionConsistent()
		&& IsSelectedTheSameCategory()
		&& IsSelectedNotFavorite()
		&& (IsChannel(hCur) || (IsCategory(hCur) && m_wndChannelsTree.GetSelectedCount() == 1))
		&& m_wndChannelsTree.GetPrevSiblingItem(hCur) != nullptr;

	pCmdUI->Enable(enable);
}

void CIPTVChannelEditorDlg::OnChannelDown()
{
	HTREEITEM hTop = m_wndChannelsTree.GetFirstSelectedItem();
	HTREEITEM hBottom = m_wndChannelsTree.GetLastSelectedItem();

	if(IsChannel(m_wndChannelsTree.GetNextSiblingItem(hBottom)))
	{
		MoveChannels(hTop, hBottom, true);
	}
	else if (IsCategory(hTop) && m_wndChannelsTree.GetSelectedCount() == 1)
	{
		SwapCategories(hTop, m_wndChannelsTree.GetNextSiblingItem(hTop));
	}
}

void CIPTVChannelEditorDlg::OnUpdateChannelDown(CCmdUI* pCmdUI)
{
	HTREEITEM hCur = m_wndChannelsTree.GetFirstSelectedItem();
	BOOL enable = (hCur != nullptr)
		&& IsChannelSelectionConsistent()
		&& IsSelectedTheSameCategory()
		&& IsSelectedNotFavorite()
		&& (IsChannel(hCur) || (IsCategory(hCur) && m_wndChannelsTree.GetSelectedCount() == 1))
		&& m_wndChannelsTree.GetNextSiblingItem(m_wndChannelsTree.GetLastSelectedItem()) != nullptr;

	pCmdUI->Enable(enable);
}

void CIPTVChannelEditorDlg::MoveChannels(HTREEITEM hBegin, HTREEITEM hEnd, bool down)
{
	auto category = GetItemCategory(hBegin);

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

	category->move_channels(GetChannel(hBegin), GetChannel(hEnd), down);

	auto channel = GetChannel(hMoved);
	TVINSERTSTRUCTW tvInsert = { nullptr };
	tvInsert.hParent = m_wndChannelsTree.GetParentItem(hBegin);
	tvInsert.item.pszText = (LPWSTR)channel->get_title().c_str();
	tvInsert.item.lParam = (LPARAM)channel;
	tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
	tvInsert.hInsertAfter = hAfter;

	m_wndChannelsTree.SetItemColor(m_wndChannelsTree.InsertItem(&tvInsert), m_wndChannelsTree.GetItemColor(hMoved));
	m_wndChannelsTree.DeleteItem(hMoved);

	set_allow_save();
}

void CIPTVChannelEditorDlg::SwapCategories(const HTREEITEM hLeft, const HTREEITEM hRight)
{
	auto lCat = GetCategory(hLeft);
	auto rCat = GetCategory(hRight);

	// swap categories id
	lCat->swap_id(*rCat);

	// swap pointers in map
	std::swap(m_categoriesMap[lCat->get_key()], m_categoriesMap[rCat->get_key()]);
	// swap HTREEITEM in map
	std::swap(m_categoriesTreeMap[lCat->get_key()], m_categoriesTreeMap[rCat->get_key()]);

	// запоминаем ItemData для нод и подменяем на счетчик
	std::vector<std::pair<HTREEITEM, DWORD_PTR>> itemData;
	DWORD_PTR idx = 0;
	for (HTREEITEM hItem = m_wndChannelsTree.GetChildItem(nullptr); hItem != nullptr; hItem = m_wndChannelsTree.GetNextSiblingItem(hItem))
	{
		itemData.emplace_back(hItem, m_wndChannelsTree.GetItemData(hItem));
		m_wndChannelsTree.SetItemData(hItem, idx++);
	}

	// Меняем местами нужные ItemData для сортировки
	idx = m_wndChannelsTree.GetItemData(hRight);
	m_wndChannelsTree.SetItemData(hRight, m_wndChannelsTree.GetItemData(hLeft));
	m_wndChannelsTree.SetItemData(hLeft, idx);

	// сортируем. Пусть TreeCtrl сам переупорядочит внутренний список
	TVSORTCB sortInfo = { nullptr };
	sortInfo.lpfnCompare = &CBCompareForSwap;
	m_wndChannelsTree.SortChildrenCB(&sortInfo);

	// Восстанавливаем значения ItemData
	for (const auto& it : itemData)
	{
		m_wndChannelsTree.SetItemData(it.first, it.second);
	}

	m_wndChannelsTree.SelectItem(hLeft);

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
	pCmdUI->Enable(m_wndChannelsTree.GetSelectedCount() == 1 && GetChannel(m_wndChannelsTree.GetSelectedItem()) != nullptr && IsSelectedNotFavorite());
}

void CIPTVChannelEditorDlg::OnBnClickedCheckCustomize()
{
	BOOL checked = m_wndCustom.GetCheck();
	m_wndStreamUrl.EnableWindow(checked);
	m_wndStreamID.EnableWindow(!checked);

	set_allow_save();
}

void CIPTVChannelEditorDlg::OnTvnSelchangedTreeChannels(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (m_bInFillTree)
		return;

	HTREEITEM hSelected = reinterpret_cast<LPNMTREEVIEW>(pNMHDR)->itemNew.hItem;
	int state = 0;
	bool bSameType = IsSelectedTheSameType();
	if (bSameType)
	{
		auto channel = GetChannel(hSelected);
		if (channel != nullptr)
		{
			LoadChannelInfo(hSelected);
			state = 2;
			m_wndIcon.EnableWindow(FALSE);
		}

		if (m_wndChannelsTree.GetSelectedCount() == 1)
		{
			if (channel != nullptr)
			{
				state = 1;
				if (!channel->get_stream_uri()->is_template())
					m_streamID.Empty();
				m_wndIcon.EnableWindow(TRUE);
			}
			else if (IsCategory(hSelected))
			{
				m_epgID2 = 0;
				m_epgID1 = 0;
				m_hasArchive = 0;
				m_isAdult = 0;
				m_streamUrl.Empty();
				m_streamID.Empty();
				m_infoAudio.Empty();
				m_infoVideo.Empty();

				m_wndIcon.EnableWindow(TRUE);
				auto category = GetCategory(hSelected);
				if (category)
				{
					m_iconUrl = category->get_icon_uri().get_uri().c_str();
					CImage img;
					if (utils::LoadImage(category->get_icon_absolute_path(), img))
					{
						utils::SetImage(img, m_wndIcon);
					}
					else
					{
						m_wndIcon.SetBitmap(nullptr);
					}
				}
			}
		}
	}

	BOOL enable = (state == 1);
	bool bSameCategory = IsSelectedTheSameCategory();

	m_wndCustom.EnableWindow(enable);
	m_wndTvgID.EnableWindow(enable && m_pluginType == StreamType::enEdem);
	m_wndEpgID.EnableWindow(enable && m_pluginType == StreamType::enEdem);
	m_wndArchive.EnableWindow(state);
	m_wndAdult.EnableWindow(state);
	m_wndTestTVG.EnableWindow(enable && m_pluginType == StreamType::enEdem);
	m_wndTestEPG.EnableWindow(enable);
	m_wndStreamID.EnableWindow(enable && !m_streamID.IsEmpty());
	m_wndStreamUrl.EnableWindow(enable && m_streamID.IsEmpty());
	m_wndCheckArchive.EnableWindow(enable && !m_probe.IsEmpty() && !m_loading);
	m_wndTimeShift.EnableWindow(state);
	m_wndSpinTimeShift.EnableWindow(state);
	m_wndInfoVideo.EnableWindow(enable);
	m_wndInfoAudio.EnableWindow(enable);
	m_wndSearch.EnableWindow(TRUE);

	if (state == 2)
	{
		BOOL bEnable = !m_probe.IsEmpty();
		if (m_lastTree)
		{
			bEnable &= (GetBaseInfo(m_lastTree, m_lastTree->GetFirstSelectedItem()) != nullptr);
		}
		else
		{
			bEnable = FALSE;
		}

		bEnable = bEnable && bSameType;
		m_wndGetInfo.EnableWindow(bEnable && !m_loading);
	}
	else
	{
		m_wndGetInfo.EnableWindow(state && !m_loading);
	}

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
		auto entry = GetBaseInfo(&m_wndChannelsTree, pTVDispInfo->item.hItem);
		if (entry)
		{
			entry->set_title(pTVDispInfo->item.pszText);
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
	if (hItem && (uFlags & TVHT_ONITEM))
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

	UINT uFlags;
	HTREEITEM hItem = m_wndChannelsTree.HitTest(ptClient, &uFlags);

	if(!hItem || !(TVHT_ONITEM & uFlags))
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

		auto itemCategory = GetItemCategory(m_wndChannelsTree.GetFirstSelectedItem());
		if (itemCategory != nullptr && itemCategory->get_key() != ID_ADD_TO_FAVORITE)
		{
			for (const auto& category : m_categoriesMap)
			{
				if (ID_ADD_TO_FAVORITE == category.first || itemCategory->get_key() == category.first) continue;

				subMenuCopy.AppendMenu(MF_STRING | MF_ENABLED, ID_COPY_TO_START + category.first, category.second->get_title().c_str());
				subMenuMove.AppendMenu(MF_STRING | MF_ENABLED, ID_MOVE_TO_START + category.first, category.second->get_title().c_str());
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

		auto entry = GetPlaylistEntry(m_wndPlaylistTree.GetFirstSelectedItem());

		for (const auto& category : m_categoriesMap)
		{
			subMenuCopy.AppendMenu(MF_STRING | MF_ENABLED, ID_ADD_TO_START + category.first, category.second->get_title().c_str());
		}

		pMenu->InsertMenu(ID_PLAYLISTMENU_SUBMENU, MF_BYCOMMAND | MF_POPUP, (UINT_PTR)subMenuCopy.Detach(), _T("Add/Update To"));
	}

	pMenu->DeleteMenu(ID_PLAYLISTMENU_SUBMENU, MF_BYCOMMAND);

	CContextMenuManager* manager = theApp.GetContextMenuManager();
	//for CDialogEx:
	theApp.GetContextMenuManager()->ShowPopupMenu(pMenu->GetSafeHmenu(), ptScreen.x, ptScreen.y, this, TRUE, TRUE, FALSE);
}

void CIPTVChannelEditorDlg::OnAddToFavorite()
{
	auto pair = m_categoriesMap.find(ID_ADD_TO_FAVORITE);
	if (pair == m_categoriesMap.end())
		return;

	bool changed = false;
	HTREEITEM hTarget = GetCategoryTreeItemById(pair->first);
	HTREEITEM hLastItem = nullptr;

	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		auto channel = FindChannel(hItem);
		if (!channel) continue;

		changed = true;
		channel->set_favorite(true);
		pair->second->add_channel(channel);

		TVINSERTSTRUCTW tvInsert = { nullptr };
		tvInsert.hParent = hTarget;
		tvInsert.item.pszText = (LPWSTR)channel->get_title().c_str();
		tvInsert.item.lParam = (LPARAM)channel.get();
		tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
		m_wndChannelsTree.InsertItem(&tvInsert);
	}

	if (changed)
	{
		CheckForExistingChannels(hTarget);
		m_wndChannelsTree.SelectItem(hLastItem);
		set_allow_save();
	}
}

void CIPTVChannelEditorDlg::OnUpdateAddToFavorite(CCmdUI* pCmdUI)
{
	auto itemCategory = GetItemCategory(m_wndChannelsTree.GetFirstSelectedItem());

	pCmdUI->Enable(itemCategory != nullptr && itemCategory->get_key() != ID_ADD_TO_FAVORITE);
}

void CIPTVChannelEditorDlg::OnCopyTo(UINT id)
{
	UINT category_id = id - ID_COPY_TO_START;
	auto pair = m_categoriesMap.find(category_id);
	if (pair == m_categoriesMap.end())
		return;

	bool changed = false;
	HTREEITEM hTarget = GetCategoryTreeItemById(pair->first);
	HTREEITEM hLastItem = nullptr;

	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		auto channel = FindChannel(hItem);
		if (!channel) continue;

		changed = true;
		pair->second->add_channel(channel);

		TVINSERTSTRUCTW tvInsert = { nullptr };
		tvInsert.hParent = hTarget;
		tvInsert.item.pszText = (LPWSTR)channel->get_title().c_str();
		tvInsert.item.lParam = (LPARAM)channel.get();
		tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
		m_wndChannelsTree.InsertItem(&tvInsert);
	}

	if (changed)
	{
		CheckForExistingChannels(hTarget);
		m_wndChannelsTree.SelectItem(hLastItem);
		set_allow_save();
	}
}

void CIPTVChannelEditorDlg::OnMoveTo(UINT id)
{
	UINT category_id = id - ID_MOVE_TO_START;
	auto pair = m_categoriesMap.find(category_id);
	if (pair == m_categoriesMap.end())
		return;

	bool changed = false;
	HTREEITEM hTarget = GetCategoryTreeItemById(pair->first);
	HTREEITEM hNewItem = nullptr;
	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		auto category = GetItemCategory(hItem);
		auto pair_ch = m_channelsMap.find(GetChannel(hItem)->get_id());
		if (pair_ch == m_channelsMap.end()) continue;

		auto& channel = pair_ch->second;
		changed = true;

		pair->second->add_channel(channel);

		category->remove_channel(channel->get_id());
		m_wndChannelsTree.DeleteItem(hItem);

		TVINSERTSTRUCTW tvInsert = { nullptr };
		tvInsert.hParent = hTarget;
		tvInsert.item.pszText = (LPWSTR)channel->get_title().c_str();
		tvInsert.item.lParam = (LPARAM)channel.get();
		tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
		hNewItem = m_wndChannelsTree.InsertItem(&tvInsert);

	}

	if (changed)
	{
		CheckForExistingChannels(hTarget);
		m_wndChannelsTree.SelectItem(hNewItem);
		set_allow_save();
	}
}

void CIPTVChannelEditorDlg::OnAddTo(UINT id)
{
	UINT category_id = id - ID_ADD_TO_START;
	auto pair = m_categoriesMap.find(category_id);
	if (pair == m_categoriesMap.end())
		return;

	m_wndChannelsTree.LockWindowUpdate();

	HTREEITEM hTarget = GetCategoryTreeItemById(pair->first);
	bool changed = false;
	for (const auto& hSelectedItem : m_wndPlaylistTree.GetSelectedItems())
	{
		changed |= AddChannel(hSelectedItem, category_id);
	}

	OnSyncTreeItem();

	if (changed)
	{
		CheckForExistingChannels(hTarget);
		CheckForExistingPlaylist();
		UpdateChannelsCount();
		set_allow_save();
	}

	m_wndChannelsTree.UnlockWindowUpdate();
}

void CIPTVChannelEditorDlg::OnEditChangeTvIdd()
{
	set_allow_save();
}

void CIPTVChannelEditorDlg::OnBnClickedCheckAdult()
{
	UpdateData(TRUE);
	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		auto channel = GetChannel(hItem);
		if (channel)
			channel->set_adult(m_isAdult);
	}

	set_allow_save();
}

void CIPTVChannelEditorDlg::OnBnClickedCheckArchive()
{
	UpdateData(TRUE);
	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		auto channel = GetChannel(hItem);
		if (channel)
			channel->set_archive(m_hasArchive);
	}

	set_allow_save();
}

void CIPTVChannelEditorDlg::OnEnChangeEditTvgID()
{
	UpdateData(TRUE);
	if (m_wndChannelsTree.GetSelectedCount() == 1)
	{
		auto channel = GetChannel(m_wndChannelsTree.GetSelectedItem());
		if (channel)
		{
			channel->set_epg2_id(m_epgID2);
			set_allow_save();
		}
	}
}

void CIPTVChannelEditorDlg::OnEnChangeEditEpgID()
{
	UpdateData(TRUE);
	if (m_wndChannelsTree.GetSelectedCount() == 1)
	{
		auto channel = GetChannel(m_wndChannelsTree.GetSelectedItem());
		if (channel)
		{
			channel->set_epg1_id(m_epgID1);
			set_allow_save();
		}
	}
}

void CIPTVChannelEditorDlg::OnEnChangeEditStreamUrl()
{
	UpdateData(TRUE);
	if (m_wndChannelsTree.GetSelectedCount() == 1)
	{
		auto channel = GetChannel(m_wndChannelsTree.GetSelectedItem());
		if (channel && m_wndCustom.GetCheck())
		{
			channel->set_stream_uri(utils::utf16_to_utf8(m_streamUrl.GetString()));
			set_allow_save();
		}
	}
}

void CIPTVChannelEditorDlg::OnEnChangeEditUrlID()
{
	UpdateData(TRUE);
	if (m_wndChannelsTree.GetSelectedCount() == 1)
	{
		auto channel = GetChannel(m_wndChannelsTree.GetSelectedItem());
		if (channel && channel->get_id() != CStringA(m_streamID).GetString())
		{
			channel->set_id(CStringA(m_streamID).GetString());
			CheckForExistingChannels(m_wndChannelsTree.GetParentItem(m_wndChannelsTree.GetSelectedItem()));
			CheckForExistingPlaylist();
			set_allow_save();
		}
	}
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
		auto channel = GetChannel(hItem);
		if (channel)
		{
			channel->set_time_shift_hours(m_timeShiftHours);
		}
	}

	set_allow_save();
}

void CIPTVChannelEditorDlg::OnEnChangeEditArchiveCheck()
{
	UpdateData(TRUE);

	if (m_archiveCheck < 0)
		m_archiveCheck = 0;

	UpdateData(FALSE);

	theApp.WriteProfileInt(REG_SETTINGS, REG_HOURS_BACK, m_archiveCheck);
}

void CIPTVChannelEditorDlg::OnDeltaposSpinTimeShiftHours(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateData(TRUE);
	m_timeShiftHours -= reinterpret_cast<LPNMUPDOWN>(pNMHDR)->iDelta;
	UpdateData(FALSE);
	OnEnChangeEditTimeShiftHours();
	*pResult = 0;
}

void CIPTVChannelEditorDlg::OnDeltaposSpinArchiveCheck(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateData(TRUE);
	m_archiveCheck -= reinterpret_cast<LPNMUPDOWN>(pNMHDR)->iDelta;
	UpdateData(FALSE);
	OnEnChangeEditArchiveCheck();
	*pResult = 0;
}

void CIPTVChannelEditorDlg::OnBnClickedButtonTestEpg1()
{
	auto channel = GetChannel(m_wndChannelsTree.GetSelectedItem());
	if (channel)
	{
		COleDateTime dt = COleDateTime::GetCurrentTime();
		std::string url;
		switch (m_pluginType)
		{
			case StreamType::enEdem:
				url = fmt::format(GetEpg1Template(), channel->get_epg1_id());
				break;
			case StreamType::enSharovoz:
				url = fmt::format(GetEpg1Template(), channel->get_epg1_id(), dt.GetYear(), dt.GetMonth(), dt.GetDay());
				break;
			case StreamType::enGlanz:
				break;
			case StreamType::enSharaclub:
				break;
			default:
				break;
		}

		if (!url.empty())
			ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	}
}

void CIPTVChannelEditorDlg::OnBnClickedButtonTestEpg2()
{
	auto channel = GetChannel(m_wndChannelsTree.GetSelectedItem());
	if (channel)
	{
		COleDateTime dt = COleDateTime::GetCurrentTime();
		std::string url;
		switch (m_pluginType)
		{
			case StreamType::enEdem:
				url = fmt::format(GetEpg2Template(), channel->get_epg2_id(), dt.GetYear(), dt.GetMonth(), dt.GetDay());
				break;
			case StreamType::enSharovoz:
				url = fmt::format(GetEpg2Template(), channel->get_epg2_id(), dt.GetYear(), dt.GetMonth(), dt.GetDay());
				break;
			case StreamType::enGlanz:
				break;
			case StreamType::enSharaclub:
				break;
			default:
				break;
		}

		ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	}
}

void CIPTVChannelEditorDlg::PlayItem(HTREEITEM hItem, int archive_hour /*= 0*/) const
{
	if (auto entry = GetBaseInfo(m_lastTree, hItem); entry != nullptr)
	{
		const auto& access_domain = CIPTVChannelEditorDlg::GetAccessDomain();
		const auto& access_key = CIPTVChannelEditorDlg::GetAccessKey();
		PlayStream(GetPlayableURL(entry->get_stream_uri(), access_domain, access_key), archive_hour);
	}
}

void CIPTVChannelEditorDlg::PlayStream(const std::string& stream_url, int archive_hour /*= 0*/) const
{
	TRACE(_T("Test URL: %s\n"), stream_url.c_str());
	CStringW test(stream_url.c_str());
	if (archive_hour)
	{
		test.Format(L"%hs?utc=%d&lutc=%d", stream_url.c_str(), _time32(nullptr) - 3600 * archive_hour, _time32(nullptr));
	}

	ShellExecuteW(nullptr, L"open", m_player, test, nullptr, SW_SHOWNORMAL);
}

void CIPTVChannelEditorDlg::OnBnClickedButtonCustomPlaylist()
{
	CCustomPlaylistDlg dlg;
	dlg.m_isFile = (BOOL)m_wndPlaylist.GetItemData(m_wndPlaylist.GetCurSel());
	LPCTSTR szType = dlg.m_isFile ? REG_CUSTOM_FILE : REG_CUSTOM_URL;
	dlg.m_url = theApp.GetProfileString(GetPluginRegPath().c_str(), szType);
	if (dlg.DoModal() == IDOK)
	{
		theApp.WriteProfileString(GetPluginRegPath().c_str(), szType, dlg.m_url);
		LoadPlaylist();
	}
}

void CIPTVChannelEditorDlg::FillTreePlaylist()
{
	m_bInFillTree = true;

	m_wndPlaylistTree.DeleteAllItems();

	// Filter out playlist
	const auto& regPath = GetPluginRegPath();
	auto filter = theApp.GetProfileString(regPath.c_str(), REG_FILTER_STRING);
	auto bRegex = theApp.GetProfileInt(regPath.c_str(), REG_FILTER_REGEX, FALSE);
	auto bCase = theApp.GetProfileInt(regPath.c_str(), REG_FILTER_CASE, FALSE);

	std::wregex re;
	if (bRegex)
	{
		try
		{
			re = filter.GetString();
		}
		catch (std::regex_error& ex)
		{
			ex;
			bRegex = false;
			filter.Empty();
		}
	}

	m_playlistIds.clear();
	m_playlistMap.clear();
	m_pl_categoriesTreeMap.clear();

	// list of playlist categories in the same order as in the playlist
	// Must not contains duplicates!
	std::vector<std::wstring> pl_categories;

	if (m_playlistEntries)
	{
		// for fast search categories
		std::set<std::wstring> categories;
		for (auto& entry : *m_playlistEntries)
		{
			auto res = m_playlistMap.emplace(entry->get_id(), entry);
			if (!res.second)
			{
				TRACE("Duplicate channel: %s (%d)\n", res.first->second->get_title().c_str(), res.first->second->get_id());
				continue;
			}

			const auto& plEntry = res.first;
			bool found = false;
			if (bRegex)
			{
				try
				{
					found = std::regex_search(plEntry->second->get_title(), re);
				}
				catch (std::regex_error& ex)
				{
					ex;
					found = true;
				}
			}
			else if (!filter.IsEmpty())
			{
				if (bCase)
				{
					found = (plEntry->second->get_title().find(filter.GetString()) != std::wstring::npos);
				}
				else
				{
					found = (StrStrI(plEntry->second->get_title().c_str(), filter.GetString()) != nullptr);
				}
			}

			if (!found)
			{
				m_playlistIds.emplace_back(plEntry->first);
				const auto& category = plEntry->second->get_category();
				if (categories.find(category) == categories.end())
				{
					pl_categories.emplace_back(category);
					categories.emplace(category);
				}
			}
		}
	}

	// fill playlist tree
	if (!m_playlistIds.empty())
	{
		for (auto& category : pl_categories)
		{
			TVINSERTSTRUCTW tvInsert = { nullptr };
			tvInsert.hParent = TVI_ROOT;
			tvInsert.item.pszText = (LPWSTR)category.c_str();
			tvInsert.item.mask = TVIF_TEXT;
			auto item = m_wndPlaylistTree.InsertItem(&tvInsert);
			m_pl_categoriesTreeMap[category] = item;
		}

		int step = 0;
		for (const auto& item : m_playlistIds)
		{
			auto pair = m_playlistMap.find(item);
			ASSERT(pair != m_playlistMap.end());
			if (pair == m_playlistMap.end()) continue;

			const auto& entry = pair->second;
			auto found = m_pl_categoriesTreeMap.find(entry->get_category());
			ASSERT(found != m_pl_categoriesTreeMap.end());
			if (found == m_pl_categoriesTreeMap.end()) continue;

			TVINSERTSTRUCTW tvInsert = { nullptr };
			tvInsert.hParent = found->second;
			tvInsert.item.pszText = (LPWSTR)entry->get_title().c_str();
			tvInsert.item.lParam = (LPARAM)entry.get();
			tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
			m_wndPlaylistTree.InsertItem(&tvInsert);
		}
	}

	UpdatePlaylistCount();
	CheckForExistingChannels();
	CheckForExistingPlaylist();

	m_bInFillTree = false;

	UpdateData(FALSE);
}

void CIPTVChannelEditorDlg::GetStreamInfo(std::vector<uri_stream*>& container, CStatic& staticCtrl)
{
	const auto& access_domain = CIPTVChannelEditorDlg::GetAccessDomain();
	const auto& access_key = CIPTVChannelEditorDlg::GetAccessKey();
	const auto max_threads = 3;

	auto newEnd = std::unique(container.begin(), container.end());
	m_wndProgress.SetRange32(0, std::distance(container.begin(), newEnd));
	for (auto it = container.begin(); it != newEnd;)
	{
		std::array<std::thread, max_threads> workers;
		std::array<std::string, max_threads> audio;
		std::array<std::string, max_threads> video;
		auto pool = it;
		int j = 0;
		while (j < max_threads && pool != container.end())
		{
			const auto& url = GetPlayableURL(*pool, access_domain, access_key);
			workers[j] = std::thread(GetChannelStreamInfo, url, std::ref(audio[j]), std::ref(video[j]));
			j++;
			++pool;
		}

		j = 0;
		for (auto& w : workers)
		{
			if (!w.joinable()) continue;

			w.join();

			auto hash = (*it)->get_hash();
			std::pair<std::string, std::string> info(audio[j], video[j]);
			auto& pair = m_stream_infos.emplace(hash, info);
			if (!pair.second)
			{
				pair.first->second = std::move(info);
			}

			++it;
			j++;

			auto step = std::distance(container.begin(), it);
			CString str;
			str.Format(_T("Get Stream Info: %d from %d"), step, container.size());
			staticCtrl.SetWindowText(str);
			m_wndProgress.SetPos(step);
		}
	}
}

void CIPTVChannelEditorDlg::OnSave()
{
	// Категория должна содержать хотя бы один канал. Иначе плагин падает с ошибкой
	// [plugin] error: invalid plugin TV info: wrong num_channels(0) for group id '' in num_channels_by_group_id.

	// renumber categories id
	int cat_id = 1;
	std::map<int, std::shared_ptr<ChannelCategory>> ren_categories;
	for (auto& pair : m_categoriesMap)
	{
		if (!pair.second->is_empty() || pair.first == ID_ADD_TO_FAVORITE)
		{
			int new_id = pair.first == ID_ADD_TO_FAVORITE ? ID_ADD_TO_FAVORITE : cat_id;
			ren_categories.emplace(new_id, pair.second);
			pair.second->set_key(new_id);
			cat_id++;
		}
	}

	m_categoriesMap = std::move(ren_categories);

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

		if (m_embedded_info)
		{
			auto setup_node = doc.allocate_node(rapidxml::node_element, utils::CHANNELS_SETUP);
			setup_node->append_node(utils::alloc_node(doc, utils::ACCESS_KEY, GetAccessKey().c_str()));
			setup_node->append_node(utils::alloc_node(doc, utils::ACCESS_DOMAIN, GetAccessDomain().c_str()));
			tv_info->append_node(setup_node);
		}

		// create <tv_categories> node
		auto cat_node = doc.allocate_node(rapidxml::node_element, utils::TV_CATEGORIES);

		// append <tv_category> to <tv_categories> node
		for (auto& category : m_categoriesMap)
		{
			if (category.first != ID_ADD_TO_FAVORITE)
			{
				cat_node->append_node(category.second->GetNode(doc));
			}
		}
		// append <tv_categories> to <tv_info> node
		tv_info->append_node(cat_node);

		// create <tv_channels> node
		auto ch_node = doc.allocate_node(rapidxml::node_element, utils::TV_CHANNELS);
		// append <tv_channel> to <v_channels> node
		for (const auto& pair : m_categoriesMap)
		{
			if (pair.first != ID_ADD_TO_FAVORITE)
			{
				for (auto& channel : pair.second->get_channels())
				{
					channel->get_category_ids().clear();
					channel->get_category_ids().emplace(pair.first);
					ch_node->append_node(channel->GetNode(doc));
				}
			}
		}
		// append <tv_channel> to <tv_info> node
		tv_info->append_node(ch_node);

		doc.append_node(tv_info);

		// write document
		auto& playlistPath = fmt::format(theApp.GetAppPath(utils::PLAYLISTS_ROOT).c_str(), GetPluginName().c_str());
		playlistPath += m_chFileName;

		std::ofstream os(playlistPath, std::istream::binary);
		os << doc;

		set_allow_save(FALSE);
		FillTreeChannels();
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
	auto category_id = GetNewCategoryID();
	auto newCategory = std::make_unique<ChannelCategory>(StreamType::enBase, theApp.GetAppPath(utils::PLUGIN_ROOT));
	newCategory->set_key(category_id);
	newCategory->set_title(L"New Category");
	newCategory->set_icon_uri(utils::ICON_TEMPLATE);

	CImage img;
	if (utils::LoadImage(newCategory->get_icon_absolute_path(), img))
	{
		utils::SetImage(img, m_wndIcon);
	}

	TVINSERTSTRUCTW tvInsert = { nullptr };
	tvInsert.hParent = TVI_ROOT;
	tvInsert.item.pszText = (LPWSTR)newCategory->get_title().c_str();
	tvInsert.item.lParam = (LPARAM)newCategory.get();
	tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
	auto hNewItem = m_wndChannelsTree.InsertItem(&tvInsert);

	m_categoriesMap.emplace(category_id, std::move(newCategory));
	m_categoriesTreeMap.emplace(category_id, hNewItem);

	m_wndChannelsTree.SelectItem(hNewItem);
	m_wndChannelsTree.EditLabel(hNewItem);
}

void CIPTVChannelEditorDlg::OnUpdateNewCategory(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CIPTVChannelEditorDlg::OnRemoveCategory()
{
	CString msg(m_wndChannelsTree.GetSelectedCount() > 1 ? _T("Delete categories. Are your sure?") : _T("Delete category. Are your sure?"));
	if (AfxMessageBox(msg, MB_YESNO | MB_ICONWARNING) != IDYES)
		return;

	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		auto category_id = GetCategory(hItem)->get_key();
		if (category_id == ID_ADD_TO_FAVORITE) continue;

		m_categoriesMap.erase(category_id);
		m_categoriesTreeMap.erase(category_id);
		m_wndChannelsTree.DeleteItem(hItem);
	}

	RemoveOrphanChannels();
	CheckForExistingPlaylist();
	set_allow_save();
	UpdateChannelsCount();
}

void CIPTVChannelEditorDlg::OnUpdateRemoveCategory(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsSelectedCategory() && IsSelectedNotFavorite());
}

void CIPTVChannelEditorDlg::OnSortCategory()
{
	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		auto category = GetCategory(hItem);
		category->sort_channels();

		const auto& channels = category->get_channels();
		auto it = channels.begin();
		for (auto hChildItem = m_wndChannelsTree.GetChildItem(hItem); hChildItem != nullptr; hChildItem = m_wndChannelsTree.GetNextSiblingItem(hChildItem))
		{
			m_wndChannelsTree.SetItemText(hChildItem, (*it)->get_title().c_str());
			m_wndChannelsTree.SetItemData(hChildItem, (DWORD_PTR)*it);
			++it;
		}
	}

	CheckForExistingChannels();
	set_allow_save();
}

void CIPTVChannelEditorDlg::OnUpdateSortCategory(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsSelectedCategory() && IsSelectedNotFavorite());
}

void CIPTVChannelEditorDlg::OnStnClickedStaticIcon()
{
	auto hCur = m_wndChannelsTree.GetSelectedItem();
	auto entry = GetBaseInfo(&m_wndChannelsTree, hCur);
	if (!entry)
		return;

	if (IsCategory(hCur) && entry->get_key() == ID_ADD_TO_FAVORITE)
		return;

	CFileDialog dlg(TRUE);
	CString path = theApp.GetAppPath(IsChannel(hCur) ? utils::CHANNELS_LOGO_PATH : utils::CATEGORIES_LOGO_PATH).c_str();
	CString file(path);
	file.Replace('/', '\\');

	CString filter(_T("PNG file(*.png)|*.png|All Files (*.*)|*.*||"));
	filter.Replace('|', '\0');

	OPENFILENAME& oFN = dlg.GetOFN();
	oFN.lpstrFilter = filter.GetString();
	oFN.nMaxFile = MAX_PATH;
	oFN.nFilterIndex = 0;
	oFN.lpstrFile = file.GetBuffer(MAX_PATH);
	oFN.lpstrTitle = _T("Load logotype image");
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
				AfxMessageBox(_T("Non ASCII symbols in the icon name is not allowed!"), MB_ICONERROR | MB_OK);
				return;
			}
		}

		CString newPath = file.Left(file.GetLength() - len);
		if (path.CompareNoCase(newPath) != 0)
		{
			path += oFN.lpstrFileTitle;
			CopyFile(file, path, FALSE);
			CImage img;
			if (utils::LoadImage(path.GetString(), img))
			{
				utils::SetImage(img, m_wndIcon);
			}
		}

		m_iconUrl = uri_base::PLUGIN_SCHEME;
		m_iconUrl += IsChannel(hCur) ? utils::CHANNELS_LOGO_URL : utils::CATEGORIES_LOGO_URL;
		m_iconUrl += oFN.lpstrFileTitle;

		if (m_iconUrl != entry->get_icon_uri().get_uri().c_str())
		{
			entry->set_icon_uri(m_iconUrl.GetString());
			const auto& img = GetIconCache().get_icon(entry->get_title(), entry->get_icon_absolute_path());
			utils::SetImage(img, m_wndIcon);
		}

		set_allow_save();
		UpdateData(FALSE);
	}
}

void CIPTVChannelEditorDlg::OnBnClickedButtonAbout()
{
	CAboutDlg dlg;
	dlg.DoModal();
}

void CIPTVChannelEditorDlg::OnBnClickedButtonPack()
{
	if (is_allow_save() && AfxMessageBox(_T("You have unsaved changes.\nContinue?"), MB_YESNO) != IDYES)
		return;

	const auto& name = GetPluginName();

	const auto& packFolder = fmt::format(theApp.GetAppPath(utils::PACK_PATH).c_str(), name.c_str());

	std::error_code err;
	// remove previous packed folder if exist
	std::filesystem::remove_all(packFolder, err);

	// copy new one
	std::filesystem::copy(theApp.GetAppPath(utils::PLUGIN_ROOT),
						  packFolder,
						  std::filesystem::copy_options::recursive,
						  err);

	// set plugin config
	std::filesystem::rename(packFolder + fmt::format(_T("{:s}_plugin.xml.in"), name),
							packFolder + _T("dune_plugin.xml"),
							err);

	// remove over configs
	for (const auto& dir_entry : std::filesystem::directory_iterator{ packFolder })
	{
		if (dir_entry.path().extension() == _T(".in"))
			std::filesystem::remove(dir_entry, err);
	}

	// copy channel lists
	for (const auto& file : m_all_channels_lists)
	{
		std::filesystem::path src(std::filesystem::absolute(file.second));
		std::filesystem::copy_file(src, packFolder + file.first, std::filesystem::copy_options::overwrite_existing, err);
		ASSERT(!err.value());
	}

	// write setup file
	auto& capsName = utils::utf16_to_utf8(name);
	capsName[0] = toupper(capsName[0]);
	unsigned char smarker[3] = { 0xEF, 0xBB, 0xBF }; // UTF8 BOM
	std::ofstream os(packFolder + _T("plugin_type.php"), std::ios::out | std::ios::binary);
	os.write((const char*)smarker, sizeof(smarker));
	os << fmt::format("<?php\ndefine(\"PLUGIN_TYPE\", '{:s}PluginConfig')\n?>\n", capsName.c_str());
	os.close();

	// pack folder
	SevenZipWrapper archiver(theApp.GetAppPath(utils::PACK_DLL));
	archiver.GetCompressor().SetCompressionFormat(CompressionFormat::Zip);
	bool res = archiver.GetCompressor().AddFiles(packFolder, _T("*.*"), true);
	if (!res)
		return;

	const auto& pluginName = fmt::format(utils::DUNE_PLUGIN_NAME, name.c_str());

	res = archiver.CreateArchive(pluginName);
	if (res)
	{
		AfxMessageBox(_T("Plugin created.\nInstall it to the DUNE mediaplayer"), MB_OK);
	}
	else
	{
		std::filesystem::remove(pluginName, err);
	}

	// remove temporary folder
	std::filesystem::remove_all(packFolder, err);
}

void CIPTVChannelEditorDlg::OnUpdateButtonSearchNext(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_search.IsEmpty());
}

void CIPTVChannelEditorDlg::OnBnClickedButtonSearchNext()
{
	if (m_search.IsEmpty())
		return;

	SearchParams params;
	if (m_search.GetLength() > 1 && m_search.GetAt(0) == '\\')
	{
		params.id = _tstoi(m_search.Mid(1).GetString());
		if (m_channelsMap.find(params.id) == m_channelsMap.end())
			return;
	}
	else
	{
		params.searchString = m_search;
	}

	SelectTreeItem(m_wndChannelsTree, params);
}

void CIPTVChannelEditorDlg::OnUpdateButtonPlSearchNext(CCmdUI* pCmdUI)
{
	UpdateData(TRUE);

	pCmdUI->Enable(!m_plSearch.IsEmpty());
}

void CIPTVChannelEditorDlg::OnBnClickedButtonPlSearchNext()
{
	if (m_plSearch.IsEmpty())
		return;

	SearchParams params;
	if (m_plSearch.GetLength() > 1 && m_plSearch.GetAt(0) == '\\')
	{
		params.id = _tstoi(m_plSearch.Mid(1).GetString());
		if (m_playlistMap.find(params.id) == m_playlistMap.end())
			return;
	}
	else
	{
		params.searchString = m_plSearch;
	}

	SelectTreeItem(m_wndPlaylistTree, params);
}

bool CIPTVChannelEditorDlg::IsChannel(HTREEITEM hItem) const
{
	auto info = GetBaseInfo(&m_wndChannelsTree, hItem);
	return (info && info->is_type(InfoType::enChannel));
}

bool CIPTVChannelEditorDlg::IsCategory(HTREEITEM hItem) const
{
	auto info = GetBaseInfo(&m_wndChannelsTree, hItem);
	return (info && info->is_type(InfoType::enCategory));
}

bool CIPTVChannelEditorDlg::IsPlaylistEntry(HTREEITEM hItem) const
{
	auto info = GetBaseInfo(&m_wndPlaylistTree, hItem);
	return (info && info->is_type(InfoType::enPlEntry));
}

bool CIPTVChannelEditorDlg::IsPlaylistCategory(HTREEITEM hItem) const
{
	return (hItem != nullptr && m_wndPlaylistTree.GetParentItem(hItem) == nullptr);
}

void CIPTVChannelEditorDlg::OnAddUpdateChannel()
{
	CWaitCursor cur;

	bool needCheckExisting = false;
	for (const auto& hSelectedItem : m_wndPlaylistTree.GetSelectedItems())
	{
		needCheckExisting |= AddChannel(hSelectedItem);
	}

	OnSyncTreeItem();

	if (needCheckExisting)
	{
		CheckForExistingChannels();
		CheckForExistingPlaylist();
	}

	UpdateChannelsCount();
	set_allow_save();
}

void CIPTVChannelEditorDlg::OnUpdateAddUpdateChannel(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsSelectedChannelsOrEntries());
}

void CIPTVChannelEditorDlg::OnBnClickedButtonSettings()
{
	CSettingsDlg dlg;
	dlg.m_player = m_player;
	dlg.m_probe = m_probe;
	dlg.m_bAutoSync = m_bAutoSync;

	if (dlg.DoModal() == IDOK)
	{
		m_player = dlg.m_player;
		m_probe = dlg.m_probe;
		m_bAutoSync = dlg.m_bAutoSync;

		theApp.WriteProfileString(REG_SETTINGS, REG_PLAYER, m_player);
		theApp.WriteProfileString(REG_SETTINGS, REG_FFPROBE, m_probe);
		theApp.WriteProfileInt(REG_SETTINGS, REG_AUTOSYNC, m_bAutoSync);
	}
}

void CIPTVChannelEditorDlg::OnBnClickedButtonAccessInfo()
{
	const auto& regPath = GetPluginRegPath();

	CAccessDlg dlg;
	dlg.m_bEmbedded = m_embedded_info;
	dlg.m_accessKey = GetAccessKey().c_str();
	dlg.m_domain = GetAccessDomain().c_str();
	dlg.m_streamType = m_pluginType;
	dlg.m_url = theApp.GetProfileString(regPath.c_str(), REG_ACCESS_URL);

	if (dlg.DoModal() == IDOK)
	{
		m_embedded_info = dlg.m_bEmbedded;
		theApp.WriteProfileString(regPath.c_str(), REG_ACCESS_URL, dlg.m_url);

		SetAccessKey(dlg.m_accessKey);
		SetDomain(dlg.m_domain);

		if (m_embedded_info)
		{
			set_allow_save();
		}
		else
		{
			theApp.WriteProfileString(regPath.c_str(), REG_ACCESS_KEY, dlg.m_accessKey);
			theApp.WriteProfileString(regPath.c_str(), REG_DOMAIN, dlg.m_domain);
		}
	}
}

void CIPTVChannelEditorDlg::OnUpdateIcon()
{
	auto channel = GetBaseInfo(&m_wndPlaylistTree, m_wndChannelsTree.GetSelectedItem());
	auto entry = GetBaseInfo(&m_wndPlaylistTree, m_wndPlaylistTree.GetSelectedItem());

	if (entry && channel && channel->get_icon_uri() != entry->get_icon_uri())
	{
		channel->set_icon_uri(entry->get_icon_uri());
		const auto& img = GetIconCache().get_icon(channel->get_title(), channel->get_icon_absolute_path());
		utils::SetImage(img, m_wndIcon);

		OnSyncTreeItem();
		set_allow_save();
	}
}

void CIPTVChannelEditorDlg::OnUpdateUpdateIcon(CCmdUI* pCmdUI)
{
	BOOL enable = FALSE;
	if (m_wndPlaylistTree.GetSelectedCount() == 1 && !m_bInFillTree)
	{
		auto channel = GetBaseInfo(&m_wndChannelsTree, m_wndChannelsTree.GetSelectedItem());
		auto entry = GetBaseInfo(&m_wndPlaylistTree, m_wndPlaylistTree.GetSelectedItem());
		if (channel && entry)
		{
			enable = channel->get_id() == entry->get_id() && channel->get_icon_uri() != entry->get_icon_uri();
		}
	}

	pCmdUI->Enable(enable);
}

void CIPTVChannelEditorDlg::OnBnClickedButtonCacheIcon()
{
	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		auto channel = GetChannel(hItem);
		if (!channel) continue;

		auto fname = channel->get_icon_uri().get_path();
		auto pos = fname.rfind('/');
		if (pos == std::string::npos) continue;

		fname = fname.substr(pos + 1);
		std::string path = utils::CHANNELS_LOGO_URL;
		path += fname;

		uri_base icon_uri;
		icon_uri.set_uri(utils::ICON_TEMPLATE);
		icon_uri.set_path(path);

		std::vector<BYTE> image;
		if (!utils::DownloadFile(channel->get_icon_uri().get_uri(), image)) continue;

		channel->set_icon_uri(icon_uri.get_uri());

		const auto& fullPath = icon_uri.get_filesystem_path(theApp.GetAppPath(utils::PLUGIN_ROOT));
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
	auto channel = GetChannel(m_wndChannelsTree.GetSelectedItem());
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
		auto entry = GetBaseInfo(&m_wndPlaylistTree, m_wndPlaylistTree.GetSelectedItem());
		auto channel = GetBaseInfo(&m_wndChannelsTree, m_wndChannelsTree.GetSelectedItem());
		if (channel && entry)
		{
			enable = channel->get_icon_uri() != entry->get_icon_uri();
		}
	}

	LoadPlayListInfo(pNMTreeView->itemNew.hItem);

	m_wndUpdateIcon.EnableWindow(enable);
}

void CIPTVChannelEditorDlg::OnBnClickedButtonAddNewChannelsList()
{
	CFileDialog dlg(FALSE);

	const auto& pluginName = GetPluginName();
	const auto& name = fmt::format(_T("{:s}_channel_list.xml"), pluginName.c_str());

	auto& newList = fmt::format(theApp.GetAppPath(utils::PLAYLISTS_ROOT).c_str(), pluginName.c_str());
	std::filesystem::create_directory(newList);

	newList += name;

	CString filter(_T("Channels xml(*.xml)|*.xml||"));
	filter.Replace('|', '\0');

	CString buffer(newList.c_str());
	OPENFILENAME& oFN = dlg.GetOFN();
	oFN.lpstrFilter = filter.GetString();
	oFN.nMaxFile = MAX_PATH;
	oFN.nFilterIndex = 0;
	oFN.lpstrTitle = _T("Add new channels list");
	oFN.Flags |= OFN_EXPLORER | OFN_ENABLESIZING | OFN_LONGNAMES | OFN_OVERWRITEPROMPT | OFN_NONETWORKBUTTON;
	oFN.lpstrFile = buffer.GetBuffer(MAX_PATH);

	dlg.ApplyOFNToShellDialog();

	INT_PTR nResult = dlg.DoModal();
	if (nResult == IDOK)
	{
		size_t len = _tcslen(oFN.lpstrFileTitle);
		for (size_t i = 0; i < len; i++)
		{
			if (oFN.lpstrFileTitle[i] > 127)
			{
				AfxMessageBox(_T("Non ASCII symbols in the channel list name is not allowed!"), MB_ICONERROR | MB_OK);
				return;
			}
		}

		CFile cFile;
		CFileException ex;
		if (cFile.Open(dlg.GetPathName(), CFile::modeWrite | CFile::modeCreate | CFile::typeBinary | CFile::shareDenyRead, &ex))
		{
			cFile.Close();
			auto found = std::find_if(m_all_channels_lists.begin(), m_all_channels_lists.end(), [&dlg](const auto& item)
									  {
										  return dlg.GetFileName().CompareNoCase(item.first.c_str()) == 0;
									  });

			if (found == m_all_channels_lists.end())
			{
				m_channelsMap.clear();
				m_categoriesMap.clear();
				const auto& pair = m_all_channels_lists.emplace_back(dlg.GetFileName(), dlg.GetPathName());
				m_wndChannels.EnableWindow(TRUE);
				int idx = m_wndChannels.AddString(pair.first.c_str());
				m_wndChannels.SetItemData(idx, (DWORD_PTR)pair.second.c_str());
				m_wndChannels.SetCurSel(idx);
				set_allow_save();
			}
			else
			{
				m_wndChannels.SetCurSel(m_wndChannels.FindString(-1, dlg.GetFileName()));
			}
		}
	}
}

void CIPTVChannelEditorDlg::OnGetStreamInfo()
{
	if (!m_lastTree)
		return;

	m_wndProgress.ShowWindow(SW_SHOW);
	CWaitCursor cur;

	std::vector<uri_stream*> streams;
	for (const auto& hItem : m_lastTree->GetSelectedItems())
	{
		auto channel = GetBaseInfo(m_lastTree, hItem);
		if (channel)
		{
			streams.emplace_back(channel->get_stream_uri());
		}
	}

	if (m_lastTree == &m_wndChannelsTree)
	{
		GetStreamInfo(streams, m_wndChInfo);
		LoadChannelInfo(m_lastTree->GetFirstSelectedItem());
	}
	else if (m_lastTree == &m_wndPlaylistTree)
	{
		GetStreamInfo(streams, m_wndPlInfo);
		LoadPlayListInfo(m_lastTree->GetFirstSelectedItem());
	}

	UpdateChannelsCount();
	UpdatePlaylistCount();
	m_wndProgress.ShowWindow(SW_HIDE);

	UpdateData(FALSE);
}

void CIPTVChannelEditorDlg::OnUpdateGetStreamInfo(CCmdUI* pCmdUI)
{
	BOOL enable = !m_probe.IsEmpty();
	if (m_lastTree)
	{
		enable &= (GetBaseInfo(m_lastTree, m_lastTree->GetFirstSelectedItem()) != nullptr);
	}
	else
	{
		enable = FALSE;
	}

	enable = enable && IsSelectedTheSameType() && !m_loading;

	pCmdUI->Enable(enable);
}

void CIPTVChannelEditorDlg::OnGetStreamInfoAll()
{
	if (!m_lastTree)
		return;

	CWaitCursor cur;
	m_wndProgress.ShowWindow(SW_SHOW);

	std::vector<uri_stream*> container;
	if (m_lastTree == &m_wndChannelsTree)
	{
		for (const auto& item : GetItemCategory(m_lastTree->GetSelectedItem())->get_channels())
		{
			auto info = dynamic_cast<BaseInfo*>(item);
			if (info)
				container.emplace_back(info->get_stream_uri());
		}
		GetStreamInfo(container, m_wndChInfo);
		LoadChannelInfo(m_lastTree->GetSelectedItem());
	}
	else if (m_lastTree == &m_wndPlaylistTree)
	{

		auto curEntry = GetPlaylistEntry(m_lastTree->GetSelectedItem());
		for (const auto& pair : m_playlistMap)
		{
			if (curEntry == nullptr || curEntry->get_category() == pair.second->get_category())
			{
				// add all
				auto info = dynamic_cast<BaseInfo*>(pair.second.get());
				if (info)
					container.emplace_back(info->get_stream_uri());
			}
		}

		GetStreamInfo(container, m_wndPlInfo);
		LoadPlayListInfo(m_lastTree->GetSelectedItem());
	}

	UpdateChannelsCount();
	UpdatePlaylistCount();
	m_wndProgress.ShowWindow(SW_HIDE);
}

void CIPTVChannelEditorDlg::OnUpdateGetStreamInfoAll(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_lastTree != nullptr && !m_probe.IsEmpty() && !m_loading);
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
	BOOL enable = (m_wndChannelsTree.GetSelectedCount() == 1 && IsChannel(m_wndChannelsTree.GetSelectedItem()))
		|| (m_wndPlaylistTree.GetSelectedCount() == 1 && IsPlaylistEntry(m_wndPlaylistTree.GetSelectedItem()));

	pCmdUI->Enable(!m_probe.IsEmpty() && enable);
}

void CIPTVChannelEditorDlg::OnPlayChannelStreamArchive()
{
	UpdateData(TRUE);

	if (m_lastTree)
	{
		PlayItem(m_lastTree->GetSelectedItem(), m_archiveCheck);
	}
}

void CIPTVChannelEditorDlg::OnSyncTreeItem()
{
	if (m_loading || !m_lastTree)
		return;

	CWaitCursor cur;
	SearchParams params;

	auto info = GetBaseInfo(m_lastTree, m_lastTree->GetSelectedItem());
	if (!info)
		return;

	params.id = info->get_id();
	if (m_lastTree == &m_wndPlaylistTree && m_channelsMap.find(params.id) != m_channelsMap.end() ||
		m_lastTree == &m_wndChannelsTree && m_playlistMap.find(params.id) != m_playlistMap.end())
	{
		SelectTreeItem(*m_lastTree, params);
	}
}

void CIPTVChannelEditorDlg::OnUpdateSyncTreeItem(CCmdUI* pCmdUI)
{
	BOOL enable = (!m_loading
				   && m_lastTree
				   && m_lastTree->GetSelectedCount() == 1
				   && GetBaseInfo(m_lastTree, m_lastTree->GetSelectedItem()) != nullptr
				   && !m_bAutoSync);

	pCmdUI->Enable(enable);
}

void CIPTVChannelEditorDlg::OnToggleChannel()
{
	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		if (auto channel = GetChannel(hItem); channel != nullptr)
		{
			channel->set_disabled(!m_menu_enable_channel);
			m_wndChannelsTree.SetItemColor(hItem, ::GetSysColor(COLOR_GRAYTEXT));
			set_allow_save();
		}
	}
}

void CIPTVChannelEditorDlg::OnUpdateToggleChannel(CCmdUI* pCmdUI)
{
	BOOL enable = FALSE;
	if (IsSelectedChannelsOrEntries() && IsSelectedNotFavorite())
	{
		for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
		{
			if (auto channel = GetChannel(hItem); channel != nullptr)
			{
				m_menu_enable_channel = channel->is_disabled();
				if (m_menu_enable_channel)
				{
					pCmdUI->SetText(_T("Enable Channel"));
				}
				enable = TRUE;
			}
		}
	}

	pCmdUI->Enable(enable);
}

void CIPTVChannelEditorDlg::OnBnClickedButtonDownloadPlaylist()
{
	LoadPlaylist(true);
}

void CIPTVChannelEditorDlg::OnCbnSelchangeComboPluginType()
{
	SwitchPlugin();
}

void CIPTVChannelEditorDlg::OnCbnSelchangeComboPlaylist()
{
	BOOL enableDownload = TRUE;
	BOOL enableCustom = FALSE;
	int pl_idx = m_wndPlaylist.GetCurSel();
	switch (m_wndPluginType.GetCurSel())
	{
		case 0: // Edem
		{
			switch (pl_idx)
			{
				case 0:
				case 1:
					break;
				case 2:
					enableCustom = TRUE;
					break;
				case 3:
					enableDownload = FALSE;
					enableCustom = TRUE;
					break;
				default:
					break;
			}
			break;
		}
		case 1: // Sharavoz
		{
			switch (pl_idx)
			{
				case 0:
					enableCustom = TRUE;
					break;
				case 1:
					enableDownload = FALSE;
					enableCustom = TRUE;
					break;
				default:
					break;
			}
			break;
		}
	}

	m_wndDownloadUrl.EnableWindow(enableDownload);
	m_wndChooseUrl.EnableWindow(enableCustom);
	theApp.WriteProfileInt(GetPluginRegPath().c_str(), REG_PLAYLIST_TYPE, pl_idx);
	LoadPlaylist();
}

BaseInfo* CIPTVChannelEditorDlg::GetBaseInfo(const CTreeCtrl* pTreeCtrl, HTREEITEM hItem)
{
	if (pTreeCtrl == nullptr || hItem == nullptr)
		return nullptr;

	return (BaseInfo*)(pTreeCtrl->GetItemData(hItem));
}

HTREEITEM CIPTVChannelEditorDlg::FindTreeItem(CTreeCtrl& ctl, DWORD_PTR entry)
{
	HTREEITEM hSub = nullptr;
	HTREEITEM root = ctl.GetRootItem();
	while (root != nullptr)
	{
		// iterate subitems
		hSub = FindTreeSubItem(ctl, ctl.GetChildItem(root), entry);
		if (hSub) break;

		root = ctl.GetNextSiblingItem(root);
	}

	return hSub;
}

void CIPTVChannelEditorDlg::SelectTreeItem(CTreeCtrl& ctl, const SearchParams& searchParams)
{
	HTREEITEM hItem = ctl.GetSelectedItem();
	HTREEITEM root = ctl.GetParentItem(hItem);
	HTREEITEM hStart = hItem;
	if (root)
	{
		// shift to next item
		hItem = ctl.GetNextSiblingItem(hItem);
	}
	else
	{
		// Category selected or none of item selected. Select first child item
		root = hItem ? hItem : ctl.GetRootItem();
		hStart = hItem = ctl.GetChildItem(root);
	}

	// cyclic search thru channel list
	HTREEITEM hFound = nullptr;
	while (root != nullptr)
	{
		// iterate subitems
		while (hItem)
		{
			BaseInfo* entry = nullptr;
			if (ctl.GetParentItem(hItem) != nullptr)
				entry = (BaseInfo*)ctl.GetItemData(hItem);

			if (entry)
			{
				if (!searchParams.id.empty())
				{
					if (entry->get_id() == searchParams.id)
					{
						hFound = hItem;
						break;
					}
				}
				else if (searchParams.hash)
				{
					if (entry->get_stream_uri()->get_hash() == searchParams.hash)
					{
						hFound = hItem;
						break;
					}
				}
				else if (!searchParams.searchString.IsEmpty())
				{
					if (StrStrI(entry->get_title().c_str(), searchParams.searchString.GetString()) != nullptr)
					{
						hFound = hItem;
						break;
					}
				}
			}

			// get the next sibling item
			hItem = ctl.GetNextSiblingItem(hItem);

			if (hItem == hStart)
			{
				// We make full circle. Exit from search loop
				hFound = hItem;
				break;
			}
		}

		if (hFound) break;

		root = ctl.GetNextSiblingItem(root);
		if (!root)
			root = ctl.GetRootItem();

		hItem = ctl.GetChildItem(root);
	}

	if (hFound)
	{
		ctl.SelectItem(hFound);
		ctl.Expand(ctl.GetParentItem(hFound), TVE_EXPAND);
		ctl.EnsureVisible(hFound);
	}
}

HTREEITEM CIPTVChannelEditorDlg::FindTreeNextItem(CTreeCtrl& ctl, HTREEITEM hItem, DWORD_PTR entry)
{
	HTREEITEM root = ctl.GetParentItem(hItem);
	hItem = ctl.GetNextSiblingItem(hItem);
	while (root != nullptr)
	{
		// iterate subitems
		hItem = FindTreeSubItem(ctl, hItem, entry);
		if (hItem) break;

		root = ctl.GetNextSiblingItem(root);
	}

	return hItem;
}

HTREEITEM CIPTVChannelEditorDlg::FindTreeSubItem(CTreeCtrl& ctl, HTREEITEM hItem, DWORD_PTR entry)
{
	while (hItem)
	{
		if (entry == ctl.GetItemData(hItem)) break;

		// get the next sibling item
		hItem = ctl.GetNextSiblingItem(hItem);
	}

	return hItem;
}

bool CIPTVChannelEditorDlg::AddChannel(HTREEITEM hSelectedItem, int categoryId /*= -1*/)
{
	bool needCheckExisting = false;

	auto entry = GetPlaylistEntry(hSelectedItem);
	if (!entry)
		return needCheckExisting;

	// is add to category?
	if (categoryId == -1)
		categoryId = GetCategoryIdByName(entry->get_category());

	const auto& root_path = theApp.GetAppPath(utils::PLUGIN_ROOT);
	std::shared_ptr<ChannelCategory> category;
	if (categoryId != -1)
	{
		category = m_categoriesMap[categoryId];
	}
	else
	{
		// Category not exist, create new
		category = std::make_shared<ChannelCategory>(StreamType::enBase, root_path);
		categoryId = GetNewCategoryID();
		category->set_key(categoryId);
		category->set_title(entry->get_category());

		TVINSERTSTRUCTW tvCategory = { nullptr };
		tvCategory.hParent = TVI_ROOT;
		tvCategory.item.pszText = (LPWSTR)category->get_title().c_str();
		tvCategory.item.lParam = (LPARAM)category.get();
		tvCategory.item.mask = TVIF_TEXT | TVIF_PARAM;
		auto hParent = m_wndChannelsTree.InsertItem(&tvCategory);

		m_categoriesTreeMap.emplace(categoryId, hParent);
		m_categoriesMap.emplace(categoryId, category);
	}

	HTREEITEM hFoundItem = nullptr;
	auto pair = m_channelsMap.find(entry->get_id());
	if (pair != m_channelsMap.end())
	{
		// Channel already exist
		auto entry = (DWORD_PTR)pair->second.get();
		HTREEITEM root = m_wndChannelsTree.GetRootItem();
		while (root != nullptr)
		{
			// iterate subitems
			hFoundItem = FindTreeSubItem(m_wndChannelsTree, m_wndChannelsTree.GetChildItem(root), entry);
			if (hFoundItem) break;

			root = m_wndChannelsTree.GetNextSiblingItem(root);
		}
	}
	else
	{
		// Create new channel
		auto newChannel = std::make_unique<ChannelInfo>(StreamType::enChannels, root_path);
		newChannel->set_id(entry->get_id());
		newChannel->set_title(entry->get_title());
		// Add to channel array
		pair = m_channelsMap.emplace(entry->get_id(), std::move(newChannel)).first;
	}

	auto& channel = pair->second;
	// is channel in this category?
	if (category->find_channel(channel->get_id()) == nullptr)
	{
		// add channel to category tree leaf
		TVINSERTSTRUCTW tvChannel = { nullptr };
		tvChannel.hParent = m_categoriesTreeMap[categoryId];
		tvChannel.item.pszText = (LPWSTR)channel->get_title().c_str();
		tvChannel.item.lParam = (LPARAM)channel.get();
		tvChannel.item.mask = TVIF_TEXT | TVIF_PARAM;
		hFoundItem = m_wndChannelsTree.InsertItem(&tvChannel);

		category->add_channel(channel);
		needCheckExisting = true;
	}

	// Is title changed?
	if (channel->get_title() != entry->get_title())
	{
		channel->set_title(entry->get_title());
		// Search and update tree items present in other leafs
		while (hFoundItem != nullptr)
		{
			m_wndChannelsTree.SetItemText(hFoundItem, channel->get_title().c_str());
			hFoundItem = FindTreeNextItem(m_wndChannelsTree, hFoundItem, (DWORD_PTR)channel.get());
		}
	}

	// is tvg_id changed?
	if (entry->get_epg1_id() > 0)
	{
		channel->set_epg1_id(entry->get_epg2_id());
	}

	if (entry->get_epg2_id() > 0)
	{
		channel->set_epg2_id(entry->get_epg2_id());
	}

	channel->set_archive(entry->get_archive());
	channel->set_stream_uri(entry->get_stream_uri());
	if (entry->get_icon_uri() != channel->get_icon_uri() && !channel->get_icon_uri().get_uri().empty())
	{
		channel->set_icon_uri(entry->get_icon_uri());
	}

	// Channel for adult
	channel->set_adult(entry->get_adult());

	return needCheckExisting;
}

void CIPTVChannelEditorDlg::GetChannelStreamInfo(const std::string& url, std::string& audio, std::string& video)
{
	if (url.empty())
		return;

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = nullptr;

	// Create a pipe for the child process's STDOUT
	HANDLE hStdoutRd;
	HANDLE hChildStdoutWr = nullptr;
	HANDLE hChildStdoutRd = nullptr;
	CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &sa, 0);

	// Duplicate the read handle to the pipe so it is not inherited and close src handle
	HANDLE hSelf = GetCurrentProcess();
	if (!DuplicateHandle(hSelf, hChildStdoutRd, hSelf, &hStdoutRd, 0, FALSE, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE))
	{
		TRACE(_T("Failed! Can't create stdout pipe to child process. Code: %u\n"), GetLastError());
		return;
	}

	std::string result;
	BOOL bResult = FALSE;

	STARTUPINFO si = { 0 };
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow = SW_HIDE;
	si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	si.hStdInput = nullptr;
	si.hStdOutput = hChildStdoutWr;

	PROCESS_INFORMATION pi = { nullptr };

	// argv[0] имя исполняемого файла
	CString csCommand;
	csCommand.Format(_T("\"%s\" -hide_banner -show_streams \"%hs\""), CIPTVChannelEditorDlg::m_probe.GetString(), url.c_str());

	BOOL bRunProcess = CreateProcess(CIPTVChannelEditorDlg::m_probe.GetString(),	// 	__in_opt     LPCTSTR lpApplicationName
									 csCommand.GetBuffer(0),	// 	__inout_opt  LPTSTR lpCommandLine
									 nullptr,					// 	__in_opt     LPSECURITY_ATTRIBUTES lpProcessAttributes
									 nullptr,					// 	__in_opt     LPSECURITY_ATTRIBUTES lpThreadAttributes
									 TRUE,						// 	__in         BOOL bInheritHandles
									 CREATE_SUSPENDED,			// 	__in         DWORD dwCreationFlags
									 nullptr,					// 	__in_opt     LPVOID lpEnvironment
									 nullptr,					// 	__in_opt     LPCTSTR lpCurrentDirectory
									 &si,						// 	__in         LPSTARTUPINFO lpStartupInfo
									 &pi);						// 	__out        LPPROCESS_INFORMATION lpProcessInformation

	if (!bRunProcess)
	{
		TRACE(_T("Failed! Can't execute command: %s\nCode: %u\n"), csCommand.GetString(), GetLastError());
	}
	else
	{
		ResumeThread(pi.hThread);

		long nTimeout = 30;

		int nErrorCount = 0;
		DWORD dwExitCode = STILL_ACTIVE;
		DWORD dwStart = GetTickCount();
		BOOL bTimeout = FALSE;
		for (;;)
		{
			if (dwExitCode != STILL_ACTIVE)
			{
				TRACE("Success! Exit code: %u for %s\n", dwExitCode, url.c_str());
				break;
			}

			if (CheckForTimeOut(dwStart, nTimeout * 1000))
			{
				bTimeout = TRUE;
				::TerminateProcess(pi.hProcess, 0);
				TRACE("Failed! Execution Timeout. %s\n", url.c_str());
				break;
			}

			if (::GetExitCodeProcess(pi.hProcess, &dwExitCode))
			{
				for (;;)
				{
					if (CheckForTimeOut(dwStart, nTimeout * 1000)) break;

					// Peek data from stdout pipe
					DWORD dwAvail = 0;
					if (!::PeekNamedPipe(hStdoutRd, nullptr, 0, nullptr, &dwAvail, nullptr) || !dwAvail) break;

					char szBuf[4096] = { 0 };
					DWORD dwReaded = dwAvail;
					if (!ReadFile(hStdoutRd, szBuf, min(4095, dwAvail), &dwReaded, nullptr)) break;

					result.append(szBuf, dwReaded);
				}
			}
			else
			{
				// По каким то причинам обломался
				TRACE("GetExitCodeProcess failed. ErrorCode: %0u, try count: %0d, source %hs\n", ::GetLastError(), nErrorCount, url.c_str());
				nErrorCount++;
				if (nErrorCount > 10) break;
				continue;
			}
		}

		bResult = TRUE;
	}

	::CloseHandle(hStdoutRd);
	::CloseHandle(hChildStdoutWr);
	::CloseHandle(pi.hProcess);
	::CloseHandle(pi.hThread);

	// Parse stdout
	std::stringstream ss(result);
	std::string item;
	std::vector<std::map<std::string, std::string>> streams;
	bool skip = false;
	size_t idx = 0;
	while (std::getline(ss, item))
	{
		utils::string_rtrim(item, "\r\n");
		if (item == "[STREAM]")
		{
			skip = false;
			streams.emplace_back(std::map<std::string, std::string>());
			idx = streams.size() - 1;
			continue;
		}

		if (item == "[/STREAM]")
		{
			skip = true;
			continue;
		}

		if (!skip)
		{
			auto pair = utils::string_split(item, '=');
			if (pair.size() > 1)
				streams[idx].emplace(pair[0], pair[1]);
		}
	}

	audio = "Not available";
	video = "Not available";
	for (auto& stream : streams)
	{
		if (stream["codec_type"] == "audio")
		{
			audio = stream["codec_long_name"] + " ";
			audio += stream["sample_rate"] + " ";
			audio += stream["channel_layout"];
		}
		else if (stream["codec_type"] == "video")
		{
			video = stream["width"] + "x";
			video += stream["height"] + " ";
			video += stream["codec_long_name"];
		}
	}
}

void CIPTVChannelEditorDlg::OnCbnSelchangeComboChannels()
{
	if (is_allow_save() && AfxMessageBox(_T("Changes not saved. Are you sure?"), MB_YESNO | MB_ICONWARNING) != IDYES)
	{
		m_wndChannels.SetCurSel(theApp.GetProfileInt(REG_SETTINGS, REG_CHANNELS_TYPE, 0));
		return;
	}

	int idx = m_wndChannels.GetCurSel();
	if (idx == -1)
		return;

	bool changed = false;
	if (LoadChannels((LPCTSTR)m_wndChannels.GetItemData(idx), changed))
	{
		FillTreeChannels();
		set_allow_save(changed != false);
	}

	GetDlgItem(IDC_BUTTON_ADD_NEW_CHANNELS_LIST)->EnableWindow(idx > 0);
	theApp.WriteProfileInt(REG_SETTINGS, REG_CHANNELS_TYPE, idx);
}

void CIPTVChannelEditorDlg::OnBnClickedButtonPlFilter()
{
	CFilterDialog dlg;

	const auto& regPath = GetPluginRegPath();
	dlg.m_filterString = theApp.GetProfileString(regPath.c_str(), REG_FILTER_STRING);
	dlg.m_filterRegex = theApp.GetProfileInt(regPath.c_str(), REG_FILTER_REGEX, FALSE);
	dlg.m_filterCase = theApp.GetProfileInt(regPath.c_str(), REG_FILTER_CASE, FALSE);

	if (dlg.DoModal() == IDOK)
	{
		theApp.WriteProfileString(regPath.c_str(), REG_FILTER_STRING, dlg.m_filterString);
		theApp.WriteProfileInt(regPath.c_str(), REG_FILTER_REGEX, dlg.m_filterRegex);
		theApp.WriteProfileInt(regPath.c_str(), REG_FILTER_CASE, dlg.m_filterCase);

		FillTreePlaylist();
	}
}

bool CIPTVChannelEditorDlg::IsSelectedTheSameType() const
{
	if (!m_lastTree)
		return false;

	auto selected = m_lastTree->GetSelectedItems();
	if (selected.empty())
		return false;

	bool isEntry = (GetBaseInfo(m_lastTree, selected[0]) != nullptr);
	for (const auto& hItem : selected)
	{
		if (isEntry != (GetBaseInfo(m_lastTree, hItem) != nullptr))
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
	return (m_lastTree == &m_wndChannelsTree && GetCategory(m_wndChannelsTree.GetFirstSelectedItem()) != nullptr && IsSelectedTheSameType());
}

bool CIPTVChannelEditorDlg::IsSelectedNotFavorite() const
{
	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		auto category = GetItemCategory(hItem);
		if (!category || category->get_key() != ID_ADD_TO_FAVORITE) continue;
		return false;
	}

	return true;
}

bool CIPTVChannelEditorDlg::IsSelectedTheSameCategory() const
{
	if (m_lastTree != &m_wndChannelsTree || !m_wndChannelsTree.GetSelectedCount() || !IsSelectedTheSameType())
		return false;

	auto category = GetCategory(m_wndChannelsTree.GetFirstSelectedItem());
	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		if (GetCategory(hItem) != category)
			return false;
	}

	return true;
}

bool CIPTVChannelEditorDlg::IsChannelSelectionConsistent() const
{
	bool continues = false;
	auto hItem = m_wndChannelsTree.GetFirstSelectedItem();
	auto hNext = hItem;
	for (;hItem != nullptr; hItem = m_wndChannelsTree.GetNextSelectedItem(hItem))
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
		auto ch_id = entry->get_id();
		CString categories;
		for (const auto& pair : m_categoriesMap)
		{
			if (pair.second->find_channel(ch_id))
			{
				if (!categories.IsEmpty())
					categories += _T(", ");
				categories.Append(pair.second->get_title().c_str());
			}
		}

		m_toolTipText.Format(_T("Name: %s\nID: %hs\nArchive: %s\nAdult: %s\nIn categories: %s"),
							 entry->get_title().c_str(),
							 entry->get_stream_uri()->is_template() ? ch_id.c_str() : "Custom",
							 entry->get_archive() ? _T("Yes") : _T("No"),
							 entry->get_adult() ? _T("Yes") : _T("No"),
							 categories.GetString());


		pGetInfoTip->pszText = m_toolTipText.GetBuffer();
	}

	*pResult = 0;
}

void CIPTVChannelEditorDlg::OnTvnPlaylistGetInfoTip(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMTVGETINFOTIP>(pNMHDR);

	auto entry = GetBaseInfo(&m_wndPlaylistTree, pGetInfoTip->hItem);
	if (entry)
	{
		m_toolTipText.Format(_T("Name: %s\nID: %hs\nEPG: %d\nArchive: %s\nAdult: %s"),
							 entry->get_title().c_str(),
							 entry->get_stream_uri()->is_template() ? entry->get_id().c_str() : "Custom",
							 entry->get_epg2_id(),
							 entry->get_archive() ? _T("Yes") : _T("No"),
							 entry->get_adult() ? _T("Yes") : _T("No"));

		pGetInfoTip->pszText = m_toolTipText.GetBuffer();
	}

	*pResult = 0;
}

void CIPTVChannelEditorDlg::RestoreWindowPos()
{
	WINDOWPLACEMENT wp = { 0 };
	UINT nSize = 0;
	WINDOWPLACEMENT* pwp = nullptr;
	if (!AfxGetApp()->GetProfileBinary(REG_SETTINGS, _T("WindowPos"), (LPBYTE*)&pwp, &nSize))
		return;

	// Success
	::memcpy((void*)&wp, pwp, sizeof(wp));
	delete[] pwp; // free the buffer

	// Get a handle to the monitor
	HMONITOR hMonitor = ::MonitorFromPoint(CPoint(wp.rcNormalPosition.left, wp.rcNormalPosition.top), MONITOR_DEFAULTTONEAREST);

	// Get the monitor info
	MONITORINFO monInfo;
	monInfo.cbSize = sizeof(MONITORINFO);
	if (::GetMonitorInfo(hMonitor, &monInfo))
	{
		// Adjust for work area
		CRect rc = wp.rcNormalPosition;
		rc.OffsetRect(monInfo.rcWork.left - monInfo.rcMonitor.left, monInfo.rcWork.top - monInfo.rcMonitor.top);

		// Ensure top left point is on screen
		CRect rc_monitor(monInfo.rcWork);
		if (rc_monitor.PtInRect(rc.TopLeft()) == FALSE)
		{
			rc.OffsetRect(rc_monitor.TopLeft());
		}
		wp.rcNormalPosition = rc;
	}

	SetWindowPlacement(&wp);
}

BOOL CIPTVChannelEditorDlg::DestroyWindow()
{
	// Get the window position
	WINDOWPLACEMENT wp;
	wp.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(&wp);
	// Save the info
	CWinApp* pApp = AfxGetApp();
	pApp->WriteProfileBinary(REG_SETTINGS, _T("WindowPos"), (LPBYTE)&wp, sizeof(wp));

	return __super::DestroyWindow();
}
