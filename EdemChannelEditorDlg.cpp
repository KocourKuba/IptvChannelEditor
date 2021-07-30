
// EdemChannelEditorDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "framework.h"
#include <afxdialogex.h>
#include <array>
#include <thread>

#include "EdemChannelEditor.h"
#include "EdemChannelEditorDlg.h"
#include "AboutDlg.h"
#include "SettingsDlg.h"
#include "AccessDlg.h"
#include "FilterDialog.h"
#include "CustomPlaylistDlg.h"
#include "PlaylistParseThread.h"
#include "utils.h"

#include "rapidxml.hpp"
#include "rapidxml_print.hpp"

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

BOOL CEdemChannelEditorDlg::m_embedded_info = FALSE;
CString CEdemChannelEditorDlg::m_gl_domain;
CString CEdemChannelEditorDlg::m_gl_access_key;
CString CEdemChannelEditorDlg::m_ch_domain;
CString CEdemChannelEditorDlg::m_ch_access_key;
CString CEdemChannelEditorDlg::m_probe;

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

BEGIN_MESSAGE_MAP(CEdemChannelEditorDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()

	ON_BN_CLICKED(IDC_BUTTON_ABOUT, &CEdemChannelEditorDlg::OnBnClickedButtonAbout)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_PLAYLIST, &CEdemChannelEditorDlg::OnBnClickedButtonCustomPlaylist)
	ON_BN_CLICKED(IDC_BUTTON_PL_SEARCH_NEXT, &CEdemChannelEditorDlg::OnBnClickedButtonPlSearchNext)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_PL_SEARCH_NEXT, &CEdemChannelEditorDlg::OnUpdateButtonPlSearchNext)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH_NEXT, &CEdemChannelEditorDlg::OnBnClickedButtonSearchNext)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_SEARCH_NEXT, &CEdemChannelEditorDlg::OnUpdateButtonSearchNext)
	ON_BN_CLICKED(IDC_BUTTON_PL_FILTER, &CEdemChannelEditorDlg::OnBnClickedButtonPlFilter)
	ON_BN_CLICKED(IDC_BUTTON_GET_INFO, &CEdemChannelEditorDlg::OnGetStreamInfo)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_GET_INFO, &CEdemChannelEditorDlg::OnUpdateGetStreamInfo)
	ON_BN_CLICKED(IDC_BUTTON_ADD_NEW_CHANNELS_LIST, &CEdemChannelEditorDlg::OnBnClickedButtonAddNewChannelsList)
	ON_BN_CLICKED(IDC_BUTTON_ACCESS_INFO, &CEdemChannelEditorDlg::OnBnClickedButtonAccessInfo)
	ON_BN_CLICKED(IDC_BUTTON_DOWNLOAD_PLAYLIST, &CEdemChannelEditorDlg::OnBnClickedButtonDownloadPlaylist)
	ON_BN_CLICKED(IDC_BUTTON_TEST_TVG, &CEdemChannelEditorDlg::OnBnClickedButtonTestTvg)
	ON_BN_CLICKED(IDC_BUTTON_TEST_EPG, &CEdemChannelEditorDlg::OnBnClickedButtonTestEpg)
	ON_BN_CLICKED(IDC_CHECK_ADULT, &CEdemChannelEditorDlg::OnBnClickedCheckAdult)
	ON_BN_CLICKED(IDC_CHECK_ARCHIVE, &CEdemChannelEditorDlg::OnBnClickedCheckArchive)
	ON_BN_CLICKED(IDC_CHECK_CUSTOMIZE, &CEdemChannelEditorDlg::OnBnClickedCheckCustomize)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CEdemChannelEditorDlg::OnSave)
	ON_BN_CLICKED(IDC_BUTTON_PACK, &CEdemChannelEditorDlg::OnBnClickedButtonPack)
	ON_BN_CLICKED(IDC_BUTTON_SETTINGS, &CEdemChannelEditorDlg::OnBnClickedButtonSettings)
	ON_BN_CLICKED(IDC_BUTTON_CACHE_ICON, &CEdemChannelEditorDlg::OnBnClickedButtonCacheIcon)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_CACHE_ICON, &CEdemChannelEditorDlg::OnUpdateButtonCacheIcon)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE_ICON, &CEdemChannelEditorDlg::OnUpdateIcon)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_UPDATE_ICON, &CEdemChannelEditorDlg::OnUpdateUpdateIcon)
	ON_BN_CLICKED(IDC_BUTTON_CHECK_ARCHIVE, &CEdemChannelEditorDlg::OnPlayChannelStreamArchive)

	ON_EN_CHANGE(IDC_EDIT_TVG_ID, &CEdemChannelEditorDlg::OnEnChangeEditTvgID)
	ON_EN_CHANGE(IDC_EDIT_EPG_ID, &CEdemChannelEditorDlg::OnEnChangeEditEpgID)
	ON_EN_CHANGE(IDC_EDIT_TIME_SHIFT, &CEdemChannelEditorDlg::OnEnChangeEditTimeShiftHours)
	ON_EN_CHANGE(IDC_EDIT_ARCHIVE_CHECK, &CEdemChannelEditorDlg::OnEnChangeEditArchiveCheck)
	ON_EN_CHANGE(IDC_EDIT_STREAM_URL, &CEdemChannelEditorDlg::OnEnChangeEditStreamUrl)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_TIME_SHIFT, &CEdemChannelEditorDlg::OnDeltaposSpinTimeShiftHours)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_ARCHIVE_CHECK, &CEdemChannelEditorDlg::OnDeltaposSpinArchiveCheck)
	ON_EN_CHANGE(IDC_EDIT_URL_ID, &CEdemChannelEditorDlg::OnEnChangeEditUrlID)
	ON_STN_CLICKED(IDC_STATIC_ICON, &CEdemChannelEditorDlg::OnStnClickedStaticIcon)
	ON_CBN_SELCHANGE(IDC_COMBO_CHANNELS, &CEdemChannelEditorDlg::OnCbnSelchangeComboChannels)
	ON_CBN_SELCHANGE(IDC_COMBO_PLAYLIST, &CEdemChannelEditorDlg::OnCbnSelchangeComboPlaylist)

	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_CHANNELS, &CEdemChannelEditorDlg::OnTvnSelchangedTreeChannels)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_CHANNELS, &CEdemChannelEditorDlg::OnNMDblclkTreeChannels)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_TREE_CHANNELS, &CEdemChannelEditorDlg::OnTvnEndlabeleditTreeChannels)
	ON_NOTIFY(NM_RCLICK, IDC_TREE_CHANNELS, &CEdemChannelEditorDlg::OnNMRclickTreeChannel)
	ON_NOTIFY(NM_SETFOCUS, IDC_TREE_CHANNELS, &CEdemChannelEditorDlg::OnNMSetfocusTree)
	ON_NOTIFY(TVN_GETINFOTIP, IDC_TREE_CHANNELS, &CEdemChannelEditorDlg::OnTvnChannelsGetInfoTip)

	ON_NOTIFY(NM_DBLCLK, IDC_TREE_PLAYLIST, &CEdemChannelEditorDlg::OnNMDblclkTreePaylist)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_PLAYLIST, &CEdemChannelEditorDlg::OnTvnSelchangedTreePaylist)
	ON_NOTIFY(NM_RCLICK, IDC_TREE_PLAYLIST, &CEdemChannelEditorDlg::OnNMRclickTreePlaylist)
	ON_NOTIFY(NM_SETFOCUS, IDC_TREE_PLAYLIST, &CEdemChannelEditorDlg::OnNMSetfocusTree)
	ON_NOTIFY(TVN_GETINFOTIP, IDC_TREE_PLAYLIST, &CEdemChannelEditorDlg::OnTvnPlaylistGetInfoTip)

	ON_COMMAND(ID_SAVE, &CEdemChannelEditorDlg::OnSave)
	ON_UPDATE_COMMAND_UI(ID_SAVE, &CEdemChannelEditorDlg::OnUpdateSave)
	ON_COMMAND(ID_NEW_CHANNEL, &CEdemChannelEditorDlg::OnNewChannel)
	ON_UPDATE_COMMAND_UI(ID_NEW_CHANNEL, &CEdemChannelEditorDlg::OnUpdateNewChannel)
	ON_COMMAND(ID_EDIT_RENAME, &CEdemChannelEditorDlg::OnRenameChannel)
	ON_UPDATE_COMMAND_UI(ID_EDIT_RENAME, &CEdemChannelEditorDlg::OnUpdateRenameChannel)
	ON_COMMAND(ID_REMOVE_CHANNEL, &CEdemChannelEditorDlg::OnRemoveChannel)
	ON_UPDATE_COMMAND_UI(ID_REMOVE_CHANNEL, &CEdemChannelEditorDlg::OnUpdateRemoveChannel)
	ON_COMMAND(ID_UPDATE_CHANNEL, &CEdemChannelEditorDlg::OnAddUpdateChannel)
	ON_UPDATE_COMMAND_UI(ID_UPDATE_CHANNEL, &CEdemChannelEditorDlg::OnUpdateAddUpdateChannel)
	ON_COMMAND(ID_UPDATE_ICON, &CEdemChannelEditorDlg::OnUpdateIcon)
	ON_UPDATE_COMMAND_UI(ID_UPDATE_ICON, &CEdemChannelEditorDlg::OnUpdateUpdateIcon)
	ON_COMMAND(ID_CHANNEL_UP, &CEdemChannelEditorDlg::OnChannelUp)
	ON_UPDATE_COMMAND_UI(ID_CHANNEL_UP, &CEdemChannelEditorDlg::OnUpdateChannelUp)
	ON_COMMAND(ID_CHANNEL_DOWN, &CEdemChannelEditorDlg::OnChannelDown)
	ON_UPDATE_COMMAND_UI(ID_CHANNEL_DOWN, &CEdemChannelEditorDlg::OnUpdateChannelDown)
	ON_COMMAND(ID_ADD_CATEGORY, &CEdemChannelEditorDlg::OnAddCategory)
	ON_UPDATE_COMMAND_UI(ID_ADD_CATEGORY, &CEdemChannelEditorDlg::OnUpdateAddCategory)
	ON_COMMAND(ID_NEW_CATEGORY, &CEdemChannelEditorDlg::OnNewCategory)
	ON_COMMAND(ID_REMOVE_CATEGORY, &CEdemChannelEditorDlg::OnRemoveCategory)
	ON_UPDATE_COMMAND_UI(ID_REMOVE_CATEGORY, &CEdemChannelEditorDlg::OnUpdateRemoveCategory)
	ON_COMMAND(ID_SORT_CATEGORY, &CEdemChannelEditorDlg::OnSortCategory)
	ON_UPDATE_COMMAND_UI(ID_SORT_CATEGORY, &CEdemChannelEditorDlg::OnUpdateSortCategory)
	ON_COMMAND(ID_TOGGLE_CHANNEL, &CEdemChannelEditorDlg::OnToggleChannel)
	ON_UPDATE_COMMAND_UI(ID_TOGGLE_CHANNEL, &CEdemChannelEditorDlg::OnUpdateToggleChannel)
	ON_COMMAND(ID_GET_STREAM_INFO, &CEdemChannelEditorDlg::OnGetStreamInfo)
	ON_UPDATE_COMMAND_UI(ID_GET_STREAM_INFO, &CEdemChannelEditorDlg::OnUpdateGetStreamInfo)
	ON_COMMAND(ID_GET_STREAM_INFO_ALL, &CEdemChannelEditorDlg::OnGetStreamInfoAll)
	ON_UPDATE_COMMAND_UI(ID_GET_STREAM_INFO_ALL, &CEdemChannelEditorDlg::OnUpdateGetStreamInfoAll)
	ON_COMMAND(ID_PLAY_STREAM, &CEdemChannelEditorDlg::OnPlayChannelStream)
	ON_UPDATE_COMMAND_UI(ID_PLAY_STREAM, &CEdemChannelEditorDlg::OnUpdatePlayChannelStream)
	ON_COMMAND(ID_PLAY_STREAM_PL, &CEdemChannelEditorDlg::OnPlayPlaylistStream)
	ON_UPDATE_COMMAND_UI(ID_PLAY_STREAM_PL, &CEdemChannelEditorDlg::OnUpdatePlayPlaylistStream)
	ON_COMMAND(ID_SYNC_TREE_ITEM, &CEdemChannelEditorDlg::OnSyncTreeItem)
	ON_UPDATE_COMMAND_UI(ID_SYNC_TREE_ITEM, &CEdemChannelEditorDlg::OnUpdateSyncTreeItem)
	ON_COMMAND(ID_ADD_TO_FAVORITE, &CEdemChannelEditorDlg::OnAddToFavorite)
	ON_UPDATE_COMMAND_UI(ID_ADD_TO_FAVORITE, &CEdemChannelEditorDlg::OnUpdateAddToFavorite)

	ON_MESSAGE_VOID(WM_KICKIDLE, OnKickIdle)
	ON_MESSAGE(WM_END_LOAD_PLAYLIST, &CEdemChannelEditorDlg::OnEndLoadPlaylist)
	ON_MESSAGE(WM_UPDATE_PROGRESS, &CEdemChannelEditorDlg::OnUpdateProgress)

	ON_COMMAND_RANGE(ID_COPY_TO_START, ID_COPY_TO_END, &CEdemChannelEditorDlg::OnCopyTo)
	ON_COMMAND_RANGE(ID_MOVE_TO_START, ID_MOVE_TO_END, &CEdemChannelEditorDlg::OnMoveTo)
	ON_COMMAND_RANGE(ID_ADD_TO_START, ID_ADD_TO_END, &CEdemChannelEditorDlg::OnAddTo)
