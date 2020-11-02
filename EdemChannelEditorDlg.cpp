
// EdemChannelEditorDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "framework.h"
#include <afxdialogex.h>
#include <fstream>
#include <sstream>
#include <regex>

#include "EdemChannelEditor.h"
#include "EdemChannelEditorDlg.h"
#include "AboutDlg.h"
#include "NewCategoryDlg.h"
#include "NewChannelDlg.h"
#include "SettingsDlg.h"

#include "utils.h"
#include "rapidxml.hpp"
#include "rapidxml_print.hpp"

#include "SevenZip/7zip/SevenZipWrapper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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

using namespace SevenZip;

static constexpr auto TV_INFO = "tv_info";
static constexpr auto TV_CATEGORIES = "tv_categories";
static constexpr auto TV_CHANNELS = "tv_channels";
static constexpr auto CHANNELS_LOGO_URL  = "icons/channels/";

#ifdef _DEBUG
static constexpr auto PLUGIN_ROOT        = L"..\\edem_plugin\\";
static constexpr auto CHANNELS_CONFIG    = L"..\\edem_plugin\\edem_channel_list.xml";
static constexpr auto CHANNELS_LOGO_PATH = L"..\\edem_plugin\\icons\\channels\\";
#else
static constexpr auto PLUGIN_ROOT        = L".\\edem_plugin\\";
static constexpr auto CHANNELS_CONFIG    = L".\\edem_plugin\\edem_channel_list.xml";
static constexpr auto CHANNELS_LOGO_PATH = L".\\edem_plugin\\icons\\channels\\";
#endif // _DEBUG

// CEdemChannelEditorDlg dialog

BEGIN_MESSAGE_MAP(CEdemChannelEditorDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_ABOUT, &CEdemChannelEditorDlg::OnBnClickedButtonAbout)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CEdemChannelEditorDlg::OnBnClickedButtonAddToShowIn)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_ADD, &CEdemChannelEditorDlg::OnUpdateButtonAddToShowIn)
	ON_BN_CLICKED(IDC_BUTTON_ADD_CATEGORY, &CEdemChannelEditorDlg::OnBnClickedButtonAddCategory)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_CATEGORY, &CEdemChannelEditorDlg::OnBnClickedButtonEditCategory)
	ON_BN_CLICKED(IDC_BUTTON_IMPORT, &CEdemChannelEditorDlg::OnBnClickedButtonImport)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_IMPORT, &CEdemChannelEditorDlg::OnUpdateButtonImport)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_PLAYLIST, &CEdemChannelEditorDlg::OnBnClickedButtonLoadPlaylist)
	ON_BN_CLICKED(IDC_BUTTON_PACK, &CEdemChannelEditorDlg::OnBnClickedButtonPack)
	ON_BN_CLICKED(IDC_BUTTON_PL_SEARCH_NEXT, &CEdemChannelEditorDlg::OnBnClickedButtonPlSearchNext)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_PL_SEARCH_NEXT, &CEdemChannelEditorDlg::OnUpdateButtonPlSearchNext)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, &CEdemChannelEditorDlg::OnBnClickedButtonRemoveFromShowIn)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_REMOVE, &CEdemChannelEditorDlg::OnUpdateButtonRemoveFromShow)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_CATEGORY, &CEdemChannelEditorDlg::OnBnClickedButtonRemoveCategory)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH_NEXT, &CEdemChannelEditorDlg::OnBnClickedButtonSearchNext)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_SEARCH_NEXT, &CEdemChannelEditorDlg::OnUpdateButtonSearchNext)
	ON_BN_CLICKED(IDC_BUTTON_TEST_EPG, &CEdemChannelEditorDlg::OnBnClickedButtonTestEpg)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_TEST_EPG, &CEdemChannelEditorDlg::OnUpdateButtonTestEpg)
	ON_BN_CLICKED(IDC_BUTTON_TEST_URL, &CEdemChannelEditorDlg::OnBnClickedButtonTestUrl)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_TEST_URL, &CEdemChannelEditorDlg::OnUpdateButtonTestUrl)
	ON_BN_CLICKED(IDC_CHECK_ADULT, &CEdemChannelEditorDlg::OnChanges)
	ON_BN_CLICKED(IDC_CHECK_ARCHIVE, &CEdemChannelEditorDlg::OnChanges)
	ON_BN_CLICKED(IDC_CHECK_CUSTOMIZE, &CEdemChannelEditorDlg::OnBnClickedCheckCustomize)
	ON_EN_CHANGE(IDC_EDIT_CHANNEL_NAME, &CEdemChannelEditorDlg::OnChanges)
	ON_EN_CHANGE(IDC_EDIT_NEXT_EPG, &CEdemChannelEditorDlg::OnEnChangeEditNum)
	ON_EN_CHANGE(IDC_EDIT_PREV_EPG, &CEdemChannelEditorDlg::OnEnChangeEditNum)
	ON_EN_KILLFOCUS(IDC_EDIT_CHANNEL_NAME, &CEdemChannelEditorDlg::OnEnKillfocusEditChannelName)
	ON_EN_KILLFOCUS(IDC_EDIT_STREAM_URL, &CEdemChannelEditorDlg::OnEnKillfocusEditStreamUrl)
	ON_EN_KILLFOCUS(IDC_EDIT_TVG_ID, &CEdemChannelEditorDlg::OnEnKillfocusEditTvgId)
	ON_EN_KILLFOCUS(IDC_EDIT_URL_ID, &CEdemChannelEditorDlg::OnEnKillfocusEditUrlId)
	ON_LBN_DBLCLK(IDC_LIST_CHANNELS, &CEdemChannelEditorDlg::OnBnClickedButtonTestUrl)
	ON_LBN_SELCHANGE(IDC_LIST_CHANNELS, &CEdemChannelEditorDlg::OnLbnSelchangeListChannels)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_NEXT, &CEdemChannelEditorDlg::OnDeltaposSpinNext)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_PREV, &CEdemChannelEditorDlg::OnDeltaposSpinPrev)
	ON_STN_CLICKED(IDC_STATIC_ICON, &CEdemChannelEditorDlg::OnStnClickedStaticIcon)
	ON_BN_CLICKED(IDC_BUTTON_SETTINGS, &CEdemChannelEditorDlg::OnBnClickedButtonSettings)
	ON_BN_CLICKED(IDC_BUTTON_CACHE_ICON, &CEdemChannelEditorDlg::OnBnClickedButtonCacheIcon)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_CACHE_ICON, &CEdemChannelEditorDlg::OnUpdateButtonCacheIcon)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE_ICON, &CEdemChannelEditorDlg::OnBnClickedButtonUpdateIcon)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_UPDATE_ICON, &CEdemChannelEditorDlg::OnUpdateButtonUpdateIcon)
	ON_BN_CLICKED(IDC_BUTTON_ADD_CHANNEL, &CEdemChannelEditorDlg::OnBnClickedButtonAddChannel)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_CHANNEL, &CEdemChannelEditorDlg::OnBnClickedButtonRemoveChannel)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_REMOVE_CHANNEL, &CEdemChannelEditorDlg::OnUpdateButtonRemoveChannel)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CEdemChannelEditorDlg::OnBnClickedButtonSave)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_SAVE, &CEdemChannelEditorDlg::OnUpdateButtonSave)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_PAYLIST, &CEdemChannelEditorDlg::OnTvnSelchangedTreePaylist)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_PAYLIST, &CEdemChannelEditorDlg::OnNMDblclkTreePaylist)
	ON_BN_CLICKED(IDC_BUTTON_SORT, &CEdemChannelEditorDlg::OnBnClickedButtonSort)
	ON_BN_CLICKED(IDC_BUTTON_GET_INFO, &CEdemChannelEditorDlg::OnBnClickedButtonGetInfo)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_GET_INFO, &CEdemChannelEditorDlg::OnUpdateButtonGetInfo)
	ON_BN_CLICKED(IDC_BUTTON_GET_ALL_INFO, &CEdemChannelEditorDlg::OnBnClickedButtonGetAllInfo)

	ON_COMMAND(ID_ACC_UPDATE_ICON, &CEdemChannelEditorDlg::OnBnClickedButtonUpdateIcon)
	ON_COMMAND(ID_ACC_ADD_CHANNEL, &CEdemChannelEditorDlg::OnBnClickedButtonAddChannel)
	ON_COMMAND(ID_ACC_SAVE, &CEdemChannelEditorDlg::OnBnClickedButtonSave)
	ON_COMMAND(ID_ACC_DELETE_CHANNEL, &CEdemChannelEditorDlg::OnAccelRemoveChannel)
	ON_MESSAGE_VOID(WM_KICKIDLE, OnKickIdle)
