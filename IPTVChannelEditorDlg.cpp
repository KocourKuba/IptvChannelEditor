
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
#include "AccessOttKeyDlg.h"
#include "AccessInfoPassDlg.h"
#include "AccessInfoPinDlg.h"
#include "FilterDialog.h"
#include "CustomPlaylistDlg.h"
#include "PlaylistParseThread.h"
#include "IconCache.h"
#include "IconsListDlg.h"
#include "utils.h"
#include "uri_antifriz.h"

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

// Common
constexpr auto REG_SETTINGS = _T("Settings");
constexpr auto REG_PLAYER = _T("Player");
constexpr auto REG_FFPROBE = _T("FFProbe");
constexpr auto REG_DAYS_BACK = _T("DaysBack");
constexpr auto REG_HOURS_BACK = _T("HoursBack");
constexpr auto REG_AUTOSYNC = _T("AutoSyncChannel");
constexpr auto REG_PLUGIN = _T("PluginType");
constexpr auto REG_ICON_SOURCE = _T("IconSource");

// Plugin dependent
constexpr auto REG_LOGIN = _T("Login");
constexpr auto REG_LOGIN_EMBEDDED = _T("LoginEmbedded");
constexpr auto REG_PASSWORD = _T("Password");
constexpr auto REG_PASSWORD_EMBEDDED = _T("PasswordEmbedded");
constexpr auto REG_TOKEN = _T("AccessKey");
constexpr auto REG_TOKEN_EMBEDDED = _T("AccessKeyEmbedded");
constexpr auto REG_DOMAIN = _T("Domain");
constexpr auto REG_DOMAIN_EMBEDDED = _T("DomainEmbedded");
constexpr auto REG_ACCESS_URL = _T("AccessUrl");
constexpr auto REG_INT_ID = _T("IntId");
constexpr auto REG_INT_ID_EMBEDDED = _T("IntIdEmbedded");
constexpr auto REG_HOST = _T("Host");
constexpr auto REG_HOST_EMBEDDED = _T("HostEmbedded");
constexpr auto REG_ACCESS_EMBED = _T("Embed");
constexpr auto REG_FILTER_STRING = _T("FilterString");
constexpr auto REG_FILTER_REGEX = _T("FilterUseRegex");
constexpr auto REG_FILTER_CASE = _T("FilterUseCase");
constexpr auto REG_CHANNELS_TYPE = _T("ChannelsType");
constexpr auto REG_PLAYLIST_TYPE = _T("PlaylistType");
constexpr auto REG_STREAM_TYPE = _T("StreamType");
constexpr auto REG_CUSTOM_URL = _T("CustomUrl");
constexpr auto REG_CUSTOM_FILE = _T("CustomPlaylist");

CString CIPTVChannelEditorDlg::m_probe;

// ���������� ������� ����� �������� � ������� ��������� ������� � �����
inline DWORD GetTimeDiff(DWORD dwStartTime)
{
	DWORD dwCurrent = ::GetTickCount();

	if (dwStartTime > dwCurrent)
		return (0xffffffff - dwCurrent - dwStartTime);

	return (dwCurrent - dwStartTime);
}

// ������� ��������� ����� � ������� � �������������
// ���������� TRUE, ���� ������� �����
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

	ON_BN_CLICKED(IDC_BUTTON_ABOUT, &CIPTVChannelEditorDlg::OnBnClickedButtonAbout)
	ON_BN_CLICKED(IDC_BUTTON_CHOOSE_PLAYLIST, &CIPTVChannelEditorDlg::OnBnClickedButtonCustomPlaylist)
	ON_BN_CLICKED(IDC_BUTTON_PL_SEARCH_NEXT, &CIPTVChannelEditorDlg::OnBnClickedButtonPlSearchNext)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_PL_SEARCH_NEXT, &CIPTVChannelEditorDlg::OnUpdateButtonPlSearchNext)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH_NEXT, &CIPTVChannelEditorDlg::OnBnClickedButtonSearchNext)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_SEARCH_NEXT, &CIPTVChannelEditorDlg::OnUpdateButtonSearchNext)
	ON_BN_CLICKED(IDC_BUTTON_PL_FILTER, &CIPTVChannelEditorDlg::OnBnClickedButtonPlFilter)
	ON_BN_CLICKED(IDC_BUTTON_ADD_NEW_CHANNELS_LIST, &CIPTVChannelEditorDlg::OnBnClickedButtonAddNewChannelsList)
	ON_BN_CLICKED(IDC_BUTTON_DOWNLOAD_PLAYLIST, &CIPTVChannelEditorDlg::OnBnClickedButtonDownloadPlaylist)
	ON_BN_CLICKED(IDC_BUTTON_TEST_EPG2, &CIPTVChannelEditorDlg::OnBnClickedButtonTestEpg2)
	ON_BN_CLICKED(IDC_BUTTON_TEST_EPG1, &CIPTVChannelEditorDlg::OnBnClickedButtonTestEpg1)
	ON_BN_CLICKED(IDC_CHECK_ADULT, &CIPTVChannelEditorDlg::OnBnClickedCheckAdult)
	ON_BN_CLICKED(IDC_CHECK_ARCHIVE, &CIPTVChannelEditorDlg::OnBnClickedCheckArchive)
	ON_BN_CLICKED(IDC_CHECK_CUSTOMIZE, &CIPTVChannelEditorDlg::OnBnClickedCheckCustomize)
	ON_BN_CLICKED(IDC_BUTTON_CHECK_ARCHIVE, &CIPTVChannelEditorDlg::OnBnClickCheckArchive)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CIPTVChannelEditorDlg::OnSave)
	ON_BN_CLICKED(IDC_BUTTON_PACK, &CIPTVChannelEditorDlg::OnBnClickedButtonPack)
	ON_BN_CLICKED(IDC_BUTTON_SETTINGS, &CIPTVChannelEditorDlg::OnBnClickedButtonSettings)
	ON_BN_CLICKED(IDC_BUTTON_CACHE_ICON, &CIPTVChannelEditorDlg::OnBnClickedButtonCacheIcon)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_CACHE_ICON, &CIPTVChannelEditorDlg::OnUpdateButtonCacheIcon)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE_ICON, &CIPTVChannelEditorDlg::OnUpdateIcon)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_UPDATE_ICON, &CIPTVChannelEditorDlg::OnUpdateUpdateIcon)

	ON_EN_CHANGE(IDC_EDIT_EPG1_ID, &CIPTVChannelEditorDlg::OnEnChangeEditEpg1ID)
	ON_EN_CHANGE(IDC_EDIT_EPG2_ID, &CIPTVChannelEditorDlg::OnEnChangeEditEpg2ID)
	ON_EN_CHANGE(IDC_EDIT_TIME_SHIFT, &CIPTVChannelEditorDlg::OnEnChangeEditTimeShiftHours)
	ON_EN_CHANGE(IDC_EDIT_ARCHIVE_CHECK_DAYS, &CIPTVChannelEditorDlg::OnEnChangeEditArchiveCheckDays)
	ON_EN_CHANGE(IDC_EDIT_ARCHIVE_CHECK_HOURS, &CIPTVChannelEditorDlg::OnEnChangeEditArchiveCheckHours)
	ON_EN_CHANGE(IDC_EDIT_STREAM_URL, &CIPTVChannelEditorDlg::OnEnChangeEditStreamUrl)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_TIME_SHIFT, &CIPTVChannelEditorDlg::OnDeltaposSpinTimeShiftHours)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_ARCHIVE_CHECK_DAYS, &CIPTVChannelEditorDlg::OnDeltaposSpinArchiveCheckDay)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_ARCHIVE_CHECK_HOURS, &CIPTVChannelEditorDlg::OnDeltaposSpinArchiveCheckHour)
	ON_EN_CHANGE(IDC_EDIT_URL_ID, &CIPTVChannelEditorDlg::OnEnChangeEditUrlID)
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
	DDX_Check(pDX, IDC_CHECK_ARCHIVE, m_isArchive);
	DDX_Control(pDX, IDC_CHECK_ARCHIVE, m_wndArchive);
	DDX_Check(pDX, IDC_CHECK_ADULT, m_isAdult);
	DDX_Control(pDX, IDC_CHECK_ADULT, m_wndAdult);
	DDX_Control(pDX, IDC_TREE_CHANNELS, m_wndChannelsTree);
	DDX_Control(pDX, IDC_CHECK_CUSTOMIZE, m_wndCustom);
	DDX_Text(pDX, IDC_EDIT_URL_ID, m_streamID);
	DDX_Control(pDX, IDC_EDIT_URL_ID, m_wndStreamID);
	DDX_Text(pDX, IDC_EDIT_EPG2_ID, m_epgID2);
	DDX_Control(pDX, IDC_EDIT_EPG2_ID, m_wndEpg2ID);
	DDX_Control(pDX, IDC_BUTTON_TEST_EPG2, m_wndTestEPG2);
	DDX_Text(pDX, IDC_EDIT_EPG1_ID, m_epgID1);
	DDX_Control(pDX, IDC_EDIT_EPG1_ID, m_wndEpgID1);
	DDX_Control(pDX, IDC_BUTTON_TEST_EPG1, m_wndTestEPG1);
	DDX_Control(pDX, IDC_EDIT_STREAM_URL, m_wndStreamUrl);
	DDX_Text(pDX, IDC_EDIT_STREAM_URL, m_streamUrl);
	DDX_Text(pDX, IDC_EDIT_TIME_SHIFT, m_timeShiftHours);
	DDX_Control(pDX, IDC_EDIT_TIME_SHIFT, m_wndTimeShift);
	DDX_Text(pDX, IDC_EDIT_ARCHIVE_CHECK_DAYS, m_archiveCheckDays);
	DDX_Control(pDX, IDC_EDIT_ARCHIVE_CHECK_DAYS, m_wndArchiveCheckDays);
	DDX_Text(pDX, IDC_EDIT_ARCHIVE_CHECK_HOURS, m_archiveCheckHours);
	DDX_Control(pDX, IDC_EDIT_ARCHIVE_CHECK_HOURS, m_wndArchiveCheckHours);
	DDX_Control(pDX, IDC_STATIC_ICON, m_wndChannelIcon);
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
	DDX_Text(pDX, IDC_STATIC_PL_EPG, m_plEPG);
	DDX_Control(pDX, IDC_CHECK_PL_ARCHIVE, m_wndPlArchive);
	DDX_Text(pDX, IDC_EDIT_INFO_VIDEO, m_infoVideo);
	DDX_Control(pDX, IDC_EDIT_INFO_VIDEO, m_wndInfoVideo);
	DDX_Text(pDX, IDC_EDIT_INFO_AUDIO, m_infoAudio);
	DDX_Text(pDX, IDC_EDIT_PL_ARCHIVE_DAYS, m_archivePlDays);
	DDX_Control(pDX, IDC_EDIT_INFO_AUDIO, m_wndInfoAudio);
	DDX_Control(pDX, IDC_STATIC_CHANNELS, m_wndChInfo);
	DDX_Control(pDX, IDC_BUTTON_CHECK_ARCHIVE, m_wndCheckArchive);
	DDX_Control(pDX, IDC_COMBO_STREAM_TYPE, m_wndStreamType);
	DDX_CBIndex(pDX, IDC_COMBO_STREAM_TYPE, m_StreamType);
	DDX_Control(pDX, IDC_COMBO_PLAYLIST, m_wndPlaylist);
	DDX_Control(pDX, IDC_COMBO_CHANNELS, m_wndChannels);
	DDX_Control(pDX, IDC_BUTTON_CHOOSE_PLAYLIST, m_wndChooseUrl);
	DDX_Control(pDX, IDC_BUTTON_DOWNLOAD_PLAYLIST, m_wndDownloadUrl);
	DDX_Control(pDX, IDC_PROGRESS_LOAD, m_wndProgress);
	DDX_Control(pDX, IDC_BUTTON_CACHE_ICON, m_wndCacheIcon);
	DDX_Control(pDX, IDC_BUTTON_UPDATE_ICON, m_wndUpdateIcon);
	DDX_Control(pDX, IDC_BUTTON_SAVE, m_wndSave);
	DDX_Control(pDX, IDC_SPIN_TIME_SHIFT, m_wndSpinTimeShift);
	DDX_Control(pDX, IDC_STATIC_PROGRESS_INFO, m_wndProgressInfo);
	DDX_Control(pDX, IDC_COMBO_ICON_SOURCE, m_wndIconSource);
}