END_MESSAGE_MAP()

CEdemChannelEditorDlg::CEdemChannelEditorDlg(CWnd* pParent /*=nullptr*/)
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

void CEdemChannelEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_ARCHIVE, m_hasArchive);
	DDX_Control(pDX, IDC_CHECK_ARCHIVE, m_wndArchive);
	DDX_Check(pDX, IDC_CHECK_ADULT, m_isAdult);
	DDX_Control(pDX, IDC_CHECK_ADULT, m_wndAdult);
	DDX_Control(pDX, IDC_TREE_CHANNELS, m_wndChannelsTree);
	DDX_Control(pDX, IDC_CHECK_CUSTOMIZE, m_wndCustom);
	DDX_Text(pDX, IDC_EDIT_URL_ID, m_streamID);
	DDX_Control(pDX, IDC_EDIT_URL_ID, m_wndStreamID);
	DDX_Text(pDX, IDC_EDIT_TVG_ID, m_tvgID);
	DDX_Control(pDX, IDC_EDIT_TVG_ID, m_wndTvgID);
	DDX_Control(pDX, IDC_BUTTON_TEST_TVG, m_wndTestTVG);
	DDX_Text(pDX, IDC_EDIT_EPG_ID, m_epgID);
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

BOOL CEdemChannelEditorDlg::OnInitDialog()
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

	SetAccessKey(theApp.GetProfileString(_T("Setting"), _T("AccessKey")));
	SetDomain(theApp.GetProfileString(_T("Setting"), _T("Domain")));
	m_player = theApp.GetProfileString(_T("Setting"), _T("Player"));
	m_probe = theApp.GetProfileString(_T("Setting"), _T("FFProbe"));
	m_archiveCheck = theApp.GetProfileInt(_T("Setting"), _T("HoursBack"), 0);
	m_bAutoSync = theApp.GetProfileInt(_T("Setting"), _T("AutoSyncChannel"), FALSE);

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

	m_wndPlaylist.SetCurSel(theApp.GetProfileInt(_T("Setting"), _T("PlaylistType"), 0));

	m_all_channels_lists.emplace_back(_T("Standard"), theApp.GetAppPath(utils::CHANNELS_CONFIG));
	CFileFind ffind;
	BOOL bFound = ffind.FindFile(_T("edem_plugin\\*.xml"));
	while (bFound)
	{
		bFound = ffind.FindNextFile();
		if (!ffind.IsDirectory() && ffind.GetFileName() != "dune_plugin.xml" && ffind.GetFileName() != "edem_channel_list.xml")
		{
			m_all_channels_lists.emplace_back(ffind.GetFileName(), ffind.GetFilePath());
		}
	}

	for(const auto& playlist : m_all_channels_lists)
	{
		int idx = m_wndChannels.AddString(playlist.first);
		m_wndChannels.SetItemData(idx, (DWORD_PTR)playlist.second.GetString());
	}

	int idx = theApp.GetProfileInt(_T("Setting"), _T("ChannelsType"), 0);
	if (idx < m_wndChannels.GetCount())
		m_wndChannels.SetCurSel(idx);

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

	OnCbnSelchangeComboPlaylist();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CEdemChannelEditorDlg::LoadPlaylist(bool saveToFile /*= false*/)
{
	// #EXTM3U <--- header
	// #EXTINF:0 tvg-rec="3",Первый FHD <-- caption
	// #EXTGRP:Общие <-- Category
	// http://6646b6bc.akadatel.com/iptv/PWXQ2KD5G2VNSK/2402/index.m3u8

	AfxGetApp()->BeginWaitCursor();

	CString slashed = theApp.GetProfileString(_T("Setting"), _T("Playlist"));
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

	m_playlistIds.clear();
	m_playlistMap.clear();
	m_pl_categories.clear();
	m_pl_categoriesMap.clear();

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

	const CString& file = theApp.GetProfileString(_T("Setting"), _T("Playlist"));
	auto data = std::make_unique<std::vector<BYTE>>();
	if (utils::CrackUrl(utils::utf16_to_utf8(file.GetString())))
	{
		if (utils::DownloadFile(utils::utf16_to_utf8(file.GetString()), *data) && saveToFile)
		{
			std::ofstream os(m_plFileName);
			os.write((char*)data->data(), data->size());
			os.close();
			return;
		}
	}
	else
	{
		std::ifstream stream(file.GetString());
		data->assign((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
	}

	if (!data->empty())
	{
		m_wndProgress.SetRange32(0, (int)std::count(data->begin(), data->end(), '\n'));
		m_wndProgress.SetPos(0);
		m_wndProgress.ShowWindow(SW_SHOW);

		auto* pThread = (CPlaylistParseThread*)AfxBeginThread(RUNTIME_CLASS(CPlaylistParseThread), THREAD_PRIORITY_HIGHEST, 0, CREATE_SUSPENDED);
		if (pThread)
		{
			m_loading = TRUE;
			m_wndGetInfo.EnableWindow(FALSE);
			m_wndCheckArchive.EnableWindow(FALSE);
			m_wndPlaylist.EnableWindow(FALSE);
			m_evtStop.ResetEvent();

			CPlaylistParseThread::ThreadConfig cfg;
			cfg.m_parent = this;
			cfg.m_data = data.release();
			cfg.m_hStop = m_evtStop;
			cfg.m_filter = theApp.GetProfileString(_T("Setting"), _T("FilterString"));
			cfg.m_regex = theApp.GetProfileInt(_T("Setting"), _T("FilterUseRegex"), FALSE);
			cfg.m_case = theApp.GetProfileInt(_T("Setting"), _T("FilterUseCase"), FALSE);

			pThread->SetData(cfg);
			pThread->ResumeThread();

			return;
		}
	}

	AfxGetApp()->EndWaitCursor();
}

LRESULT CEdemChannelEditorDlg::OnEndLoadPlaylist(WPARAM wParam, LPARAM lParam /*= 0*/)
{
	std::unique_ptr<std::vector<std::unique_ptr<PlaylistEntry>>> entries;
	entries.reset((std::vector<std::unique_ptr<PlaylistEntry>>*)wParam);
	if (entries)
	{
		std::set<std::wstring> categories;
		for (auto& entry : *entries)
		{
			auto res = m_playlistMap.emplace(entry->get_id(), std::move(entry));
			if (res.second)
			{
				m_playlistIds.emplace_back(res.first->second->get_id());

				const auto& category = res.first->second->get_category();
				if (categories.find(category) == categories.end())
				{
					m_pl_categories.emplace_back(category);
					categories.emplace(category);
				}
			}
			else
			{
				TRACE("Duplicate channel: %s (%d)\n", res.first->second->get_title().c_str(), res.first->second->get_id());
			}
		}
	}

	m_wndProgress.ShowWindow(SW_HIDE);
	m_wndPlSearch.EnableWindow(!m_channelsMap.empty());

	FillTreePlaylist();

	m_loading = FALSE;
	m_wndPlaylist.EnableWindow(TRUE);
	m_wndGetInfo.EnableWindow(TRUE);
	m_wndCheckArchive.EnableWindow(TRUE);

	AfxGetApp()->EndWaitCursor();

	return 0;
}

LRESULT CEdemChannelEditorDlg::OnUpdateProgress(WPARAM wParam, LPARAM lParam /*= 0*/)
{
	m_wndProgress.SetPos(wParam);

	return 0;
}

void CEdemChannelEditorDlg::OnKickIdle()
{
	UpdateDialogControls(this, FALSE);
}

void CEdemChannelEditorDlg::OnOK()
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

void CEdemChannelEditorDlg::OnCancel()
{
	if (is_allow_save() && AfxMessageBox(_T("You have unsaved changes.\nAre you sure?"), MB_YESNO | MB_ICONWARNING) != IDYES)
	{
		return;
	}

	m_evtStop.SetEvent();

	EndDialog(IDCANCEL);
}

BOOL CEdemChannelEditorDlg::PreTranslateMessage(MSG* pMsg)
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

void CEdemChannelEditorDlg::OnDestroy()
{
	VERIFY(DestroyAcceleratorTable(m_hAccel));

	__super::OnDestroy();
}

void CEdemChannelEditorDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
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

void CEdemChannelEditorDlg::set_allow_save(BOOL val)
{
	m_allow_save = val;
	if (m_wndSave.GetSafeHwnd())
		m_wndSave.EnableWindow(m_allow_save);
}

void CEdemChannelEditorDlg::FillTreeChannels()
{
	m_wndChannelsTree.LockWindowUpdate();

	m_wndChannelsTree.DeleteAllItems();

	for (auto& pair : m_categoriesMap)
	{
		TVINSERTSTRUCTW tvCategory = { nullptr };
		tvCategory.hParent = TVI_ROOT;
		tvCategory.item.pszText = (LPWSTR)pair.second->get_caption().c_str();
		tvCategory.item.mask = TVIF_TEXT | TVIF_PARAM;
		tvCategory.item.lParam = (DWORD_PTR)pair.second->get_id();
		tvCategory.hInsertAfter = (pair.first == ID_ADD_TO_FAVORITE) ? TVI_FIRST : nullptr;
		auto hParent = m_wndChannelsTree.InsertItem(&tvCategory);

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

	m_wndChannelsTree.UnlockWindowUpdate();

	UpdateChannelsCount();
	CheckForExistingChannels();

	if (!m_channelsMap.empty())
	{
		SelectTreeItem(true, m_wndChannelsTree, _T(""), true, m_categoriesMap.begin()->second->get_channels().front()->get_id());
	}
}

void CEdemChannelEditorDlg::UpdateChannelsCount()
{
	CString str;
	str.Format(_T("Channels: %s (%d)"), m_chFileName.GetString(), m_channelsMap.size());
	m_wndChInfo.SetWindowText(str);

	UpdateData(FALSE);
}

void CEdemChannelEditorDlg::UpdatePlaylistCount()
{
	const auto& filterString = theApp.GetProfileString(_T("Setting"), _T("FilterString"));

	CString str;
	str.Format(_T("Playlist: %s (%d%s)"), m_plFileName.GetString(), m_playlistMap.size(), (filterString.IsEmpty() ? _T("") : _T("*")));
	m_wndPlInfo.SetWindowText(str);
	UpdateData(FALSE);
}

void CEdemChannelEditorDlg::RemoveOrphanChannels()
{
	std::set<int> ids;
	for (const auto& pair : m_categoriesMap)
	{
		if (pair.first == ID_ADD_TO_FAVORITE) continue;

		for (const auto& ch : pair.second->get_channels())
		{
			ids.emplace(ch->get_id());
		}
	}

	auto pair = m_channelsMap.begin();
	while (pair != m_channelsMap.end())
	{
		if (ids.find(pair->second->get_id()) == ids.end())
		{
			pair = m_channelsMap.erase(pair);
		}
		else
		{
			++pair;
		}
	}
}

void CEdemChannelEditorDlg::CheckForExistingChannels(HTREEITEM root /*= nullptr*/)
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

void CEdemChannelEditorDlg::CheckForExistingPlaylist()
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

void CEdemChannelEditorDlg::LoadChannelInfo(HTREEITEM hItem)
{
	TRACE("LoadChannelInfo\n");

	m_infoAudio.Empty();
	m_infoVideo.Empty();

	auto channel = GetChannel(hItem);
	if (channel)
	{
		m_tvgID = channel->get_tvg_id();
		m_epgID = channel->get_epg_id();
		m_streamUrl = channel->get_stream_uri().get_uri().c_str();
		m_streamID = channel->get_id();
		m_infoAudio = channel->get_audio().c_str();
		m_infoVideo = channel->get_video().c_str();
		m_wndCustom.SetCheck(!channel->get_stream_uri().is_template());
		m_timeShiftHours = channel->get_time_shift_hours();
		m_hasArchive = channel->get_archive();
		m_isAdult = channel->get_adult();

		if(m_iconUrl != channel->get_icon_uri().get_uri().c_str())
		{
			m_iconUrl = channel->get_icon_uri().get_uri().c_str();
			CImage img;
			if (theApp.LoadImage(m_iconUrl, img))
			{
				channel->set_icon(img);
			}

			theApp.SetImage(channel->get_icon(), m_wndIcon);
		}
	}
	else
	{
		m_tvgID = 0;
		m_epgID = 0;
		m_timeShiftHours = 0;
		m_iconUrl.Empty();
		m_streamUrl.Empty();
		m_streamID = 0;
		m_infoAudio.Empty();
		m_infoVideo.Empty();
		m_wndIcon.SetBitmap(nullptr);
		m_wndCustom.SetCheck(FALSE);
	}

	UpdateData(FALSE);
}

void CEdemChannelEditorDlg::LoadPlayListInfo(HTREEITEM hItem)
{
	UpdateData(TRUE);

	auto entry = GetPlaylistEntry(hItem);
	if (entry)
	{
		m_plIconName = entry->get_icon_uri().get_uri().c_str();
		int id = entry->get_id();
		m_plID.Format(_T("ID: %d"), id);

		if (entry->get_tvg_id() != -1)
			m_plEPG.Format(_T("EPG: %d"), entry->get_tvg_id());

		m_wndPlArchive.SetCheck(!!entry->is_archive());
		m_archiveDays = entry->get_archive();
		m_infoAudio = entry->get_audio().c_str();
		m_infoVideo = entry->get_video().c_str();

		CImage img;
		if (theApp.LoadImage(utils::utf8_to_utf16(entry->get_icon_uri().get_uri()).c_str(), img))
		{
			entry->set_icon(img);
		}

		theApp.SetImage(entry->get_icon(), m_wndPlIcon);

		if (m_bAutoSync)
		{
			OnSyncTreeItem();
		}

		UpdateData(FALSE);
	}
}

ChannelInfo* CEdemChannelEditorDlg::GetChannel(HTREEITEM hItem) const
{
	return IsChannel(hItem) ? (ChannelInfo*)m_wndChannelsTree.GetItemData(hItem) : nullptr;
}

std::shared_ptr<ChannelInfo> CEdemChannelEditorDlg::FindChannel(HTREEITEM hItem) const
{
	auto channel = GetChannel(hItem);
	if (!channel)
		return nullptr;

	return m_channelsMap.at(channel->get_id());
}

ChannelCategory* CEdemChannelEditorDlg::GetItemCategory(HTREEITEM hItem) const
{
	if (IsChannel(hItem))
	{
		hItem = m_wndChannelsTree.GetParentItem(hItem);
	}

	return GetCategory(hItem);
}

ChannelCategory* CEdemChannelEditorDlg::GetCategory(HTREEITEM hItem) const
{
	if (!IsCategory(hItem))
		return nullptr;

	auto found = m_categoriesMap.find((int)m_wndChannelsTree.GetItemData(hItem));

	return found != m_categoriesMap.end() ? found->second.get() : nullptr;
}

HTREEITEM CEdemChannelEditorDlg::GetCategoryTreeItemById(int id) const
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

PlaylistEntry* CEdemChannelEditorDlg::GetPlaylistEntry(HTREEITEM hItem) const
{
	if (hItem == nullptr || m_wndPlaylistTree.GetParentItem(hItem) == nullptr)
		return nullptr;

	return (PlaylistEntry*)m_wndPlaylistTree.GetItemData(hItem);
}

int CEdemChannelEditorDlg::GetNewCategoryID() const
{
	int id = 0;
	if (!m_categoriesMap.empty())
	{
		id = m_categoriesMap.crbegin()->first;
	}

	return ++id;
}

int CEdemChannelEditorDlg::GetCategoryIdByName(const std::wstring& categoryName)
{
	// Search for existing category
	for (const auto& category : m_categoriesMap)
	{
		if (category.second->get_caption() == categoryName)
		{
			return category.first;
		}
	}

	return -1;
}

bool CEdemChannelEditorDlg::LoadChannels(const CString& path, bool& changed)
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
	m_ch_access_key.Empty();
	m_ch_domain.Empty();
	auto setup_node = i_node->first_node(utils::CHANNELS_SETUP);
	if (setup_node)
	{
		m_ch_access_key = utils::get_value_wstring(setup_node->first_node(utils::ACCESS_KEY)).c_str();
		m_ch_domain = utils::get_value_wstring(setup_node->first_node(utils::ACCESS_DOMAIN)).c_str();
		m_embedded_info = TRUE;
	}

	auto cat_node = i_node->first_node(utils::TV_CATEGORIES)->first_node(ChannelCategory::TV_CATEGORY);
	// Iterate <tv_category> nodes
	while (cat_node)
	{
		auto category = std::make_unique<ChannelCategory>(cat_node);
		m_categoriesMap.emplace(category->get_id(), std::move(category));
		cat_node = cat_node->next_sibling();
	}

	auto fav_category = std::make_unique<ChannelCategory>();
	fav_category->set_icon_uri("plugin_file:////icons//fav.png");
	fav_category->set_caption(L"Favorites");
	fav_category->set_id(ID_ADD_TO_FAVORITE);

	auto ch_node = i_node->first_node(utils::TV_CHANNELS)->first_node(ChannelInfo::TV_CHANNEL);
	// Iterate <tv_channel> nodes
	while (ch_node)
	{
		auto channel = std::make_shared<ChannelInfo>(ch_node);
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

void CEdemChannelEditorDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CEdemChannelEditorDlg::OnPaint()
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
HCURSOR CEdemChannelEditorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CEdemChannelEditorDlg::OnAddCategory()
{
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

void CEdemChannelEditorDlg::OnUpdateAddCategory(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsPlaylistCategory(m_wndPlaylistTree.GetFirstSelectedItem()) && IsSelectedTheSameType());
}

void CEdemChannelEditorDlg::OnNewChannel()
{
	auto hCategory = m_wndChannelsTree.GetSelectedItem();
	if (auto hRoot = m_wndChannelsTree.GetParentItem(hCategory); hRoot != nullptr)
	{
		hCategory = hRoot;
	}

	auto category = GetCategory(hCategory);
	if (!category)
		return;

	auto channel = std::make_shared<ChannelInfo>();
	channel->set_title(L"New Channel");
	channel->set_icon_uri(utils::ICON_TEMPLATE);

	const auto& root = utils::utf16_to_utf8(theApp.GetAppPath(utils::PLUGIN_ROOT).GetString());
	const auto& path = utils::utf8_to_utf16(channel->get_icon_uri().get_icon_absolute_path(root));
	CImage img;
	if (theApp.LoadImage(path.c_str(), img))
	{
		theApp.SetImage(img, m_wndIcon);
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

void CEdemChannelEditorDlg::OnUpdateNewChannel(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!!IsSelectedNotFavorite());
}

void CEdemChannelEditorDlg::OnRemoveChannel()
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

		if (category->get_id() == ID_ADD_TO_FAVORITE)
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

void CEdemChannelEditorDlg::OnUpdateRemoveChannel(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!!IsSelectedChannelsOrEntries(true) && IsSelectedTheSameCategory());
}

void CEdemChannelEditorDlg::OnChannelUp()
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

void CEdemChannelEditorDlg::OnUpdateChannelUp(CCmdUI* pCmdUI)
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

void CEdemChannelEditorDlg::OnChannelDown()
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

void CEdemChannelEditorDlg::OnUpdateChannelDown(CCmdUI* pCmdUI)
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

void CEdemChannelEditorDlg::MoveChannels(HTREEITEM hBegin, HTREEITEM hEnd, bool down)
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

void CEdemChannelEditorDlg::SwapCategories(const HTREEITEM hLeft, const HTREEITEM hRight)
{
	auto lCat = GetCategory(hLeft);
	auto rCat = GetCategory(hRight);

	// swap categories id
	lCat->swap_id(*rCat);

	// swap pointers in map
	std::swap(m_categoriesMap[lCat->get_id()], m_categoriesMap[rCat->get_id()]);

	// swap ItemData
	DWORD_PTR data = m_wndChannelsTree.GetItemData(hRight);
	m_wndChannelsTree.SetItemData(hRight, m_wndChannelsTree.GetItemData(hLeft));
	m_wndChannelsTree.SetItemData(hLeft, data);

	// запоминаем ItemData для нод и подменяем на на счетчик
	struct pair
	{
		HTREEITEM hItem;
		DWORD_PTR lParam;
	};

	std::vector<pair> itemData;
	DWORD_PTR idx = 0;
	HTREEITEM hItem = m_wndChannelsTree.GetChildItem(nullptr);
	while (hItem)
	{
		itemData.emplace_back(pair({ hItem, m_wndChannelsTree.GetItemData(hItem) }));
		m_wndChannelsTree.SetItemData(hItem, idx++);
		hItem = m_wndChannelsTree.GetNextSiblingItem(hItem);
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
		m_wndChannelsTree.SetItemData(it.hItem, it.lParam);
	}

	m_wndChannelsTree.SelectItem(hLeft);

	set_allow_save();
}

void CEdemChannelEditorDlg::OnRenameChannel()
{
	if (::GetFocus() == m_wndChannelsTree.GetSafeHwnd())
	{
		m_wndChannelsTree.EditLabel(m_wndChannelsTree.GetSelectedItem());
	}
}

void CEdemChannelEditorDlg::OnUpdateRenameChannel(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_wndChannelsTree.GetSelectedCount() == 1 && GetChannel(m_wndChannelsTree.GetSelectedItem()) != nullptr && IsSelectedNotFavorite());
}

void CEdemChannelEditorDlg::OnBnClickedCheckCustomize()
{
	BOOL checked = m_wndCustom.GetCheck();
	m_wndStreamUrl.EnableWindow(checked);
	m_wndStreamID.EnableWindow(!checked);

	set_allow_save();
}

void CEdemChannelEditorDlg::OnTvnSelchangedTreeChannels(NMHDR* pNMHDR, LRESULT* pResult)
{
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
				m_streamID = channel->get_stream_uri().is_template() ? m_streamID : 0;
				m_wndIcon.EnableWindow(TRUE);
			}
			else if (IsCategory(hSelected))
			{
				m_tvgID = 0;
				m_epgID = 0;
				m_hasArchive = 0;
				m_isAdult = 0;
				m_streamUrl.Empty();
				m_streamID = 0;
				m_infoAudio.Empty();
				m_infoVideo.Empty();

				m_wndIcon.EnableWindow(TRUE);
				auto category = GetCategory(hSelected);
				if (category)
				{
					m_iconUrl = category->get_icon_uri().get_uri().c_str();
					const auto& root = utils::utf16_to_utf8(theApp.GetAppPath(utils::PLUGIN_ROOT).GetString());
					const auto& path = utils::utf8_to_utf16(category->get_icon_uri().get_icon_absolute_path(root));
					CImage img;
					if (theApp.LoadImage(path.c_str(), img))
					{
						theApp.SetImage(img, m_wndIcon);
					}
				}
			}
		}
	}

	BOOL enable = (state == 1);
	bool bSameCategory = IsSelectedTheSameCategory();

	m_wndCustom.EnableWindow(enable);
	m_wndTvgID.EnableWindow(enable);
	m_wndEpgID.EnableWindow(enable);
	m_wndArchive.EnableWindow(state);
	m_wndAdult.EnableWindow(state);
	m_wndTestTVG.EnableWindow(enable);
	m_wndTestEPG.EnableWindow(enable);
	m_wndStreamID.EnableWindow(enable && m_streamID != 0);
	m_wndStreamUrl.EnableWindow(enable && m_streamID == 0);
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
			if (m_lastTree == m_wndChannelsTree.GetSafeHwnd())
				bEnable &= GetChannel(m_wndChannelsTree.GetFirstSelectedItem()) != nullptr;
			else
				bEnable &= GetPlaylistEntry(m_wndPlaylistTree.GetFirstSelectedItem()) != nullptr;
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

void CEdemChannelEditorDlg::OnTvnEndlabeleditTreeChannels(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);

	if (pTVDispInfo->item.pszText && pTVDispInfo->item.pszText[0])
	{
		*pResult = TRUE;

		auto category = GetCategory(pTVDispInfo->item.hItem);
		if (category)
		{
			category->set_caption(pTVDispInfo->item.pszText);
			set_allow_save();
			return;
		}

		auto channel = GetChannel(pTVDispInfo->item.hItem);
		if (channel)
		{
			channel->set_title(pTVDispInfo->item.pszText);
		}
	}
	else
	{
		*pResult = 0;
	}
}