END_MESSAGE_MAP()

CEdemChannelEditorDlg::CEdemChannelEditorDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EDEMCHANNELEDITOR_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CEdemChannelEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_COMBO_CATEGORY, m_wndCategories);
	DDX_Control(pDX, IDC_LIST_CATEGORIES, m_wndCategoriesList);
	DDX_Control(pDX, IDC_CHECK_CUSTOMIZE, m_wndCustom);
	DDX_Text(pDX, IDC_EDIT_CHANNEL_NAME, m_channelName);
	DDX_Text(pDX, IDC_EDIT_URL_ID, m_streamID);
	DDX_Text(pDX, IDC_EDIT_TVG_ID, m_tvgID);
	DDX_Control(pDX, IDC_EDIT_URL_ID, m_wndStreamID);
	DDX_Control(pDX, IDC_EDIT_STREAM_URL, m_wndStreamUrl);
	DDX_Text(pDX, IDC_EDIT_STREAM_URL, m_streamUrl);
	DDX_Check(pDX, IDC_CHECK_ARCHIVE, m_hasArchive);
	DDX_Check(pDX, IDC_CHECK_ADULT, m_isAdult);
	DDX_Text(pDX, IDC_EDIT_PREV_EPG, m_prevDays);
	DDX_Text(pDX, IDC_EDIT_NEXT_EPG, m_nextDays);
	DDX_Control(pDX, IDC_STATIC_ICON, m_wndIcon);
	DDX_Control(pDX, IDC_LIST_CHANNELS, m_wndChannelsList);
	DDX_Text(pDX, IDC_EDIT_SEARCH, m_search);
	DDX_Control(pDX, IDC_TREE_PAYLIST, m_wndPlaylistTree);
	DDX_Text(pDX, IDC_EDIT_PL_SEARCH, m_plSearch);
	DDX_Text(pDX, IDC_STATIC_ICON_NAME, m_iconUrl);
	DDX_Control(pDX, IDC_STATIC_PL_ICON, m_wndPlIcon);
	DDX_Text(pDX, IDC_STATIC_PL_ICON_NAME, m_plIconName);
	DDX_Text(pDX, IDC_STATIC_PLAYLIST, m_plInfo);
	DDX_Text(pDX, IDC_STATIC_PL_TVG, m_plEPG);
	DDX_Control(pDX, IDC_CHECK_PL_ARCHIVE, m_wndPlArchive);
	DDX_CBIndex(pDX, IDC_COMBO_SORT, m_sortType);
	DDX_Text(pDX, IDC_EDIT_INFO_VIDEO, m_infoVideo);
	DDX_Text(pDX, IDC_EDIT_INFO_AUDIO, m_infoAudio);
	DDX_Text(pDX, IDC_STATIC_CHANNELS, m_channelsInfo);
	DDX_Control(pDX, IDC_BUTTON_GET_ALL_INFO, m_wndGetAllInfo);
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
	m_wndChannelsList.m_color = RGB(0, 200, 0);
	m_wndPlaylistTree.m_color = RGB(200, 0, 0);

	LoadSetting();

	m_accessKey = theApp.GetProfileString(_T("Setting"), _T("AccessKey"));
	m_domain = theApp.GetProfileString(_T("Setting"), _T("Domain"));
	m_player = theApp.GetProfileString(_T("Setting"), _T("Player"));
	m_probe = theApp.GetProfileString(_T("Setting"), _T("FFProbe"));

	LoadPlaylist(theApp.GetProfileString(_T("Setting"), _T("Playlist")));

	if (m_current == CB_ERR && !m_channels.empty())
	{
		SetCurrentChannel(0);
		LoadChannelInfo();
	}

	UpdateData(FALSE);

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

	return __super::PreTranslateMessage(pMsg);
}

