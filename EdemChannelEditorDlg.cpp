
// EdemChannelEditorDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "framework.h"
#include <afxdialogex.h>

#include "EdemChannelEditor.h"
#include "EdemChannelEditorDlg.h"
#include "AboutDlg.h"
#include "SettingsDlg.h"
#include "utils.h"

#include "rapidxml.hpp"
#include "rapidxml_print.hpp"

#include "SevenZip/7zip/SevenZipWrapper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CString CEdemChannelEditorDlg::m_domain;
CString CEdemChannelEditorDlg::m_accessKey;
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
	ON_BN_CLICKED(IDC_BUTTON_ADD_TO_SHOW, &CEdemChannelEditorDlg::OnBnClickedButtonAddToShowIn)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_ADD_TO_SHOW, &CEdemChannelEditorDlg::OnUpdateButtonAddToShowIn)
	ON_BN_CLICKED(IDC_BUTTON_IMPORT, &CEdemChannelEditorDlg::OnCreateUpdateChannel)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_IMPORT, &CEdemChannelEditorDlg::OnUpdateCreateUpdateChannel)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_PLAYLIST, &CEdemChannelEditorDlg::OnBnClickedButtonLoadPlaylist)
	ON_BN_CLICKED(IDC_BUTTON_PL_SEARCH_NEXT, &CEdemChannelEditorDlg::OnBnClickedButtonPlSearchNext)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_PL_SEARCH_NEXT, &CEdemChannelEditorDlg::OnUpdateButtonPlSearchNext)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_FROM_SHOW, &CEdemChannelEditorDlg::OnBnClickedButtonRemoveFromShowIn)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_REMOVE_FROM_SHOW, &CEdemChannelEditorDlg::OnUpdateButtonRemoveFromShow)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH_NEXT, &CEdemChannelEditorDlg::OnBnClickedButtonSearchNext)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_SEARCH_NEXT, &CEdemChannelEditorDlg::OnUpdateButtonSearchNext)
	ON_EN_CHANGE(IDC_EDIT_TVG_ID, &CEdemChannelEditorDlg::OnChanges)
	ON_BN_CLICKED(IDC_BUTTON_TEST_TVG, &CEdemChannelEditorDlg::OnBnClickedButtonTestTvg)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_TEST_TVG, &CEdemChannelEditorDlg::OnUpdateButtonTestEpg)
	ON_BN_CLICKED(IDC_BUTTON_TEST_EPG, &CEdemChannelEditorDlg::OnBnClickedButtonTestEpg)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_TEST_EPG, &CEdemChannelEditorDlg::OnUpdateButtonTestEpg)
	ON_BN_CLICKED(IDC_CHECK_ADULT, &CEdemChannelEditorDlg::OnChanges)
	ON_BN_CLICKED(IDC_CHECK_ARCHIVE, &CEdemChannelEditorDlg::OnChanges)
	ON_BN_CLICKED(IDC_CHECK_CUSTOMIZE, &CEdemChannelEditorDlg::OnBnClickedCheckCustomize)
	ON_EN_CHANGE(IDC_EDIT_NEXT_EPG, &CEdemChannelEditorDlg::OnEnChangeEditNum)
	ON_EN_CHANGE(IDC_EDIT_PREV_EPG, &CEdemChannelEditorDlg::OnEnChangeEditNum)
	ON_EN_CHANGE(IDC_EDIT_STREAM_URL, &CEdemChannelEditorDlg::OnChanges)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_NEXT, &CEdemChannelEditorDlg::OnDeltaposSpinNext)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_PREV, &CEdemChannelEditorDlg::OnDeltaposSpinPrev)
	ON_EN_CHANGE(IDC_EDIT_URL_ID, &CEdemChannelEditorDlg::OnChanges)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CEdemChannelEditorDlg::OnBnClickedButtonSave)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_SAVE, &CEdemChannelEditorDlg::OnUpdateButtonSave)
	ON_STN_CLICKED(IDC_STATIC_ICON, &CEdemChannelEditorDlg::OnStnClickedStaticIcon)
	ON_BN_CLICKED(IDC_BUTTON_PACK, &CEdemChannelEditorDlg::OnBnClickedButtonPack)
	ON_BN_CLICKED(IDC_BUTTON_SETTINGS, &CEdemChannelEditorDlg::OnBnClickedButtonSettings)
	ON_BN_CLICKED(IDC_BUTTON_CACHE_ICON, &CEdemChannelEditorDlg::OnBnClickedButtonCacheIcon)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_CACHE_ICON, &CEdemChannelEditorDlg::OnUpdateButtonCacheIcon)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE_ICON, &CEdemChannelEditorDlg::OnBnClickedButtonUpdateIcon)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_UPDATE_ICON, &CEdemChannelEditorDlg::OnUpdateButtonUpdateIcon)
	ON_BN_CLICKED(IDC_BUTTON_DOWNLOAD_PLAYLIST, &CEdemChannelEditorDlg::OnBnClickedButtonDownloadPlaylist)
	ON_CBN_SELCHANGE(IDC_COMBO_PLAYLIST, &CEdemChannelEditorDlg::OnCbnSelchangeComboPlaylist)
	ON_BN_CLICKED(IDC_BUTTON_GET_INFO, &CEdemChannelEditorDlg::OnGetChannelStreamInfo)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_GET_INFO, &CEdemChannelEditorDlg::OnUpdateGetChannelStreamInfo)
	ON_BN_CLICKED(IDC_BUTTON_GET_INFO_PL, &CEdemChannelEditorDlg::OnBnClickedButtonGetInfoPl)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_GET_INFO_PL, &CEdemChannelEditorDlg::OnUpdateButtonGetInfoPl)

	ON_NOTIFY(TVN_SELCHANGING, IDC_TREE_CHANNELS, &CEdemChannelEditorDlg::OnTvnSelchangingTreeChannels)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_CHANNELS, &CEdemChannelEditorDlg::OnTvnSelchangedTreeChannels)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_CHANNELS, &CEdemChannelEditorDlg::OnNMDblclkTreeChannels)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_TREE_CHANNELS, &CEdemChannelEditorDlg::OnTvnEndlabeleditTreeChannels)
	ON_NOTIFY(NM_RCLICK, IDC_TREE_CHANNELS, &CEdemChannelEditorDlg::OnNMRclickTreeChannel)

	ON_NOTIFY(NM_DBLCLK, IDC_TREE_PLAYLIST, &CEdemChannelEditorDlg::OnNMDblclkTreePaylist)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_PLAYLIST, &CEdemChannelEditorDlg::OnTvnSelchangedTreePaylist)
	ON_NOTIFY(NM_RCLICK, IDC_TREE_PLAYLIST, &CEdemChannelEditorDlg::OnNMRclickTreePlaylist)

	ON_COMMAND(ID_SAVE, &CEdemChannelEditorDlg::OnBnClickedButtonSave)
	ON_COMMAND(ID_EDIT_RENAME, &CEdemChannelEditorDlg::OnTreeItemRename)
	ON_COMMAND(ID_REMOVE_CHANNEL, &CEdemChannelEditorDlg::OnRemoveChannel)
	ON_UPDATE_COMMAND_UI(ID_REMOVE_CHANNEL, &CEdemChannelEditorDlg::OnUpdateRemoveChannel)
	ON_COMMAND(ID_ADD_CHANNEL, &CEdemChannelEditorDlg::OnAddChannel)
	ON_COMMAND(ID_UPDATE_CHANNEL, &CEdemChannelEditorDlg::OnCreateUpdateChannel)
	ON_UPDATE_COMMAND_UI(ID_UPDATE_CHANNEL, &CEdemChannelEditorDlg::OnUpdateCreateUpdateChannel)
	ON_COMMAND(ID_UPDATE_ICON, &CEdemChannelEditorDlg::OnBnClickedButtonUpdateIcon)
	ON_COMMAND(ID_CHANNEL_UP, &CEdemChannelEditorDlg::OnChannelUp)
	ON_UPDATE_COMMAND_UI(ID_CHANNEL_UP, &CEdemChannelEditorDlg::OnUpdateChannelUp)
	ON_COMMAND(ID_CHANNEL_DOWN, &CEdemChannelEditorDlg::OnChannelDown)
	ON_UPDATE_COMMAND_UI(ID_CHANNEL_DOWN, &CEdemChannelEditorDlg::OnUpdateChannelDown)
	ON_COMMAND(ID_ADD_CATEGORY, &CEdemChannelEditorDlg::OnAddCategory)
	ON_COMMAND(ID_REMOVE_CATEGORY, &CEdemChannelEditorDlg::OnRemoveCategory)
	ON_UPDATE_COMMAND_UI(ID_REMOVE_CATEGORY, &CEdemChannelEditorDlg::OnUpdateRemoveCategory)
	ON_COMMAND(ID_TOGGLE_CHANNEL, &CEdemChannelEditorDlg::OnToggleChannel)
	ON_UPDATE_COMMAND_UI(ID_TOGGLE_CHANNEL, &CEdemChannelEditorDlg::OnUpdateToggleChannel)
	ON_COMMAND(ID_GET_STREAM_INFO, &CEdemChannelEditorDlg::OnGetChannelStreamInfo)
	ON_UPDATE_COMMAND_UI(ID_GET_STREAM_INFO, &CEdemChannelEditorDlg::OnUpdateGetChannelStreamInfo)
	ON_COMMAND(ID_GET_STREAM_INFO_PL, &CEdemChannelEditorDlg::OnGetChannelStreamInfoPl)
	ON_UPDATE_COMMAND_UI(ID_GET_STREAM_INFO_PL, &CEdemChannelEditorDlg::OnUpdateGetChannelStreamInfoPl)
	ON_COMMAND(ID_PLAY_STREAM, &CEdemChannelEditorDlg::OnPlayChannelStream)
	ON_UPDATE_COMMAND_UI(ID_PLAY_STREAM, &CEdemChannelEditorDlg::OnUpdatePlayChannelStream)
	ON_COMMAND(ID_PLAY_STREAM_PL, &CEdemChannelEditorDlg::OnPlayChannelStreamPl)
	ON_UPDATE_COMMAND_UI(ID_PLAY_STREAM_PL, &CEdemChannelEditorDlg::OnUpdatePlayChannelStreamPl)
	ON_COMMAND(ID_SORTBY_NAME, &CEdemChannelEditorDlg::OnSortByName)
	ON_COMMAND(ID_SORTBY_CHANNEL_ID, &CEdemChannelEditorDlg::OnSortByChannelID)

	ON_MESSAGE_VOID(WM_KICKIDLE, OnKickIdle)
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
	DDX_Text(pDX, IDC_EDIT_TVG_ID, m_tvgID);
	DDX_Control(pDX, IDC_BUTTON_TEST_TVG, m_wndTestTVG);
	DDX_Text(pDX, IDC_EDIT_EPG_ID, m_epgID);
	DDX_Control(pDX, IDC_BUTTON_TEST_EPG, m_wndTestEPG);
	DDX_Control(pDX, IDC_EDIT_URL_ID, m_wndStreamID);
	DDX_Control(pDX, IDC_EDIT_STREAM_URL, m_wndStreamUrl);
	DDX_Text(pDX, IDC_EDIT_STREAM_URL, m_streamUrl);
	DDX_Text(pDX, IDC_EDIT_PREV_EPG, m_prevDays);
	DDX_Text(pDX, IDC_EDIT_NEXT_EPG, m_nextDays);
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
	DDX_Text(pDX, IDC_EDIT_INFO_AUDIO, m_infoAudio);
	DDX_Text(pDX, IDC_STATIC_CHANNELS, m_channelsInfo);
	DDX_Control(pDX, IDC_BUTTON_GET_INFO, m_wndGetInfo);
	DDX_Control(pDX, IDC_COMBO_PLAYLIST, m_wndPlaylistType);
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

	m_accessKey = theApp.GetProfileString(_T("Setting"), _T("AccessKey"));
	m_domain = theApp.GetProfileString(_T("Setting"), _T("Domain"));
	m_player = theApp.GetProfileString(_T("Setting"), _T("Player"));
	m_probe = theApp.GetProfileString(_T("Setting"), _T("FFProbe"));

	if (LoadChannels(theApp.GetAppPath(utils::CHANNELS_CONFIG).GetString()))
	{
		FillCategories();
		FillChannels();
	}

	m_wndPlaylistType.SetCurSel(theApp.GetProfileInt(_T("Setting"), _T("PlaylistType"), 0));
	OnCbnSelchangeComboPlaylist();
	LoadPlaylist(theApp.GetProfileString(_T("Setting"), _T("Playlist")));
	FillPlaylist();

	if (m_current == nullptr && !m_channels.empty())
	{
		SetCurrentChannel(FindTreeItem(m_wndChannelsTree, (DWORD_PTR)m_channels[0].get()));
	}

	UpdateData(FALSE);

	set_allow_save(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
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
}