void CEdemChannelEditorDlg::OnNMDblclkTreeChannels(NMHDR* pNMHDR, LRESULT* pResult)
{
	CPoint pt(GetMessagePos());
	m_wndChannelsTree.ScreenToClient(&pt);

	*pResult = 0;
	UINT uFlags = 0;
	HTREEITEM hItem = m_wndChannelsTree.HitTest(pt, &uFlags);
	if (hItem && (uFlags & TVHT_ONITEM))
	{
		PlayChannel(hItem);
	}
}

void CEdemChannelEditorDlg::OnNMRclickTreeChannel(NMHDR* pNMHDR, LRESULT* pResult)
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
		if (itemCategory != nullptr && itemCategory->get_id() != ID_ADD_TO_FAVORITE)
		{
			for (const auto& category : m_categoriesMap)
			{
				if (ID_ADD_TO_FAVORITE == category.first || itemCategory->get_id() == category.first) continue;

				subMenuCopy.AppendMenu(MF_STRING | MF_ENABLED, ID_COPY_TO_START + category.first, category.second->get_caption().c_str());
				subMenuMove.AppendMenu(MF_STRING | MF_ENABLED, ID_MOVE_TO_START + category.first, category.second->get_caption().c_str());
			}

			pMenu->InsertMenu(ID_EDIT_RENAME, MF_BYCOMMAND | MF_POPUP, (UINT_PTR)subMenuCopy.Detach(), _T("Copy To"));
			pMenu->InsertMenu(ID_EDIT_RENAME, MF_BYCOMMAND | MF_POPUP, (UINT_PTR)subMenuMove.Detach(), _T("Move To"));
		}
	}

	CContextMenuManager* manager = theApp.GetContextMenuManager();
	//for CDialogEx:
	theApp.GetContextMenuManager()->ShowPopupMenu(pMenu->GetSafeHmenu(), ptScreen.x, ptScreen.y, this, TRUE, TRUE, FALSE);
}