void CEdemChannelEditorDlg::OnDestroy()
{
	VERIFY(DestroyAcceleratorTable(m_hAccel));

	__super::OnDestroy();
}

void CEdemChannelEditorDlg::set_allow_save(BOOL val)
{
	m_allow_save = val;
}

BOOL CEdemChannelEditorDlg::LoadSetting()
{
	if (!LoadFromFile(theApp.GetAppPath(CHANNELS_CONFIG).GetString()))
		return FALSE;

	FillCategories();
	LoadChannels();

	set_allow_save(FALSE);

	return TRUE;
}

void CEdemChannelEditorDlg::LoadChannels()
{
	m_wndChannelsList.ResetContent();

	std::map<int, std::shared_ptr<ChannelInfo>> channelsMap;

	for (const auto& channel : m_channels)
	{
		int id = channel->get_channel_id();
		if (id != 0)
		{
			auto res = channelsMap.emplace(id, channel);
			if (!res.second)
			{
				CString msg;
				msg.Format(_T("Removed duplicate channel: %s (%d), %s"),
						   channel->get_title().c_str(),
						   channel->get_channel_id(),
						   res.first->second->get_title().c_str());
				AfxMessageBox(msg);
				continue;
			}
		}

		int idx = m_wndChannelsList.AddString(channel->get_title().c_str());
		m_wndChannelsList.SetItemData(idx, (DWORD_PTR)channel.get());
	}

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

	std::wstring path = theApp.GetAppPath(CHANNELS_CONFIG).GetString();

	// Категория должна содержать хотя бы один канал. Иначе плагин падает с ошибкой
	// [plugin] error: invalid plugin TV info: wrong num_channels(0) for group id '' in num_channels_by_group_id.

	std::set<int> group_ids;
	for (const auto& channel : m_channels)
	{
		const auto& cats = channel->get_categores();
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
		auto root = doc.allocate_node(rapidxml::node_element, TV_INFO);

		// create <tv_categories> node
		auto cat_node = doc.allocate_node(rapidxml::node_element, TV_CATEGORIES);

		// append <tv_category> to <tv_categories> node
		for (auto& category : m_categories)
		{
			cat_node->append_node(category.second->GetNode(doc));
		}
		// append <tv_categories> to <tv_info> node
		root->append_node(cat_node);

		// create <tv_channels> node
		auto ch_node = doc.allocate_node(rapidxml::node_element, TV_CHANNELS);
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
		int id = item.second->get_channel_id();
		item.second->set_colored(ids.find(id) == ids.end());
	}
}

void CEdemChannelEditorDlg::LoadChannelInfo()
{
	m_wndCategoriesList.ResetContent();

	auto channel = GetChannel(m_current);
	if (!channel)
	{
		m_channelName.Empty();
		m_tvgID = 0;
		m_prevDays = 0;
		m_nextDays = 0;
		m_hasArchive = 0;
		m_isAdult = 0;
		m_iconUrl.Empty();
		m_streamUrl.Empty();
		m_streamID = 0;
		m_infoAudio.Empty();
		m_infoVideo.Empty();
		m_wndIcon.SetBitmap(nullptr);
		m_wndCustom.SetCheck(FALSE);
	}
	else
	{
		m_channelName = channel->get_title().c_str();
		m_tvgID = channel->get_tvguide_id();
		m_prevDays = channel->get_prev_epg_days();
		m_nextDays = channel->get_next_epg_days();
		m_hasArchive = channel->get_has_archive();
		m_isAdult = channel->get_adult();
		m_iconUrl = channel->get_icon_uri().get_uri().c_str();
		m_streamUrl = channel->get_stream_uri().get_uri().c_str();
		m_streamID = channel->get_channel_id();
		m_infoAudio = channel->get_audio().c_str();
		m_infoVideo = channel->get_video().c_str();

		CheckForExisting();

		theApp.LoadImage(m_wndIcon, channel->get_icon_uri().get_icon_relative_path(theApp.GetAppPath(PLUGIN_ROOT)).c_str());

		for (const auto& id : channel->get_categores())
		{
			auto category = m_categories[id];
			int pos = m_wndCategoriesList.AddString(category->get_caption().c_str());
			m_wndCategoriesList.SetItemData(pos, (DWORD_PTR)category.get());
		}
		m_wndCustom.SetCheck(m_streamID == 0 && channel->is_custom());
	}

	m_wndStreamID.EnableWindow(m_streamID != 0);
	m_wndStreamUrl.EnableWindow(m_streamID == 0);

	UpdateData(FALSE);
}