void CEdemChannelEditorDlg::UpdateChannelsCount()
{
	m_channelsInfo.Format(_T("Plugin channels (%d)"), m_channels.size());
	UpdateData(FALSE);
}

void CEdemChannelEditorDlg::SaveChannels()
{
	SaveChannelInfo();

	std::wstring path = theApp.GetAppPath(utils::CHANNELS_CONFIG).GetString();

	// Категория должна содержать хотя бы один канал. Иначе плагин падает с ошибкой
	// [plugin] error: invalid plugin TV info: wrong num_channels(0) for group id '' in num_channels_by_group_id.

	std::set<int> group_ids;
	for (const auto& channel : m_channels)
	{
		const auto& cats = channel->get_category_ids();
		group_ids.insert(cats.begin(), cats.end());
	}

	for (auto it = m_categories.begin(); it != m_categories.end();)
	{
		if (group_ids.find(it->first) == group_ids.end())
			it = m_categories.erase(it);
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
		auto root = doc.allocate_node(rapidxml::node_element, utils::TV_INFO);

		// create <tv_categories> node
		auto cat_node = doc.allocate_node(rapidxml::node_element, utils::TV_CATEGORIES);

		// append <tv_category> to <tv_categories> node
		for (auto& category : m_categories)
		{
			cat_node->append_node(category.second->GetNode(doc));
		}
		// append <tv_categories> to <tv_info> node
		root->append_node(cat_node);

		// create <tv_channels> node
		auto ch_node = doc.allocate_node(rapidxml::node_element, utils::TV_CHANNELS);
		// append <tv_channel> to <tv_channels> node
		for (auto& channel : m_channels)
		{
			ch_node->append_node(channel->GetNode(doc));
		}
		// append <tv_channel> to <tv_info> node
		root->append_node(ch_node);

		doc.append_node(root);

		// write document
		std::ofstream os(path, std::istream::binary);
		os << doc;

		set_allow_save(FALSE);
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
		item->set_colored(m_playlist.find(id) != m_playlist.end());
		ids.emplace(id);
	}

	for (auto& item : m_playlist)
	{
		item.second->set_colored(ids.find(item.first) == ids.end());
	}
}