void CEdemChannelEditorDlg::OnNMDblclkTreePaylist(NMHDR* pNMHDR, LRESULT* pResult)
{
	CPoint pt(GetMessagePos());
	m_wndPlaylistTree.ScreenToClient(&pt);

	*pResult = 0;
	UINT uFlags = 0;
	HTREEITEM hItem = m_wndPlaylistTree.HitTest(pt, &uFlags);
	if (hItem && (uFlags & TVHT_ONITEM))
	{
		PlayPlaylistEntry(hItem);
	}
}

void CEdemChannelEditorDlg::OnNMRclickTreePlaylist(NMHDR* pNMHDR, LRESULT* pResult)
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
		pMenu->SetDefaultItem(ID_PLAY_STREAM_PL);

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
			subMenuCopy.AppendMenu(MF_STRING | MF_ENABLED, ID_ADD_TO_START + category.first, category.second->get_caption().c_str());
		}

		pMenu->InsertMenu(ID_PLAYLISTMENU_SUBMENU, MF_BYCOMMAND | MF_POPUP, (UINT_PTR)subMenuCopy.Detach(), _T("Add/Update To"));
	}

	pMenu->DeleteMenu(ID_PLAYLISTMENU_SUBMENU, MF_BYCOMMAND);

	CContextMenuManager* manager = theApp.GetContextMenuManager();
	//for CDialogEx:
	theApp.GetContextMenuManager()->ShowPopupMenu(pMenu->GetSafeHmenu(), ptScreen.x, ptScreen.y, this, TRUE, TRUE, FALSE);
}

void CEdemChannelEditorDlg::OnAddToFavorite()
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

void CEdemChannelEditorDlg::OnUpdateAddToFavorite(CCmdUI* pCmdUI)
{
	auto itemCategory = GetItemCategory(m_wndChannelsTree.GetFirstSelectedItem());

	pCmdUI->Enable(itemCategory != nullptr && itemCategory->get_id() != ID_ADD_TO_FAVORITE);
}

void CEdemChannelEditorDlg::OnCopyTo(UINT id)
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

void CEdemChannelEditorDlg::OnMoveTo(UINT id)
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

void CEdemChannelEditorDlg::OnAddTo(UINT id)
{
	UINT category_id = id - ID_ADD_TO_START;
	auto pair = m_categoriesMap.find(category_id);
	if (pair == m_categoriesMap.end())
		return;

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
}

