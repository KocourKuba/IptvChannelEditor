
// EdemChannelEditorDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "framework.h"
#include <afxdialogex.h>
#include <fstream>
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
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_CHANNEL, &CEdemChannelEditorDlg::OnBnClickedButtonRemoveChannel)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_REMOVE_CHANNEL, &CEdemChannelEditorDlg::OnUpdateButtonRemoveChannel)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CEdemChannelEditorDlg::OnBnClickedButtonSave)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_SAVE, &CEdemChannelEditorDlg::OnUpdateButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_ADD_CHANNEL, &CEdemChannelEditorDlg::OnBnClickedButtonAddChannel)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_ADD_CHANNEL, &CEdemChannelEditorDlg::OnUpdateButtonAddChannel)

	ON_COMMAND(ID_ACC_UPDATE_ICON, &CEdemChannelEditorDlg::OnBnClickedButtonUpdateIcon)
	ON_COMMAND(ID_ACC_ADD_CHANNEL, &CEdemChannelEditorDlg::OnBnClickedButtonAddChannel)
	ON_COMMAND(ID_ACC_SAVE, &CEdemChannelEditorDlg::OnBnClickedButtonSave)
	ON_COMMAND(ID_ACC_DELETE_CHANNEL, &CEdemChannelEditorDlg::OnBnClickedButtonRemoveChannel)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_PAYLIST, &CEdemChannelEditorDlg::OnTvnSelchangedTreePaylist)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_PAYLIST, &CEdemChannelEditorDlg::OnNMDblclkTreePaylist)
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
	DDX_Text(pDX, IDC_STATIC_PLAYLIST, m_plFileName);
	DDX_Text(pDX, IDC_STATIC_PL_TVG, m_plEPG);
	DDX_Control(pDX, IDC_CHECK_PL_ARCHIVE, m_wndPlArchive);
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

	m_player = theApp.GetProfileString(_T("Setting"), _T("Player"));
	m_accessKey = theApp.GetProfileString(_T("Setting"), _T("AccessKey"));
	m_domain = theApp.GetProfileString(_T("Setting"), _T("Domain"));

	LoadPlaylist(theApp.GetProfileString(_T("Setting"), _T("Playlist")));

	if (m_current == CB_ERR && !m_channels.empty())
	{
		m_current = 0;
		m_wndChannelsList.SetCurSel(m_current);
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

	if (!m_search.IsEmpty())
	{
		OnBnClickedButtonSearchNext();
	}
	else if (!m_plSearch.IsEmpty())
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
						   channel->get_name().c_str(),
						   channel->get_channel_id(),
						   res.first->second->get_name().c_str());
				AfxMessageBox(msg);
				continue;
			}
		}

		int idx = m_wndChannelsList.AddString(channel->get_name().c_str());
		m_wndChannelsList.SetItemData(idx, (DWORD_PTR)channel.get());
	}
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
	auto channel = GetChannel(m_current);
	if (!channel)
		return;

	m_channelName = channel->get_name().c_str();
	m_tvgID = channel->get_tvguide_id();
	m_prevDays = channel->get_prev_epg_days();
	m_nextDays = channel->get_next_epg_days();
	m_hasArchive = channel->get_has_archive();
	m_isAdult = channel->get_adult();
	m_iconUrl = channel->get_icon_uri().get_uri().c_str();
	m_streamUrl = channel->get_streaming_uri().get_uri().c_str();
	m_streamID = channel->get_channel_id();

	CheckForExisting();

	theApp.LoadImage(m_wndIcon, channel->get_icon_uri().get_icon_relative_path(theApp.GetAppPath(PLUGIN_ROOT)).c_str());

	m_wndCategoriesList.ResetContent();
	for (const auto& id : channel->get_categores())
	{
		auto category = m_categories[id];
		int pos = m_wndCategoriesList.AddString(category->get_caption().c_str());
		m_wndCategoriesList.SetItemData(pos, (DWORD_PTR)category.get());
	}

	m_wndCustom.SetCheck(m_streamID == 0 && channel->is_custom());
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

	channel->get_icon_uri().set_uri(CStringA(m_iconUrl).GetString());

	if (m_wndCustom.GetCheck())
		channel->get_streaming_uri().set_uri(CStringA(m_streamUrl).GetString());

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
		m_wndChannelsList.SetCurSel(idx);
		m_current = idx;
		LoadChannelInfo();
		set_allow_save();
	}
}

void CEdemChannelEditorDlg::OnUpdateButtonAddChannel(CCmdUI* pCmdUI)
{
	auto entry = GetPlaylistEntry(m_wndPlaylistTree.GetSelectedItem());
	pCmdUI->Enable(entry != nullptr);
}