void CEdemChannelEditorDlg::LoadChannelInfo()
{
	TRACE("LoadChannelInfo\n");

	m_infoAudio.Empty();
	m_infoVideo.Empty();

	auto channel = GetChannel(m_current);
	if (!channel)
	{
		m_wndCategoriesList.ResetContent();
		m_tvgID = 0;
		m_epgID = 0;
		m_prevDays = 0;
		m_nextDays = 0;
		m_hasArchive = 0;
		m_isAdult = 0;
		m_iconUrl.Empty();
		m_streamUrl.Empty();
		m_streamID = 0;
		m_wndIcon.SetBitmap(nullptr);
		m_wndCustom.SetCheck(FALSE);
	}
	else
	{
		m_tvgID = channel->get_tvg_id();
		m_epgID = channel->get_epg_id();
		m_prevDays = channel->get_prev_epg_days();
		m_nextDays = channel->get_next_epg_days();
		m_hasArchive = channel->get_has_archive();
		m_isAdult = channel->get_adult();
		m_streamUrl = channel->get_stream_uri().get_uri().c_str();
		m_streamID = channel->get_channel_id();

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

		m_wndCustom.SetCheck(m_streamID == 0 && channel->is_custom());

		FillCategoriesList(channel);
	}

	m_wndStreamID.EnableWindow(m_streamID != 0);
	m_wndStreamUrl.EnableWindow(m_streamID == 0);

	UpdateData(FALSE);
}

void CEdemChannelEditorDlg::FillCategoriesList(ChannelInfo* channel)
{
	if (!channel)
		return;

	m_wndCategoriesList.ResetContent();
	for (const auto& category : channel->get_categores())
	{
		int pos = m_wndCategoriesList.AddString(category.second->get_caption().c_str());
		m_wndCategoriesList.SetItemData(pos, (DWORD_PTR)category.second);
	}
}

void CEdemChannelEditorDlg::SaveChannelInfo()
{
	UpdateData(TRUE);
	// Save changes
	auto channel = GetChannel(m_current);
	if (!channel)
		return;

	channel->set_tvg_id(m_tvgID);
	channel->set_epg_id(m_epgID);
	channel->set_prev_epg_days(m_prevDays);
	channel->set_next_epg_days(m_nextDays);
	channel->set_has_archive(m_hasArchive);
	channel->set_adult(m_isAdult);

	if(CStringA(m_iconUrl) != channel->get_icon_uri().get_uri().c_str())
	{
		channel->set_icon_uri(m_iconUrl.GetString());
		const auto& path = channel->get_icon_uri().get_icon_relative_path(theApp.GetAppPath(utils::PLUGIN_ROOT));
		CImage img;
		if (theApp.LoadImage(path.c_str(), img))
		{
			channel->set_icon(img);
		}
	}

	if (m_wndCustom.GetCheck())
		channel->set_stream_uri(CStringA(m_streamUrl).GetString());

	if(channel->get_channel_id() != m_streamID)
	{
		channel->set_channel_id(m_streamID);
		CheckForExisting();
	}
}