void CEdemChannelEditorDlg::OnEditChangeTvIdd()
{
	set_allow_save();
}

void CEdemChannelEditorDlg::OnBnClickedCheckAdult()
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

void CEdemChannelEditorDlg::OnBnClickedCheckArchive()
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

void CEdemChannelEditorDlg::OnEnChangeEditTvgID()
{
	UpdateData(TRUE);
	if (m_wndChannelsTree.GetSelectedCount() == 1)
	{
		auto channel = GetChannel(m_wndChannelsTree.GetSelectedItem());
		if (channel)
		{
			channel->set_tvg_id(m_tvgID);
			set_allow_save();
		}
	}
}

void CEdemChannelEditorDlg::OnEnChangeEditEpgID()
{
	UpdateData(TRUE);
	if (m_wndChannelsTree.GetSelectedCount() == 1)
	{
		auto channel = GetChannel(m_wndChannelsTree.GetSelectedItem());
		if (channel)
		{
			channel->set_epg_id(m_epgID);
			set_allow_save();
		}
	}
}

void CEdemChannelEditorDlg::OnEnChangeEditStreamUrl()
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

void CEdemChannelEditorDlg::OnEnChangeEditUrlID()
{
	UpdateData(TRUE);
	if (m_wndChannelsTree.GetSelectedCount() == 1)
	{
		auto channel = GetChannel(m_wndChannelsTree.GetSelectedItem());
		if (channel && channel->get_id() != m_streamID)
		{
			channel->set_id(m_streamID);
			CheckForExistingChannels(m_wndChannelsTree.GetParentItem(m_wndChannelsTree.GetSelectedItem()));
			CheckForExistingPlaylist();
			set_allow_save();
		}
	}
}

void CEdemChannelEditorDlg::OnEnChangeEditTimeShiftHours()
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

void CEdemChannelEditorDlg::OnEnChangeEditArchiveCheck()
{
	UpdateData(TRUE);

	if (m_archiveCheck < 0)
		m_archiveCheck = 0;

	UpdateData(FALSE);

	theApp.WriteProfileInt(_T("Setting"), _T("HoursBack"), m_archiveCheck);
}

void CEdemChannelEditorDlg::OnDeltaposSpinTimeShiftHours(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateData(TRUE);
	m_timeShiftHours -= reinterpret_cast<LPNMUPDOWN>(pNMHDR)->iDelta;
	UpdateData(FALSE);
	OnEnChangeEditTimeShiftHours();
	*pResult = 0;
}

void CEdemChannelEditorDlg::OnDeltaposSpinArchiveCheck(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateData(TRUE);
	m_archiveCheck -= reinterpret_cast<LPNMUPDOWN>(pNMHDR)->iDelta;
	UpdateData(FALSE);
	OnEnChangeEditArchiveCheck();
	*pResult = 0;
}

void CEdemChannelEditorDlg::OnBnClickedButtonTestTvg()
{
	static LPCSTR url = "http://www.teleguide.info/kanal%d_%4d%02d%02d.html";

	auto channel = GetChannel(m_wndChannelsTree.GetSelectedItem());
	if (channel)
	{
		COleDateTime dt = COleDateTime::GetCurrentTime();
		CStringA tvg_url;
		tvg_url.Format(url, channel->get_tvg_id(), dt.GetYear(), dt.GetMonth(), dt.GetDay());
		ShellExecuteA(nullptr, "open", tvg_url.GetString(), nullptr, nullptr, SW_SHOWNORMAL);
	}
}

void CEdemChannelEditorDlg::OnBnClickedButtonTestEpg()
{
	static LPCSTR url = "http://epg.ott-play.com/php/show_prog.php?f=edem/epg/%d.json";

	auto channel = GetChannel(m_wndChannelsTree.GetSelectedItem());
	if (channel)
	{
		CStringA epg_url;
		epg_url.Format(url, channel->get_epg_id());
		ShellExecuteA(nullptr, "open", epg_url.GetString(), nullptr, nullptr, SW_SHOWNORMAL);
	}
}

void CEdemChannelEditorDlg::PlayChannel(HTREEITEM hItem, int archive_hour /*= 0*/) const
{
	if (auto channel = GetChannel(hItem); channel != nullptr)
	{
		PlayStream(TranslateStreamUri(channel->get_stream_uri().get_ts_translated_url()), archive_hour);
	}
}

void CEdemChannelEditorDlg::PlayPlaylistEntry(HTREEITEM hItem, int archive_hour /*= 0*/) const
{
	if (auto entry = GetPlaylistEntry(hItem); entry != nullptr)
	{
		PlayStream(TranslateStreamUri(entry->get_stream_uri().get_ts_translated_url()), archive_hour);
	}
}

void CEdemChannelEditorDlg::PlayStream(const std::string& stream_url, int archive_hour /*= 0*/) const
{
	TRACE(_T("Test URL: %s\n"), stream_url.c_str());
	CStringW test(stream_url.c_str());
	if (archive_hour)
	{
		test.Format(L"%hs?utc=%d&lutc=%d", stream_url.c_str(), _time32(nullptr) - 3600 * archive_hour, _time32(nullptr));
	}

	ShellExecuteW(nullptr, L"open", m_player, test, nullptr, SW_SHOWNORMAL);
}

void CEdemChannelEditorDlg::OnBnClickedButtonCustomPlaylist()
{
	CCustomPlaylistDlg dlg;
	dlg.m_url = theApp.GetProfileString(_T("Setting"), _T("CustomPlaylist"));
	dlg.m_isFile = (m_wndPlaylist.GetCurSel() == 3);
	if (dlg.DoModal() == IDOK)
	{
		theApp.WriteProfileString(_T("Setting"), _T("CustomPlaylist"), dlg.m_url);
		LoadPlaylist();
	}
}

void CEdemChannelEditorDlg::FillTreePlaylist()
{
	m_wndPlaylistTree.DeleteAllItems();
	// fill playlist tree
	if (!m_playlistIds.empty())
	{
		for (auto& category : m_pl_categories)
		{
			TVINSERTSTRUCTW tvInsert = { nullptr };
			tvInsert.hParent = TVI_ROOT;
			tvInsert.item.pszText = (LPWSTR)category.c_str();
			tvInsert.item.mask = TVIF_TEXT;
			auto item = m_wndPlaylistTree.InsertItem(&tvInsert);
			m_pl_categoriesMap[category] = item;
		}

		int step = 0;
		for (const auto& item : m_playlistIds)
		{
			auto pair = m_playlistMap.find(item);
			ASSERT(pair != m_playlistMap.end());
			if (pair == m_playlistMap.end()) continue;

			const auto& entry = pair->second;
			auto found = m_pl_categoriesMap.find(entry->get_category());
			ASSERT(found != m_pl_categoriesMap.end());
			if (found == m_pl_categoriesMap.end()) continue;

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

	UpdateData(FALSE);
}

void CEdemChannelEditorDlg::OnSave()
{
	std::wstring path = theApp.GetAppPath(utils::PLUGIN_ROOT + m_chFileName).GetString();

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
			pair.second->set_id(new_id);
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
			setup_node->append_node(utils::alloc_node(doc, utils::ACCESS_KEY, utils::utf16_to_utf8(GetAccessKey().GetString()).c_str()));
			setup_node->append_node(utils::alloc_node(doc, utils::ACCESS_DOMAIN, utils::utf16_to_utf8(GetAccessDomain().GetString()).c_str()));
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
		std::ofstream os(path, std::istream::binary);
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

void CEdemChannelEditorDlg::OnUpdateSave(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(is_allow_save());
}

void CEdemChannelEditorDlg::OnNewCategory()
{
	auto id = GetNewCategoryID();
	auto newCategory = std::make_unique<ChannelCategory>();
	newCategory->set_id(id);
	newCategory->set_caption(L"New Category");
	newCategory->set_icon_uri(utils::ICON_TEMPLATE);

	const auto& root = utils::utf16_to_utf8(theApp.GetAppPath(utils::PLUGIN_ROOT).GetString());
	const auto& path = utils::utf8_to_utf16(newCategory->get_icon_uri().get_icon_absolute_path(root));
	CImage img;
	if (theApp.LoadImage(path.c_str(), img))
	{
		theApp.SetImage(img, m_wndIcon);
	}

	TVINSERTSTRUCTW tvInsert = { nullptr };
	tvInsert.hParent = TVI_ROOT;
	tvInsert.item.pszText = (LPWSTR)newCategory->get_caption().c_str();
	tvInsert.item.lParam = (LPARAM)id;
	tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
	auto hNewItem = m_wndChannelsTree.InsertItem(&tvInsert);

	m_categoriesMap.emplace(id, std::move(newCategory));

	m_wndChannelsTree.SelectItem(hNewItem);
	m_wndChannelsTree.EditLabel(hNewItem);
}

void CEdemChannelEditorDlg::OnUpdateNewCategory(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CEdemChannelEditorDlg::OnRemoveCategory()
{
	CString msg(m_wndChannelsTree.GetSelectedCount() > 1 ? _T("Delete categories. Are your sure?") : _T("Delete category. Are your sure?"));
	if (AfxMessageBox(msg, MB_YESNO | MB_ICONWARNING) != IDYES)
		return;

	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		auto category_id = GetCategory(hItem)->get_id();
		if (category_id == ID_ADD_TO_FAVORITE) continue;

		m_categoriesMap.erase(category_id);
		m_wndChannelsTree.DeleteItem(hItem);
	}

	RemoveOrphanChannels();
	CheckForExistingPlaylist();
	set_allow_save();
	UpdateChannelsCount();
}

void CEdemChannelEditorDlg::OnUpdateRemoveCategory(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsSelectedCategory() && IsSelectedNotFavorite());
}

void CEdemChannelEditorDlg::OnSortCategory()
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

void CEdemChannelEditorDlg::OnUpdateSortCategory(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsSelectedCategory() && IsSelectedNotFavorite());
}

void CEdemChannelEditorDlg::OnStnClickedStaticIcon()
{
	auto hCur = m_wndChannelsTree.GetSelectedItem();
	if (!hCur)
		return;

	auto category = GetCategory(hCur);
	if (category != nullptr && category->get_id() == ID_ADD_TO_FAVORITE)
		return;

	bool isChannel = IsChannel(hCur);

	CFileDialog dlg(TRUE);
	CString path = theApp.GetAppPath(isChannel ? utils::CHANNELS_LOGO_PATH : utils::CATEGORIES_LOGO_PATH);
	CString file = theApp.GetAppPath(utils::PLUGIN_ROOT) + m_iconUrl;
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
			if (theApp.LoadImage(path, img))
			{
				theApp.SetImage(img, m_wndIcon);
			}
		}

		m_iconUrl = uri::PLUGIN_SCHEME;
		m_iconUrl += isChannel ? utils::CHANNELS_LOGO_URL : utils::CATEGORIES_LOGO_URL;
		m_iconUrl += oFN.lpstrFileTitle;

		UpdateData(FALSE);

		if (isChannel)
		{
			auto channel = GetChannel(hCur);
			if (channel && m_iconUrl != channel->get_icon_uri().get_uri().c_str())
			{
				channel->set_icon_uri(utils::utf16_to_utf8(m_iconUrl.GetString()));
				const auto& root = utils::utf16_to_utf8(theApp.GetAppPath(utils::PLUGIN_ROOT).GetString());
				const auto& path = utils::utf8_to_utf16(channel->get_icon_uri().get_icon_absolute_path(root));
				CImage img;
				if (theApp.LoadImage(path.c_str(), img))
				{
					channel->set_icon(img);
				}
			}
		}
		else
		{
			if (category && m_iconUrl != category->get_icon_uri().get_uri().c_str())
			{
				category->set_icon_uri(utils::utf16_to_utf8(m_iconUrl.GetString()));
				const auto& root = utils::utf16_to_utf8(theApp.GetAppPath(utils::PLUGIN_ROOT).GetString());
				const auto& path = utils::utf8_to_utf16(category->get_icon_uri().get_icon_absolute_path(root));
				CImage img;
				if (theApp.LoadImage(path.c_str(), img))
				{
					category->set_icon(img);
				}
			}
		}
		set_allow_save();
	}
}

