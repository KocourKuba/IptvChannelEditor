
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
#include "utils.h"

#include "rapidxml.hpp"
#include "rapidxml_print.hpp"

#include "SevenZip/7zip/SevenZipWrapper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WN_START_LOAD_PLAYLIST (WM_USER + 301)

BOOL CEdemChannelEditorDlg::m_embedded_info = FALSE;
CString CEdemChannelEditorDlg::m_gl_domain;
CString CEdemChannelEditorDlg::m_gl_access_key;
CString CEdemChannelEditorDlg::m_ch_domain;
CString CEdemChannelEditorDlg::m_ch_access_key;
CString CEdemChannelEditorDlg::m_probe;

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

BEGIN_MESSAGE_MAP(CEdemChannelEditorDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()

	ON_BN_CLICKED(IDC_BUTTON_ABOUT, &CEdemChannelEditorDlg::OnBnClickedButtonAbout)
	ON_BN_CLICKED(IDC_BUTTON_ADD_TO_SHOW, &CEdemChannelEditorDlg::OnBnClickedButtonAddToShowIn)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_PLAYLIST, &CEdemChannelEditorDlg::OnBnClickedButtonCustomPlaylist)
	ON_BN_CLICKED(IDC_BUTTON_PL_SEARCH_NEXT, &CEdemChannelEditorDlg::OnBnClickedButtonPlSearchNext)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_PL_SEARCH_NEXT, &CEdemChannelEditorDlg::OnUpdateButtonPlSearchNext)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_FROM_SHOW, &CEdemChannelEditorDlg::OnBnClickedButtonRemoveFromShowIn)
	ON_BN_CLICKED(IDC_BUTTON_ADD_CATEGORY, &CEdemChannelEditorDlg::OnBnClickedButtonAddCategory)
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
	ON_EN_CHANGE(IDC_EDIT_NEXT_EPG, &CEdemChannelEditorDlg::OnEnChangeEditNumNext)
	ON_EN_CHANGE(IDC_EDIT_PREV_EPG, &CEdemChannelEditorDlg::OnEnChangeEditNumPrev)
	ON_EN_CHANGE(IDC_EDIT_ARCHIVE_CHECK, &CEdemChannelEditorDlg::OnEnChangeEditArchiveCheck)
	ON_EN_CHANGE(IDC_EDIT_STREAM_URL, &CEdemChannelEditorDlg::OnEnChangeEditStreamUrl)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_NEXT, &CEdemChannelEditorDlg::OnDeltaposSpinNext)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_PREV, &CEdemChannelEditorDlg::OnDeltaposSpinPrev)
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

	ON_NOTIFY(NM_DBLCLK, IDC_TREE_PLAYLIST, &CEdemChannelEditorDlg::OnNMDblclkTreePaylist)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_PLAYLIST, &CEdemChannelEditorDlg::OnTvnSelchangedTreePaylist)
	ON_NOTIFY(NM_RCLICK, IDC_TREE_PLAYLIST, &CEdemChannelEditorDlg::OnNMRclickTreePlaylist)
	ON_NOTIFY(NM_SETFOCUS, IDC_TREE_PLAYLIST, &CEdemChannelEditorDlg::OnNMSetfocusTree)

	ON_COMMAND(ID_SAVE, &CEdemChannelEditorDlg::OnSave)
	ON_UPDATE_COMMAND_UI(ID_SAVE, &CEdemChannelEditorDlg::OnUpdateSave)
	ON_COMMAND(ID_NEW_CHANNEL, &CEdemChannelEditorDlg::OnNewChannel)
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
	ON_COMMAND(ID_SYNC_ENTRY, &CEdemChannelEditorDlg::OnSyncEntry)
	ON_UPDATE_COMMAND_UI(ID_SYNC_ENTRY, &CEdemChannelEditorDlg::OnUpdateSyncEntry)

	ON_MESSAGE_VOID(WM_KICKIDLE, OnKickIdle)
	ON_MESSAGE(WN_START_LOAD_PLAYLIST, &CEdemChannelEditorDlg::OnStartLoadPlaylist)
	ON_LBN_SELCHANGE(IDC_LIST_CATEGORIES, &CEdemChannelEditorDlg::OnLbnSelchangeListCategories)
END_MESSAGE_MAP()

CEdemChannelEditorDlg::CEdemChannelEditorDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EDEMCHANNELEDITOR_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_plID = _T("ID:");
	m_plEPG = _T("EPG:");
}

void CEdemChannelEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_ARCHIVE, m_hasArchive);
	DDX_Control(pDX, IDC_CHECK_ARCHIVE, m_wndArchive);
	DDX_Check(pDX, IDC_CHECK_ADULT, m_isAdult);
	DDX_Control(pDX, IDC_CHECK_ADULT, m_wndAdult);
	DDX_Control(pDX, IDC_TREE_CHANNELS, m_wndChannelsTree);
	DDX_Control(pDX, IDC_COMBO_CATEGORY, m_wndCategories);
	DDX_Control(pDX, IDC_LIST_CATEGORIES, m_wndCategoriesList);
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
	DDX_Text(pDX, IDC_EDIT_PREV_EPG, m_prevDays);
	DDX_Control(pDX, IDC_EDIT_PREV_EPG, m_wndPrevDays);
	DDX_Text(pDX, IDC_EDIT_NEXT_EPG, m_nextDays);
	DDX_Control(pDX, IDC_EDIT_NEXT_EPG, m_wndNextDays);
	DDX_Text(pDX, IDC_EDIT_ARCHIVE_CHECK, m_archiveCheck);
	DDX_Control(pDX, IDC_BUTTON_ADD_TO_SHOW, m_wndAddToShow);
	DDX_Control(pDX, IDC_BUTTON_REMOVE_FROM_SHOW, m_wndRemoveFromShow);
	DDX_Control(pDX, IDC_STATIC_ICON, m_wndIcon);
	DDX_Text(pDX, IDC_EDIT_SEARCH, m_search);
	DDX_Control(pDX, IDC_TREE_PLAYLIST, m_wndPlaylistTree);
	DDX_Text(pDX, IDC_EDIT_PL_SEARCH, m_plSearch);
	DDX_Text(pDX, IDC_STATIC_ICON_NAME, m_iconUrl);
	DDX_Control(pDX, IDC_STATIC_PL_ICON, m_wndPlIcon);
	DDX_Text(pDX, IDC_STATIC_PL_ICON_NAME, m_plIconName);
	DDX_Text(pDX, IDC_STATIC_PLAYLIST, m_plInfo);
	DDX_Text(pDX, IDC_STATIC_PL_ID, m_plID);
	DDX_Text(pDX, IDC_STATIC_PL_TVG, m_plEPG);
	DDX_Control(pDX, IDC_CHECK_PL_ARCHIVE, m_wndPlArchive);
	DDX_Text(pDX, IDC_EDIT_INFO_VIDEO, m_infoVideo);
	DDX_Control(pDX, IDC_EDIT_INFO_VIDEO, m_wndInfoVideo);
	DDX_Text(pDX, IDC_EDIT_INFO_AUDIO, m_infoAudio);
	DDX_Control(pDX, IDC_EDIT_INFO_AUDIO, m_wndInfoAudio);
	DDX_Text(pDX, IDC_STATIC_CHANNELS, m_chInfo);
	DDX_Control(pDX, IDC_BUTTON_GET_INFO, m_wndGetInfo);
	DDX_Control(pDX, IDC_BUTTON_CHECK_ARCHIVE, m_wndCheckArchive);
	DDX_Control(pDX, IDC_COMBO_PLAYLIST, m_wndPlaylistType);
	DDX_Control(pDX, IDC_COMBO_CHANNELS, m_wndChannels);
	DDX_Control(pDX, IDC_BUTTON_LOAD_PLAYLIST, m_wndChooseUrl);
	DDX_Control(pDX, IDC_BUTTON_DOWNLOAD_PLAYLIST, m_wndDownloadUrl);
	DDX_Control(pDX, IDC_PROGRESS_LOAD, m_wndProgress);
	DDX_Control(pDX, IDC_BUTTON_CACHE_ICON, m_wndCacheIcon);
	DDX_Control(pDX, IDC_BUTTON_UPDATE_ICON, m_wndUpdateIcon);
	DDX_Control(pDX, IDC_BUTTON_SAVE, m_wndSave);
	DDX_Control(pDX, IDC_SPIN_PREV, m_wndSpinPrev);
	DDX_Control(pDX, IDC_SPIN_NEXT, m_wndSpinNext);
}