void CEdemChannelEditorDlg::SaveCategoryInfo()
{
	UpdateData(TRUE);

	auto hSel = m_wndChannelsTree.GetSelectedItem();
	auto category = GetCategory(hSel);
	if (!category)
		return;

	if (CStringA(m_iconUrl) != category->get_icon_uri().get_uri().c_str())
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

ChannelInfo* CEdemChannelEditorDlg::GetChannel(HTREEITEM hItem)
{
	return IsChannel(hItem) ? (ChannelInfo*)m_wndChannelsTree.GetItemData(hItem) : nullptr;
}

ChannelInfo* CEdemChannelEditorDlg::GetCurrentChannel()
{
	return GetChannel(m_wndChannelsTree.GetSelectedItem());
}

ChannelCategory* CEdemChannelEditorDlg::GetItemCategory(HTREEITEM hItem)
{
	if (IsChannel(hItem))
	{
		hItem = m_wndChannelsTree.GetParentItem(hItem);
	}

	return GetCategory(hItem);
}

ChannelCategory* CEdemChannelEditorDlg::GetCategory(HTREEITEM hItem)
{
	if (!IsCategory(hItem))
		return nullptr;

	auto found = m_categories.find((int)m_wndChannelsTree.GetItemData(hItem));

	return found != m_categories.end() ? found->second.get() : nullptr;
}

HTREEITEM CEdemChannelEditorDlg::GetCategory(int id)
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

PlaylistEntry* CEdemChannelEditorDlg::GetPlaylistEntry(HTREEITEM hItem)
{
	if (hItem == nullptr || m_wndPlaylistTree.GetParentItem(hItem) == nullptr)
		return nullptr;

	return (PlaylistEntry*)m_wndPlaylistTree.GetItemData(hItem);
}

PlaylistEntry* CEdemChannelEditorDlg::GetCurrentPlaylistEntry()
{
	return GetPlaylistEntry(m_wndPlaylistTree.GetSelectedItem());
}

int CEdemChannelEditorDlg::GetNewCategoryID()
{
	int id = 0;
	if (!m_categories.empty())
	{
		id = m_categories.crbegin()->first;
	}

	return ++id;
}

bool CEdemChannelEditorDlg::LoadChannels(const std::wstring& path)
{
	m_categories.clear();
	m_channels.clear();

	std::ifstream is(path, std::istream::binary);
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

	return true;
}

void CEdemChannelEditorDlg::SetCurrentChannel(HTREEITEM hCur)
{
	m_current = hCur;
	m_wndChannelsTree.SelectItem(hCur);
}

void CEdemChannelEditorDlg::ChangeControlsState(BOOL enable)
{
	m_wndCategories.EnableWindow(enable);
	m_wndCategoriesList.EnableWindow(enable);
	m_wndCustom.EnableWindow(enable);
	m_wndArchive.EnableWindow(enable);
	m_wndAdult.EnableWindow(enable);
	m_wndTestTVG.EnableWindow(enable);
	m_wndTestEPG.EnableWindow(enable);
	m_wndAddToShow.EnableWindow(enable);
	m_wndRemoveFromShow.EnableWindow(enable);
	m_wndStreamID.EnableWindow(enable);
	m_wndStreamUrl.EnableWindow(enable);
	m_wndGetInfo.EnableWindow(enable);
	GetDlgItem(IDC_EDIT_URL_ID)->EnableWindow(enable);
	GetDlgItem(IDC_EDIT_TVG_ID)->EnableWindow(enable);
	GetDlgItem(IDC_EDIT_EPG_ID)->EnableWindow(enable);
	GetDlgItem(IDC_EDIT_PREV_EPG)->EnableWindow(enable);
	GetDlgItem(IDC_SPIN_PREV)->EnableWindow(enable);
	GetDlgItem(IDC_EDIT_NEXT_EPG)->EnableWindow(enable);
	GetDlgItem(IDC_SPIN_NEXT)->EnableWindow(enable);
	GetDlgItem(IDC_EDIT_INFO_AUDIO)->EnableWindow(enable);
	GetDlgItem(IDC_EDIT_INFO_VIDEO)->EnableWindow(enable);
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

void CEdemChannelEditorDlg::OnAddChannel()
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

	auto hItem = m_wndChannelsTree.GetSelectedItem();
	auto channel = GetChannel(hItem);
	if (channel && AfxMessageBox(_T("Delete channel. Are your sure?"), MB_YESNO | MB_ICONWARNING) == IDYES)
	{
		HTREEITEM hNext = m_wndChannelsTree.GetNextSiblingItem(hItem);
		if (!hNext)
		{
			hNext = m_wndChannelsTree.GetPrevSiblingItem(hItem);
		}

		auto hFound = FindTreeItem(m_wndChannelsTree, (DWORD_PTR)channel);
		while (hFound != nullptr)
		{
			m_wndChannelsTree.DeleteItem(hFound);
			hFound = FindTreeItem(m_wndChannelsTree, (DWORD_PTR)channel);
		}

		auto found = std::find_if(m_channels.begin(), m_channels.end(), [channel](const auto& item)
								  {
									  return item.get() == channel;
								  });

		ASSERT(found != m_channels.end());
		if (found != m_channels.end())
		{
			m_channels.erase(found);
		}

		SetCurrentChannel(hNext);
		set_allow_save();
		UpdateChannelsCount();
	}
}

void CEdemChannelEditorDlg::OnUpdateRemoveChannel(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsChannel(m_wndChannelsTree.GetSelectedItem()));
}

void CEdemChannelEditorDlg::OnChannelUp()
{
	if (GetFocus() == &m_wndChannelsTree)
	{
		HTREEITEM hCur = m_wndChannelsTree.GetSelectedItem();
		HTREEITEM hPrev = m_wndChannelsTree.GetPrevSiblingItem(hCur);

		if (IsChannel(hCur))
		{
			SwapChannels(hCur, hPrev);
		}
		else if (IsCategory(hCur))
		{
			SwapCategories(hCur, hPrev);
		}
	}
}

void CEdemChannelEditorDlg::OnUpdateChannelUp(CCmdUI* pCmdUI)
{
	HTREEITEM hCur = m_wndChannelsTree.GetSelectedItem();
	BOOL enable = hCur != nullptr && m_wndChannelsTree.GetPrevSiblingItem(hCur) != nullptr;

	pCmdUI->Enable(enable);
}

void CEdemChannelEditorDlg::OnChannelDown()
{
	if (GetFocus() == &m_wndChannelsTree)
	{
		HTREEITEM hCur = m_wndChannelsTree.GetSelectedItem();
		HTREEITEM hNext = m_wndChannelsTree.GetNextSiblingItem(hCur);
		if(IsChannel(hCur))
		{
			SwapChannels(hCur, hNext);
		}
		else if (IsCategory(hCur))
		{
			SwapCategories(hCur, hNext);
		}
	}
}

void CEdemChannelEditorDlg::OnUpdateChannelDown(CCmdUI* pCmdUI)
{
	HTREEITEM hCur = m_wndChannelsTree.GetSelectedItem();
	BOOL enable = hCur != nullptr && m_wndChannelsTree.GetNextSiblingItem(hCur) != nullptr;

	pCmdUI->Enable(enable);
}

void CEdemChannelEditorDlg::SwapChannels(HTREEITEM hLeft, HTREEITEM hRight)
{
	DWORD_PTR dwCur = m_wndChannelsTree.GetItemData(hLeft);
	CString cur = m_wndChannelsTree.GetItemText(hLeft);

	DWORD_PTR dwNext = m_wndChannelsTree.GetItemData(hRight);
	CString next = m_wndChannelsTree.GetItemText(hRight);

	m_wndChannelsTree.SetItemData(hLeft, dwNext);
	m_wndChannelsTree.SetItemText(hLeft, next);

	m_wndChannelsTree.SetItemData(hRight, dwCur);
	m_wndChannelsTree.SetItemText(hRight, cur);

	auto left = std::find_if(m_channels.begin(), m_channels.end(), [dwCur](const auto& item)
							 {
								 return (DWORD_PTR)item.get() == dwCur;
							 });

	auto right = std::find_if(m_channels.begin(), m_channels.end(), [dwNext](const auto& item)
							  {
								  return (DWORD_PTR)item.get() == dwNext;
							  });

	std::iter_swap(left, right);
	m_current = hRight;
	m_wndChannelsTree.SelectItem(hRight);

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
	sortInfo.hParent = nullptr;
	sortInfo.lpfnCompare = &CBCompareForSwap;
	m_wndChannelsTree.SortChildrenCB(&sortInfo);

	// Восстанавливаем значения ItemData
	for (const auto& it : itemData)
	{
		m_wndChannelsTree.SetItemData(it.hItem, it.lParam);
	}

	// Перестраиваем списки категорий для каналов
	for (auto& channel : m_channels)
	{
		channel->rebiuld_categories();
	}

	m_current = hLeft;
	m_wndChannelsTree.SelectItem(hLeft);

	set_allow_save();
}

void CEdemChannelEditorDlg::OnTreeItemRename()
{
	if (GetFocus() == &m_wndChannelsTree)
	{
		m_wndChannelsTree.EditLabel(m_wndChannelsTree.GetSelectedItem());
	}
}

void CEdemChannelEditorDlg::OnUpdateButtonRemoveChannel(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetCurrentChannel() != nullptr);
}