// CEdemChannelEditorDlg message handlers

BOOL CIPTVChannelEditorDlg::OnInitDialog()
{
	__super::OnInitDialog();

	theApp.RestoreWindowPos(GetSafeHwnd(), _T("WindowPos"));

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

	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_COMBO_PLUGIN_TYPE), _T("Plugin type"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_COMBO_CHANNELS), _T("Choose channel list to edit"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_COMBO_PLAYLIST), _T("Choose a playlist to import. Standard and Thematic downloaded from it999.ru"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_BUTTON_ADD_NEW_CHANNELS_LIST), _T("Add custom playlist"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_EDIT_SEARCH), _T("Search in channels. Use \\ prefix to find by ID"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_BUTTON_SEARCH_NEXT), _T("Search next"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_EDIT_EPG2_ID), _T("EPG ID from teleguide.info"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_BUTTON_TEST_EPG2), _T("Test EPG teleguide.info URL"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_EDIT_EPG1_ID), _T("EPG ID from it999.ru"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_BUTTON_TEST_EPG1), _T("Test EPG it999.ru URL"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_CHECK_CUSTOMIZE), _T("Use custom stream URL for the channel"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_CHECK_ARCHIVE), _T("Channel archive is supported"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_CHECK_ADULT), _T("Channel contents for adults"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_BUTTON_CACHE_ICON), _T("Store icon to the local folder instead of downloading it from internet"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_BUTTON_SAVE), _T("Save channels list"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_BUTTON_PACK), _T("Make a plugin to install on player"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_BUTTON_UPDATE_ICON), _T("Set channel icon from original playlist"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_BUTTON_CHOOSE_PLAYLIST), _T("Choose playlist"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_BUTTON_DOWNLOAD_PLAYLIST), _T("Save downloaded playlist to disk"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_EDIT_PL_SEARCH), _T("Search in the playlist. Use \\ prefix to find by ID"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_BUTTON_PL_SEARCH_NEXT), _T("Search next"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_BUTTON_PL_FILTER), _T("Filter the playlist"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_STATIC_ICON), _T("Click to change the icon"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_EDIT_ARCHIVE_CHECK_DAYS), _T("Days in the past to test archive play"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_SPIN_ARCHIVE_CHECK_DAYS), _T("Days in the past to test archive play"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_EDIT_ARCHIVE_CHECK_HOURS), _T("Hours added to day in the past to test archive play"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_SPIN_ARCHIVE_CHECK_HOURS), _T("Hours added to day in the past to test archive play"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_EDIT_TIME_SHIFT), _T("EPG Time shift for channel, hours"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_SPIN_TIME_SHIFT), _T("EPG Time shift for channel, hours"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_EDIT_INFO_VIDEO), _T("Video stream info"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_EDIT_INFO_AUDIO), _T("Audio stream info"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_COMBO_STREAM_TYPE), _T("Stream type used to test play stream"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_COMBO_ICON_SOURCE), _T("Source type for loaded icon. Local file or internet link"));

	m_player = ReadRegStringT(REG_PLAYER);
	m_probe = ReadRegStringT(REG_FFPROBE);
	m_archiveCheckDays = ReadRegInt(REG_DAYS_BACK);
	m_archiveCheckHours = ReadRegInt(REG_HOURS_BACK);
	m_bAutoSync = ReadRegInt(REG_AUTOSYNC);

	UpdateData(FALSE);

	m_wndPluginType.AddString(_T("Edem (iLook TV)"));
	m_wndPluginType.AddString(_T("Sharavoz TV"));
	m_wndPluginType.AddString(_T("Sharaclub TV"));
	m_wndPluginType.AddString(_T("Glanz TV"));
	m_wndPluginType.AddString(_T("Antifriz TV"));

	m_wndSearch.EnableWindow(FALSE);
	m_wndPlSearch.EnableWindow(FALSE);
	m_wndCustom.EnableWindow(FALSE);
	m_wndEpg2ID.EnableWindow(FALSE);
	m_wndEpgID1.EnableWindow(FALSE);
	m_wndArchive.EnableWindow(FALSE);
	m_wndAdult.EnableWindow(FALSE);
	m_wndTestEPG2.EnableWindow(FALSE);
	m_wndTestEPG1.EnableWindow(FALSE);
	m_wndStreamID.EnableWindow(FALSE);
	m_wndStreamUrl.EnableWindow(FALSE);
	m_wndCheckArchive.EnableWindow(FALSE);
	m_wndCheckArchive.EnableWindow(FALSE);
	m_wndTimeShift.EnableWindow(FALSE);
	m_wndSpinTimeShift.EnableWindow(FALSE);
	m_wndInfoVideo.EnableWindow(FALSE);
	m_wndInfoAudio.EnableWindow(FALSE);
	m_wndChannelsTree.EnableToolTips(TRUE);

	m_wndPluginType.SetCurSel(ReadRegInt(REG_PLUGIN));
	m_wndIconSource.SetCurSel(ReadRegInt(REG_ICON_SOURCE));

	SwitchPlugin();

	set_allow_save(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CIPTVChannelEditorDlg::SwitchPlugin()
{
	// Rebuild available playlist types and set current plugin parameters
	BOOL bStreamType = TRUE;
	BOOL bPlaylist = TRUE;
	m_token.clear();
	m_domain.clear();
	m_login.clear();
	m_password.clear();
	m_host.clear();

	m_wndPlaylist.ResetContent();

	switch (m_wndPluginType.GetCurSel())
	{
		case 0: // Edem
		{
			m_pluginType = StreamType::enEdem;
			bStreamType = FALSE;

			m_wndPlaylist.AddString(_T("Edem Standard"));
			m_wndPlaylist.AddString(_T("Edem Thematic"));
			m_wndPlaylist.AddString(_T("Custom URL"));
			m_token = ReadRegStringPluginA(REG_TOKEN);
			m_domain = ReadRegStringPluginA(REG_DOMAIN);
			break;
		}
		case 1: // Sharavoz
		{
			m_pluginType = StreamType::enSharavoz;

			m_wndPlaylist.AddString(_T("Playlist"));
			m_password = ReadRegStringPluginA(REG_PASSWORD);
			break;
		}
		case 2: // Sharaclub
		{
			m_pluginType = StreamType::enSharaclub;

			m_wndPlaylist.AddString(_T("Playlist"));
			//m_wndPlaylist.AddString(_T("Mediateka"));
			//m_wndPlaylist.EnableWindow(FALSE);
			m_login = ReadRegStringPluginA(REG_LOGIN);
			m_password = ReadRegStringPluginA(REG_PASSWORD);
			break;
		}
		case 3: // glanz
		{
			m_pluginType = StreamType::enGlanz;

			m_wndPlaylist.AddString(_T("Playlist"));
			m_login = ReadRegStringPluginA(REG_LOGIN);
			m_password = ReadRegStringPluginA(REG_PASSWORD);
			break;
		}
		case 4: // antifriz
		{
			m_pluginType = StreamType::enAntifriz;

			m_wndPlaylist.AddString(_T("Playlist"));
			m_password = ReadRegStringPluginA(REG_PASSWORD);
			break;
		}
		default:
			ASSERT(false);
			break;
	}

	GetDlgItem(IDC_STATIC_STREAM_TYPE)->ShowWindow(bStreamType ? SW_SHOW : SW_HIDE);
	m_wndStreamType.ShowWindow(bStreamType);
	m_wndPlaylist.EnableWindow(bPlaylist);
	int idx = m_wndPlaylist.AddString(_T("Custom File"));
	m_wndPlaylist.SetItemData(idx, TRUE);

	m_pluginName = StreamContainer::get_name(m_pluginType);
	m_wndStreamType.SetCurSel(ReadRegIntPlugin(REG_STREAM_TYPE));

	// Set selected playlist
	int pl_idx = ReadRegIntPlugin(REG_PLAYLIST_TYPE);
	if (pl_idx > m_wndPlaylist.GetCount() || pl_idx < 0)
		pl_idx = 0;

	m_wndPlaylist.SetCurSel(pl_idx);

	// Load channel lists
	const auto& channelsPath = fmt::format(GetAbsPath(utils::PLAYLISTS_ROOT).c_str(), GetPluginNameW().c_str());
	const auto& default_tv_name = fmt::format(L"{:s}_channel_list.xml", GetPluginNameW().c_str());
	const auto& default_vod_name = fmt::format(L"{:s}_mediateka_list.xml", GetPluginNameW().c_str());

	m_all_channels_lists.clear();

	m_wndChannels.ResetContent();
	m_all_channels_lists.emplace_back(_T("Standard"), channelsPath + default_tv_name);

	std::error_code err;
	std::filesystem::directory_iterator dir_iter(channelsPath, err);
	for (auto const& dir_entry : dir_iter)
	{
		const auto& path = dir_entry.path();
		if (path.extension() == _T(".xml")
			&& path.filename() != default_tv_name
			&& path.filename() != default_vod_name)
		{
			m_all_channels_lists.emplace_back(path.filename(), path);
		}
	}

	for (const auto& item : m_all_channels_lists)
	{
		int idx = m_wndChannels.AddString(item.first.c_str());
		m_wndChannels.SetItemData(idx, (DWORD_PTR)item.second.c_str());
	}

	idx = ReadRegIntPlugin(REG_CHANNELS_TYPE);
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

std::wstring CIPTVChannelEditorDlg::GetPluginNameW(bool bCamel /*= false*/) const
{
	switch (m_wndPluginType.GetCurSel())
	{
		case 0: // Edem
			return bCamel ? L"Edem" : L"edem";
		case 1: // Sharavoz
			return bCamel ? L"Sharavoz" : L"sharavoz";
		case 2: // Sharaclub
			return bCamel ? L"Sharaclub" : L"sharaclub";
		case 3: // Glanz
			return bCamel ? L"Glanz" : L"glanz";
		case 4: // Antifriz
			return bCamel ? L"Antifriz" : L"antifriz";
	}

	return L"";
}

std::string CIPTVChannelEditorDlg::GetPluginNameA(bool bCamel /*= false*/) const
{
	switch (m_wndPluginType.GetCurSel())
	{
		case 0: // Edem
			return bCamel ? "Edem" : "edem";
		case 1: // Sharavoz
			return bCamel ? "Sharavoz" : "sharavoz";
		case 2: // Sharaclub
			return bCamel ? "Sharaclub" : "sharaclub";
		case 3: // Glanz
			return bCamel ? "Glanz" : "glanz";
		case 4: // Antifriz
			return bCamel ? "Antifriz" : "antifriz";
	}

	return "";
}

void CIPTVChannelEditorDlg::LoadPlaylist(bool saveToFile /*= false*/)
{
	m_plFileName.Empty();
	std::wstring url;
	int idx = m_wndPlaylist.GetCurSel();
	BOOL isFile = (BOOL)m_wndPlaylist.GetItemData(idx);

	switch (m_wndPluginType.GetCurSel())
	{
		case 0: // Edem
		{
			switch (idx)
			{
				case 0: // Standard
					url = L"http://epg.it999.ru/edem_epg_ico.m3u8";
					break;
				case 1: // Thematic
					url = L"http://epg.it999.ru/edem_epg_ico2.m3u8";
					break;
				case 2: // Custom URL
					url = ReadRegStringPluginW(REG_CUSTOM_URL);
					break;
				case 3: // Custom file
					url = ReadRegStringPluginW(REG_CUSTOM_FILE);
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
				case 0: // Playlist
					url = utils::utf8_to_utf16(fmt::format("http://sharavoz.tk/iptv/p/{:s}/Sharavoz.Tv.navigator-ott.m3u", m_password.c_str()));
					m_plFileName = _T("Sharavoz_Playlist.m3u8");
					break;
				case 1: // Custom file
					url = ReadRegStringPluginW(REG_CUSTOM_FILE);
					break;
				default:
					break;
			}
			break;
		}
		case 2: // Sharaclub
		{
			switch (idx)
			{
				case 0: // Playlist
					url = utils::utf8_to_utf16(fmt::format("http://list.playtv.pro/tv_live-m3u8/{:s}-{:s}", m_login.c_str(), m_password.c_str()));
					m_plFileName = _T("SharaClub_Playlist.m3u8");
					break;
				case 1: // Custom file
					url = ReadRegStringPluginT(REG_CUSTOM_FILE);
					break;
				case 2: // Mediateka
					url = utils::utf8_to_utf16(fmt::format("http://list.playtv.pro/kino-full/{:s}-{:s}", m_login.c_str(), m_password.c_str()));
					m_plFileName = _T("SharaClub_Movie.m3u8");
					break;
				default:
					break;
			}
			break;
		}
		case 3: // Glanz
		{
			switch (idx)
			{
				case 0: // Playlist
					url = utils::utf8_to_utf16(fmt::format("http://pl.ottglanz.tv/get.php?username={:s}&password={:s}&type=m3u&output=hls", m_login.c_str(), m_password.c_str()));
					m_plFileName = _T("Glanz_Playlist.m3u8");
					break;
				case 1: // Custom file
					url = ReadRegStringPluginW(REG_CUSTOM_FILE);
					break;
				default:
					break;
			}
			break;
		}
		case 4: // Antifriz
		{
			switch (idx)
			{
				case 0:
					url = utils::utf8_to_utf16(fmt::format("https://antifriz.tv/playlist/{:s}.m3u8", m_password.c_str()));
					m_plFileName = _T("Antifriz_Playlist.m3u8");
					break;
				case 1: // Custom file
					url = ReadRegStringPluginW(REG_CUSTOM_FILE);
					break;
				default:
					break;
			}
			break;
		}
		default:
			return;
	}

	if (url.empty())
	{
		AfxMessageBox(_T("Playlist source not set!"), MB_OK | MB_ICONERROR);
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

	// #EXTINF:<DURATION> [<KEY>="<VALUE>"]*,<TITLE>
	// Full playlist format
	// #EXTM3U tvg-shift="1"
	// #EXTINF:-1 channel-id="204" group-title="�����" tvg-id="983" tvg-logo="http://epg.it999.ru/img/983.png" tvg-name="������ HD" tvg-shift="0",������ HD
	// http://aaaaaa.akadatel.com/iptv/xxxxxxxxxxxxxx/204/index.m3u8
	//
	// Short (OTTPplay.es) format
	// #EXTM3U
	// #EXTINF:0 tvg-rec="3",������ HD
	// #EXTGRP:�����
	// http://6646b6bc.akadatel.com/iptv/PWXQ2KD5G2VNSK/204/index.m3u8

	auto data = std::make_unique<std::vector<BYTE>>();
	if (isFile)
	{
		std::ifstream stream(url);
		data->assign((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
	}
	else if (utils::CrackUrl(utils::utf16_to_utf8(url)))
	{
		if (utils::DownloadFile(utils::utf16_to_utf8(url), *data))
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
			OnEndLoadPlaylist(0);
			return;
		}
	}

	if (data->empty())
	{
		AfxMessageBox(_T("Empty playlist!"), MB_OK | MB_ICONERROR);
		OnEndLoadPlaylist(0);
		return;
	}

	m_wndProgress.SetRange32(0, (int)std::count(data->begin(), data->end(), '\n'));
	m_wndProgress.SetPos(0);
	m_wndProgress.ShowWindow(SW_SHOW);
	m_wndProgressInfo.ShowWindow(SW_SHOW);

	auto* pThread = (CPlaylistParseThread*)AfxBeginThread(RUNTIME_CLASS(CPlaylistParseThread), THREAD_PRIORITY_HIGHEST, 0, CREATE_SUSPENDED);
	if (!pThread)
	{
		AfxMessageBox(_T("Problem with starting load playlist thread!"), MB_OK | MB_ICONERROR);
		OnEndLoadPlaylist(0);
		return;
	}

	m_loading = TRUE;

	m_wndPluginType.EnableWindow(FALSE);
	m_wndCheckArchive.EnableWindow(FALSE);
	m_wndPlaylist.EnableWindow(FALSE);
	m_wndPlaylistTree.EnableWindow(FALSE);
	m_wndChannels.EnableWindow(FALSE);
	m_wndChannelsTree.EnableWindow(FALSE);
	m_wndFilter.EnableWindow(FALSE);
	m_wndDownloadUrl.EnableWindow(FALSE);
	m_wndChooseUrl.EnableWindow(FALSE);
	m_evtStop.ResetEvent();

	std::unique_ptr<std::vector<std::shared_ptr<PlaylistEntry>>> playlistEntriesOld = std::move(m_playlistEntries);

	FillTreePlaylist();

	CPlaylistParseThread::ThreadConfig cfg;
	cfg.m_parent = this;
	cfg.m_data = data.release();
	cfg.m_hStop = m_evtStop;
	cfg.m_pluginType = m_pluginType;
	cfg.m_rootPath = GetAbsPath(utils::PLUGIN_ROOT);

	pThread->SetData(cfg);
	pThread->ResumeThread();

	UpdatePlaylistCount();
}

LRESULT CIPTVChannelEditorDlg::OnEndLoadPlaylist(WPARAM wParam, LPARAM lParam /*= 0*/)
{
	m_playlistEntries.reset((std::vector<std::shared_ptr<PlaylistEntry>>*)wParam);

	m_wndPluginType.EnableWindow(TRUE);
	m_wndProgress.ShowWindow(SW_HIDE);
	m_wndProgressInfo.ShowWindow(SW_HIDE);
	m_wndFilter.EnableWindow(TRUE);
	m_wndPlSearch.EnableWindow(!m_channelsMap.empty());
	m_wndPlaylistTree.EnableWindow(TRUE);
	m_wndChannels.EnableWindow(TRUE);
	m_wndChannelsTree.EnableWindow(TRUE);

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
					enableCustom = TRUE;
					break;
				case 2:
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
		case 2: // Sharaclub
		case 3: // Glanz
		case 4: // Antifriz
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
			}
			break;
		}
		default:
			break;
	}

	if (m_playlistEntries && !m_playlistEntries->empty())
	{
		bool bSet = false;
		for (const auto& entry : *m_playlistEntries)
		{
			const auto& stream = entry->get_uri_stream();
			switch (m_pluginType)
			{
				case StreamType::enEdem: // ott_key
				case StreamType::enSharavoz: // pin
				case StreamType::enAntifriz:
				case StreamType::enSharaclub:
				{
					const auto& token = stream->get_token();
					const auto& domain = stream->get_domain();
					if (!token.empty() && token != "00000000000000" && !domain.empty() && domain != "localhost")
					{
						m_token = stream->get_token();
						m_domain = stream->get_domain();
					}
					bSet = true;
					break;
				}
				case StreamType::enGlanz: // login/pass
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
					}
					bSet = true;
					break;
				default:
					break;
			}

			if (bSet) break;
		}
	}

	FillTreePlaylist();

	m_loading = FALSE;
	m_wndChooseUrl.EnableWindow(enableCustom);
	m_wndDownloadUrl.EnableWindow(enableDownload);
	m_wndPlaylist.EnableWindow(TRUE);
	m_wndCheckArchive.EnableWindow(TRUE);

	AfxGetApp()->EndWaitCursor();

	return 0;
}