// CEdemChannelEditorDlg message handlers

BOOL CEdemChannelEditorDlg::OnInitDialog()
{
	__super::OnInitDialog();

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
	m_wndChannelsTree.m_color = RGB(0, 200, 0);
	m_wndChannelsTree.class_hash = typeid(ChannelInfo).hash_code();

	m_wndPlaylistTree.m_color = RGB(200, 0, 0);
	m_wndPlaylistTree.class_hash = typeid(PlaylistEntry).hash_code();

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
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_BUTTON_ADD_TO_SHOW), _T("Assign a category to channel"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_BUTTON_REMOVE_FROM_SHOW), _T("Un-assign a category to channel"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_COMBO_CATEGORY), _T("Available categories"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_BUTTON_ADD_CATEGORY), _T("Add new category"));
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
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_EDIT_INFO_VIDEO), _T("Video stream info"));
	m_pToolTipCtrl.AddTool(GetDlgItem(IDC_EDIT_INFO_AUDIO), _T("Audio stream info"));

	m_wndPlaylistType.SetCurSel(theApp.GetProfileInt(_T("Setting"), _T("PlaylistType"), 0));

	m_all_playlists.emplace_back(_T("Standard"), theApp.GetAppPath(utils::CHANNELS_CONFIG));
	CFileFind ffind;
	BOOL bFound = ffind.FindFile(_T("edem_plugin\\*.xml"));
	while (bFound)
	{
		bFound = ffind.FindNextFile();
		if (!ffind.IsDirectory() && ffind.GetFileName() != "dune_plugin.xml" && ffind.GetFileName() != "edem_channel_list.xml")
		{
			m_all_playlists.emplace_back(ffind.GetFileName(), ffind.GetFilePath());
		}
	}

	for(const auto& playlist : m_all_playlists)
	{
		int idx = m_wndChannels.AddString(playlist.first);
		m_wndChannels.SetItemData(idx, (DWORD_PTR)playlist.second.GetString());
	}

	int idx = theApp.GetProfileInt(_T("Setting"), _T("ChannelsType"), 0);
	if (idx < m_wndChannels.GetCount())
		m_wndChannels.SetCurSel(idx);

	UpdateData(FALSE);

	set_allow_save(FALSE);

	CString channels = (LPCTSTR)m_wndChannels.GetItemData(m_wndChannels.GetCurSel());
	if (LoadChannels(channels))
	{
		FillCategories();
		FillChannels();
	}

	if (!m_channels.empty())
	{
		m_wndChannelsTree.SelectItem(FindTreeItem(m_wndChannelsTree, (DWORD_PTR)m_channels[0].get()));
	}

	OnCbnSelchangeComboPlaylist();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

LRESULT CEdemChannelEditorDlg::OnStartLoadPlaylist(WPARAM wParam, LPARAM lParam /*= 0*/)
{
	// #EXTM3U <--- header
	// #EXTINF:0 tvg-rec="3",������ FHD <-- caption
	// #EXTGRP:����� <-- Category
	// http://6646b6bc.akadatel.com/iptv/PWXQ2KD5G2VNSK/2402/index.m3u8

	CWaitCursor cur;

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

	const CString& file = theApp.GetProfileString(_T("Setting"), _T("Playlist"));
	std::vector<BYTE> data;
	std::unique_ptr<std::istream> pl_stream;
	if (utils::CrackUrl(file.GetString()))
	{
		if (utils::DownloadFile(file.GetString(), data))
		{
			if (wParam)
			{
				std::ofstream os(m_plFileName);
				os.write((char*)data.data(), data.size());
				os.close();
				return 0;
			}

			utils::vector_to_streambuf<char> buf(data);
			pl_stream = std::make_unique<std::istream>(&buf);
		}
	}
	else
	{
		pl_stream = std::make_unique<std::ifstream>(file.GetString());
	}

	m_playlist.clear();
	m_playlistIds.clear();
	m_pl_categories.clear();

	if (pl_stream && pl_stream->good())
	{
		int step = 0;
		size_t lines = std::count(data.begin(), data.end(), '\n');
		m_wndProgress.SetRange32(0, lines);
		m_wndProgress.SetPos(0);
		m_wndProgress.ShowWindow(SW_SHOW);

		m_filterString = theApp.GetProfileString(_T("Setting"), _T("FilterString"));
		BOOL filterRegex = theApp.GetProfileInt(_T("Setting"), _T("FilterUseRegex"), FALSE);
		BOOL filterCase = theApp.GetProfileInt(_T("Setting"), _T("FilterUseCase"), FALSE);

		std::string line;
		auto entry = std::make_unique<PlaylistEntry>();
		while (std::getline(*pl_stream, line))
		{
			m_wndProgress.SetPos(step++);
			utils::string_rtrim(line, "\r");
			if (line.empty()) continue;

			entry->Parse(line);
			if (entry->get_directive() == ext_pathname
				&& !AddPlaylistEntry(entry, filterRegex, filterCase)) break;
		}

		m_wndProgress.ShowWindow(SW_HIDE);
		FillPlaylist();
	}

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

	CWnd* pFocus = GetFocus();
	if (pFocus == GetDlgItem(IDC_EDIT_SEARCH) && !m_search.IsEmpty())
	{
		OnBnClickedButtonSearchNext();
	}
	else if (pFocus == GetDlgItem(IDC_EDIT_PL_SEARCH) && !m_plSearch.IsEmpty())
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

void CEdemChannelEditorDlg::FillChannels()
{
	m_wndChannelsTree.LockWindowUpdate();

	m_wndChannelsTree.DeleteAllItems();

	std::map<int, HTREEITEM> tree_categories;

	for (auto& category : m_categories)
	{
		TVINSERTSTRUCTW tvInsert = { nullptr };
		tvInsert.hParent = TVI_ROOT;
		tvInsert.item.pszText = (LPWSTR)category.second->get_caption().c_str();
		tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
		tvInsert.item.lParam = (DWORD_PTR)category.second->get_id();
		auto item = m_wndChannelsTree.InsertItem(&tvInsert);
		tree_categories.emplace(category.first, item);
	}

	for (const auto& channel : m_channels)
	{
		for(const auto& category : channel->get_categores())
		{
			auto pair = tree_categories.find(category.first);
			ASSERT(pair != tree_categories.end());

			TVINSERTSTRUCTW tvInsert = { nullptr };
			tvInsert.hParent = pair->second;
			tvInsert.item.pszText = (LPWSTR)channel->get_title().c_str();
			tvInsert.item.lParam = (LPARAM)channel.get();
			tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
			m_wndChannelsTree.InsertItem(&tvInsert);
		}
	}

	m_wndChannelsTree.UnlockWindowUpdate();

	UpdateChannelsCount();
	CheckForExisting();
}

void CEdemChannelEditorDlg::UpdateChannelsCount()
{
	m_chInfo.Format(_T("Channels: %s (%d)"), m_chFileName.GetString(), m_channels.size());

	UpdateData(FALSE);
}

void CEdemChannelEditorDlg::UpdatePlaylistCount()
{
	m_plInfo.Format(_T("Playlist: %s (%d%s)"), m_plFileName.GetString(), m_playlist.size(), (m_filterString.IsEmpty() ? _T("") : _T("*")));

	UpdateData(FALSE);
}

void CEdemChannelEditorDlg::FillCategories()
{
	m_wndCategories.ResetContent();
	for (const auto& category : m_categories)
	{
		int idx = m_wndCategories.AddString(category.second->get_caption().c_str());
		m_wndCategories.SetItemData(idx, (DWORD_PTR)category.second.get());
	}

	if (!m_categories.empty())
		m_wndCategories.SetCurSel(0);
}

void CEdemChannelEditorDlg::CheckForExisting()
{
	std::set<int> ids;
	for (auto& item : m_channels)
	{
		int id = item->get_channel_id();
		item->set_colored(m_playlistIds.find(id) != m_playlistIds.end());
		ids.emplace(id);
	}

	for (auto& item : m_playlist)
	{
		item->set_colored(ids.find(item->get_channel_id()) == ids.end());
	}

	m_wndChannelsTree.RedrawWindow();
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
		m_streamID = channel->get_channel_id();
		m_infoAudio = channel->get_audio().c_str();
		m_infoVideo = channel->get_video().c_str();
		m_wndCustom.SetCheck(m_streamID == 0 && channel->get_stream_uri().is_template());
		if (channel)
		{
			m_wndCategoriesList.ResetContent();
			for (const auto& category : channel->get_categores())
			{
				int pos = m_wndCategoriesList.AddString(category.second->get_caption().c_str());
				m_wndCategoriesList.SetItemData(pos, (DWORD_PTR)category.second);
			}
		}

		m_prevDays = channel->get_prev_epg_days();
		m_nextDays = channel->get_next_epg_days();
		m_hasArchive = channel->get_has_archive();
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
		m_wndCategoriesList.ResetContent();
		m_tvgID = 0;
		m_epgID = 0;
		m_iconUrl.Empty();
		m_streamUrl.Empty();
		m_streamID = 0;
		m_infoAudio.Empty();
		m_infoVideo.Empty();
		m_wndIcon.SetBitmap(nullptr);
		m_wndCustom.SetCheck(FALSE);
		m_wndCategoriesList.ResetContent();
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
		int id = entry->get_channel_id();
		m_plID.Format(_T("ID: %d"), id);

		if (entry->get_tvg_id() != -1)
			m_plEPG.Format(_T("EPG: %d"), entry->get_tvg_id());

		m_wndPlArchive.SetCheck(!!entry->is_archive());
		m_infoAudio = entry->get_audio().c_str();
		m_infoVideo = entry->get_video().c_str();

		CImage img;
		if (theApp.LoadImage(entry->get_icon_uri().get_uri().c_str(), img))
		{
			entry->set_icon(img);
		}

		theApp.SetImage(entry->get_icon(), m_wndPlIcon);

		if (m_bAutoSync)
		{
			auto found = std::find_if(m_channels.begin(), m_channels.end(), [id](const auto& channel)
									  {
										  return channel->get_channel_id() == id;
									  });

			if (found != m_channels.end())
			{
				if (auto hSelected = FindTreeItem(m_wndChannelsTree, (DWORD_PTR)found->get()); hSelected != nullptr)
				{
					m_wndChannelsTree.SelectItem(hSelected);
				}
			}
		}

		UpdateData(FALSE);
	}
}