void CEdemChannelEditorDlg::OnBnClickedCheckCustomize()
{
	BOOL checked = m_wndCustom.GetCheck();
	m_wndStreamUrl.EnableWindow(checked);
	m_wndStreamID.EnableWindow(!checked);

	set_allow_save();
}

void CEdemChannelEditorDlg::OnTvnSelchangingTreeChannels(NMHDR* pNMHDR, LRESULT* pResult)
{
	TRACE("SelChanging\n");
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	if (IsChannel(pNMTreeView->itemOld.hItem))
	{
		SaveChannelInfo();
	}
	else if (IsCategory(pNMTreeView->itemOld.hItem))
	{
		SaveCategoryInfo();
	}

	*pResult = 0;
}

void CEdemChannelEditorDlg::OnTvnSelchangedTreeChannels(NMHDR* pNMHDR, LRESULT* pResult)
{
	TRACE("SelChanged\n");
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	HTREEITEM hSelected = pNMTreeView->itemNew.hItem;
	if(IsChannel(hSelected))
	{
		ChangeControlsState(TRUE);
		m_current = hSelected;
		LoadChannelInfo();
		auto channel = GetCurrentChannel();
		m_infoAudio = channel->get_audio().c_str();
		m_infoVideo = channel->get_video().c_str();
	}
	else if (IsCategory(hSelected))
	{
		ChangeControlsState(FALSE);
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
		else
		{
			m_iconUrl.Empty();
			m_wndIcon.SetBitmap(nullptr);
		}
	}
	else
	{
		ChangeControlsState(TRUE);
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

	popup->SetDefaultItem(ID_PLAY_STREAM);

	m_wndChannelsTree.SelectItem(hItem);

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

	popup->SetDefaultItem(ID_PLAY_STREAM_PL);

	m_wndPlaylistTree.SelectItem(hItem);

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
	auto channel = GetCurrentChannel();
	if (!channel)
		return;

	SaveChannelInfo();

	int idx = m_wndCategories.GetCurSel();
	if (idx == CB_ERR)
		return;

	auto toAdd = (ChannelCategory*)m_wndCategories.GetItemData(idx);
	if (!toAdd)
		return;

	channel->set_category(toAdd->get_id());

	TVINSERTSTRUCTW tvInsert = { nullptr };
	tvInsert.hParent = GetCategory(toAdd->get_id());
	tvInsert.item.pszText = (LPWSTR)channel->get_title().c_str();
	tvInsert.item.lParam = (LPARAM)channel;
	tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
	m_wndChannelsTree.InsertItem(&tvInsert);

	LoadChannelInfo();

	set_allow_save();
}

void CEdemChannelEditorDlg::OnUpdateButtonAddToShowIn(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_wndCategories.GetCount() != 0 && m_wndCategories.IsWindowEnabled());
}

void CEdemChannelEditorDlg::OnBnClickedButtonRemoveFromShowIn()
{
	HTREEITEM hSelected = m_wndChannelsTree.GetSelectedItem();
	auto channel = GetChannel(hSelected);
	if (!channel)
		return;

	int idx = m_wndCategoriesList.GetCurSel();
	if (idx == CB_ERR)
		return;

	if (m_wndCategoriesList.GetCount() == 1)
	{
		AfxMessageBox(_T("Channel have only one category. Just remove channel or add additional category."), MB_ICONWARNING);
		return;
	}

	SaveChannelInfo();

	auto category = (ChannelCategory*)m_wndCategoriesList.GetItemData(idx);
	channel->erase_category(category->get_id());

	auto hSub = FindTreeSubItem(m_wndChannelsTree, m_wndChannelsTree.GetChildItem(GetCategory(category->get_id())), (DWORD_PTR)channel);
	if (hSub)
		m_wndChannelsTree.DeleteItem(hSub);

	if (auto found = FindTreeItem(m_wndChannelsTree, (DWORD_PTR)channel); found != nullptr)
	{
		if(hSelected == found)
			LoadChannelInfo();
		else
			m_wndChannelsTree.SelectItem(found);
	}

	set_allow_save();
}

void CEdemChannelEditorDlg::OnUpdateButtonRemoveFromShow(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_wndCategoriesList.GetCurSel() != LB_ERR);
}

void CEdemChannelEditorDlg::OnEditChangeTvIdd()
{
	set_allow_save();
}

void CEdemChannelEditorDlg::OnChanges()
{
	SaveChannelInfo();
	set_allow_save();
}

void CEdemChannelEditorDlg::CheckLimits()
{
	if (m_prevDays < 0)
		m_prevDays = 0;

	if (m_prevDays > 7)
		m_prevDays = 7;

	if (m_nextDays < 0)
		m_nextDays = 0;

	if (m_nextDays > 7)
		m_nextDays = 7;

	UpdateData(FALSE);

	set_allow_save();
}

void CEdemChannelEditorDlg::OnEnChangeEditNum()
{
	UpdateData(TRUE);
	CheckLimits();
}

void CEdemChannelEditorDlg::OnDeltaposSpinPrev(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateData(TRUE);
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	m_prevDays -= pNMUpDown->iDelta;
	CheckLimits();
	*pResult = 0;
}

void CEdemChannelEditorDlg::OnDeltaposSpinNext(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateData(TRUE);
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	m_nextDays -= pNMUpDown->iDelta;
	CheckLimits();
	*pResult = 0;
}

void CEdemChannelEditorDlg::OnBnClickedButtonTestTvg()
{
	static LPCSTR url = "http://www.teleguide.info/kanal%d_%4d%02d%02d.html";

	SaveChannelInfo();

	auto channel = GetCurrentChannel();
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

	SaveChannelInfo();

	auto channel = GetCurrentChannel();
	if (channel)
	{
		CStringA epg_url;
		epg_url.Format(url, channel->get_epg_id());
		ShellExecuteA(nullptr, "open", epg_url.GetString(), nullptr, nullptr, SW_SHOWNORMAL);
	}
}

void CEdemChannelEditorDlg::OnUpdateButtonTestEpg(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetCurrentChannel() != nullptr);
}

void CEdemChannelEditorDlg::PlayChannel(HTREEITEM hItem)
{
	if (auto channel = GetChannel(hItem); channel != nullptr)
	{
		PlayStream(TranslateStreamUri(channel->get_stream_uri().get_ts_translated_url()));
	}
}

void CEdemChannelEditorDlg::PlayPlaylistEntry(HTREEITEM hItem)
{
	if (auto entry = GetCurrentPlaylistEntry(); entry != nullptr)
	{
		PlayStream(TranslateStreamUri(entry->get_stream_uri().get_ts_translated_url()));
	}
}

void CEdemChannelEditorDlg::PlayStream(const std::wstring& stream_url)
{
	TRACE(_T("Test URL: %s\n"), stream_url.c_str());
	ShellExecute(nullptr, _T("open"), m_player, stream_url.c_str(), nullptr, SW_SHOWNORMAL);
}