void CEdemChannelEditorDlg::OnBnClickedButtonRemoveChannel()
{
	auto channel = GetChannel(m_current);
	if (channel && AfxMessageBox(_T("Delete channel. Are your sure?"), MB_YESNO | MB_ICONWARNING) == IDYES)
	{
		int count = m_wndChannelsList.DeleteString(m_current);
		if (m_current >= count)
			m_current = count - 1;

		auto found = std::find_if(m_channels.begin(), m_channels.end(), [channel](const auto& item)
								  {
									  return item.get() == channel;
								  });

		ASSERT(found != m_channels.end());
		if(found != m_channels.end())
		{
			m_channels.erase(found);
		}

		m_wndChannelsList.SetCurSel(m_current);
		LoadChannelInfo();
		set_allow_save();
	}
}

void CEdemChannelEditorDlg::OnUpdateButtonRemoveChannel(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_wndChannelsList.GetCurSel() != LB_ERR);
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
	if(m_current != CB_ERR)
	{
		SaveChannelInfo();
	}

	int idx = m_wndChannelsList.GetCurSel();
	if (idx != CB_ERR && idx != m_current)
	{
		m_current = idx;
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
	static LPCSTR url = "http://www.teleguide.info/kanal%d.html";

	UpdateData(TRUE);

	auto channel = GetChannel(m_wndChannelsList.GetCurSel());
	if (channel)
	{
		CStringA tvg_url;
		tvg_url.Format(url, channel->get_tvguide_id());
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
	auto channel = GetChannel(m_wndChannelsList.GetCurSel());
	if (channel)
	{
		PlayStream(channel->get_streaming_uri().get_ts_translated_url());
	}
}

void CEdemChannelEditorDlg::OnUpdateButtonTestUrl(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_wndChannelsList.GetCurSel() != LB_ERR);
}