ChannelInfo* CEdemChannelEditorDlg::GetChannel(HTREEITEM hItem) const
{
	return IsChannel(hItem) ? (ChannelInfo*)m_wndChannelsTree.GetItemData(hItem) : nullptr;
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

	auto found = m_categories.find((int)m_wndChannelsTree.GetItemData(hItem));

	return found != m_categories.end() ? found->second.get() : nullptr;
}

HTREEITEM CEdemChannelEditorDlg::GetCategoryItem(int id) const
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
	if (!m_categories.empty())
	{
		id = m_categories.crbegin()->first;
	}

	return ++id;
}

bool CEdemChannelEditorDlg::LoadChannels(const CString& path)
{
	m_categories.clear();
	m_channels.clear();

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
		m_categories.emplace(category->get_id(), std::move(category));
		cat_node = cat_node->next_sibling();
	}

	auto ch_node = i_node->first_node(utils::TV_CHANNELS)->first_node(ChannelInfo::TV_CHANNEL);
	// Iterate <tv_channel> nodes
	while (ch_node)
	{
		m_channels.emplace_back(std::move(std::make_unique<ChannelInfo>(ch_node, m_categories)));
		ch_node = ch_node->next_sibling();
	}

	m_cur_it = m_channels.end();

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
		CheckForExisting();

	m_bAutoSync = autoSyncOld;

	UpdateChannelsCount();
	LoadChannelInfo(m_wndChannelsTree.GetSelectedItem());
	set_allow_save();
}

void CEdemChannelEditorDlg::OnUpdateAddCategory(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsSelectedTheSameType() && IsPlaylistCategory(m_wndPlaylistTree.GetFirstSelectedItem()));
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

	auto channel = std::make_unique<ChannelInfo>(m_categories);
	channel->set_title(L"New Channel");
	channel->set_category(category->get_id());
	channel->set_icon_uri(utils::ICON_TEMPLATE);

	const auto& path = channel->get_icon_uri().get_icon_relative_path(theApp.GetAppPath(utils::PLUGIN_ROOT));
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

	m_channels.emplace_back(std::move(channel));
	m_cur_it = m_channels.end();

	m_wndChannelsTree.SelectItem(hNewItem);
	m_wndChannelsTree.EditLabel(hNewItem);
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

	std::vector<HTREEITEM> toDelete;
	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		auto channel = GetChannel(hItem);
		if (channel == nullptr) continue;

		toDelete.emplace_back(hItem);
		auto category = GetItemCategory(hItem);
		if (!category) continue;

		channel->erase_category(category->get_id());

		if (channel->get_categores().empty())
		{
			m_channels.erase(std::remove_if(m_channels.begin(), m_channels.end(), [channel](const auto& item)
											{
												return item.get() == channel;
											}),
							 m_channels.end());
			m_cur_it = m_channels.end();
		}
	}

	for (const auto& hItem : toDelete)
		m_wndChannelsTree.DeleteItem(hItem);

	set_allow_save();
	UpdateChannelsCount();
}

void CEdemChannelEditorDlg::OnUpdateRemoveChannel(CCmdUI* pCmdUI)
{
	BOOL enable = GetChannel(m_wndChannelsTree.GetFirstSelectedItem()) != nullptr && IsSelectedTheSameType();
	pCmdUI->Enable(enable);
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
		&& (IsChannel(hCur) || IsCategory(hCur) && m_wndChannelsTree.GetSelectedCount() == 1)
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
		&& (IsChannel(hCur) || IsCategory(hCur) && m_wndChannelsTree.GetSelectedCount() == 1)
		&& m_wndChannelsTree.GetNextSiblingItem(m_wndChannelsTree.GetLastSelectedItem()) != nullptr;

	pCmdUI->Enable(enable);
}