void CEdemChannelEditorDlg::OnBnClickedButtonLoadPlaylist()
{
	CFileDialog dlg(TRUE);

	CString file;
	CString filter(_T("Playlist m3u8(*.m3u8)|*.m3u8|All Files (*.*)|*.*||"));
	filter.Replace('|', '\0');

	OPENFILENAME& oFN = dlg.GetOFN();
	oFN.lpstrFilter = filter.GetString();
	oFN.nMaxFile = MAX_PATH;
	oFN.nFilterIndex = 0;
	oFN.lpstrFile = file.GetBuffer(MAX_PATH);
	oFN.lpstrTitle = _T("Load Edem TV playlist");
	oFN.Flags |= OFN_EXPLORER | OFN_ENABLESIZING | OFN_LONGNAMES | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NONETWORKBUTTON;

	INT_PTR nResult = dlg.DoModal();
	file.ReleaseBuffer();

	if (nResult == IDOK)
	{
		LoadPlaylist(file);
		FillPlaylist();
		theApp.WriteProfileString(_T("Setting"), _T("Playlist"), file);
	}
}

void CEdemChannelEditorDlg::LoadPlaylist(const CString& file)
{
	// #EXTM3U <--- header
	// #EXTINF:0 tvg-rec="3",Первый FHD <-- caption
	// #EXTGRP:Общие <-- Category
	// http://6646b6bc.akadatel.com/iptv/PWXQ2KD5G2VNSK/2402/index.m3u8

	auto pos = file.ReverseFind('\\');
	if (pos != -1)
	{
		m_plFileName = file.Mid(++pos);
	}
	else
	{
		m_plFileName = file;
	}
	UpdateData(FALSE);

	m_playlist.clear();
	m_pl_categories.clear();

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

	std::ifstream is(file.GetString());
	if (is.good())
	{
		int step = 0;
		std::string line;
		auto entry = std::make_unique<PlaylistEntry>();
		while (std::getline(is, line))
		{
			utils::string_rtrim(line, "\r");
			entry->Parse(line);
			switch (entry->get_directive())
			{
				case ext_pathname:
				{
					const auto& category = entry->get_category();
					int id = entry->get_channel_id();
					if (auto res = m_playlist.emplace(id, std::move(entry)); !res.second)
					{
						TRACE("Duplicate channel: %d\n", id);
					}

					auto found = std::find_if(m_pl_categories.begin(), m_pl_categories.end(), [category](const auto& item)
												  {
													  return category == item.first;
												  });

					if (found == m_pl_categories.end())
					{
						m_pl_categories.emplace_back(category, nullptr);
					}
					entry = std::make_unique<PlaylistEntry>();
					break;
				}
				default:
					break;
			}
		}
	}

	// if access key and domain not set, try to load it from playlist
	if (m_accessKey.IsEmpty() && m_domain.IsEmpty() && !m_playlist.empty())
	{
		for(const auto& pair : m_playlist)
		{
			const auto& access_key = pair.second->get_access_key();
			const auto& domain = pair.second->get_domain();
			if(!access_key.empty() && !domain.empty() && access_key != "00000000000000" && domain != "localhost")
			{
				theApp.WriteProfileString(_T("Setting"), _T("AccessKey"), CString(access_key.c_str()));
				theApp.WriteProfileString(_T("Setting"), _T("Domain"), CString(domain.c_str()));
				break;
			}
		}
	}
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

		for (const auto& pair : m_playlist)
		{
			const auto& entry = pair.second.get();
			auto found = std::find_if(m_pl_categories.begin(), m_pl_categories.end(), [entry](const auto& item)
									  {
										  return item.first == entry->get_category();
									  });

			if (found == m_pl_categories.end()) continue;

			TVINSERTSTRUCTW tvInsert = { nullptr };
			tvInsert.hParent = found->second;
			tvInsert.item.pszText = (LPWSTR)entry->get_title().c_str();
			tvInsert.item.lParam = (LPARAM)entry;
			tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
			m_wndPlaylistTree.InsertItem(&tvInsert);
		}
	}

	CheckForExisting();
	m_pl_cur_it = m_playlist.end();
	m_plInfo.Format(_T("Playlist: %s (%d)"), m_plFileName.GetString(), m_playlist.size());
	UpdateData(FALSE);
}

void CEdemChannelEditorDlg::OnBnClickedButtonSave()
{
	SaveChannels();
}

void CEdemChannelEditorDlg::OnUpdateButtonSave(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(is_allow_save());
}

void CEdemChannelEditorDlg::OnAddCategory()
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