void CEdemChannelEditorDlg::SaveChannelInfo()
{
	UpdateData(TRUE);
	// Save changes
	auto channel = GetChannel(m_current);
	if (!channel)
		return;

	channel->set_title(m_channelName.GetString());
	channel->set_tvguide_id(m_tvgID);
	channel->set_prev_epg_days(m_prevDays);
	channel->set_next_epg_days(m_nextDays);
	channel->set_has_archive(m_hasArchive);
	channel->set_adult(m_isAdult);

	channel->set_icon_uri(CStringA(m_iconUrl).GetString());

	if (m_wndCustom.GetCheck())
		channel->set_stream_uri(CStringA(m_streamUrl).GetString());

	channel->set_channel_id(m_streamID);

	CheckForExisting();
	std::set<int> newCategories;
	for (int i = 0; i < m_wndCategoriesList.GetCount(); i++)
	{
		auto category = (ChannelCategory*)m_wndCategoriesList.GetItemData(i);
		newCategories.emplace(category->get_id());
	}
	channel->set_categores(newCategories);
}

ChannelInfo* CEdemChannelEditorDlg::GetChannel(int idx)
{
	if (idx == LB_ERR)
		return nullptr;

	return (ChannelInfo*)m_wndChannelsList.GetItemData(idx);
}

ChannelInfo* CEdemChannelEditorDlg::GetCurrentChannel()
{
	return GetChannel(m_wndChannelsList.GetCurSel());
}

ChannelCategory* CEdemChannelEditorDlg::GetCategory(int idx)
{
	if (idx == CB_ERR)
		return nullptr;

	return (ChannelCategory*)m_wndCategories.GetItemData(idx);
}

PlaylistEntry* CEdemChannelEditorDlg::GetPlaylistEntry(HTREEITEM item)
{
	if (item == nullptr)
		return nullptr;

	return (PlaylistEntry*)m_wndPlaylistTree.GetItemData(item);
}

PlaylistEntry* CEdemChannelEditorDlg::GetCurrentPlaylistEntry()
{
	return GetPlaylistEntry(m_wndPlaylistTree.GetSelectedItem());
}

int CEdemChannelEditorDlg::FindCategory(const std::wstring& name)
{
	for (const auto& category : m_categories)
	{
		if (!_wcsicmp(category.second->get_caption().c_str(), name.c_str()))
			return category.second->get_id();
	}

	return -1;
}

int CEdemChannelEditorDlg::GetFreeCategoryID()
{
	std::set<int> busy;
	for (const auto& category : m_categories)
	{
		busy.emplace(category.second->get_id());
	}

	if (busy.empty())
		return 1;

	int free_id = *busy.begin();
	for (const auto& id : busy)
	{
		if (id != free_id) break;

		free_id = id;
		free_id++;
	}

	return free_id;
}

ChannelInfo* CEdemChannelEditorDlg::CreateChannel()
{
	auto category = std::make_unique<ChannelInfo>();
	auto retVal = category.get();
	m_channels.emplace_back(std::move(category));

	return retVal;
}

bool CEdemChannelEditorDlg::LoadFromFile(const std::wstring& path)
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

	auto i_node = doc.first_node(TV_INFO);

	auto cat_node = i_node->first_node(TV_CATEGORIES)->first_node(ChannelCategory::TV_CATEGORY);
	// Iterate <tv_category> nodes
	while (cat_node)
	{
		auto category = std::make_shared<ChannelCategory>(cat_node);
		m_categories.emplace(category->get_id(), category);
		cat_node = cat_node->next_sibling();
	}

	auto ch_node = i_node->first_node(TV_CHANNELS)->first_node(ChannelInfo::TV_CHANNEL);
	// Iterate <tv_channel> nodes
	while (ch_node)
	{
		m_channels.emplace_back(std::move(std::make_unique<ChannelInfo>(ch_node)));
		ch_node = ch_node->next_sibling();
	}

	return true;
}

void CEdemChannelEditorDlg::SetCurrentChannel(int idx)
{
	m_wndChannelsList.SetCurSel(idx);
	m_current = idx;
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

void CEdemChannelEditorDlg::OnBnClickedButtonAddChannel()
{
	UpdateData(TRUE);

	CNewChannelDlg dlg;

	if (dlg.DoModal() == IDOK)
	{
		auto channel = CreateChannel();
		channel->set_title(dlg.m_name.GetString());

		int idx = m_wndChannelsList.AddString(dlg.m_name);
		m_wndChannelsList.SetItemData(idx, (DWORD_PTR)channel);
		SetCurrentChannel(idx);
		LoadChannelInfo();
		set_allow_save();
		UpdateChannelsCount();
	}
}

void CEdemChannelEditorDlg::OnAccelRemoveChannel()
{
	CWnd* pFocused = GetFocus();
	if (pFocused == &m_wndChannelsList)
	{
		OnBnClickedButtonRemoveChannel();
		return;
	}

	if (pFocused == &m_wndCategoriesList)
	{
		OnBnClickedButtonRemoveCategory();
		return;
	}

	pFocused->SendMessage(WM_KEYDOWN, VK_DELETE);
}

void CEdemChannelEditorDlg::OnBnClickedButtonRemoveChannel()
{
	int idx = m_wndChannelsList.GetCurSel();
	auto channel = GetCurrentChannel();
	if (channel && AfxMessageBox(_T("Delete channel. Are your sure?"), MB_YESNO | MB_ICONWARNING) == IDYES)
	{
		int count = m_wndChannelsList.DeleteString(idx);
		if (idx >= count)
			idx = count - 1;

		auto found = std::find_if(m_channels.begin(), m_channels.end(), [channel](const auto& item)
								  {
									  return item.get() == channel;
								  });

		ASSERT(found != m_channels.end());
		if(found != m_channels.end())
		{
			m_channels.erase(found);
		}

		SetCurrentChannel(idx);
		LoadChannelInfo();
		set_allow_save();
		UpdateChannelsCount();
	}
}

void CEdemChannelEditorDlg::OnUpdateButtonRemoveChannel(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetCurrentPlaylistEntry() != nullptr);
}