void CEdemChannelEditorDlg::MoveChannels(HTREEITEM hBegin, HTREEITEM hEnd, bool down)
{
	auto range_start = GetChannel(hBegin);
	auto range_end = GetChannel(hEnd);

	HTREEITEM hMoved = nullptr;
	HTREEITEM hAfter = nullptr;
	if (down)
	{
		hMoved = m_wndChannelsTree.GetNextSiblingItem(hEnd);
		hAfter = m_wndChannelsTree.GetPrevSiblingItem(hBegin);
		hAfter = hAfter ? hAfter : TVI_FIRST;

		auto it_bgn = std::find_if(m_channels.begin(), m_channels.end(), [range_start](const auto& item)
								   {
									   return item.get() == range_start;
								   });

		auto it_end = std::find_if(m_channels.begin(), m_channels.end(), [range_end](const auto& item)
								   {
									   return item.get() == range_end;
								   });

		std::rotate(it_bgn, it_end + 1, it_end + 2);
	}
	else
	{
		hMoved = m_wndChannelsTree.GetPrevSiblingItem(hBegin);
		hAfter = hEnd;

		auto it_bgn = std::find_if(m_channels.rbegin(), m_channels.rend(), [range_start](const auto& item)
								   {
									   return item.get() == range_start;
								   });

		auto it_end = std::find_if(m_channels.rbegin(), m_channels.rend(), [range_end](const auto& item)
								   {
									   return item.get() == range_end;
								   });

		std::rotate(it_end, it_bgn + 1, it_bgn + 2);
	}

	auto channel = GetChannel(hMoved);
	TVINSERTSTRUCTW tvInsert = { nullptr };
	tvInsert.hParent = m_wndChannelsTree.GetParentItem(hBegin);
	tvInsert.item.pszText = (LPWSTR)channel->get_title().c_str();
	tvInsert.item.lParam = (LPARAM)channel;
	tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
	tvInsert.hInsertAfter = hAfter;

	m_wndChannelsTree.InsertItem(&tvInsert);
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
	std::swap(m_categories[lCat->get_id()], m_categories[rCat->get_id()]);

	// swap ItemData
	DWORD_PTR data = m_wndChannelsTree.GetItemData(hRight);
	m_wndChannelsTree.SetItemData(hRight, m_wndChannelsTree.GetItemData(hLeft));
	m_wndChannelsTree.SetItemData(hLeft, data);

	// ���������� ItemData ��� ��� � ��������� �� �� �������
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

	// ������ ������� ������ ItemData ��� ����������
	idx = m_wndChannelsTree.GetItemData(hRight);
	m_wndChannelsTree.SetItemData(hRight, m_wndChannelsTree.GetItemData(hLeft));
	m_wndChannelsTree.SetItemData(hLeft, idx);

	// ���������. ����� TreeCtrl ��� �������������� ���������� ������
	TVSORTCB sortInfo = { nullptr };
	sortInfo.hParent = nullptr;
	sortInfo.lpfnCompare = &CBCompareForSwap;
	m_wndChannelsTree.SortChildrenCB(&sortInfo);

	// ��������������� �������� ItemData
	for (const auto& it : itemData)
	{
		m_wndChannelsTree.SetItemData(it.hItem, it.lParam);
	}

	// ������������� ������ ��������� ��� �������
	for (auto& channel : m_channels)
	{
		channel->rebiuld_categories();
	}

	m_wndChannelsTree.SelectItem(hLeft);

	set_allow_save();
}

void CEdemChannelEditorDlg::OnRenameChannel()
{
	if (GetFocus() == &m_wndChannelsTree)
	{
		m_wndChannelsTree.EditLabel(m_wndChannelsTree.GetSelectedItem());
	}
}

void CEdemChannelEditorDlg::OnUpdateRenameChannel(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_wndChannelsTree.GetSelectedCount() == 1 && GetChannel(m_wndChannelsTree.GetSelectedItem()) != nullptr);
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
	if (IsSelectedTheSameType())
	{
		if (IsChannel(hSelected))
		{
			LoadChannelInfo(hSelected);
			state = 2;
			m_wndIcon.EnableWindow(FALSE);
		}

		if (m_wndChannelsTree.GetSelectedCount() == 1)
		{
			if (IsChannel(hSelected))
			{
				state = 1;
				m_wndIcon.EnableWindow(TRUE);
			}
			else if (IsCategory(hSelected))
			{
				m_tvgID = 0;
				m_epgID = 0;
				m_prevDays = 0;
				m_nextDays = 0;
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
					const auto& path = category->get_icon_uri().get_icon_relative_path(theApp.GetAppPath(utils::PLUGIN_ROOT));
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
	bool bSameType = IsSelectedTheSameType();

	m_wndCategories.EnableWindow(state && bSameCategory);
	m_wndCategoriesList.EnableWindow(state && bSameCategory);
	m_wndAddToShow.EnableWindow(state && m_wndCategories.GetCount() > 0 && bSameCategory);
	m_wndRemoveFromShow.EnableWindow(state && m_wndCategoriesList.GetCurSel() != -1 && bSameCategory);
	m_wndCustom.EnableWindow(enable);
	m_wndTvgID.EnableWindow(enable);
	m_wndEpgID.EnableWindow(enable);
	m_wndArchive.EnableWindow(state);
	m_wndAdult.EnableWindow(state);
	m_wndTestTVG.EnableWindow(enable);
	m_wndTestEPG.EnableWindow(enable);
	m_wndStreamID.EnableWindow(enable && m_streamID != 0);
	m_wndStreamUrl.EnableWindow(enable && m_streamID == 0);
	m_wndCheckArchive.EnableWindow(enable && !m_probe.IsEmpty());
	m_wndPrevDays.EnableWindow(state);
	m_wndSpinPrev.EnableWindow(state);
	m_wndNextDays.EnableWindow(state);
	m_wndSpinNext.EnableWindow(state);
	m_wndInfoVideo.EnableWindow(enable);
	m_wndInfoAudio.EnableWindow(enable);

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
		m_wndGetInfo.EnableWindow(bEnable);
	}
	else
	{
		m_wndGetInfo.EnableWindow(state);
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
			FillCategories();
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

	CMenu* popup = menu.GetSubMenu(0);
	if (!popup)
		return;

	if (m_wndChannelsTree.GetSelectedCount() == 1)
		popup->SetDefaultItem(ID_PLAY_STREAM);

	CCmdUI cmdUI;
	cmdUI.m_nIndexMax = popup->GetMenuItemCount();
	for (UINT i = 0; i < cmdUI.m_nIndexMax; ++i)
	{
		cmdUI.m_nIndex = i;
		cmdUI.m_nID = popup->GetMenuItemID(i);
		cmdUI.m_pMenu = popup;
		cmdUI.DoUpdate(this, FALSE);
	}

	popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptScreen.x, ptScreen.y, this, nullptr);
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

	CMenu* popup = menu.GetSubMenu(0);
	if (!popup)
		return;

	if (m_wndPlaylistTree.GetSelectedCount() == 1)
		popup->SetDefaultItem(ID_PLAY_STREAM_PL);

	CCmdUI cmdUI;
	cmdUI.m_nIndexMax = popup->GetMenuItemCount();
	for (UINT i = 0; i < cmdUI.m_nIndexMax; ++i)
	{
		cmdUI.m_nIndex = i;
		cmdUI.m_nID = popup->GetMenuItemID(i);
		cmdUI.m_pMenu = popup;
		cmdUI.DoUpdate(this, FALSE);
	}

	popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptScreen.x, ptScreen.y, this, nullptr);
}

void CEdemChannelEditorDlg::OnBnClickedButtonAddToShowIn()
{
	int idx = m_wndCategories.GetCurSel();
	if (idx == CB_ERR)
		return;

	auto toAdd = (ChannelCategory*)m_wndCategories.GetItemData(idx);
	if (!toAdd)
		return;

	bool changed = false;
	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		auto channel = GetChannel(hItem);
		if (!channel || channel->find_category(toAdd->get_id())) continue;

		changed = true;

		channel->set_category(toAdd->get_id());

		TVINSERTSTRUCTW tvInsert = { nullptr };
		tvInsert.hParent = GetCategoryItem(toAdd->get_id());
		tvInsert.item.pszText = (LPWSTR)channel->get_title().c_str();
		tvInsert.item.lParam = (LPARAM)channel;
		tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
		m_wndChannelsTree.InsertItem(&tvInsert);

		LoadChannelInfo(hItem);
	}

	if (changed)
		set_allow_save();
}