void CEdemChannelEditorDlg::OnBnClickedButtonAbout()
{
	CAboutDlg dlg;
	dlg.DoModal();
}

void CEdemChannelEditorDlg::OnBnClickedButtonPack()
{
	if (is_allow_save() && AfxMessageBox(_T("You have unsaved changes.\nContinue?"), MB_YESNO) != IDYES)
		return;

#ifdef _DEBUG
	const auto& dllFile = theApp.GetAppPath(_T("..\\dll\\7za.dll"));
#else
	const auto& dllFile = theApp.GetAppPath(_T("7za.dll"));
#endif // _DEBUG

	const auto& plugin_folder = theApp.GetAppPath(utils::PLUGIN_ROOT);

	SevenZipWrapper archiver(dllFile.GetString());
	archiver.GetCompressor().SetCompressionFormat(CompressionFormat::Zip);
	bool res = archiver.GetCompressor().AddFiles(plugin_folder.GetString(), _T("*.*"), true);
	if (!res)
		return;

	res = archiver.CreateArchive(_T("dune_plugin_edem_mod.zip"));
	if (res)
	{
		AfxMessageBox(_T("Plugin created.\nInstall it to the DUNE mediaplayer"), MB_OK);
	}
	else
	{
		::DeleteFile(_T("dune_plugin_edem_mod.zip"));
	}
}

void CEdemChannelEditorDlg::OnUpdateButtonSearchNext(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_search.IsEmpty());
}

void CEdemChannelEditorDlg::OnBnClickedButtonSearchNext()
{
	if (m_search.IsEmpty())
		return;

	bool byId = false;
	int id = 0;
	if (m_search.GetLength() > 1 && m_search.GetAt(0) == '\\')
	{
		id = _tstoi(m_search.Mid(1).GetString());
		if (m_channelsMap.find(id) == m_channelsMap.end())
			return;

		byId = true;
	}

	SelectTreeItem(true, m_wndChannelsTree, m_search, byId, id);
}

void CEdemChannelEditorDlg::OnUpdateButtonPlSearchNext(CCmdUI* pCmdUI)
{
	UpdateData(TRUE);

	pCmdUI->Enable(!m_plSearch.IsEmpty());
}

void CEdemChannelEditorDlg::OnBnClickedButtonPlSearchNext()
{
	if (m_plSearch.IsEmpty())
		return;

	bool byId = false;
	int id = 0;
	if (m_plSearch.GetLength() > 1 && m_plSearch.GetAt(0) == '\\')
	{
		id = _tstoi(m_plSearch.Mid(1).GetString());
		if (m_playlistMap.find(id) == m_playlistMap.end())
			return;

		byId = true;
	}

	SelectTreeItem(false, m_wndPlaylistTree, m_plSearch, byId, id);
}

bool CEdemChannelEditorDlg::IsChannel(HTREEITEM hItem) const
{
	return (hItem != nullptr && m_wndChannelsTree.GetParentItem(hItem) != nullptr);
}

bool CEdemChannelEditorDlg::IsCategory(HTREEITEM hItem) const
{
	return (hItem != nullptr && m_wndChannelsTree.GetParentItem(hItem) == nullptr);
}

bool CEdemChannelEditorDlg::IsPlaylistEntry(HTREEITEM hItem) const
{
	return (hItem != nullptr && m_wndPlaylistTree.GetParentItem(hItem) != nullptr);
}

bool CEdemChannelEditorDlg::IsPlaylistCategory(HTREEITEM hItem) const
{
	return (hItem != nullptr && m_wndPlaylistTree.GetParentItem(hItem) == nullptr);
}

std::map<int, HTREEITEM> CEdemChannelEditorDlg::GetCategoriesTreeMap() const
{
	std::map<int, HTREEITEM> tree_categories;

	HTREEITEM hRoot = m_wndChannelsTree.GetRootItem();
	while (hRoot != nullptr)
	{
		int id = (int)m_wndChannelsTree.GetItemData(hRoot);
		tree_categories.emplace(id, hRoot);
		hRoot = m_wndChannelsTree.GetNextSiblingItem(hRoot);
	}

	return tree_categories;
}

void CEdemChannelEditorDlg::OnAddUpdateChannel()
{
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

void CEdemChannelEditorDlg::OnUpdateAddUpdateChannel(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsSelectedChannelsOrEntries());
}

void CEdemChannelEditorDlg::OnBnClickedButtonSettings()
{
	CSettingsDlg dlg;
	if (dlg.DoModal() == IDOK)
	{
		m_player = theApp.GetProfileString(_T("Setting"), _T("Player"));
		m_probe = theApp.GetProfileString(_T("Setting"), _T("FFProbe"));
		m_bAutoSync = theApp.GetProfileInt(_T("Setting"), _T("AutoSyncChannel"), FALSE);
	}
}

void CEdemChannelEditorDlg::OnBnClickedButtonAccessInfo()
{
	CAccessDlg dlg;
	dlg.m_bEmbedded = m_embedded_info;
	dlg.m_accessKey = GetAccessKey();
	dlg.m_domain = GetAccessDomain();

	if (dlg.DoModal() == IDOK)
	{
		m_embedded_info = dlg.m_bEmbedded;
		SetAccessKey(dlg.m_accessKey);
		SetDomain(dlg.m_domain);

		if (m_embedded_info)
		{
			set_allow_save();
		}
		else
		{
			theApp.WriteProfileString(_T("Setting"), _T("AccessKey"), dlg.m_accessKey);
			theApp.WriteProfileString(_T("Setting"), _T("Domain"), dlg.m_domain);
		}
	}
}

void CEdemChannelEditorDlg::OnUpdateIcon()
{
	auto entry = GetPlaylistEntry(m_wndPlaylistTree.GetSelectedItem());
	auto channel = GetChannel(m_wndChannelsTree.GetSelectedItem());

	if (entry && channel && channel->get_icon_uri() != entry->get_icon_uri())
	{
		channel->set_icon_uri(entry->get_icon_uri());
		channel->copy_icon(entry->get_icon());
		theApp.SetImage(channel->get_icon(), m_wndIcon);
		OnSyncTreeItem();
		set_allow_save();
	}
}

void CEdemChannelEditorDlg::OnUpdateUpdateIcon(CCmdUI* pCmdUI)
{
	BOOL enable = FALSE;
	if (m_wndPlaylistTree.GetSelectedCount() == 1)
	{
		auto entry = GetPlaylistEntry(m_wndPlaylistTree.GetSelectedItem());
		auto channel = GetChannel(m_wndChannelsTree.GetSelectedItem());
		if (channel && entry)
		{
			enable = channel->get_id() == entry->get_id() && channel->get_icon_uri() != entry->get_icon_uri();
		}
	}

	pCmdUI->Enable(enable);
}

void CEdemChannelEditorDlg::OnBnClickedButtonCacheIcon()
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

		uri icon_uri;
		icon_uri.set_uri(utils::ICON_TEMPLATE);
		icon_uri.set_path(path);

		std::vector<BYTE> image;
		if (!utils::DownloadFile(channel->get_icon_uri().get_uri(), image)) continue;

		const auto& root = utils::utf16_to_utf8(theApp.GetAppPath(utils::PLUGIN_ROOT).GetString());
		const auto& fullPath = utils::utf8_to_utf16(icon_uri.get_icon_absolute_path(root));
		std::ofstream os(fullPath.c_str(), std::ios::out | std::ios::binary);
		os.write((char*)&image[0], image.size());
		os.close();

		CImage img;
		if (theApp.LoadImage(fullPath.c_str(), img))
		{
			channel->set_icon_uri(icon_uri.get_uri());
			channel->set_icon(img);
		}

		LoadChannelInfo(hItem);
		set_allow_save();
	}
}

void CEdemChannelEditorDlg::OnUpdateButtonCacheIcon(CCmdUI* pCmdUI)
{
	BOOL enable = FALSE;
	auto channel = GetChannel(m_wndChannelsTree.GetSelectedItem());
	pCmdUI->Enable(channel && !channel->is_icon_local());
}

void CEdemChannelEditorDlg::OnTvnSelchangedTreePaylist(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	*pResult = 0;

	BOOL enable = FALSE;
	if (m_wndPlaylistTree.GetSelectedCount() == 1)
	{
		auto entry = GetPlaylistEntry(m_wndPlaylistTree.GetSelectedItem());
		auto channel = GetChannel(m_wndChannelsTree.GetSelectedItem());
		if (channel && entry)
		{
			enable = channel->get_icon_uri() != entry->get_icon_uri();
		}
	}

	LoadPlayListInfo(pNMTreeView->itemNew.hItem);

	m_wndUpdateIcon.EnableWindow(enable);
}