void CEdemChannelEditorDlg::OnBnClickedCheckCustomize()
{
	BOOL checked = m_wndCustom.GetCheck();
	m_wndStreamUrl.EnableWindow(checked);
	m_wndStreamID.EnableWindow(!checked);

	set_allow_save();
}

void CEdemChannelEditorDlg::OnLbnSelchangeListChannels()
{
	int idx = m_wndChannelsList.GetCurSel();
	if(m_current != idx)
	{
		SaveChannelInfo();
		SetCurrentChannel(idx);
		LoadChannelInfo();
	}
}

void CEdemChannelEditorDlg::OnBnClickedButtonAddToShowIn()
{
	auto toAdd = GetCategory(m_wndCategories.GetCurSel());
	if (!toAdd)
		return;

	for (int i = 0; i < m_wndCategoriesList.GetCount(); i++)
	{
		auto category = (ChannelCategory*)m_wndCategoriesList.GetItemData(i);
		if(category->get_id() == toAdd->get_id()) return;
	}

	int added = m_wndCategoriesList.AddString(toAdd->get_caption().c_str());
	m_wndCategoriesList.SetItemData(added, (DWORD_PTR)toAdd);

	GetDlgItem(IDC_BUTTON_ADD)->EnableWindow(m_wndCategoriesList.GetCount());
	set_allow_save();
}

void CEdemChannelEditorDlg::OnUpdateButtonAddToShowIn(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_wndCategories.GetCount() != 0);
}

void CEdemChannelEditorDlg::OnBnClickedButtonRemoveFromShowIn()
{
	int idx = m_wndCategoriesList.GetCurSel();
	if (idx != CB_ERR)
	{
		m_wndCategoriesList.DeleteString(idx);
		set_allow_save();
	}
}

void CEdemChannelEditorDlg::OnUpdateButtonRemoveFromShow(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_wndCategoriesList.GetCurSel() != LB_ERR);
}

void CEdemChannelEditorDlg::OnEnKillfocusEditChannelName()
{
	UpdateData(TRUE);

	auto data = m_wndChannelsList.GetItemData(m_current);
	m_wndChannelsList.DeleteString(m_current);
	m_wndChannelsList.InsertString(m_current, m_channelName);
	m_wndChannelsList.SetItemData(m_current, data);

	set_allow_save();
}

void CEdemChannelEditorDlg::OnChanges()
{
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
	SaveChannelInfo();
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

void CEdemChannelEditorDlg::OnBnClickedButtonTestEpg()
{
	static LPCSTR url = "http://www.teleguide.info/kanal%d_%4d%02d%02d.html";

	UpdateData(TRUE);

	auto channel = GetCurrentChannel();
	if (channel)
	{
		COleDateTime dt = COleDateTime::GetCurrentTime();
		CStringA tvg_url;
		tvg_url.Format(url, channel->get_tvguide_id(), dt.GetYear(), dt.GetMonth(), dt.GetDay());
		ShellExecuteA(nullptr, "open", tvg_url.GetString(), nullptr, nullptr, SW_SHOWNORMAL);
	}
}

void CEdemChannelEditorDlg::OnUpdateButtonTestEpg(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_wndChannelsList.GetCurSel() != LB_ERR);
}

void CEdemChannelEditorDlg::OnBnClickedButtonTestUrl()
{
	UpdateData(TRUE);

	SaveChannelInfo();
	auto channel = GetCurrentChannel();
	if (channel)
	{
		PlayStream(TranslateStreamUri(channel->get_stream_uri().get_ts_translated_url()));
	}
}

void CEdemChannelEditorDlg::OnUpdateButtonTestUrl(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_wndChannelsList.GetCurSel() != LB_ERR);
}

std::wstring CEdemChannelEditorDlg::TranslateStreamUri(const std::string& stream_uri)
{
	// http://ts://{SUBDOMAIN}/iptv/{UID}/205/index.m3u8 -> http://ts://domain.com/iptv/000000000000/205/index.m3u8

	std::wregex re_domain(LR"(\{SUBDOMAIN\})");
	std::wregex re_uid(LR"(\{UID\})");

	std::wstring stream_url = utils::utf8_to_utf16(stream_uri);
	stream_url = std::regex_replace(stream_url, re_domain, m_domain.GetString());
	return std::regex_replace(stream_url, re_uid, m_accessKey.GetString());
}

void CEdemChannelEditorDlg::PlayStream(const std::wstring& stream_url)
{
	TRACE(_T("Test URL: %s"), stream_url.c_str());
	ShellExecute(nullptr, _T("open"), m_player, stream_url.c_str(), nullptr, SW_SHOWNORMAL);
}