void CEdemChannelEditorDlg::OnBnClickedButtonRemoveFromShowIn()
{
	int idx = m_wndCategoriesList.GetCurSel();
	if (idx == CB_ERR)
		return;

	if (m_wndCategoriesList.GetCount() == 1)
	{
		AfxMessageBox(_T("Channel have only one category. Just remove channel or add additional category."), MB_ICONWARNING);
		return;
	}

	auto toDelete = (ChannelCategory*)m_wndCategoriesList.GetItemData(idx);

	bool changed = false;
	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		auto channel = GetChannel(hItem);
		if (!channel) continue;

		changed = true;

		channel->erase_category(toDelete->get_id());
		auto hSub = FindTreeSubItem(m_wndChannelsTree, m_wndChannelsTree.GetChildItem(GetCategoryItem(toDelete->get_id())), (DWORD_PTR)channel);
		if (hSub)
			m_wndChannelsTree.DeleteItem(hSub);

		if (auto hFound = FindTreeItem(m_wndChannelsTree, (DWORD_PTR)channel); hFound != nullptr)
		{
			if(hItem == hFound)
				LoadChannelInfo(hItem);
		}
	}

	if (changed)
		set_allow_save();
}

void CEdemChannelEditorDlg::OnBnClickedButtonAddCategory()
{
	OnNewCategory();
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
			channel->set_has_archive(m_hasArchive);
	}

	set_allow_save();
}

void CEdemChannelEditorDlg::OnEnChangeEditTvgID()
{
	UpdateData(TRUE);
	if (m_wndChannelsTree.GetSelectedCount() > 1)
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
	if (m_wndChannelsTree.GetSelectedCount() > 1)
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
	if (m_wndChannelsTree.GetSelectedCount() > 1)
	{
		auto channel = GetChannel(m_wndChannelsTree.GetSelectedItem());
		if (channel && m_wndCustom.GetCheck())
		{
			channel->set_stream_uri(m_streamUrl.GetString());
			set_allow_save();
		}
	}
}

void CEdemChannelEditorDlg::OnEnChangeEditUrlID()
{
	UpdateData(TRUE);
	if (m_wndChannelsTree.GetSelectedCount() > 1)
	{
		auto channel = GetChannel(m_wndChannelsTree.GetSelectedItem());
		if (channel && channel->get_channel_id() != m_streamID)
		{
			channel->set_channel_id(m_streamID);
			CheckForExisting();
			set_allow_save();
		}
	}
}

void CEdemChannelEditorDlg::OnEnChangeEditNumNext()
{
	if (m_nextDays < 0)
		m_nextDays = 0;

	if (m_nextDays > 7)
		m_nextDays = 7;

	UpdateData(FALSE);

	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		auto channel = GetChannel(hItem);
		if (channel)
		{
			channel->set_next_epg_days(m_nextDays);
		}
	}

	set_allow_save();
}

void CEdemChannelEditorDlg::OnEnChangeEditNumPrev()
{
	if (m_prevDays < 0)
		m_prevDays = 0;

	if (m_prevDays > 7)
		m_prevDays = 7;

	UpdateData(FALSE);

	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		auto channel = GetChannel(hItem);
		if (channel)
		{
			channel->set_prev_epg_days(m_prevDays);
		}
	}

	set_allow_save();
}

void CEdemChannelEditorDlg::OnEnChangeEditArchiveCheck()
{
	if (m_archiveCheck < 0)
		m_archiveCheck = 0;

	UpdateData(FALSE);

	theApp.WriteProfileInt(_T("Setting"), _T("HoursBack"), m_archiveCheck);
}

void CEdemChannelEditorDlg::OnDeltaposSpinPrev(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateData(TRUE);
	m_prevDays -= reinterpret_cast<LPNMUPDOWN>(pNMHDR)->iDelta;
	UpdateData(FALSE);
	OnEnChangeEditNumPrev();
	*pResult = 0;
}

void CEdemChannelEditorDlg::OnDeltaposSpinNext(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateData(TRUE);
	m_nextDays -= reinterpret_cast<LPNMUPDOWN>(pNMHDR)->iDelta;
	UpdateData(FALSE);
	OnEnChangeEditNumNext();
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

void CEdemChannelEditorDlg::PlayStream(const std::wstring& stream_url, int archive_hour /*= 0*/) const
{
	TRACE(_T("Test URL: %s\n"), stream_url.c_str());
	CStringW test(stream_url.c_str());
	if (archive_hour)
	{
		test.Format(L"%s?utc=%d&lutc=%d", stream_url.c_str(), _time32(nullptr) - 3600 * archive_hour, _time32(nullptr));
	}

	ShellExecuteW(nullptr, L"open", m_player, test, nullptr, SW_SHOWNORMAL);
}

void CEdemChannelEditorDlg::OnBnClickedButtonCustomPlaylist()
{
	CCustomPlaylistDlg dlg;
	dlg.m_url = theApp.GetProfileString(_T("Setting"), _T("CustomPlaylist"));
	dlg.m_isFile = (m_wndPlaylistType.GetCurSel() == 3);
	if (dlg.DoModal() == IDOK)
	{
		theApp.WriteProfileString(_T("Setting"), _T("CustomPlaylist"), dlg.m_url);
		PostMessage(WN_START_LOAD_PLAYLIST, FALSE);
	}
}

bool CEdemChannelEditorDlg::AddPlaylistEntry(std::unique_ptr<PlaylistEntry>& entry, BOOL bRegex, BOOL bCase)
{
	if (!m_filterString.IsEmpty())
	{
		bool found = false;
		if (bRegex)
		{
			try
			{
				std::wregex re(m_filterString.GetString());
				found = std::regex_search(entry->get_title(), re);
			}
			catch (std::regex_error& ex)
			{
				ex;
				entry->Clear();
				return false;
			}
		}
		else
		{
			if (bCase)
			{
				found = (entry->get_title().find(m_filterString.GetString()) != std::wstring::npos);
			}
			else
			{
				found = (StrStrI(entry->get_title().c_str(), m_filterString.GetString()) != nullptr);
			}
		}

		if (found)
		{
			entry->Clear();
			return true;
		}
	}

	if (auto res = m_playlistIds.emplace(entry->get_channel_id()); !res.second)
	{
		TRACE("Duplicate channel: %s (%d)\n", entry->get_title().c_str(), entry->get_channel_id());
	}

	const auto& category = entry->get_category();
	auto found = std::find_if(m_pl_categories.begin(), m_pl_categories.end(), [category](const auto& item)
							  {
								  return category == item.first;
							  });

	if (found == m_pl_categories.end())
	{
		m_pl_categories.emplace_back(category, nullptr);
	}

	m_playlist.emplace_back(std::move(entry));
	entry = std::make_unique<PlaylistEntry>();
	return true;
}

void CEdemChannelEditorDlg::FillPlaylist()
{
	m_wndPlaylistTree.DeleteAllItems();
	// fill playlist tree
	if (!m_playlist.empty())
	{
		for (auto& category : m_pl_categories)
		{
			TVINSERTSTRUCTW tvInsert = { nullptr };
			tvInsert.hParent = TVI_ROOT;
			tvInsert.item.pszText = (LPWSTR)category.first.c_str();
			tvInsert.item.mask = TVIF_TEXT;
			auto item = m_wndPlaylistTree.InsertItem(&tvInsert);
			category.second = item;
		}

		int step = 0;
		for (const auto& item : m_playlist)
		{
			auto found = std::find_if(m_pl_categories.begin(), m_pl_categories.end(), [&item](const auto& pair)
									  {
										  return pair.first == item->get_category();
									  });

			if (found == m_pl_categories.end()) continue;

			TVINSERTSTRUCTW tvInsert = { nullptr };
			tvInsert.hParent = found->second;
			tvInsert.item.pszText = (LPWSTR)item->get_title().c_str();
			tvInsert.item.lParam = (LPARAM)item.get();
			tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
			m_wndPlaylistTree.InsertItem(&tvInsert);
		}
	}

	m_pl_cur_it = m_playlist.end();

	UpdatePlaylistCount();
	CheckForExisting();

	UpdateData(FALSE);
}

void CEdemChannelEditorDlg::OnSave()
{
	std::wstring path = theApp.GetAppPath(utils::PLUGIN_ROOT + m_chFileName).GetString();

	// ��������� ������ ��������� ���� �� ���� �����. ����� ������ ������ � �������
	// [plugin] error: invalid plugin TV info: wrong num_channels(0) for group id '' in num_channels_by_group_id.

	std::set<int> group_ids;
	for (const auto& channel : m_channels)
	{
		const auto& cats = channel->get_category_ids();
		group_ids.insert(cats.begin(), cats.end());
	}

	bool need_reload = false;
	for (auto it = m_categories.begin(); it != m_categories.end();)
	{
		if (group_ids.find(it->first) == group_ids.end())
		{
			it = m_categories.erase(it);
			need_reload = true;
		}
		else
			++it;
	}

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
		for (auto& category : m_categories)
		{
			cat_node->append_node(category.second->GetNode(doc));
		}
		// append <tv_categories> to <tv_info> node
		tv_info->append_node(cat_node);

		// create <tv_channels> node
		auto ch_node = doc.allocate_node(rapidxml::node_element, utils::TV_CHANNELS);
		// append <tv_channel> to <tv_channels> node
		for (auto& channel : m_channels)
		{
			ch_node->append_node(channel->GetNode(doc));
		}
		// append <tv_channel> to <tv_info> node
		tv_info->append_node(ch_node);

		doc.append_node(tv_info);

		// write document
		std::ofstream os(path, std::istream::binary);
		os << doc;

		set_allow_save(FALSE);
		if (need_reload)
		{
			FillCategories();
			FillChannels();
		}
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

	const auto& path = newCategory->get_icon_uri().get_icon_relative_path(theApp.GetAppPath(utils::PLUGIN_ROOT));
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

	m_categories.emplace(id, std::move(newCategory));

	FillCategories();

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

	std::vector<ChannelCategory*> channels;
	std::vector<HTREEITEM> toDelete;
	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		auto category = GetCategory(hItem);
		if (category)
		{
			channels.emplace_back(category);
		}

		toDelete.emplace_back(hItem);
	}

	for (const auto& hItem : toDelete)
		m_wndChannelsTree.DeleteItem(hItem);

	for (auto& channel : m_channels)
	{
		for (const auto& category : channels)
		{
			channel->erase_category(category->get_id());
		}

		if (channel->get_categores().empty())
		{
			channel.release();
		}
	}

	for (const auto& category : channels)
	{
		m_categories.erase(category->get_id());
	}

	m_channels.erase(std::remove_if(m_channels.begin(), m_channels.end(), [](const auto& elem) { return elem == nullptr; }), m_channels.end());

	UpdateChannelsCount();
	FillCategories();
	set_allow_save();
}