void CEdemChannelEditorDlg::OnUpdateAddCategory(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CEdemChannelEditorDlg::OnRemoveCategory()
{
	auto hItem = m_wndChannelsTree.GetSelectedItem();
	auto category = GetCategory(hItem);
	if (!category)
		return;

	if (IsCategoryInChannels(category))
	{
		AfxMessageBox(_T("Category must be empty!"), MB_OK | MB_ICONERROR);
		return;
	}

	m_wndChannelsTree.SetItemData(hItem, 0);
	m_wndChannelsTree.DeleteItem(hItem);

	m_categories.erase(category->get_id());

	FillCategories();
}

void CEdemChannelEditorDlg::OnUpdateRemoveCategory(CCmdUI* pCmdUI)
{
	auto category = GetItemCategory(m_wndChannelsTree.GetSelectedItem());
	BOOL enable = category ? !IsCategoryInChannels(category) : FALSE;

	pCmdUI->Enable(enable);
}

void CEdemChannelEditorDlg::OnStnClickedStaticIcon()
{
	auto hCur = m_wndChannelsTree.GetSelectedItem();
	if (!hCur)
		return;

	bool channel = IsChannel(hCur);

	CFileDialog dlg(TRUE);
	CString path = theApp.GetAppPath(channel ? utils::CHANNELS_LOGO_PATH : utils::CATEGORIES_LOGO_PATH);
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
		m_iconUrl += channel ? utils::CHANNELS_LOGO_URL : utils::CATEGORIES_LOGO_URL;
		m_iconUrl += oFN.lpstrFileTitle;

		set_allow_save();
		UpdateData(FALSE);
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
		::DeleteFile(_T("dune_plugin_edem_free4_mod.zip"));
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
			if (entry && StrStrI(entry->get_title().c_str(), m_plSearch.GetString()) != nullptr)
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
			const auto& entry = it->second;
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
			const auto& entry = it->second;
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

void CEdemChannelEditorDlg::OnCreateUpdateChannel()
{
	auto entry = GetCurrentPlaylistEntry();
	if (!entry)
		return;

	bool isNew = false;
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

		if(categoryId == -1)
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

		isNew = true;
		m_channels.emplace_back(std::move(channel));
	}
	else
	{
		hFoundItem = FindTreeItem(m_wndChannelsTree, (DWORD_PTR)found);
	}

	m_wndChannelsTree.SelectItem(hFoundItem);

	auto channel = GetCurrentChannel();
	if(channel->get_title() != entry->get_title())
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
	if (cat_name.find(L"зрослые") != std::wstring::npos)
	{
		channel->set_adult(TRUE);
	}

	auto it = std::find_if(m_categories.begin(), m_categories.end(), [cat_name](const auto& category)
						   {
							   return (0 == _wcsicmp(category.second->get_caption().c_str(), cat_name.c_str()));
						   });

	if (it != m_categories.end())
		channel->set_category(it->second->get_id());

	if (isNew)
		CheckForExisting();

	LoadChannelInfo();
	set_allow_save();
}

std::map<int, HTREEITEM> CEdemChannelEditorDlg::GetCategoriesTreeMap()
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

void CEdemChannelEditorDlg::OnUpdateCreateUpdateChannel(CCmdUI* pCmdUI)
{
	BOOL enable = FALSE;
	do
	{
		auto entry = GetCurrentPlaylistEntry();
		if (entry == nullptr) break;

		auto found = FindChannelByEntry(entry);
		if (pCmdUI->m_nID != IDC_BUTTON_IMPORT)
		{
			CString str;
			if (found == nullptr)
			{
				str.Format(_T("Add \'%s\'\tF5"), entry->get_title().c_str());
			}
			else
			{
				str.Format(_T("Update \'%s\'\tF5"), found->get_title().c_str());
			}
			pCmdUI->SetText(str);
		}

		enable = TRUE;

		if (found == nullptr) break;

		if (!entry->get_title().empty() && found->get_title() != entry->get_title()) break;

		if (auto id = entry->get_tvg_id(); id != -1 && found->get_epg_id() != id) break;

		if ((found->get_has_archive() != 0) != entry->is_archive()) break;

		if (!entry->get_icon_uri().get_uri().empty() && entry->get_icon_uri() != found->get_icon_uri()) break;

		const auto& cat_name = entry->get_category();
		auto it = std::find_if(m_categories.begin(), m_categories.end(), [cat_name](const auto& category)
							   {
								   return (0 == _wcsicmp(category.second->get_caption().c_str(), cat_name.c_str()));
							   });
		if (it == m_categories.end()) break;

		bool adult = cat_name.find(L"зрослые") != std::wstring::npos;
		if (adult && found->get_adult() == 0) break;

		enable = FALSE;
	} while (false);

	pCmdUI->Enable(enable);
}

void CEdemChannelEditorDlg::OnBnClickedButtonSettings()
{
	CSettingsDlg dlg;
	if (dlg.DoModal() == IDOK)
	{
		m_accessKey = theApp.GetProfileString(_T("Setting"), _T("AccessKey"));
		m_domain = theApp.GetProfileString(_T("Setting"), _T("Domain"));
		m_player = theApp.GetProfileString(_T("Setting"), _T("Player"));
		m_probe = theApp.GetProfileString(_T("Setting"), _T("FFProbe"));
	}
}

void CEdemChannelEditorDlg::OnBnClickedButtonUpdateIcon()
{
	auto entry = GetCurrentPlaylistEntry();
	auto channel = GetCurrentChannel();

	if (entry && channel && channel->get_icon_uri() != entry->get_icon_uri())
	{
		channel->set_icon_uri(entry->get_icon_uri());
		channel->copy_icon(entry->get_icon());
		theApp.SetImage(channel->get_icon(), m_wndIcon);
		LoadChannelInfo();
		set_allow_save();
	}
}

void CEdemChannelEditorDlg::OnUpdateButtonUpdateIcon(CCmdUI* pCmdUI)
{
	BOOL enable = FALSE;
	auto entry = GetCurrentPlaylistEntry();
	auto channel = GetCurrentChannel();
	if (channel && entry)
	{
		enable = channel->get_icon_uri() != entry->get_icon_uri();
	}

	pCmdUI->Enable(enable);
}

void CEdemChannelEditorDlg::OnBnClickedButtonCacheIcon()
{
	auto channel = GetCurrentChannel();
	if (channel)
	{
		auto fname = channel->get_icon_uri().get_path();
		if (auto pos = fname.rfind('/'); pos != std::string::npos)
		{
			fname = fname.substr(pos + 1);
			std::string path = utils::CHANNELS_LOGO_URL;
			path += fname;

			uri icon_uri;
			icon_uri.set_uri(utils::ICON_TEMPLATE);
			icon_uri.set_path(path);

			std::vector<BYTE> image;
			if (utils::DownloadFile(utils::utf8_to_utf16(channel->get_icon_uri().get_uri()), image))
			{
				std::wstring fullPath = icon_uri.get_icon_relative_path(theApp.GetAppPath(utils::PLUGIN_ROOT));
				std::ofstream os(fullPath.c_str(), std::ios::out | std::ios::binary);
				os.write((char*)&image[0], image.size());
				os.close();

				m_iconUrl = icon_uri.get_uri().c_str();
				UpdateData(FALSE);
				SaveChannelInfo();
				set_allow_save();
			}
		}
	}
}

void CEdemChannelEditorDlg::OnUpdateButtonCacheIcon(CCmdUI* pCmdUI)
{
	auto channel = GetCurrentChannel();
	pCmdUI->Enable(channel && !channel->is_icon_local());
}

void CEdemChannelEditorDlg::OnTvnSelchangedTreePaylist(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	UpdateData(TRUE);

	auto entry = GetCurrentPlaylistEntry();
	if (entry)
	{
		m_pl_current = m_wndPlaylistTree.GetSelectedItem();
		m_plIconName = entry->get_icon_uri().get_uri().c_str();
		int id = entry->get_channel_id();
		m_plID.Format(_T("ID: %d"), id);

		if (entry->get_tvg_id() != -1)
			m_plEPG.Format(_T("EPG: %d"), entry->get_tvg_id());

		m_wndPlArchive.SetCheck(!!entry->is_archive());
		UpdateData(FALSE);

		CString path(entry->get_icon_uri().get_uri().c_str());
		CImage img;
		if (theApp.LoadImage(path, img))
		{
			entry->set_icon(img);
		}

		theApp.SetImage(entry->get_icon(), m_wndPlIcon);


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

		m_infoAudio = entry->get_audio().c_str();
		m_infoVideo = entry->get_video().c_str();

		UpdateData(FALSE);
	}

	*pResult = 0;
}

void CEdemChannelEditorDlg::OnSortByName()
{
	auto category = GetItemCategory(m_wndChannelsTree.GetSelectedItem());
	if (!category)
		return;

	auto id = category->get_id();
	std::sort(m_channels.begin(), m_channels.end(), [id](const auto& l, const auto& r)
			  {
				  const auto& lcat = l->get_categores();
				  const auto& rcat = r->get_categores();
				  bool lfound = lcat.find(id) != lcat.end();
				  bool rfound = rcat.find(id) != rcat.end();

				  if (lfound && rfound)
				  {
					  return l->get_title() < r->get_title();
				  }

				  return false;
			  });

	FillChannels();
	LoadChannelInfo();
	set_allow_save();
}

void CEdemChannelEditorDlg::OnSortByChannelID()
{
	std::sort(m_channels.begin(), m_channels.end(), [](const auto& l, const auto& r)
			  {
				  return l->get_channel_id() < r->get_channel_id();
			  });

	FillChannels();
	LoadChannelInfo();
	set_allow_save();
}

void CEdemChannelEditorDlg::OnGetChannelStreamInfo()
{
	CWaitCursor cur;
	auto channel = GetCurrentChannel();
	if(channel)
	{
		std::string audio;
		std::string video;
		GetChannelStreamInfo(channel->get_stream_uri().get_ts_translated_url(), audio, video);

		channel->set_audio(audio);
		channel->set_video(video);
		m_infoAudio = audio.c_str();
		m_infoVideo = video.c_str();
		UpdateData(FALSE);
	}
}

void CEdemChannelEditorDlg::OnUpdateGetChannelStreamInfo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetCurrentChannel() != nullptr && !m_probe.IsEmpty());
}