void CEdemChannelEditorDlg::OnBnClickedButtonAddNewChannelsList()
{
	CFileDialog dlg(FALSE);

	CString filter(_T("Channels xml(*.xml)|*.xml||"));
	filter.Replace('|', '\0');

	OPENFILENAME& oFN = dlg.GetOFN();
	oFN.lpstrFilter = filter.GetString();
	oFN.nMaxFile = MAX_PATH;
	oFN.nFilterIndex = 0;
	oFN.lpstrTitle = _T("Add new Edem TV channels list");
	oFN.Flags |= OFN_EXPLORER | OFN_ENABLESIZING | OFN_LONGNAMES | OFN_OVERWRITEPROMPT | OFN_NONETWORKBUTTON;

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
										  return item.first == dlg.GetFileName();
									  });

			if (found == m_all_channels_lists.end())
			{
				m_channelsMap.clear();
				m_categoriesMap.clear();
				const auto& pair = m_all_channels_lists.emplace_back(dlg.GetFileName(), dlg.GetPathName());
				m_wndChannels.EnableWindow(TRUE);
				int idx = m_wndChannels.AddString(pair.first);
				m_wndChannels.SetItemData(idx, (DWORD_PTR)pair.second.GetString());
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

void CEdemChannelEditorDlg::OnGetStreamInfo()
{
	m_wndProgress.ShowWindow(SW_SHOW);
	CWaitCursor cur;

	if (m_lastTree == m_wndChannelsTree.GetSafeHwnd())
	{
		auto selected = m_wndChannelsTree.GetSelectedItems();
		std::vector<ChannelInfo*> channels;
		for (const auto& hItem : selected)
		{
			auto channel = GetChannel(hItem);
			if (channel)
				channels.emplace_back(channel);
		}

		GetStreamInfo(channels, m_wndChInfo);
		LoadChannelInfo(m_wndChannelsTree.GetFirstSelectedItem());
		UpdateChannelsCount();
	}
	else if (m_lastTree == m_wndPlaylistTree.GetSafeHwnd())
	{
		auto selected = m_wndPlaylistTree.GetSelectedItems();
		std::vector<PlaylistEntry*> playlist;
		for (const auto& hItem : selected)
		{
			auto entry = GetPlaylistEntry(hItem);
			if (entry)
				playlist.emplace_back(entry);
		}

		GetStreamInfo(playlist, m_wndPlInfo);
		LoadPlayListInfo(m_wndPlaylistTree.GetSelectedItem());
		UpdatePlaylistCount();
	}

	m_wndProgress.ShowWindow(SW_HIDE);

	UpdateData(FALSE);
}

void CEdemChannelEditorDlg::OnUpdateGetStreamInfo(CCmdUI* pCmdUI)
{
	BOOL enable = !m_probe.IsEmpty();
	if (m_lastTree)
	{
		if (m_lastTree == m_wndChannelsTree.GetSafeHwnd())
			enable &= GetChannel(m_wndChannelsTree.GetFirstSelectedItem()) != nullptr;
		else
			enable &= GetPlaylistEntry(m_wndPlaylistTree.GetFirstSelectedItem()) != nullptr;
	}
	else
	{
		enable = FALSE;
	}

	enable = enable && IsSelectedTheSameType() && !m_loading;

	pCmdUI->Enable(enable);
}

void CEdemChannelEditorDlg::OnGetStreamInfoAll()
{
	CWaitCursor cur;
	m_wndProgress.ShowWindow(SW_SHOW);
	StreamContainer* container = nullptr;
	HWND hFocus = ::GetFocus();
	if (hFocus == m_wndChannelsTree.GetSafeHwnd())
	{
		GetStreamInfo(GetItemCategory(m_wndChannelsTree.GetSelectedItem())->get_channels(), m_wndChInfo);
		LoadChannelInfo(m_wndChannelsTree.GetFirstSelectedItem());
		UpdateChannelsCount();
	}
	else if (hFocus == m_wndPlaylistTree.GetSafeHwnd())
	{
		std::vector<PlaylistEntry*> playlist;

		auto curEntry = GetPlaylistEntry(m_wndPlaylistTree.GetSelectedItem());
		for (const auto& pair : m_playlistMap)
		{
			if (curEntry == nullptr || curEntry->get_category() == pair.second->get_category())
			{
				// add all
				playlist.emplace_back(pair.second.get());
			}
		}

		GetStreamInfo(playlist, m_wndPlInfo);
		LoadPlayListInfo(m_wndPlaylistTree.GetSelectedItem());
		UpdatePlaylistCount();
	}

	m_wndProgress.ShowWindow(SW_HIDE);
}

void CEdemChannelEditorDlg::OnUpdateGetStreamInfoAll(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_lastTree != nullptr && !m_probe.IsEmpty());
}

void CEdemChannelEditorDlg::OnPlayChannelStream()
{
	PlayChannel(m_wndChannelsTree.GetSelectedItem());
}

void CEdemChannelEditorDlg::OnUpdatePlayChannelStream(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_probe.IsEmpty() && m_wndChannelsTree.GetSelectedCount() == 1 && GetChannel(m_wndChannelsTree.GetSelectedItem()) != nullptr);
}

void CEdemChannelEditorDlg::OnPlayPlaylistStream()
{
	PlayPlaylistEntry(m_wndPlaylistTree.GetSelectedItem());
}

void CEdemChannelEditorDlg::OnUpdatePlayPlaylistStream(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_probe.IsEmpty() && m_wndPlaylistTree.GetSelectedCount() == 1 && GetPlaylistEntry(m_wndPlaylistTree.GetSelectedItem()) != nullptr);
}

void CEdemChannelEditorDlg::OnPlayChannelStreamArchive()
{
	UpdateData(TRUE);
	if (m_lastTree == m_wndChannelsTree.GetSafeHwnd())
	{
		PlayChannel(m_wndChannelsTree.GetSelectedItem(), m_archiveCheck);
	}
	else if (m_lastTree == m_wndPlaylistTree.GetSafeHwnd())
	{
		PlayPlaylistEntry(m_wndPlaylistTree.GetSelectedItem(), m_archiveCheck);
	}
}

void CEdemChannelEditorDlg::OnSyncTreeItem()
{
	if (m_loading)
		return;

	CWaitCursor cur;
	if (m_lastTree == m_wndPlaylistTree.GetSafeHwnd())
	{
		auto entry = GetPlaylistEntry(m_wndPlaylistTree.GetSelectedItem());
		if (!entry)
			return;

		if (m_channelsMap.find(entry->get_id()) != m_channelsMap.end())
		{
			SelectTreeItem(true, m_wndChannelsTree, _T(""), true, entry->get_id());
		}
	}
	else if (m_lastTree == m_wndChannelsTree.GetSafeHwnd())
	{
		auto channel = GetChannel(m_wndChannelsTree.GetSelectedItem());
		if (!channel)
			return;

		if (m_playlistMap.find(channel->get_id()) != m_playlistMap.end())
		{
			SelectTreeItem(false, m_wndPlaylistTree, _T(""), true, channel->get_id());
		}
	}
}

void CEdemChannelEditorDlg::OnUpdateSyncTreeItem(CCmdUI* pCmdUI)
{
	HWND hFocus = ::GetFocus();
	BOOL enable = FALSE;
	if (hFocus == m_wndChannelsTree.GetSafeHwnd())
		enable = (m_wndChannelsTree.GetSelectedCount() == 1 && GetChannel(m_wndChannelsTree.GetSelectedItem()) != nullptr);
	else if (hFocus == m_wndPlaylistTree.GetSafeHwnd())
		enable = (!m_bAutoSync && m_wndPlaylistTree.GetSelectedCount() == 1 && GetPlaylistEntry(m_wndPlaylistTree.GetSelectedItem()) != nullptr);

	pCmdUI->Enable(enable && !m_loading);
}

void CEdemChannelEditorDlg::OnToggleChannel()
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

void CEdemChannelEditorDlg::OnUpdateToggleChannel(CCmdUI* pCmdUI)
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

void CEdemChannelEditorDlg::OnBnClickedButtonDownloadPlaylist()
{
	LoadPlaylist(true);
}

void CEdemChannelEditorDlg::OnCbnSelchangeComboPlaylist()
{
	CString url;
	int idx = m_wndPlaylist.GetCurSel();
	BOOL enableDownload = TRUE;
	BOOL enableCustom = FALSE;
	switch (idx)
	{
		case 0:
			url = _T("http://epg.it999.ru/edem_epg_ico.m3u8");
			break;
		case 1:
			url = _T("http://epg.it999.ru/edem_epg_ico2.m3u8");
			break;
		case 2:
			url = theApp.GetProfileString(_T("Setting"), _T("CustomPlaylist"));
			enableCustom = TRUE;
			break;
		case 3:
			url = theApp.GetProfileString(_T("Setting"), _T("CustomPlaylist"));
			enableDownload = FALSE;
			enableCustom = TRUE;
			break;
		default:
			break;
	}

	m_wndDownloadUrl.EnableWindow(enableDownload);
	m_wndChooseUrl.EnableWindow(enableCustom);
	theApp.WriteProfileString(_T("Setting"), _T("Playlist"), url);
	theApp.WriteProfileInt(_T("Setting"), _T("PlaylistType"), idx);

	LoadPlaylist();
}

HTREEITEM CEdemChannelEditorDlg::FindTreeItem(CTreeCtrl& ctl, DWORD_PTR entry)
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