void CEdemChannelEditorDlg::OnBnClickedButtonLoadPlaylist()
{
	CFileDialog dlg(TRUE);

	CString file;
	CString filter(_T("Playlist m3u8(*.m3u8)#*.m3u8#All Files (*.*)#*.*#"));
	filter.Replace('#', '\0');

	OPENFILENAME& oFN = dlg.GetOFN();
	oFN.lpstrFilter = filter.GetString();
	oFN.nMaxFile = MAX_PATH;
	oFN.nFilterIndex = 0;
	oFN.lpstrFile = file.GetBuffer(MAX_PATH);
	oFN.lpstrTitle = _T("Load Edem TV playlist");
	oFN.Flags |= OFN_EXPLORER | OFN_NOREADONLYRETURN | OFN_ENABLESIZING | OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_PATHMUSTEXIST;
	oFN.Flags |= OFN_FILEMUSTEXIST | OFN_NONETWORKBUTTON;

	INT_PTR nResult = dlg.DoModal();
	file.ReleaseBuffer();

	if (nResult == IDOK)
	{
		LoadPlaylist(file);
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

	std::set<int> ids;
	for (const auto& item : m_channels)
	{
		int id = item->get_channel_id();
		if (id)
			ids.emplace(id);
	}

	m_wndPlaylistTree.DeleteAllItems();
	m_playlist.clear();
	std::vector<std::pair<std::wstring, HTREEITEM>> categories;
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
		PlaylistEntry entry;
		while (std::getline(is, line))
		{
			entry.Parse(line);
			switch (entry.get_directive())
			{
				case ext_unknown:
					break;
				case ext_pathname:
					if (auto res = m_playlist.emplace(entry.get_channel_id(), std::make_unique<PlaylistEntry>(entry)); !res.second)
					{
						TRACE("Duplicate channel: %d\n", entry.get_channel_id());
					}

					if (auto found = std::find_if(categories.begin(), categories.end(), [entry](const auto& item)
												  {
													  return entry.get_category() == item.first;
												  });
						found == categories.end())
					{
						categories.emplace_back(entry.get_category(), nullptr);
					}
					entry.Clear();
					break;
				case ext_header:
					break;
				case ext_group:
					break;
				case ext_playlist:
					break;
				case ext_info:
					break;
				default:
					break;
			}
		}
	}

	// if access key and domain not set, try to load it from playlist
	if(m_accessKey.IsEmpty() && m_domain.IsEmpty() && !m_playlist.empty())
	{
		theApp.WriteProfileString(_T("Setting"), _T("AccessKey"), CString(m_playlist[0]->get_access_key().c_str()));
		theApp.WriteProfileString(_T("Setting"), _T("Domain"), CString(m_playlist[0]->get_domain().c_str()));
	}

	// fill playlist listbox
	if(!m_playlist.empty())
	{
		CheckForExisting();

		for (auto& category : categories)
		{
			TVINSERTSTRUCT tvInsert = { nullptr };
			tvInsert.item.pszText = (LPTSTR)category.first.c_str();
			tvInsert.hParent = TVI_ROOT;
			tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
			auto item = m_wndPlaylistTree.InsertItem(&tvInsert);
			category.second = item;
		}

		for (const auto& pair : m_playlist)
		{
			const auto& entry = pair.second.get();
			if (auto found = std::find_if(categories.begin(), categories.end(), [entry](const auto& item)
							 {
								 return entry->get_category() == item.first;
							 }); found != categories.end())
			{
				TVINSERTSTRUCT tvInsert = { nullptr };
				tvInsert.hParent = found->second;
				tvInsert.item.pszText = (LPTSTR)entry->get_title().c_str();
				tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
				tvInsert.item.lParam = (DWORD_PTR)entry;
				tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
				tvInsert.item.state = TVIS_EXPANDED;
				m_wndPlaylistTree.InsertItem(&tvInsert);
			}
		}
	}

	m_pl_cur_it = m_playlist.end();
	UpdatePlaylistCount();

	UpdateData(FALSE);
}

void CEdemChannelEditorDlg::UpdatePlaylistCount()
{
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

void CEdemChannelEditorDlg::OnBnClickedButtonAddCategory()
{
	UpdateData(TRUE);

	CNewCategoryDlg dlg;

	if (dlg.DoModal() == IDOK)
	{
		auto last_id = GetFreeCategoryID();
		auto newCategory = std::make_unique<ChannelCategory>();
		newCategory->set_id(last_id);
		newCategory->set_caption(dlg.m_name.GetString());
		newCategory->set_icon_url(CStringA(dlg.m_iconUrl).GetString());
		m_categories.emplace(last_id, std::move(newCategory));

		set_allow_save();
		FillCategories();
	}
}

void CEdemChannelEditorDlg::OnBnClickedButtonRemoveCategory()
{
	auto category = GetCategory(m_wndCategories.GetCurSel());
	if (!category)
		return;

	SaveChannelInfo();

	auto id = category->get_id();

	for (const auto& channel : m_channels)
	{
		if (auto found = channel->get_categores().find(id); found != channel->get_categores().cend())
		{
			CString msg;
			msg.Format(_T("Category is assigned to the channel '%s'.\nPlease remove it first."), m_categories[id]->get_caption().c_str());
			AfxMessageBox(msg);
			break;
		}
	}
}

void CEdemChannelEditorDlg::OnBnClickedButtonEditCategory()
{
	int idx = m_wndCategories.GetCurSel();
	auto category = GetCategory(idx);
	if (!category)
		return;

	SaveChannelInfo();

	CNewCategoryDlg dlg;

	dlg.m_name = category->get_caption().c_str();
	dlg.m_iconUrl = category->GetIconPath().c_str();

	if (dlg.DoModal() == IDOK)
	{
		category->set_caption(dlg.m_name.GetString());
		category->set_icon_url(CStringA(dlg.m_iconUrl).GetString());
		FillCategories();
		SetCurrentChannel(idx);
		LoadChannelInfo();
		set_allow_save();
	}
}

void CEdemChannelEditorDlg::OnStnClickedStaticIcon()
{
	SaveChannelInfo();

	CFileDialog dlg(TRUE);
	CString path = theApp.GetAppPath(CHANNELS_LOGO_PATH);
	CString file = theApp.GetAppPath(PLUGIN_ROOT) + m_iconUrl;
	file.Replace('/', '\\');

	CString filter(_T("PNG file(*.png)#*.png#All Files (*.*)#*.*#"));
	filter.Replace('#', '\0');

	OPENFILENAME& oFN = dlg.GetOFN();
	oFN.lpstrFilter = filter.GetString();
	oFN.nMaxFile = MAX_PATH;
	oFN.nFilterIndex = 0;
	oFN.lpstrFile = file.GetBuffer(MAX_PATH);
	oFN.lpstrTitle = _T("Load logotype image");
	oFN.lpstrInitialDir = path.GetString();
	oFN.Flags |= OFN_EXPLORER | OFN_NOREADONLYRETURN | OFN_ENABLESIZING | OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_PATHMUSTEXIST;
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
			theApp.LoadImage(m_wndIcon, path);
		}

		m_iconUrl = uri::PLUGIN_SCHEME;
		m_iconUrl += CHANNELS_LOGO_URL;
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

	CString plugin_folder = theApp.GetAppPath(PLUGIN_ROOT);

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

void CEdemChannelEditorDlg::OnEnKillfocusEditStreamUrl()
{
	UpdateData(TRUE);

	auto channel = GetCurrentChannel();
	if (channel)
	{
		channel->set_stream_uri(CStringA(m_streamUrl).GetString());
		m_streamID = channel->get_channel_id();

		set_allow_save();
		UpdateData(FALSE);

		m_wndCustom.SetCheck(m_streamID == 0);
		OnBnClickedCheckCustomize();
		SaveChannelInfo();
	}
}

void CEdemChannelEditorDlg::OnEnKillfocusEditUrlId()
{
	UpdateData(TRUE);

	auto channel = GetCurrentChannel();
	if (channel)
	{
		SaveChannelInfo();
		set_allow_save();

		for (int i = 0; i < m_wndChannelsList.GetCount(); i++)
		{
			CRect rc;
			if (m_wndChannelsList.GetItemRect(i, rc) != LB_ERR)
				m_wndChannelsList.InvalidateRect(&rc, 0);
		}

		m_wndPlaylistTree.InvalidateRect(nullptr);
	}
}

void CEdemChannelEditorDlg::OnUpdateButtonSearchNext(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_search.IsEmpty());
}