void CEdemChannelEditorDlg::OnGetChannelStreamInfoPl()
{
	CWaitCursor cur;
	auto entry = GetCurrentPlaylistEntry();
	if (entry)
	{
		std::string audio;
		std::string video;
		GetChannelStreamInfo(entry->get_stream_uri().get_ts_translated_url(), audio, video);

		entry->set_audio(audio);
		entry->set_video(video);
		m_infoAudio = audio.c_str();
		m_infoVideo = video.c_str();
		UpdateData(FALSE);
	}
}

void CEdemChannelEditorDlg::OnUpdateGetChannelStreamInfoPl(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetCurrentPlaylistEntry() != nullptr && !m_probe.IsEmpty());
}

void CEdemChannelEditorDlg::OnPlayChannelStream()
{
	PlayChannel(m_wndChannelsTree.GetSelectedItem());
}

void CEdemChannelEditorDlg::OnUpdatePlayChannelStream(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetCurrentChannel() != nullptr && !m_probe.IsEmpty());
}

void CEdemChannelEditorDlg::OnPlayChannelStreamPl()
{
	PlayPlaylistEntry(m_wndPlaylistTree.GetSelectedItem());
}

void CEdemChannelEditorDlg::OnUpdatePlayChannelStreamPl(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetCurrentPlaylistEntry() != nullptr && !m_probe.IsEmpty());
}

void CEdemChannelEditorDlg::OnToggleChannel()
{
	auto hItem = m_wndChannelsTree.GetSelectedItem();
	if (auto channel = GetChannel(hItem); channel != nullptr)
	{
		channel->set_disabled(!channel->is_disabled());
		CRect rc;
		m_wndChannelsTree.GetItemRect(hItem, rc, FALSE);
		m_wndChannelsTree.InvalidateRect(rc, FALSE);
		set_allow_save();
	}
}

void CEdemChannelEditorDlg::OnUpdateToggleChannel(CCmdUI* pCmdUI)
{
	BOOL enable = FALSE;
	if (auto channel = GetCurrentChannel(); channel != nullptr && channel->is_disabled())
	{
		pCmdUI->SetText(_T("Enable Channel"));
		enable = TRUE;
	}

	pCmdUI->Enable(enable);
}

void CEdemChannelEditorDlg::OnBnClickedButtonGetInfoPl()
{
	CWaitCursor cur;
	auto entry = GetCurrentPlaylistEntry();
	if(entry)
	{
		std::string audio;
		std::string video;
		GetChannelStreamInfo(entry->get_stream_uri().get_ts_translated_url(), audio, video);
		entry->set_audio(audio);
		entry->set_video(video);
		m_infoAudio = audio.c_str();
		m_infoVideo = video.c_str();
		UpdateData(FALSE);
	}
}

void CEdemChannelEditorDlg::OnUpdateButtonGetInfoPl(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetCurrentPlaylistEntry() != nullptr && !m_probe.IsEmpty());
}

void CEdemChannelEditorDlg::OnBnClickedButtonDownloadPlaylist()
{
	CWaitCursor cur;

	int idx = m_wndPlaylistType.GetCurSel();
	std::wstring url;
	switch (idx)
	{
		case 0:
			url = L"http://epg.it999.ru/edem_epg_ico.m3u8";
			break;
		case 1:
			url = L"http://epg.it999.ru/edem_epg_ico2.m3u8";
			break;
		case 2:
			url = theApp.GetProfileString(_T("Setting"), _T("PlaylistURL")).GetString();
			break;
		default:
			return;
	}

	std::wstring name;
	if (size_t pos = url.rfind('/'); pos != std::wstring::npos)
	{
		name = url.substr(pos + 1);
	}
	else
	{
		name = L"unnamed_playlist.m3u8";
	}

	std::vector<BYTE> data;
	if (utils::DownloadFile(url, data))
	{
		CString playlist(theApp.GetAppPath() + name.c_str());

		std::ofstream os(playlist);
		os.write((char*)data.data(), data.size());
		os.close();
		LoadPlaylist(playlist);
		FillPlaylist();
		theApp.WriteProfileString(_T("Setting"), _T("Playlist"), playlist);
	}
}

void CEdemChannelEditorDlg::OnCbnSelchangeComboPlaylist()
{
	int idx = m_wndPlaylistType.GetCurSel();
	switch (idx)
	{
		case 0:
		case 1:
			GetDlgItem(IDC_BUTTON_LOAD_PLAYLIST)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_DOWNLOAD_PLAYLIST)->EnableWindow(TRUE);
			break;
		case 2:
			GetDlgItem(IDC_BUTTON_LOAD_PLAYLIST)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_DOWNLOAD_PLAYLIST)->EnableWindow(!theApp.GetProfileString(_T("Setting"), _T("PlaylistURL")).IsEmpty());
			break;
		case 3:
			GetDlgItem(IDC_BUTTON_LOAD_PLAYLIST)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_DOWNLOAD_PLAYLIST)->EnableWindow(FALSE);
			break;
		default:
			break;
	}

	theApp.WriteProfileInt(_T("Setting"), _T("PlaylistType"), idx);
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

std::wstring CEdemChannelEditorDlg::TranslateStreamUri(const std::string& stream_uri)
{
	// http://ts://{SUBDOMAIN}/iptv/{UID}/205/index.m3u8 -> http://ts://domain.com/iptv/000000000000/205/index.m3u8

	std::wregex re_domain(LR"(\{SUBDOMAIN\})");
	std::wregex re_uid(LR"(\{UID\})");

	std::wstring stream_url = utils::utf8_to_utf16(stream_uri);
	stream_url = std::regex_replace(stream_url, re_domain, CEdemChannelEditorDlg::m_domain.GetString());
	return std::regex_replace(stream_url, re_uid, CEdemChannelEditorDlg::m_accessKey.GetString());
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