void CEdemChannelEditorDlg::OnUpdateRemoveCategory(CCmdUI* pCmdUI)
{
	BOOL enable = GetCategory(m_wndChannelsTree.GetFirstSelectedItem()) != nullptr && IsSelectedTheSameType();
	pCmdUI->Enable(enable);
}

void CEdemChannelEditorDlg::OnStnClickedStaticIcon()
{
	auto hCur = m_wndChannelsTree.GetSelectedItem();
	if (!hCur)
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
		CString newPath = file.Left(file.GetLength() - _tcslen(oFN.lpstrFileTitle));
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
				channel->set_icon_uri(m_iconUrl.GetString());
				const auto& path = channel->get_icon_uri().get_icon_relative_path(theApp.GetAppPath(utils::PLUGIN_ROOT));
				CImage img;
				if (theApp.LoadImage(path.c_str(), img))
				{
					channel->set_icon(img);
				}
			}
		}
		else
		{
			auto category = GetCategory(hCur);
			if (category && m_iconUrl != category->get_icon_uri().get_uri().c_str())
			{
				category->set_icon_uri(m_iconUrl.GetString());
				const auto& path = category->get_icon_uri().get_icon_relative_path(theApp.GetAppPath(utils::PLUGIN_ROOT));
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
	CString dllFile = theApp.GetAppPath(_T("..\\dll\\7za.dll"));
#else
	CString dllFile = theApp.GetAppPath(_T("7za.dll"));
#endif // _DEBUG

	CString plugin_folder = theApp.GetAppPath(utils::PLUGIN_ROOT);

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

	HTREEITEM hSelected = nullptr;
	auto it = m_cur_it;

	if (m_search.GetLength() > 1 && m_search.GetAt(0) == '\\')
	{
		// plain search because no duplicate channels and search next not possible
		int id = _tstoi(m_search.Mid(1).GetString());
		for (it = m_channels.begin(); it != m_channels.end(); ++it)
		{
			const auto& entry = *it;
			if (entry && entry->get_channel_id() == id)
			{
				hSelected = FindTreeItem(m_wndChannelsTree, (DWORD_PTR)entry.get());
				if (hSelected) break;
			}
		}

		it = m_cur_it;
	}
	else
	{
		// cyclic search thru playlist
		if (it != m_channels.end())
		{
			++it;
		}
		else
		{
			m_cur_it = it = m_channels.begin();
		}

		do
		{
			// check whether the current item is the searched one
			const auto& entry = *it;
			if (entry && StrStrI(entry->get_title().c_str(), m_search.GetString()) != nullptr)
			{
				hSelected = FindTreeItem(m_wndChannelsTree, (DWORD_PTR)entry.get());
				if (hSelected) break;
			}

			if (++it == m_channels.end())
			{
				it = m_channels.begin();
			}
		} while (it != m_cur_it);
	}

	if (hSelected)
	{
		m_cur_it = it;
		m_wndChannelsTree.SelectItem(hSelected);
		m_wndChannelsTree.Expand(m_wndChannelsTree.GetParentItem(hSelected), TVE_EXPAND);
		m_wndChannelsTree.EnsureVisible(hSelected);
	}
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

	HTREEITEM hSelected = nullptr;
	auto it = m_pl_cur_it;

	if (m_plSearch.GetLength() > 1 && m_plSearch.GetAt(0) == '\\')
	{
		// plain search because no duplicate channels and search next not possible
		int id = _tstoi(m_plSearch.Mid(1).GetString());
		for (it = m_playlist.begin(); it != m_playlist.end(); ++it)
		{
			const auto& entry = *it;
			if (entry && entry->get_channel_id() == id)
			{
				hSelected = FindTreeItem(m_wndPlaylistTree, (DWORD_PTR)entry.get());
				if (hSelected) break;
			}
		}

		it = m_pl_cur_it;
	}
	else
	{
		// cyclic search thru playlist
		if (it != m_playlist.end())
		{
			++it;
		}
		else
		{
			m_pl_cur_it = it = m_playlist.begin();
		}

		do
		{
			// check whether the current item is the searched one
			const auto& entry = *it;
			if (entry && StrStrI(entry->get_title().c_str(), m_plSearch.GetString()) != nullptr)
			{
				hSelected = FindTreeItem(m_wndPlaylistTree, (DWORD_PTR)entry.get());
				if (hSelected) break;
			}

			if (++it == m_playlist.end())
			{
				it = m_playlist.begin();
			}
		} while (it != m_pl_cur_it);
	}

	if (hSelected)
	{
		m_pl_cur_it = it;
		m_wndPlaylistTree.SelectItem(hSelected);
		m_wndPlaylistTree.Expand(m_wndPlaylistTree.GetParentItem(hSelected), TVE_EXPAND);
		m_wndPlaylistTree.EnsureVisible(hSelected);
	}
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

const ChannelInfo* CEdemChannelEditorDlg::FindChannelByEntry(const PlaylistEntry* entry) const
{
	int ch_id = entry->get_channel_id();
	auto found = std::find_if(m_channels.begin(), m_channels.end(), [ch_id](const auto& item)
							  {
								  return item->get_channel_id() == ch_id;
							  });

	return found != m_channels.end() ? (*found).get() : nullptr;
}

bool CEdemChannelEditorDlg::IsCategoryInChannels(const ChannelCategory* category) const
{
	if (!category)
		return false;

	auto id = category->get_id();
	auto found = std::find_if(m_channels.begin(), m_channels.end(), [id](const auto& channel)
							  {
								  return channel->find_category(id) != nullptr;
							  });

	return found != m_channels.end();
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
		OnSyncEntry();
	}

	if (needCheckExisting)
		CheckForExisting();

	UpdateChannelsCount();
	set_allow_save();
}

void CEdemChannelEditorDlg::OnUpdateAddUpdateChannel(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsSelectedTheSameType() && IsPlaylistEntry(m_wndPlaylistTree.GetFirstSelectedItem()));
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
		OnSyncEntry();
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
			enable = channel->get_channel_id() == entry->get_channel_id() && channel->get_icon_uri() != entry->get_icon_uri();
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
		std::wstring path = utils::CHANNELS_LOGO_URL;
		path += fname;

		uri icon_uri;
		icon_uri.set_uri(utils::ICON_TEMPLATE);
		icon_uri.set_path(path);

		std::vector<BYTE> image;
		if (!utils::DownloadFile(channel->get_icon_uri().get_uri(), image)) continue;

		std::wstring fullPath = icon_uri.get_icon_relative_path(theApp.GetAppPath(utils::PLUGIN_ROOT));
		std::ofstream os(fullPath.c_str(), std::ios::out | std::ios::binary);
		os.write((char*)&image[0], image.size());
		os.close();

		CImage img;
		if (theApp.LoadImage(fullPath.c_str(), img))
		{
			channel->set_icon_uri(icon_uri.get_uri().c_str());
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
		CFile cFile;
		CFileException ex;
		if (cFile.Open(dlg.GetPathName(), CFile::modeWrite | CFile::modeCreate | CFile::typeBinary | CFile::shareDenyRead, &ex))
		{
			cFile.Close();
			auto found = std::find_if(m_all_playlists.begin(), m_all_playlists.end(), [&dlg](const auto& item)
									  {
										  return item.first == dlg.GetFileName();
									  });

			if (found == m_all_playlists.end())
			{
				m_channels.clear();
				m_categories.clear();
				const auto& pair = m_all_playlists.emplace_back(dlg.GetFileName(), dlg.GetPathName());
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

	int step = 0;
	std::string audio;
	std::string video;
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

		m_wndProgress.SetRange32(0, channels.size());
		auto it = channels.begin();
		while (it != channels.end())
		{
			std::array<std::thread, 5> workers;
			std::array<std::string, 5> audio;
			std::array<std::string, 5> video;
			auto pool = it;
			int j = 0;
			while (j < 5 && pool != channels.end())
			{
				const auto& url = (*pool)->get_stream_uri().get_ts_translated_url();
				workers[j] = std::thread(GetChannelStreamInfo, url, std::ref(audio[j]), std::ref(video[j]));
				j++;
				++pool;
			}

			j = 0;
			for (auto& w : workers)
			{
				if (w.joinable())
				{
					m_wndProgress.SetPos(++step);
					w.join();
					(*it)->set_audio(audio[j]);
					(*it)->set_video(video[j]);
					++it;
					j++;
				}
			}
		}
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

		m_wndProgress.SetRange32(0, playlist.size());
		auto it = playlist.begin();
		while (it != playlist.end())
		{
			std::array<std::thread, 5> workers;
			std::array<std::string, 5> audio;
			std::array<std::string, 5> video;
			auto group = it;
			int j = 0;
			while (j < 5 && group != playlist.end())
			{
				const auto& url = (*group)->get_stream_uri().get_ts_translated_url();
				workers[j] = std::thread(GetChannelStreamInfo, url, std::ref(audio[j]), std::ref(video[j]));
				j++;
				++group;
			}

			j = 0;
			for (auto& w : workers)
			{
				if (w.joinable())
				{
					m_wndProgress.SetPos(++step);
					w.join();
					(*it)->set_audio(audio[j]);
					(*it)->set_video(video[j]);
					++it;
					j++;
				}
			}
		}
	}

	m_infoAudio = audio.c_str();
	m_infoVideo = video.c_str();

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

	enable = enable && IsSelectedTheSameType();

	pCmdUI->Enable(enable);
}

void CEdemChannelEditorDlg::OnGetStreamInfoAll()
{
	CWaitCursor cur;
	m_wndProgress.ShowWindow(SW_SHOW);
	int step = 0;
	StreamContainer* container = nullptr;
	if (GetFocus() == &m_wndChannelsTree)
	{
		std::vector<ChannelInfo*> channels;
		auto category = GetItemCategory(m_wndChannelsTree.GetSelectedItem());
		for (const auto& channel : m_channels)
		{
			if (category == nullptr || channel->find_category(category->get_id()))
			{
				// add all
				channels.emplace_back(channel.get());
			}
		}

		size_t sz = channels.size();
		m_wndProgress.SetRange32(0, sz);
		auto it = channels.begin();
		while (it != channels.end())
		{
			std::array<std::thread, 5> workers;
			std::array<std::string, 5> audio;
			std::array<std::string, 5> video;
			auto pool = it;
			int j = 0;
			while (j < 5 && pool != channels.end())
			{
				m_chInfo.Format(_T("Channels: %s (%d) %d"), m_chFileName.GetString(), channels.size(), std::distance(channels.begin(), pool) + 1);
				UpdateData(FALSE);

				const auto& url = (*pool)->get_stream_uri().get_ts_translated_url();
				workers[j] = std::thread(GetChannelStreamInfo, url, std::ref(audio[j]), std::ref(video[j]));
				j++;
				++pool;
			}

			j = 0;
			for (auto& w : workers)
			{
				if (w.joinable())
				{
					m_wndProgress.SetPos(++step);
					w.join();
					(*it)->set_audio(audio[j]);
					(*it)->set_video(video[j]);
					++it;
					j++;
				}
			}
		}

		LoadChannelInfo(m_wndChannelsTree.GetFirstSelectedItem());
		UpdateChannelsCount();
	}
	else if (GetFocus() == &m_wndPlaylistTree)
	{
		std::vector<PlaylistEntry*> playlist;

		auto curEntry = GetPlaylistEntry(m_wndPlaylistTree.GetSelectedItem());
		for (const auto& entry : m_playlist)
		{
			if (curEntry == nullptr || curEntry->get_category() == entry->get_category())
			{
				// add all
				playlist.emplace_back(entry.get());
			}
		}

		size_t sz = playlist.size();
		m_wndProgress.SetRange32(0, sz);
		auto it = playlist.begin();
		while (it != playlist.end())
		{
			std::array<std::thread, 5> workers;
			std::array<std::string, 5> audio;
			std::array<std::string, 5> video;
			auto pool = it;
			int j = 0;
			while (j < 5 && pool != playlist.end())
			{
				m_plInfo.Format(_T("Playlist: %s (%d) %d"), m_plFileName.GetString(), playlist.size(), std::distance(playlist.begin(), pool) + 1);
				UpdateData(FALSE);

				const auto& url = (*pool)->get_stream_uri().get_ts_translated_url();
				workers[j] = std::thread(GetChannelStreamInfo, url, std::ref(audio[j]), std::ref(video[j]));
				j++;
				++pool;
			}

			j = 0;
			for (auto& w : workers)
			{
				if (w.joinable())
				{
					m_wndProgress.SetPos(++step);
					w.join();
					(*it)->set_audio(audio[j]);
					(*it)->set_video(video[j]);
					++it;
					j++;
				}
			}
		}
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

void CEdemChannelEditorDlg::OnSyncEntry()
{
	auto entry = GetPlaylistEntry(m_wndPlaylistTree.GetSelectedItem());
	if (!entry)
		return;

	int id = entry->get_channel_id();
	auto found = std::find_if(m_channels.begin(), m_channels.end(), [id](const auto& channel)
							  {
								  return channel->get_channel_id() == id;
							  });

	if (found != m_channels.end())
	{
		if (auto hSelected = FindTreeItem(m_wndChannelsTree, (DWORD_PTR)found->get()); hSelected != nullptr)
		{
			m_wndChannelsTree.SelectItem(hSelected);
			LoadChannelInfo(hSelected);
		}
	}
}

void CEdemChannelEditorDlg::OnUpdateSyncEntry(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_bAutoSync && m_wndPlaylistTree.GetSelectedCount() == 1 && GetPlaylistEntry(m_wndPlaylistTree.GetSelectedItem()) != nullptr);
}

void CEdemChannelEditorDlg::OnToggleChannel()
{
	for (const auto& hItem : m_wndChannelsTree.GetSelectedItems())
	{
		if (auto channel = GetChannel(hItem); channel != nullptr)
		{
			channel->set_disabled(!m_menu_enable_channel);
			CRect rc;
			m_wndChannelsTree.GetItemRect(hItem, rc, FALSE);
			m_wndChannelsTree.InvalidateRect(rc, FALSE);
			set_allow_save();
		}
	}
}

void CEdemChannelEditorDlg::OnUpdateToggleChannel(CCmdUI* pCmdUI)
{
	BOOL enable = FALSE;
	if (IsSelectedTheSameType())
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
	PostMessage(WN_START_LOAD_PLAYLIST, TRUE);
}

void CEdemChannelEditorDlg::OnCbnSelchangeComboPlaylist()
{
	CString url;
	int idx = m_wndPlaylistType.GetCurSel();
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

	PostMessage(WN_START_LOAD_PLAYLIST, FALSE);
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

bool CEdemChannelEditorDlg::AddChannel(HTREEITEM hSelectedItem)
{
	bool needCheckExisting = false;

	auto entry = GetPlaylistEntry(hSelectedItem);
	if (!entry) return needCheckExisting;

	HTREEITEM hFoundItem = nullptr;

	auto found = FindChannelByEntry(entry);
	if (found == nullptr)
	{
		// Search for existing category
		int categoryId = -1;
		for (const auto& category : m_categories)
		{
			if (category.second->get_caption() == entry->get_category())
			{
				categoryId = category.first;
				break;
			}
		}

		// Create new channel
		auto channel = std::make_unique<ChannelInfo>(m_categories);
		channel->set_channel_id(entry->get_channel_id());

		auto tree_categories = GetCategoriesTreeMap();

		if (categoryId == -1)
		{
			// Category not exist, create new
			categoryId = GetNewCategoryID();
			auto newCategory = std::make_unique<ChannelCategory>();
			newCategory->set_id(categoryId);
			newCategory->set_caption(entry->get_category());
			auto res = m_categories.emplace(categoryId, std::move(newCategory));
			auto param = res.first->second.get();

			TVINSERTSTRUCTW tvInsert = { nullptr };
			tvInsert.hParent = TVI_ROOT;
			tvInsert.item.pszText = (LPWSTR)param->get_caption().c_str();
			tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
			tvInsert.item.lParam = categoryId;
			auto hItem = m_wndChannelsTree.InsertItem(&tvInsert);

			tree_categories.emplace(categoryId, hItem);
		}

		TVINSERTSTRUCTW tvInsert = { nullptr };
		tvInsert.hParent = tree_categories[categoryId];
		tvInsert.item.pszText = (LPWSTR)channel->get_title().c_str();
		tvInsert.item.lParam = (LPARAM)channel.get();
		tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
		hFoundItem = m_wndChannelsTree.InsertItem(&tvInsert);

		needCheckExisting = true;
		m_channels.emplace_back(std::move(channel));
		m_cur_it = m_channels.end();
	}
	else
	{
		hFoundItem = FindTreeItem(m_wndChannelsTree, (DWORD_PTR)found);
	}

	auto channel = GetChannel(hFoundItem);
	if (channel->get_title() != entry->get_title())
	{
		channel->set_title(entry->get_title());
		// Search if channel present in other leafs
		while (hFoundItem != nullptr)
		{
			m_wndChannelsTree.SetItemText(hFoundItem, channel->get_title().c_str());
			hFoundItem = FindTreeNextItem(m_wndChannelsTree, hFoundItem, (DWORD_PTR)channel);
		}
	}

	if (auto id = entry->get_tvg_id(); id != -1)
		channel->set_epg_id(id);

	channel->set_has_archive(entry->is_archive() != 0);
	channel->set_stream_uri(entry->get_stream_uri());
	if (entry->get_icon_uri() != channel->get_icon_uri() && !channel->get_icon_uri().get_uri().empty())
	{
		channel->set_icon_uri(entry->get_icon_uri());
		channel->copy_icon(entry->get_icon());
	}

	const auto& cat_name = entry->get_category();
	if (cat_name.find(L"�������") != std::wstring::npos)
	{
		channel->set_adult(TRUE);
	}

	auto it = std::find_if(m_categories.begin(), m_categories.end(), [cat_name](const auto& category)
						   {
							   return (0 == _wcsicmp(category.second->get_caption().c_str(), cat_name.c_str()));
						   });

	if (it != m_categories.end())
		channel->set_category(it->second->get_id());

	return needCheckExisting;
}

std::wstring CEdemChannelEditorDlg::TranslateStreamUri(const std::wstring& stream_uri)
{
	// http://ts://{SUBDOMAIN}/iptv/{UID}/205/index.m3u8 -> http://ts://domain.com/iptv/000000000000/205/index.m3u8

	std::wregex re_domain(LR"(\{SUBDOMAIN\})");
	std::wregex re_uid(LR"(\{UID\})");

	std::wstring stream_url(stream_uri);
	stream_url = std::regex_replace(stream_url, re_domain, CEdemChannelEditorDlg::GetAccessDomain().GetString());
	return std::regex_replace(stream_url, re_uid, CEdemChannelEditorDlg::GetAccessKey().GetString());
}

void CEdemChannelEditorDlg::GetChannelStreamInfo(const std::wstring& url, std::string& audio, std::string& video)
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
	csCommand.Format(_T("\"%s\" -hide_banner -show_streams %s"), CEdemChannelEditorDlg::m_probe.GetString(), TranslateStreamUri(url).c_str());

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
				// �� ����� �� �������� ���������
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
	int idx = m_wndChannels.GetCurSel();
	LoadChannels((LPCTSTR)m_wndChannels.GetItemData(idx));
	FillCategories();
	FillChannels();
	GetDlgItem(IDC_BUTTON_ADD_NEW_CHANNELS_LIST)->EnableWindow(idx > 0);
	theApp.WriteProfileInt(_T("Setting"), _T("ChannelsType"), idx);
}

void CEdemChannelEditorDlg::OnBnClickedButtonPlFilter()
{
	CFilterDialog dlg;
	if (dlg.DoModal() == IDOK)
	{
		PostMessage(WN_START_LOAD_PLAYLIST, FALSE);
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
			if (isChannel && !channel || !isChannel && channel)
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
			if (isEntry && !entry || !isEntry && entry)
				return false;
		}
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

void CEdemChannelEditorDlg::OnLbnSelchangeListCategories()
{
	int selected = m_wndChannelsTree.GetSelectedCount();
	int state = selected ? (selected > 1 ? 2 : 1) : 0;
	bool bSameCategory = IsSelectedTheSameCategory();
	bool bSameType = IsSelectedTheSameType();

	m_wndRemoveFromShow.EnableWindow(state && m_wndCategoriesList.GetCurSel() != -1 && bSameCategory);
}