void CEdemChannelEditorDlg::OnBnClickedButtonSearchNext()
{
	SaveChannelInfo();

	if (m_search.IsEmpty())
		return;

	int selected = LB_ERR;
	if (m_search.GetLength() > 1 && m_search.GetAt(0) == '\\')
	{
		int id = _tstoi(m_search.Mid(1).GetString());
		for (int i = 0; i < m_wndChannelsList.GetCount(); i++)
		{
			auto channel = GetChannel(i);
			if (channel && channel->get_channel_id() == id)
			{
				selected = i;
				break;
			}
		}
	}
	else
	{
		int id = _tstoi(m_search.Mid(1).GetString());
		for (int i = 0; i < m_wndChannelsList.GetCount(); i++)
		{
			auto channel = GetChannel(i);
			if (channel && StrStrI(channel->get_title().c_str(), m_search.GetString()) != nullptr)
			{
				selected = i;
				break;
			}
		}
	}

	if(selected != LB_ERR)
	{
		SetCurrentChannel(selected);
		LoadChannelInfo();
	}
}

void CEdemChannelEditorDlg::OnEnKillfocusEditTvgId()
{
	set_allow_save();
	SaveChannelInfo();
}

void CEdemChannelEditorDlg::OnUpdateButtonPlSearchNext(CCmdUI* pCmdUI)
{
	UpdateData(TRUE);

	pCmdUI->Enable(!m_plSearch.IsEmpty());
}