void CEdemChannelEditorDlg::PlayStream(const std::string& stream_uri)
{
	// http://ts://{SUBDOMAIN}/iptv/{UID}/205/index.m3u8 -> http://ts://domain.com/iptv/000000000000/205/index.m3u8

	std::wregex re_domain(LR"(\{SUBDOMAIN\})");
	std::wregex re_uid(LR"(\{UID\})");

	std::wstring stream_url = utils::utf8_to_utf16(stream_uri);
	stream_url = std::regex_replace(stream_url, re_domain, m_domain.GetString());
	stream_url = std::regex_replace(stream_url, re_uid, m_accessKey.GetString());;

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
		m_plFileName.Format(_T("Playlist: %s"), file.Mid(++pos));
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
		m_wndCategories.SetCurSel(idx);
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
	oFN.Flags |= OFN_FILEMUSTEXIST | OFN_NONETWORKBUTTON | OFN_NOCHANGEDIR;

	INT_PTR nResult = dlg.DoModal();
	file.ReleaseBuffer();

	if (nResult == IDOK)
	{
		theApp.LoadImage(m_wndIcon, file);

		m_iconUrl = CHANNELS_LOGO_URL;
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

	auto channel = GetChannel(m_wndChannelsList.GetCurSel());
	if (channel)
	{
		channel->get_streaming_uri().set_uri(CStringA(m_streamUrl).GetString());
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

	int idx = m_wndChannelsList.GetCurSel();
	auto channel = GetChannel(idx);
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
	if (m_search.GetAt(0) == '\\')
	{
		CString num = m_search.Mid(1);
		if (!num.IsEmpty())
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
	}
	else
	{
		int idx = m_wndChannelsList.SelectString(m_current, m_search);
		if (idx != CB_ERR)
		{
			selected = idx;
		}
	}

	m_wndChannelsList.SetCurSel(selected);
	m_current = selected;
	LoadChannelInfo();
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

HTREEITEM CEdemChannelEditorDlg::FindItem(const CString& name, HTREEITEM hRoot)
{
	// check whether the current item is the searched one
	auto entry = GetPlaylistEntry(hRoot);
	if(entry && _wcsnicmp(entry->get_title().c_str(), name.GetString(), name.GetLength()) == 0)
		return hRoot;

	// get a handle to the first child item
	HTREEITEM hSub = m_wndPlaylistTree.GetChildItem(hRoot);
	// iterate as long a new item is found
	while (hSub)
	{
		// check the children of the current item
		HTREEITEM hFound = FindItem(name, hSub);
		if (hFound)
			return hFound;

		// get the next sibling of the current item
		hSub = m_wndPlaylistTree.GetNextSiblingItem(hSub);
	}

	// return NULL if nothing was found
	return nullptr;
}

HTREEITEM CEdemChannelEditorDlg::FindItem(int id, HTREEITEM hRoot)
{
	// check whether the current item is the searched one
	auto entry = GetPlaylistEntry(hRoot);
	if (entry && entry->get_channel_id() == id)
		return hRoot;

	// get a handle to the first child item
	HTREEITEM hSub = m_wndPlaylistTree.GetChildItem(hRoot);
	// iterate as long a new item is found
	while (hSub)
	{
		// check the children of the current item
		HTREEITEM hFound = FindItem(id, hSub);
		if (hFound)
			return hFound;

		// get the next sibling of the current item
		hSub = m_wndPlaylistTree.GetNextSiblingItem(hSub);
	}

	// return NULL if nothing was found
	return nullptr;
}

void CEdemChannelEditorDlg::OnBnClickedButtonPlSearchNext()
{
	SaveChannelInfo();

	if (m_plSearch.IsEmpty())
		return;

	HTREEITEM selected = nullptr;

	HTREEITEM root = m_pl_current;
	if (root == nullptr)
		root = m_wndPlaylistTree.GetRootItem();
	else
		root = m_wndPlaylistTree.GetNextItem(root, TVGN_NEXT);

	if (m_plSearch.GetAt(0) == '\\')
	{
		CString num = m_plSearch.Mid(1);
		if (!num.IsEmpty())
		{
			int id = _tstoi(m_plSearch.Mid(1).GetString());
			while (root != nullptr)
			{
				HTREEITEM hFound = FindItem(id, root);
				if (hFound)
				{
					selected = hFound;
					break;
				}

				root = m_wndPlaylistTree.GetNextSiblingItem(root);
			}
		}
	}
	else
	{
		while (root != nullptr)
		{
			HTREEITEM hFound = FindItem(m_plSearch, root);
			if (hFound)
			{
				selected = hFound;
				break;
			}

			root = m_wndPlaylistTree.GetNextSiblingItem(root);
		}
	}

	m_pl_current = selected;
	if (selected)
	{
		m_wndPlaylistTree.SelectItem(selected);
		m_wndPlaylistTree.Expand(m_wndPlaylistTree.GetParentItem(selected), TVE_EXPAND);
		m_wndPlaylistTree.EnsureVisible(selected);
	}
}

void CEdemChannelEditorDlg::OnBnClickedButtonImport()
{
	auto entry = GetPlaylistEntry(m_wndPlaylistTree.GetSelectedItem());
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
	}

	int idx = m_wndChannelsList.AddString(entry->get_title().c_str());
	m_wndChannelsList.SetItemData(idx, (DWORD_PTR)channel);
	m_wndChannelsList.SetCurSel(idx);
	m_current = idx;

	LoadChannelInfo();
	set_allow_save();
}

void CEdemChannelEditorDlg::OnUpdateButtonImport(CCmdUI* pCmdUI)
{
	BOOL enable = FALSE;
	auto entry = GetPlaylistEntry(m_wndPlaylistTree.GetSelectedItem());
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
	dlg.DoModal();
}

void CEdemChannelEditorDlg::OnBnClickedButtonUpdateIcon()
{
	auto entry = GetPlaylistEntry(m_wndPlaylistTree.GetSelectedItem());
	auto channel = GetChannel(m_wndChannelsList.GetCurSel());

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
	auto entry = GetPlaylistEntry(m_wndPlaylistTree.GetSelectedItem());
	auto channel = GetChannel(m_wndChannelsList.GetCurSel());
	if (channel && entry)
	{
		bool eq = channel->get_icon_uri() == entry->get_icon_uri();
		enable = !channel->is_icon_local() && !eq;
	}

	pCmdUI->Enable(enable);
}

void CEdemChannelEditorDlg::OnBnClickedButtonCacheIcon()
{
	auto channel = GetChannel(m_wndChannelsList.GetCurSel());
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
	BOOL enable = FALSE;
	auto channel = GetChannel(m_wndChannelsList.GetCurSel());
	if (channel)
	{
		enable = !channel->is_icon_local();
	}

	pCmdUI->Enable(enable);
}


void CEdemChannelEditorDlg::OnTvnSelchangedTreePaylist(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	UpdateData(TRUE);

	m_pl_current = m_wndPlaylistTree.GetSelectedItem();
	auto entry = GetPlaylistEntry(m_pl_current);
	if (entry)
	{
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
	m_pl_current = m_wndPlaylistTree.GetSelectedItem();
	auto entry = GetPlaylistEntry(m_pl_current);
	if (entry)
	{
		PlayStream(entry->get_streaming_uri().get_ts_translated_url());
	}
	*pResult = 0;
}