LRESULT CIPTVChannelEditorDlg::OnUpdateProgress(WPARAM wParam, LPARAM lParam /*= 0*/)
{
	CString str;
	str.Format(_T("Channels readed: %d"), wParam);
	m_wndProgressInfo.SetWindowText(str);
	m_wndProgress.SetPos(lParam);

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
	if (is_allow_save() && AfxMessageBox(_T("You have unsaved changes.\nAre you sure?"), MB_YESNO | MB_ICONWARNING) != IDYES)
	{
		return;
	}

	m_evtStop.SetEvent();

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

void CIPTVChannelEditorDlg::FillTreeChannels(LPCSTR select /*= nullptr*/)
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
		params.id = select ? select : m_categoriesMap.begin()->second->get_channels().front()->get_id();
		SelectTreeItem(m_wndChannelsTree, params);
	}
}

void CIPTVChannelEditorDlg::UpdateChannelsCount()
{
	m_wndChInfo.SetWindowText(fmt::format(_T("Channels: {:d}"), m_channelsMap.size()).c_str());

	UpdateData(FALSE);
}

void CIPTVChannelEditorDlg::UpdatePlaylistCount()
{
	if (m_playlistIds.size() != m_playlistMap.size())
		m_wndPlInfo.SetWindowText(fmt::format(_T("{:s}, Channels: {:d} ({:d})"), m_plFileName.GetString(), m_playlistIds.size(), m_playlistMap.size()).c_str());
	else
		m_wndPlInfo.SetWindowText(fmt::format(_T("{:s}, Channels: {:d}"), m_plFileName.GetString(), m_playlistIds.size()).c_str());

	UpdateData(FALSE);
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
		m_epgID1 = channel->get_epg1_id().c_str();
		m_epgID2 = channel->get_epg2_id().c_str();

		m_streamUrl = channel->stream_uri->get_uri().c_str();
		m_streamID = channel->get_id().c_str();
		auto hash = channel->stream_uri->get_hash();
		if (auto pair = m_stream_infos.find(hash); pair != m_stream_infos.end())
		{
			m_infoAudio = pair->second.first.c_str();
			m_infoVideo = pair->second.second.c_str();
		}
		m_wndCustom.SetCheck(!channel->stream_uri->is_template());
		m_timeShiftHours = channel->get_time_shift_hours();
		m_isArchive = !!channel->is_archive();
		m_isAdult = channel->get_adult();

		if (channel->get_icon_uri().get_uri().empty())
		{
			m_wndChannelIcon.SetBitmap(nullptr);
		}
		else
		{
			m_iconUrl = channel->get_icon_uri().get_uri().c_str();
			const auto& img = GetIconCache().get_icon(channel->get_title(), channel->get_icon_absolute_path());
			utils::SetImage(img, m_wndChannelIcon);
		}
	}
	else
	{
		m_epgID2.Empty();
		m_epgID1.Empty();
		m_timeShiftHours = 0;
		m_iconUrl.Empty();
		m_streamUrl.Empty();
		m_streamID.Empty();
		m_infoAudio.Empty();
		m_infoVideo.Empty();
		m_wndChannelIcon.SetBitmap(nullptr);
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

		if (!entry->get_epg1_id().empty())
			m_plEPG.Format(_T("EPG: %hs"), entry->get_epg1_id().c_str());

		m_wndPlArchive.SetCheck(!!entry->is_archive());
		m_archivePlDays = entry->get_archive_days();
		auto hash = entry->stream_uri->get_hash();
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
		auto category = GetCategory(hRoot);
		if (category && category->get_key() == id)
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

bool CIPTVChannelEditorDlg::LoadChannels(const CString& path)
{
	set_allow_save(FALSE);

	m_categoriesMap.clear();
	m_channelsMap.clear();

	auto pos = path.ReverseFind('\\');
	if (pos != -1)
	{
		m_chFileName = path.Mid(++pos);
	}
	else
	{
		m_chFileName = path;
	}

	std::ifstream is(path.GetString(), std::istream::binary);
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


	auto i_node = doc.first_node(utils::TV_INFO);

	m_embedded_info = FALSE;

	auto setup_node = i_node->first_node(utils::CHANNELS_SETUP);
	if (setup_node)
	{
		m_embedded_info = TRUE;

		switch (m_pluginType)
		{
			case StreamType::enEdem:
				m_token = utils::get_value_string(setup_node->first_node(utils::ACCESS_TOKEN));
				m_domain = utils::get_value_string(setup_node->first_node(utils::ACCESS_DOMAIN));
				break;
			case StreamType::enSharavoz:
			case StreamType::enAntifriz:
				m_password = utils::get_value_string(setup_node->first_node(utils::ACCESS_TOKEN));
				break;
			case StreamType::enGlanz:
			case StreamType::enSharaclub:
				m_login = utils::get_value_string(setup_node->first_node(utils::ACCESS_LOGIN));
				m_password = utils::get_value_string(setup_node->first_node(utils::ACCESS_TOKEN));
				break;
			default:
				break;
		}
	}

	const auto& root_path = GetAbsPath(utils::PLUGIN_ROOT);
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
		channel->set_type(m_pluginType);

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
	pCmdUI->Enable(!m_chFileName.IsEmpty() && IsPlaylistCategory(m_wndPlaylistTree.GetFirstSelectedItem()) && IsSelectedTheSameType());
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

	auto channel = std::make_shared<ChannelInfo>(m_pluginType, GetAbsPath(utils::PLUGIN_ROOT));
	channel->set_title(L"New Channel");
	channel->set_icon_uri(utils::ICON_TEMPLATE);

	CImage img;
	if (utils::LoadImage(channel->get_icon_absolute_path(), img))
	{
		utils::SetImage(img, m_wndChannelIcon);
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

	if (IsCategory(m_wndChannelsTree.GetFirstSelectedItem()))
	{
		OnRemoveCategory();
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
	auto lKey = GetCategory(hLeft)->get_key();
	auto rKey = GetCategory(hRight)->get_key();

	// swap pointers in map
	auto lCat = m_categoriesMap[lKey];
	auto rCat = m_categoriesMap[rKey];
	lCat->set_key(rKey);
	rCat->set_key(lKey);
	m_categoriesMap[lKey] = rCat;
	m_categoriesMap[rKey] = lCat;

	// swap HTREEITEM in map
	auto lItem = m_categoriesTreeMap[lKey];
	auto rItem = m_categoriesTreeMap[rKey];
	m_categoriesTreeMap[lKey] = rItem;
	m_categoriesTreeMap[rKey] = lItem;
	//std::swap(m_categoriesTreeMap[lKey], m_categoriesTreeMap[rKey]);

	// ���������� ItemData ��� ��� � ��������� �� �������
	std::vector<std::pair<HTREEITEM, DWORD_PTR>> itemData;
	DWORD_PTR idx = 0;
	for (HTREEITEM hItem = m_wndChannelsTree.GetChildItem(nullptr); hItem != nullptr; hItem = m_wndChannelsTree.GetNextSiblingItem(hItem))
	{
		itemData.emplace_back(hItem, m_wndChannelsTree.GetItemData(hItem));
		m_wndChannelsTree.SetItemData(hItem, idx++);
	}

	// ������ ������� ������ ItemData ��� ����������
	idx = m_wndChannelsTree.GetItemData(hRight);
	m_wndChannelsTree.SetItemData(hRight, m_wndChannelsTree.GetItemData(hLeft));
	m_wndChannelsTree.SetItemData(hLeft, idx);

	// ���������. ����� TreeCtrl ��� �������������� ���������� ������
	TVSORTCB sortInfo = { nullptr };
	sortInfo.lpfnCompare = &CBCompareForSwap;
	m_wndChannelsTree.SortChildrenCB(&sortInfo);

	// ��������������� �������� ItemData
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
			m_wndChannelIcon.EnableWindow(FALSE);
		}

		if (m_wndChannelsTree.GetSelectedCount() == 1)
		{
			if (channel != nullptr)
			{
				state = 1;
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

				m_wndChannelIcon.EnableWindow(TRUE);
				auto category = GetCategory(hSelected);
				if (category)
				{
					m_iconUrl = category->get_icon_uri().get_uri().c_str();
					const auto& img = GetIconCache().get_icon(category->get_title(), category->get_icon_absolute_path());
					utils::SetImage(img, m_wndChannelIcon);
				}
			}
		}
	}

	BOOL enable = (state == 1);
	bool bSameCategory = IsSelectedTheSameCategory();

	m_wndCustom.EnableWindow(enable);
	m_wndEpg2ID.EnableWindow(enable && m_pluginType == StreamType::enEdem);
	m_wndEpgID1.EnableWindow(enable && m_pluginType == StreamType::enEdem);
	m_wndArchive.EnableWindow(state);
	m_wndAdult.EnableWindow(state);
	m_wndTestEPG1.EnableWindow(enable && !m_epgID1.IsEmpty());
	m_wndTestEPG2.EnableWindow(enable && !m_epgID2.IsEmpty());
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
			channel->set_archive_days(m_isArchive ? 1 : 0);
	}

	set_allow_save();
}