void CEdemChannelEditorDlg::OnBnClickedButtonPlSearchNext()
{
	SaveChannelInfo();

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
				hSelected = FindTreeItem(entry.get());
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
				hSelected = FindTreeItem(entry.get());
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

HTREEITEM CEdemChannelEditorDlg::FindTreeItem(const PlaylistEntry* entry)
{
	HTREEITEM root = m_wndPlaylistTree.GetRootItem();
	while (root != nullptr)
	{
		HTREEITEM hSub = m_wndPlaylistTree.GetChildItem(root);

		// iterate subitems
		while (hSub)
		{
			if (entry == (PlaylistEntry*)m_wndPlaylistTree.GetItemData(hSub))
			{
				return hSub;
			}
			// get the next sibling item
			hSub = m_wndPlaylistTree.GetNextSiblingItem(hSub);
		}

		root = m_wndPlaylistTree.GetNextSiblingItem(root);
	}

	return nullptr;
}

void CEdemChannelEditorDlg::OnBnClickedButtonImport()
{
	auto entry = GetCurrentPlaylistEntry();
	if (!entry)
		return;

	auto channel = CreateChannel();
	channel->set_title(entry->get_title());
	channel->set_has_archive(entry->is_archive() != 0);
	channel->set_stream_uri(entry->get_streaming_uri());
	channel->set_icon_uri(entry->get_icon_uri());
	if (auto id = FindCategory(entry->get_category()); id != -1)
	{
		channel->get_categores().emplace(id);
		if (entry->get_category().find(L"зрослые") != std::wstring::npos)
		{
			channel->set_adult(TRUE);
		}
	}

	int idx = m_wndChannelsList.AddString(entry->get_title().c_str());
	m_wndChannelsList.SetItemData(idx, (DWORD_PTR)channel);

	SetCurrentChannel(idx);
	LoadChannelInfo();
	set_allow_save();
}

void CEdemChannelEditorDlg::OnUpdateButtonImport(CCmdUI* pCmdUI)
{
	BOOL enable = FALSE;
	auto entry = GetCurrentPlaylistEntry();
	if (entry)
	{
		auto found = std::find_if(m_channels.begin(), m_channels.end(), [entry](const auto& item)
								  {
									  return entry->get_channel_id() == item->get_channel_id();
								  });
		enable = found == m_channels.end();
	}

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

	if (entry && channel)
	{
		channel->set_icon_uri(entry->get_icon_uri());
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
			std::string path = CHANNELS_LOGO_URL;
			path += fname;

			uri icon_uri(utils::ICON_TEMPLATE);
			icon_uri.set_path(path);

			std::vector<BYTE> image;
			if (utils::DownloadIconLogo(utils::utf8_to_utf16(channel->get_icon_uri().get_uri()), image))
			{
				std::wstring fullPath = icon_uri.get_icon_relative_path(theApp.GetAppPath(PLUGIN_ROOT));
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
		m_search.Format(_T("\\%d"), entry->get_channel_id());
		m_plIconName = entry->get_icon_url().c_str();
		m_plEPG.Format(_T("EPG: %d"), entry->get_tvg_id());
		m_wndPlArchive.SetCheck(!!entry->is_archive());
		theApp.LoadImage(m_wndPlIcon, CString(entry->get_icon_url().c_str()));

		UpdateData(FALSE);
		OnBnClickedButtonSearchNext();
	}

	*pResult = 0;
}

void CEdemChannelEditorDlg::OnNMDblclkTreePaylist(NMHDR* pNMHDR, LRESULT* pResult)
{
	auto entry = GetCurrentPlaylistEntry();
	if (entry)
	{
		PlayStream(TranslateStreamUri(entry->get_streaming_uri().get_ts_translated_url()));
	}
	*pResult = 0;
}

void CEdemChannelEditorDlg::OnBnClickedButtonSort()
{
	UpdateData(TRUE);

	SaveChannelInfo();

	switch (m_sortType)
	{
		case 0:
		{
			std::sort(m_channels.begin(), m_channels.end(), [](const auto& l, const auto& r)
					  {
						  auto lit = *l->get_categores().begin();
						  auto rit = *r->get_categores().begin();
						  if (lit != rit)
							  return (lit < rit);
						 return l->get_title() < r->get_title();
					  });
			break;
		}
		case 1:
		{
			std::sort(m_channels.begin(), m_channels.end(), [](const auto& l, const auto& r)
					  {
						  return l->get_title() < r->get_title();
					  });
			break;
		}
		case 2:
		{
			std::sort(m_channels.begin(), m_channels.end(), [](const auto& l, const auto& r)
					  {
						  return l->get_channel_id() < r->get_channel_id();
					  });
			break;
		}
		default:
			break;
	}

	LoadChannels();
	LoadChannelInfo();
	set_allow_save();
}

void CEdemChannelEditorDlg::OnBnClickedButtonGetInfo()
{
	CWaitCursor cur;

	GetChannelStreamInfo(GetCurrentChannel());
	LoadChannelInfo();
}

void CEdemChannelEditorDlg::GetChannelStreamInfo(ChannelInfo* channel)
{
	if (!channel)
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
		TRACE(_T("Failed! Can't create stdout pipe to child process. Code: %u"), GetLastError());
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

	CString csCommand;
	csCommand.Format(_T("\"%s\" -hide_banner -show_streams %s"),
					 m_probe.GetString(),
					 TranslateStreamUri(channel->get_stream_uri().get_ts_translated_url()).c_str()
					 ); // argv[0] имя исполняемого файла

	BOOL bRunProcess = CreateProcess(m_probe.GetString(),		// 	__in_opt     LPCTSTR lpApplicationName
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
		TRACE(_T("Failed! Can't execute command: %s\nCode: %u"), csCommand.GetString(), GetLastError());
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
				TRACE(_T("Success! Exit code: %u"), dwExitCode);
				break;
			}

			if (nTimeout && CheckForTimeOut(dwStart, nTimeout * 1000))
			{
				bTimeout = TRUE;
				::TerminateProcess(pi.hProcess, 0);
				TRACE(_T("Failed! Execution Timeout"));
				break;
			}

			if (::GetExitCodeProcess(pi.hProcess, &dwExitCode))
			{
				for (;;)
				{
					if (nTimeout && CheckForTimeOut(dwStart, nTimeout * 1000)) break;

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
				TRACE(_T("GetExitCodeProcess failed. ErrorCode: %0u, try count: %0d"), ::GetLastError(), nErrorCount);
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
			streams[idx].emplace(pair[0], pair[1]);
		}
	}

	for (auto& stream : streams)
	{
		if (stream["codec_type"] == "audio")
		{
			std::string audio;
			audio += stream["codec_long_name"] + " ";
			audio += stream["sample_rate"] + " ";
			audio += stream["channel_layout"];
			channel->set_audio(audio);
		}

		if (stream["codec_type"] == "video")
		{
			std::string video;
			video += stream["width"] + "x";
			video += stream["height"] + " ";
			video += stream["codec_long_name"];
			channel->set_video(video);
		}
	}
}

void CEdemChannelEditorDlg::OnUpdateButtonGetInfo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_wndChannelsList.GetCurSel() != LB_ERR && !m_probe.IsEmpty());
}

void CEdemChannelEditorDlg::OnBnClickedButtonGetAllInfo()
{
	CWaitCursor cur;

	m_wndGetAllInfo.EnableWindow(FALSE);
	for(int i = 0; i < m_wndChannelsList.GetCount(); i++)
	{
		m_channelsInfo.Format(_T("Plugin channels (%d) %d"), m_channels.size(), i + 1);
		UpdateData(FALSE);
		GetChannelStreamInfo(GetChannel(i));
	}

	LoadChannelInfo();
	m_wndGetAllInfo.EnableWindow(TRUE);
	UpdateChannelsCount();
}