void CEdemChannelEditorDlg::SelectTreeItem(bool inChannelsList, CTreeCtrl& ctl, const CString& searchString, bool byId, int id)
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
			DWORD_PTR entry = 0;
			if (ctl.GetParentItem(hItem) != nullptr)
				entry = ctl.GetItemData(hItem);

			if (entry)
			{
				int entry_id = inChannelsList ? ((ChannelInfo*)entry)->get_id() : ((PlaylistEntry*)entry)->get_id();
				const auto& entry_title = inChannelsList ? ((ChannelInfo*)entry)->get_title() : ((PlaylistEntry*)entry)->get_title();

				if (byId)
				{
					if (entry_id == id)
					{
						hFound = hItem;
						break;
					}
				}
				else
				{
					if (StrStrI(entry_title.c_str(), searchString.GetString()) != nullptr)
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

HTREEITEM CEdemChannelEditorDlg::FindTreeNextItem(CTreeCtrl& ctl, HTREEITEM hItem, DWORD_PTR entry)
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

HTREEITEM CEdemChannelEditorDlg::FindTreeSubItem(CTreeCtrl& ctl, HTREEITEM hItem, DWORD_PTR entry)
{
	while (hItem)
	{
		if (entry == ctl.GetItemData(hItem)) break;

		// get the next sibling item
		hItem = ctl.GetNextSiblingItem(hItem);
	}

	return hItem;
}

bool CEdemChannelEditorDlg::AddChannel(HTREEITEM hSelectedItem, int categoryId /*= -1*/)
{
	bool needCheckExisting = false;

	auto entry = GetPlaylistEntry(hSelectedItem);
	if (!entry)
		return needCheckExisting;

	auto tree_categories = GetCategoriesTreeMap();
	// is add to category?
	if (categoryId == -1)
		categoryId = GetCategoryIdByName(entry->get_category());

	std::shared_ptr<ChannelCategory> category;
	if (categoryId != -1)
	{
		category = m_categoriesMap.at(categoryId);
	}
	else
	{
		// Category not exist, create new
		category = std::make_shared<ChannelCategory>();
		categoryId = GetNewCategoryID();
		category->set_id(categoryId);
		category->set_caption(entry->get_category());

		TVINSERTSTRUCTW tvInsert = { nullptr };
		tvInsert.hParent = TVI_ROOT;
		tvInsert.item.pszText = (LPWSTR)category->get_caption().c_str();
		tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
		tvInsert.item.lParam = categoryId;

		tree_categories.emplace(categoryId, m_wndChannelsTree.InsertItem(&tvInsert));
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
		auto newChannel = std::make_unique<ChannelInfo>();
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
		TVINSERTSTRUCTW tvInsert = { nullptr };
		tvInsert.hParent = tree_categories[categoryId];
		tvInsert.item.pszText = (LPWSTR)channel->get_title().c_str();
		tvInsert.item.lParam = (LPARAM)channel.get();
		tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
		hFoundItem = m_wndChannelsTree.InsertItem(&tvInsert);

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
	if (auto id = entry->get_tvg_id(); id != -1)
		channel->set_epg_id(id);

	channel->set_archive(entry->get_archive());
	channel->set_stream_uri(entry->get_stream_uri());
	if (entry->get_icon_uri() != channel->get_icon_uri() && !channel->get_icon_uri().get_uri().empty())
	{
		channel->set_icon_uri(entry->get_icon_uri());
		channel->copy_icon(entry->get_icon());
	}

	if (entry->get_category().find(L"зрослые") != std::wstring::npos)
	{
		// Channel for adult
		channel->set_adult(TRUE);
	}

	return needCheckExisting;
}

std::string CEdemChannelEditorDlg::TranslateStreamUri(const std::string& stream_uri)
{
	// http://ts://{SUBDOMAIN}/iptv/{UID}/205/index.m3u8 -> http://ts://domain.com/iptv/000000000000/205/index.m3u8

	static std::regex re_domain(R"(\{SUBDOMAIN\})");
	static std::regex re_uid(R"(\{UID\})");

	std::string stream_url(stream_uri);
	stream_url = std::regex_replace(stream_url, re_domain, utils::utf16_to_utf8(CEdemChannelEditorDlg::GetAccessDomain().GetString()));
	return std::regex_replace(stream_url, re_uid, utils::utf16_to_utf8(CEdemChannelEditorDlg::GetAccessKey().GetString()));
}

void CEdemChannelEditorDlg::GetChannelStreamInfo(const std::string& url, std::string& audio, std::string& video)
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
	csCommand.Format(_T("\"%s\" -hide_banner -show_streams %hs"), CEdemChannelEditorDlg::m_probe.GetString(), TranslateStreamUri(url).c_str());

	BOOL bRunProcess = CreateProcess(CEdemChannelEditorDlg::m_probe.GetString(),	// 	__in_opt     LPCTSTR lpApplicationName
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

		long nTimeout = 20;

		int nErrorCount = 0;
		DWORD dwExitCode = STILL_ACTIVE;
		DWORD dwStart = GetTickCount();
		BOOL bTimeout = FALSE;
		for (;;)
		{
			if (dwExitCode != STILL_ACTIVE)
			{
				TRACE(_T("Success! Exit code: %u\n"), dwExitCode);
				break;
			}

			if (CheckForTimeOut(dwStart, nTimeout * 1000))
			{
				bTimeout = TRUE;
				::TerminateProcess(pi.hProcess, 0);
				TRACE(_T("Failed! Execution Timeout\n"));
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
				TRACE(_T("GetExitCodeProcess failed. ErrorCode: %0u, try count: %0d\n"), ::GetLastError(), nErrorCount);
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

void CEdemChannelEditorDlg::OnCbnSelchangeComboChannels()
{
	if (is_allow_save() && AfxMessageBox(_T("Changes not saved. Are you sure?"), MB_YESNO | MB_ICONWARNING) != IDYES)
	{
		m_wndChannels.SetCurSel(theApp.GetProfileInt(_T("Setting"), _T("ChannelsType"), 0));
		return;
	}

	int idx = m_wndChannels.GetCurSel();
	bool changed = false;
	if (LoadChannels((LPCTSTR)m_wndChannels.GetItemData(idx), changed))
	{
		FillTreeChannels();
		set_allow_save(changed != false);
	}

	GetDlgItem(IDC_BUTTON_ADD_NEW_CHANNELS_LIST)->EnableWindow(idx > 0);
	theApp.WriteProfileInt(_T("Setting"), _T("ChannelsType"), idx);
}

void CEdemChannelEditorDlg::OnBnClickedButtonPlFilter()
{
	CFilterDialog dlg;
	if (dlg.DoModal() == IDOK)
	{
		LoadPlaylist();
	}
}

bool CEdemChannelEditorDlg::IsSelectedTheSameType() const
{
	if (m_lastTree == m_wndChannelsTree.GetSafeHwnd())
	{
		auto selected = m_wndChannelsTree.GetSelectedItems();
		if (selected.empty())
			return false;

		bool isChannel = GetChannel(selected[0]) != nullptr;
		for (const auto& hItem : selected)
		{
			auto channel = GetChannel(hItem);
			if (isChannel != (channel != nullptr))
				return false;
		}
	}
	else if (m_lastTree == m_wndPlaylistTree.GetSafeHwnd())
	{
		auto selected = m_wndPlaylistTree.GetSelectedItems();
		if (selected.empty())
			return false;

		bool isEntry = GetPlaylistEntry(selected[0]) != nullptr;
		for (const auto& hItem : selected)
		{
			auto entry = GetPlaylistEntry(hItem);
			if (isEntry != (entry != nullptr))
				return false;
		}
	}

	return true;
}

bool CEdemChannelEditorDlg::IsSelectedChannelsOrEntries(bool onlyChannel /*= false*/) const
{
	if (m_lastTree == m_wndChannelsTree.GetSafeHwnd())
	{
		auto selected = m_wndChannelsTree.GetSelectedItems();
		if (selected.empty())
			return false;

		for (const auto& hItem : selected)
		{
			auto channel = GetChannel(hItem);
			if (!channel)
				return false;
		}

		return true;
	}

	if (m_lastTree == m_wndPlaylistTree.GetSafeHwnd() && !onlyChannel)
	{
		auto selected = m_wndPlaylistTree.GetSelectedItems();
		if (selected.empty())
			return false;

		for (const auto& hItem : selected)
		{
			auto entry = GetPlaylistEntry(hItem);
			if (!entry)
				return false;
		}

		return true;
	}

	return false;
}

bool CEdemChannelEditorDlg::IsSelectedCategory() const
{
	return (m_lastTree == m_wndChannelsTree.GetSafeHwnd() && GetCategory(m_wndChannelsTree.GetFirstSelectedItem()) != nullptr && IsSelectedTheSameType());
}

bool CEdemChannelEditorDlg::IsSelectedNotFavorite() const
{
	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		auto category = GetItemCategory(hItem);
		if (!category || category->get_id() != ID_ADD_TO_FAVORITE) continue;
		return false;
	}

	return true;
}

bool CEdemChannelEditorDlg::IsSelectedTheSameCategory() const
{
	if (m_lastTree != m_wndChannelsTree.GetSafeHwnd() || !m_wndChannelsTree.GetSelectedCount() || !IsSelectedTheSameType())
		return false;

	auto category = GetCategory(m_wndChannelsTree.GetFirstSelectedItem());
	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		if (GetCategory(hItem) != category)
			return false;
	}

	return true;
}

bool CEdemChannelEditorDlg::IsChannelSelectionConsistent() const
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

void CEdemChannelEditorDlg::OnNMSetfocusTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	m_lastTree = pNMHDR->hwndFrom;
	*pResult = 0;
}

void CEdemChannelEditorDlg::OnTvnChannelsGetInfoTip(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMTVGETINFOTIP>(pNMHDR);

	auto channel = GetChannel(pGetInfoTip->hItem);
	if (channel)
	{
		auto ch_id = channel->get_id();
		CString categories;
		for (const auto& pair : m_categoriesMap)
		{
			if (pair.second->find_channel(ch_id))
			{
				if (!categories.IsEmpty())
					categories += _T(", ");
				categories.Append(pair.second->get_caption().c_str());
			}
		}

		m_toolTipText.Format(_T("Name: %s\nID: %s\nArchive: %s\nAdult: %s\nIn categories: %s"),
							 channel->get_title().c_str(),
							 channel->get_stream_uri().is_template() ? utils::int_to_wchar(ch_id).c_str() : _T("Custom"),
							 channel->get_archive() ? _T("Yes") : _T("No"),
							 channel->get_adult() ? _T("Yes") : _T("No"),
							 categories.GetString());


		pGetInfoTip->pszText = m_toolTipText.GetBuffer();
	}

	*pResult = 0;
}

void CEdemChannelEditorDlg::OnTvnPlaylistGetInfoTip(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMTVGETINFOTIP>(pNMHDR);

	auto entry = GetPlaylistEntry(pGetInfoTip->hItem);
	if (entry)
	{
		m_toolTipText.Format(_T("Name: %s\nID: %d\nEPG: %d\nArchive: %s"),
							 entry->get_title().c_str(),
							 entry->get_id(),
							 entry->get_tvg_id(),
							 entry->get_archive() ? _T("Yes") : _T("No"));

		pGetInfoTip->pszText = m_toolTipText.GetBuffer();
	}

	*pResult = 0;
}

void CEdemChannelEditorDlg::RestoreWindowPos()
{
	WINDOWPLACEMENT wp = { 0 };
	UINT nSize = 0;
	WINDOWPLACEMENT* pwp = nullptr;
	if (!AfxGetApp()->GetProfileBinary(_T("Setting"), _T("WindowPos"), (LPBYTE*)&pwp, &nSize))
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

BOOL CEdemChannelEditorDlg::DestroyWindow()
{
	// Get the window position
	WINDOWPLACEMENT wp;
	wp.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(&wp);
	// Save the info
	CWinApp* pApp = AfxGetApp();
	pApp->WriteProfileBinary(_T("Setting"), _T("WindowPos"), (LPBYTE)&wp, sizeof(wp));

	return __super::DestroyWindow();
}