void CIPTVChannelEditorDlg::OnEnChangeEditEpg2ID()
{
	UpdateData(TRUE);
	if (m_wndChannelsTree.GetSelectedCount() == 1)
	{
		auto channel = GetChannel(m_wndChannelsTree.GetSelectedItem());
		if (channel)
		{
			channel->set_epg2_id(utils::utf16_to_utf8(m_epgID2.GetString()));
			set_allow_save();
		}
	}
}

void CIPTVChannelEditorDlg::OnEnChangeEditEpg1ID()
{
	UpdateData(TRUE);
	if (m_wndChannelsTree.GetSelectedCount() == 1)
	{
		auto channel = GetChannel(m_wndChannelsTree.GetSelectedItem());
		if (channel)
		{
			channel->set_epg1_id(utils::utf16_to_utf8(m_epgID1.GetString()));
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
			channel->stream_uri->set_uri(utils::utf16_to_utf8(m_streamUrl.GetString()));
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

void CIPTVChannelEditorDlg::OnEnChangeEditArchiveCheckDays()
{
	UpdateData(TRUE);

	if (m_archiveCheckDays < 0)
		m_archiveCheckDays = 0;

	if (m_archiveCheckDays > 14) // really more than two weeks?
		m_archiveCheckDays = 14;

	UpdateData(FALSE);

	SaveReg(REG_DAYS_BACK, m_archiveCheckDays);
}

void CIPTVChannelEditorDlg::OnEnChangeEditArchiveCheckHours()
{
	UpdateData(TRUE);

	if (m_archiveCheckHours < 0)
		m_archiveCheckHours = 0;

	if (m_archiveCheckHours > 23)
		m_archiveCheckHours = 23;

	UpdateData(FALSE);

	SaveReg(REG_HOURS_BACK, m_archiveCheckHours);
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

void CIPTVChannelEditorDlg::OnBnClickedButtonTestEpg1()
{
	auto channel = GetChannel(m_wndChannelsTree.GetSelectedItem());
	if (channel)
	{
		std::string url = channel->stream_uri->get_epg1_uri(channel->get_epg1_id());
		if (!url.empty())
			ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	}
}

void CIPTVChannelEditorDlg::OnBnClickedButtonTestEpg2()
{
	auto channel = GetChannel(m_wndChannelsTree.GetSelectedItem());
	if (channel)
	{
		std::string url = channel->stream_uri->get_epg2_uri(channel->get_epg2_id());
		if (!url.empty())
			ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	}
}

void CIPTVChannelEditorDlg::PlayItem(HTREEITEM hItem, int archive_hour /*= 0*/, int archive_day /*= 0*/) const
{
	if (auto entry = GetBaseInfo(m_lastTree, hItem); entry != nullptr)
	{
		TemplateParams params;
		params.token = m_token;
		params.domain = m_domain;
		params.login = m_login;
		params.password = m_password;
		params.int_id = entry->stream_uri->get_int_id();
		params.host = m_host;

		int sec_back = 86400 * archive_day + 3600 * archive_hour;
		params.shift_back = sec_back ? _time32(nullptr) - sec_back : sec_back;

		const auto& url = utils::utf8_to_utf16(entry->stream_uri->get_templated((StreamSubType)m_StreamType, params));

		TRACE(L"Test URL: %s\n", url.c_str());

		ShellExecuteW(nullptr, L"open", m_player.GetString(), url.c_str(), nullptr, SW_SHOWNORMAL);
	}
}

void CIPTVChannelEditorDlg::OnBnClickedButtonCustomPlaylist()
{
	bool loaded = false;
	auto entry = std::make_shared<PlaylistEntry>(m_pluginType, theApp.GetAppPath(utils::PLUGIN_ROOT));
	switch (m_pluginType)
	{
		case StreamType::enEdem:
		{
			switch (m_wndPlaylist.GetCurSel())
			{
				case 0:
				case 1:
				{
					loaded = SetupOttKey(loaded);
					break;
				}
				case 3:
				case 4:
					loaded = SetupCustomPlaylist(loaded);
					break;
			}
			break;
		}
		case StreamType::enSharavoz:
		case StreamType::enAntifriz:
		{
			switch (m_wndPlaylist.GetCurSel())
			{
				case 0:
				{
					loaded = SetupPin(loaded);
					break;
				}
				case 1:
					loaded = SetupCustomPlaylist(loaded);
					break;
				default:
					break;
			}
			break;
		}
		case StreamType::enSharaclub:
		case StreamType::enGlanz:
		{
			switch (m_wndPlaylist.GetCurSel())
			{
				case 0:
				{
					loaded = SetupLogin(loaded);
					break;
				}
				case 1:
					loaded = SetupCustomPlaylist(loaded);
					break;
				default:
					break;
			}
			break;
		}
		default:
			break;
	}

	if (loaded)
	{
		LoadPlaylist();
	}
}

bool CIPTVChannelEditorDlg::SetupOttKey(bool loaded)
{
	CAccessOttKeyDlg dlg;
	dlg.m_bEmbed = m_embedded_info;
	dlg.m_streamType = m_pluginType;
	dlg.m_accessKey = m_token.c_str();
	dlg.m_domain = m_domain.c_str();
	dlg.m_url = ReadRegStringPluginT(REG_ACCESS_URL);

	if (dlg.DoModal() == IDOK)
	{
		loaded = utils::utf8_to_utf16(m_token) != dlg.m_accessKey.GetString() && utils::utf8_to_utf16(m_domain) != dlg.m_domain.GetString();

		if (m_embedded_info != dlg.m_bEmbed)
		{
			m_embedded_info = dlg.m_bEmbed;
			set_allow_save(TRUE);
		}

		SaveRegPlugin(m_embedded_info ? REG_TOKEN_EMBEDDED : REG_TOKEN, dlg.m_accessKey.GetString());
		SaveRegPlugin(m_embedded_info ? REG_DOMAIN_EMBEDDED : REG_DOMAIN, dlg.m_domain.GetString());
		SaveRegPlugin(REG_ACCESS_URL, dlg.m_url);

		m_token = ReadRegStringPluginA(m_embedded_info ? REG_TOKEN_EMBEDDED : REG_TOKEN);
		m_domain = ReadRegStringPluginA(m_embedded_info ? REG_DOMAIN_EMBEDDED : REG_DOMAIN);
	}

	return loaded;
}

bool CIPTVChannelEditorDlg::SetupLogin(bool loaded)
{
	auto entry = std::make_shared<PlaylistEntry>(m_pluginType, theApp.GetAppPath(utils::PLUGIN_ROOT));

	auto& stream = entry->get_uri_stream();

	stream->set_token(m_token);
	stream->set_domain(m_domain);
	stream->set_login(m_login);
	stream->set_password(m_password);
	stream->set_host(m_host);

	CAccessInfoPassDlg dlg;

	dlg.m_bEmbed = m_embedded_info;
	dlg.m_entry = entry;

	if (dlg.DoModal() == IDOK)
	{
		if (m_embedded_info != dlg.m_bEmbed)
		{
			m_embedded_info = dlg.m_bEmbed;
			set_allow_save(TRUE);
		}

		loaded = m_token != dlg.m_entry->stream_uri->get_token()
			&& m_domain != dlg.m_entry->stream_uri->get_domain()
			&& m_login != dlg.m_entry->stream_uri->get_login()
			&& m_password != dlg.m_entry->stream_uri->get_password()
			&& m_host != dlg.m_entry->stream_uri->get_host();

		m_token = dlg.m_entry->stream_uri->get_token();
		m_domain = dlg.m_entry->stream_uri->get_domain();
		m_login = dlg.m_entry->stream_uri->get_login();
		m_password = dlg.m_entry->stream_uri->get_password();
		m_host = dlg.m_entry->stream_uri->get_host();

		SaveRegPlugin(m_embedded_info ? REG_TOKEN_EMBEDDED : REG_TOKEN, m_token.c_str());
		SaveRegPlugin(m_embedded_info ? REG_DOMAIN_EMBEDDED : REG_DOMAIN, m_domain.c_str());
		SaveRegPlugin(m_embedded_info ? REG_LOGIN_EMBEDDED : REG_LOGIN, m_login.c_str());
		SaveRegPlugin(m_embedded_info ? REG_PASSWORD_EMBEDDED : REG_PASSWORD, m_password.c_str());
		SaveRegPlugin(m_embedded_info ? REG_HOST_EMBEDDED : REG_HOST, m_host.c_str());
	}

	return loaded;
}

bool CIPTVChannelEditorDlg::SetupPin(bool loaded)
{
	auto entry = std::make_shared<PlaylistEntry>(m_pluginType, theApp.GetAppPath(utils::PLUGIN_ROOT));

	CAccessInfoPinDlg dlg;
	dlg.m_bEmbed = m_embedded_info;
	dlg.m_entry = entry;

	auto& stream = entry->get_uri_stream();

	stream->set_token(m_token);
	stream->set_domain(m_domain);
	stream->set_password(m_password);

	if (dlg.DoModal() == IDOK)
	{
		if (m_embedded_info != dlg.m_bEmbed)
		{
			m_embedded_info = dlg.m_bEmbed;
			set_allow_save(TRUE);
		}

		loaded = m_token != dlg.m_entry->stream_uri->get_token()
			&& m_domain != dlg.m_entry->stream_uri->get_domain()
			&& m_password != dlg.m_entry->stream_uri->get_password();

		m_token = dlg.m_entry->stream_uri->get_token();
		m_domain = dlg.m_entry->stream_uri->get_domain();
		m_password = dlg.m_entry->stream_uri->get_password();

		SaveRegPlugin(m_embedded_info ? REG_TOKEN_EMBEDDED : REG_TOKEN, m_token.c_str());
		SaveRegPlugin(m_embedded_info ? REG_DOMAIN_EMBEDDED : REG_DOMAIN, m_domain.c_str());
		SaveRegPlugin(m_embedded_info ? REG_PASSWORD_EMBEDDED : REG_PASSWORD, m_password.c_str());
	}

	return loaded;
}

bool CIPTVChannelEditorDlg::SetupCustomPlaylist(bool loaded)
{
	CCustomPlaylistDlg dlg;
	dlg.m_isFile = (BOOL)m_wndPlaylist.GetItemData(m_wndPlaylist.GetCurSel());
	LPCTSTR szType = dlg.m_isFile ? REG_CUSTOM_FILE : REG_CUSTOM_URL;
	dlg.m_url = ReadRegStringPluginT(szType);

	if (dlg.DoModal() == IDOK)
	{
		loaded = true;
		SaveRegPlugin(szType, dlg.m_url);
	}
	return loaded;
}

void CIPTVChannelEditorDlg::FillTreePlaylist()
{
	m_bInFillTree = true;

	m_wndPlaylistTree.DeleteAllItems();

	// Filter out playlist
	auto filter = ReadRegStringPluginT(REG_FILTER_STRING);
	auto bRegex = ReadRegIntPlugin(REG_FILTER_REGEX);
	auto bCase = ReadRegIntPlugin(REG_FILTER_CASE);

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

void CIPTVChannelEditorDlg::GetStreamInfo(std::vector<uri_stream*>& container)
{
	TemplateParams params;
	params.token = m_token;
	params.domain = m_domain;
	params.login = m_login;
	params.password = m_password;
	params.host = m_host;

	const auto max_threads = 2;

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
			params.int_id = (*pool)->get_int_id();

			const auto& url = (*pool)->get_templated((StreamSubType)m_StreamType, params);
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
			m_wndProgressInfo.SetWindowText(str);
			m_wndProgress.SetPos(step);
		}
	}
}

void CIPTVChannelEditorDlg::OnSave()
{
	// ��������� ������ ��������� ���� �� ���� �����. ����� ������ ������ � �������
	// [plugin] error: invalid plugin TV info: wrong num_channels(0) for group id '' in num_channels_by_group_id.

	// renumber categories id
	LPCSTR old_selected = nullptr;
	auto channel = GetChannel(m_wndChannelsTree.GetSelectedItem());
	if (channel)
		old_selected = channel->get_id().c_str();

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
			switch (m_pluginType)
			{
				case StreamType::enEdem: // ott_key
					setup_node->append_node(utils::alloc_node(doc, utils::ACCESS_TOKEN, m_token.c_str()));
					setup_node->append_node(utils::alloc_node(doc, utils::ACCESS_DOMAIN, m_domain.c_str()));
					break;
				case StreamType::enSharavoz: // pin
				case StreamType::enAntifriz:
					setup_node->append_node(utils::alloc_node(doc, utils::ACCESS_TOKEN, m_password.c_str()));
					break;
				case StreamType::enSharaclub:
				case StreamType::enGlanz: // login/pass
					setup_node->append_node(utils::alloc_node(doc, utils::ACCESS_LOGIN, m_login.c_str()));
					setup_node->append_node(utils::alloc_node(doc, utils::ACCESS_TOKEN, m_password.c_str()));
					break;
				default:
					break;
			}
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
		auto& playlistPath = fmt::format(GetAbsPath(utils::PLAYLISTS_ROOT).c_str(), GetPluginNameW().c_str());
		std::error_code err;
		std::filesystem::create_directories(playlistPath, err);

		playlistPath += m_chFileName;

		std::ofstream os(playlistPath, std::istream::binary);
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
	auto category_id = GetNewCategoryID();
	auto newCategory = std::make_unique<ChannelCategory>(StreamType::enBase, GetAbsPath(utils::PLUGIN_ROOT));
	newCategory->set_key(category_id);
	newCategory->set_title(L"New Category");
	newCategory->set_icon_uri(utils::ICON_TEMPLATE);

	CImage img;
	if (utils::LoadImage(newCategory->get_icon_absolute_path(), img))
	{
		utils::SetImage(img, m_wndChannelIcon);
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
	UpdateData(TRUE);

	auto hCur = m_wndChannelsTree.GetSelectedItem();
	auto entry = GetBaseInfo(&m_wndChannelsTree, hCur);
	if (!entry)
		return;

	if (IsCategory(hCur) && entry->get_key() == ID_ADD_TO_FAVORITE)
		return;

	if (m_wndIconSource.GetCurSel() == 0)
	{
		CFileDialog dlg(TRUE);
		CString path = GetAbsPath(IsChannel(hCur) ? utils::CHANNELS_LOGO_PATH : utils::CATEGORIES_LOGO_PATH).c_str();
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
					utils::SetImage(img, m_wndChannelIcon);
				}
			}

			m_iconUrl = uri_base::PLUGIN_SCHEME;
			m_iconUrl += IsChannel(hCur) ? utils::CHANNELS_LOGO_URL : utils::CATEGORIES_LOGO_URL;
			m_iconUrl += oFN.lpstrFileTitle;

			if (m_iconUrl != entry->get_icon_uri().get_uri().c_str())
			{
				entry->set_icon_uri(m_iconUrl.GetString());
				const auto& img = GetIconCache().get_icon(entry->get_title(), entry->get_icon_absolute_path());
				utils::SetImage(img, m_wndChannelIcon);
			}
			set_allow_save();
		}
	}
	else
	{
		CIconsListDlg dlg(m_Icons, "http://epg.it999.ru/edem_epg_ico2.m3u8");
		dlg.m_selected = m_lastIconSelected;
		dlg.m_search = entry->get_title().c_str();

		if (dlg.DoModal() == IDOK)
		{
			const auto& choosed = m_Icons->at(dlg.m_selected);
			if (m_iconUrl != choosed->get_icon_uri().get_uri().c_str())
			{
				entry->set_icon_uri(choosed->get_icon_uri());
				const auto& img = GetIconCache().get_icon(choosed->get_title(), choosed->get_icon_absolute_path());
				utils::SetImage(img, m_wndChannelIcon);
				m_lastIconSelected = dlg.m_selected;
			}

			set_allow_save();
		}
	}

	UpdateData(FALSE);
}

void CIPTVChannelEditorDlg::OnBnClickedButtonAbout()
{
	CAboutDlg dlg;
	dlg.DoModal();
}

void CIPTVChannelEditorDlg::OnBnClickedButtonPack()
{
	if (is_allow_save() && AfxMessageBox(_T("You have unsaved changes.\nContinue?"), MB_YESNO | MB_ICONWARNING) != IDYES)
		return;

	const auto& name = GetPluginNameW();

	const auto& packFolder = fmt::format(GetAbsPath(utils::PACK_PATH).c_str(), name.c_str());

	std::error_code err;
	// remove previous packed folder if exist
	std::filesystem::remove_all(packFolder, err);

	// copy new one
	const auto& plugin_root = GetAbsPath(utils::PLUGIN_ROOT);
	std::filesystem::copy(plugin_root, packFolder, std::filesystem::copy_options::recursive, err);

	// copy plugin manifest
	const auto& manifest = fmt::format(L"{:s}manifest\\{:s}_plugin.xml", plugin_root.c_str(), name.c_str());
	const auto& config = fmt::format(L"{:s}configs\\{:s}_config.php", plugin_root.c_str(), name.c_str());
	std::filesystem::copy_file(manifest, packFolder + L"dune_plugin.xml", std::filesystem::copy_options::overwrite_existing, err);
	std::filesystem::copy_file(config, fmt::format(L"{:s}{:s}_config.php", packFolder.c_str(), name.c_str()), std::filesystem::copy_options::overwrite_existing, err);

	// remove over config's
	std::filesystem::remove_all(packFolder + L"manifest", err);
	std::filesystem::remove_all(packFolder + L"configs", err);

	// copy channel lists
	for (const auto& file : m_all_channels_lists)
	{
		std::filesystem::path src(std::filesystem::absolute(file.second));
		std::filesystem::copy_file(src, packFolder + src.filename().c_str(), std::filesystem::copy_options::overwrite_existing, err);
		ASSERT(!err.value());
	}

	// remove files for other plugins
	std::vector<std::wstring> to_remove = {
		L"bg_antifriz.jpg", L"antifriz.png",
		L"bg_edem.jpg", L"edem.png",
		L"bg_glanz.jpg", L"glanz.png",
		L"bg_sharaclub.jpg", L"sharaclub.png",
		L"bg_sharavoz.jpg", L"sharavoz.png",
	};
	to_remove.erase(std::remove(to_remove.begin(), to_remove.end(), fmt::format(L"bg_{:s}.jpg", name.c_str())), to_remove.end());
	to_remove.erase(std::remove(to_remove.begin(), to_remove.end(), fmt::format(L"{:s}.png", name.c_str())), to_remove.end());

	for (const auto& dir_entry : std::filesystem::directory_iterator{ packFolder + L"icons\\"})
	{
		if (std::find(to_remove.begin(), to_remove.end(), dir_entry.path().filename().wstring()) != to_remove.end())
			std::filesystem::remove(dir_entry, err);
	}

	// write setup file
	unsigned char smarker[3] = { 0xEF, 0xBB, 0xBF }; // UTF8 BOM
	std::ofstream os(packFolder + _T("plugin_type.php"), std::ios::out | std::ios::binary);
	os.write((const char*)smarker, sizeof(smarker));
	os << fmt::format("<?php\nrequire_once '{:s}_config.php';\n\nconst PLUGIN_TYPE = '{:s}PluginConfig';\n", GetPluginNameA().c_str(), GetPluginNameA(true).c_str());
	os.close();


	// pack folder
	SevenZipWrapper archiver(GetAbsPath(utils::PACK_DLL));
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
		params.id = utils::utf16_to_utf8(m_search.Mid(1).GetString());
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
		params.id = utils::utf16_to_utf8(m_plSearch.Mid(1).GetString());
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
	pCmdUI->Enable(!m_chFileName.IsEmpty() && IsSelectedChannelsOrEntries());
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

		SaveReg(REG_PLAYER, m_player);
		SaveReg(REG_FFPROBE, m_probe);
		SaveReg(REG_AUTOSYNC, m_bAutoSync);
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
		utils::SetImage(img, m_wndChannelIcon);

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

		const auto& fullPath = icon_uri.get_filesystem_path(GetAbsPath(utils::PLUGIN_ROOT));
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

	const auto& pluginName = GetPluginNameW();
	const auto& name = fmt::format(_T("{:s}_channel_list.xml"), pluginName.c_str());

	auto& newList = fmt::format(GetAbsPath(utils::PLAYLISTS_ROOT).c_str(), pluginName.c_str());
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
	m_wndProgressInfo.ShowWindow(SW_SHOW);
	m_wndProgress.SetPos(0);
	CWaitCursor cur;

	std::vector<uri_stream*> streams;
	for (const auto& hItem : m_lastTree->GetSelectedItems())
	{
		auto channel = GetBaseInfo(m_lastTree, hItem);
		if (channel)
		{
			streams.emplace_back(channel->stream_uri.get());
		}
	}

	if (m_lastTree == &m_wndChannelsTree)
	{
		GetStreamInfo(streams);
		LoadChannelInfo(m_lastTree->GetFirstSelectedItem());
	}
	else if (m_lastTree == &m_wndPlaylistTree)
	{
		GetStreamInfo(streams);
		LoadPlayListInfo(m_lastTree->GetFirstSelectedItem());
	}

	SaveStreamInfo();
	UpdateChannelsCount();
	UpdatePlaylistCount();
	m_wndProgress.ShowWindow(SW_HIDE);
	m_wndProgressInfo.ShowWindow(SW_HIDE);

	UpdateData(FALSE);
}

void CIPTVChannelEditorDlg::OnUpdateGetStreamInfo(CCmdUI* pCmdUI)
{
	BOOL enable = !m_probe.IsEmpty();
	if (m_lastTree)
	{
		HTREEITEM first = m_lastTree->GetFirstSelectedItem();
		enable &= IsSelectedTheSameType() && (IsChannel(first) || IsPlaylistEntry(first));
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
	m_wndProgressInfo.ShowWindow(SW_SHOW);
	m_wndProgress.SetPos(0);

	std::vector<uri_stream*> container;
	if (m_lastTree == &m_wndChannelsTree)
	{
		for (auto hItem = m_lastTree->GetFirstSelectedItem(); hItem != nullptr;  hItem = m_lastTree->GetNextSelectedItem(hItem))
		{
			for (const auto& item : GetItemCategory(hItem)->get_channels())
			{
				auto info = dynamic_cast<BaseInfo*>(item);
				if (info)
					container.emplace_back(info->stream_uri.get());
			}
		}
		GetStreamInfo(container);
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
					container.emplace_back(info->stream_uri.get());
			}
		}

		GetStreamInfo(container);
		LoadPlayListInfo(m_lastTree->GetSelectedItem());
	}

	SaveStreamInfo();
	UpdateChannelsCount();
	UpdatePlaylistCount();
	m_wndProgress.ShowWindow(SW_HIDE);
	m_wndProgressInfo.ShowWindow(SW_HIDE);
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

void CIPTVChannelEditorDlg::OnCbnSelchangeComboIconSource()
{
	SaveReg(REG_ICON_SOURCE, m_wndIconSource.GetCurSel());
}

void CIPTVChannelEditorDlg::OnCbnSelchangeComboPluginType()
{
	if (is_allow_save() && AfxMessageBox(_T("You have unsaved changes.\nAre you sure?"), MB_YESNO | MB_ICONWARNING) != IDYES)
	{
		m_wndPluginType.SetCurSel(ReadRegInt(REG_PLUGIN));
		return;
	}

	set_allow_save(FALSE);
	SwitchPlugin();
}

void CIPTVChannelEditorDlg::OnCbnSelchangeComboPlaylist()
{
	SaveReg(REG_PLUGIN, m_wndPluginType.GetCurSel());
	SaveRegPlugin(REG_PLAYLIST_TYPE, m_wndPlaylist.GetCurSel());

	LoadPlaylist();
}

void CIPTVChannelEditorDlg::OnCbnSelchangeComboStreamType()
{
	SaveRegPlugin(REG_STREAM_TYPE, m_wndStreamType.GetCurSel());
}

void CIPTVChannelEditorDlg::OnCbnSelchangeComboChannels()
{
	int idx = m_wndChannels.GetCurSel();
	if (idx == -1)
		return;

	if (is_allow_save() && AfxMessageBox(_T("You have unsaved changes.\nAre you sure??"), MB_YESNO | MB_ICONWARNING) != IDYES)
	{
		m_wndChannels.SetCurSel(ReadRegIntPlugin(REG_CHANNELS_TYPE));
		return;
	}

	LoadChannels((LPCTSTR)m_wndChannels.GetItemData(idx));
	FillTreeChannels();
	set_allow_save(FALSE);

	GetDlgItem(IDC_BUTTON_ADD_NEW_CHANNELS_LIST)->EnableWindow(idx > 0);
	SaveRegPlugin(REG_CHANNELS_TYPE, idx);
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
					if (entry->stream_uri->get_hash() == searchParams.hash)
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

	const auto& root_path = GetAbsPath(utils::PLUGIN_ROOT);
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
		auto newChannel = std::make_unique<ChannelInfo>(m_pluginType, root_path);
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
	if (!entry->get_epg1_id().empty())
	{
		channel->set_epg1_id(entry->get_epg1_id());
	}

	if (!entry->get_epg2_id().empty())
	{
		channel->set_epg2_id(entry->get_epg2_id());
	}

	if (entry->get_archive_days() != 0)
	{
		channel->set_archive_days(entry->get_archive_days());
	}

	channel->stream_uri->copy(entry->stream_uri.get());

	if (entry->get_icon_uri() != channel->get_icon_uri() && !entry->get_icon_uri().get_uri().empty())
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

	// argv[0] ��� ������������ �����
	CString csCommand;
	csCommand.Format(_T("\"%s\" -hide_banner -show_streams \"%hs\""), m_probe.GetString(), url.c_str());

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
				// �� ����� �� �������� ���������
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

void CIPTVChannelEditorDlg::OnBnClickedButtonPlFilter()
{
	CFilterDialog dlg;

	dlg.m_filterString = ReadRegStringPluginT(REG_FILTER_STRING);
	dlg.m_filterRegex = ReadRegIntPlugin(REG_FILTER_REGEX);
	dlg.m_filterCase = ReadRegIntPlugin(REG_FILTER_CASE);

	if (dlg.DoModal() == IDOK)
	{
		SaveRegPlugin(REG_FILTER_STRING, dlg.m_filterString);
		SaveRegPlugin(REG_FILTER_REGEX, dlg.m_filterRegex);
		SaveRegPlugin(REG_FILTER_CASE, dlg.m_filterCase);

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

		m_toolTipText.Format(_T("Name: %s\nID: %hs\nEPG1 ID: %hs\nEPG2 ID: %hs\nArchive: %s\nAdult: %s\nIn categories: %s"),
							 entry->get_title().c_str(),
							 entry->stream_uri->is_template() ? ch_id.c_str() : "Custom",
							 entry->get_epg1_id().c_str(),
							 entry->get_epg2_id().c_str(),
							 entry->is_archive() ? _T("Yes") : _T("No"),
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
		m_toolTipText.Format(_T("Name: %s\nID: %hs\nEPG: %hs\nArchive: %s\nAdult: %s"),
							 entry->get_title().c_str(),
							 entry->stream_uri->is_template() ? entry->get_id().c_str() : "Custom",
							 entry->get_epg1_id().c_str(),
							 entry->is_archive() ? _T("Yes") : _T("No"),
							 entry->get_adult() ? _T("Yes") : _T("No"));

		pGetInfoTip->pszText = m_toolTipText.GetBuffer();
	}

	*pResult = 0;
}

BOOL CIPTVChannelEditorDlg::DestroyWindow()
{
	theApp.SaveWindowPos(GetSafeHwnd(), _T("WindowPos"));

	return __super::DestroyWindow();
}

void CIPTVChannelEditorDlg::SaveStreamInfo()
{
	const auto& dump = m_stream_infos.serialize();
	// write document
	const auto& playlistPath = fmt::format(GetAbsPath(utils::PLAYLISTS_ROOT).c_str(), GetPluginNameW().c_str());
	const auto& path = playlistPath + _T("stream_info.bin");
	std::ofstream os(path, std::istream::binary);
	os.write(dump.data(), dump.size());
	os.close();
}

std::wstring CIPTVChannelEditorDlg::GetPluginRegPath() const
{
	return fmt::format(_T("{:s}\\{:s}"), REG_SETTINGS, GetPluginNameW().c_str());
}

void CIPTVChannelEditorDlg::SaveReg(LPCTSTR path, LPCSTR szValue)
{
	theApp.WriteProfileString(REG_SETTINGS, path, utils::utf8_to_utf16(szValue).c_str());
}

void CIPTVChannelEditorDlg::SaveReg(LPCTSTR path, LPCWSTR szValue)
{
	theApp.WriteProfileString(REG_SETTINGS, path, szValue);
}

void CIPTVChannelEditorDlg::SaveReg(LPCTSTR path, int value)
{
	theApp.WriteProfileInt(REG_SETTINGS, path, value);
}

void CIPTVChannelEditorDlg::SaveRegPlugin(LPCTSTR path, LPCSTR szValue)
{
	theApp.WriteProfileString(GetPluginRegPath().c_str(), path, utils::utf8_to_utf16(szValue).c_str());
}

void CIPTVChannelEditorDlg::SaveRegPlugin(LPCTSTR path, LPCWSTR szValue)
{
	theApp.WriteProfileString(GetPluginRegPath().c_str(), path, szValue);
}

void CIPTVChannelEditorDlg::SaveRegPlugin(LPCTSTR path, int value)
{
	theApp.WriteProfileInt(GetPluginRegPath().c_str(), path, value);
}

CString CIPTVChannelEditorDlg::ReadRegStringT(LPCTSTR path) const
{
	return theApp.GetProfileString(REG_SETTINGS, path);
}

std::string CIPTVChannelEditorDlg::ReadRegStringA(LPCTSTR path) const
{
	return utils::utf16_to_utf8(theApp.GetProfileString(REG_SETTINGS, path).GetString());
}

std::wstring CIPTVChannelEditorDlg::ReadRegStringW(LPCTSTR path) const
{
	return theApp.GetProfileString(REG_SETTINGS, path).GetString();
}

int CIPTVChannelEditorDlg::ReadRegInt(LPCTSTR path, int default /*= 0*/) const
{
	return theApp.GetProfileInt(REG_SETTINGS, path, default);
}

CString CIPTVChannelEditorDlg::ReadRegStringPluginT(LPCTSTR path) const
{
	return theApp.GetProfileString(GetPluginRegPath().c_str(), path);
}

std::string CIPTVChannelEditorDlg::ReadRegStringPluginA(LPCTSTR path) const
{
	return utils::utf16_to_utf8(theApp.GetProfileString(GetPluginRegPath().c_str(), path).GetString());
}

std::wstring CIPTVChannelEditorDlg::ReadRegStringPluginW(LPCTSTR path) const
{
	return theApp.GetProfileString(GetPluginRegPath().c_str(), path).GetString();
}

int CIPTVChannelEditorDlg::ReadRegIntPlugin(LPCTSTR path, int default /*= 0*/) const
{
	return theApp.GetProfileInt(GetPluginRegPath().c_str(), path, default);
}
