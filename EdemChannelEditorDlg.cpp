
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
#include "NewCategory.h"
#include "NewChannel.h"
#include "utils.h"

#include "SevenZip/7zip/SevenZipWrapper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace SevenZip;

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
	ON_BN_CLICKED(IDC_BUTTON_ABOUT, &CEdemChannelEditorDlg::OnBnClickedButtonAbout)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CEdemChannelEditorDlg::OnBnClickedButtonAddToShowIn)
	ON_BN_CLICKED(IDC_BUTTON_ADD_CATEGORY, &CEdemChannelEditorDlg::OnBnClickedButtonAddCategory)
	ON_BN_CLICKED(IDC_BUTTON_ADD_CHANNEL, &CEdemChannelEditorDlg::OnBnClickedButtonAddChannel)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_CATEGORY, &CEdemChannelEditorDlg::OnBnClickedButtonEditCategory)
	ON_BN_CLICKED(IDC_BUTTON_IMPORT, &CEdemChannelEditorDlg::OnBnClickedButtonImport)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_PLAYLIST, &CEdemChannelEditorDlg::OnBnClickedButtonLoadPlaylist)
	ON_BN_CLICKED(IDC_BUTTON_PACK, &CEdemChannelEditorDlg::OnBnClickedButtonPack)
	ON_BN_CLICKED(IDC_BUTTON_PL_SEARCH_NEXT, &CEdemChannelEditorDlg::OnEnChangeEditPlSearch)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, &CEdemChannelEditorDlg::OnBnClickedButtonRemoveFromShowIn)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_CATEGORY, &CEdemChannelEditorDlg::OnBnClickedButtonRemoveCategory)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_CHANNEL, &CEdemChannelEditorDlg::OnBnClickedButtonRemoveChannel)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CEdemChannelEditorDlg::OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH_NEXT, &CEdemChannelEditorDlg::OnEnChangeEditSearch)
	ON_BN_CLICKED(IDC_BUTTON_TEST_EPG, &CEdemChannelEditorDlg::OnBnClickedButtonTestEpg)
	ON_BN_CLICKED(IDC_BUTTON_TEST_URL, &CEdemChannelEditorDlg::OnBnClickedButtonTestUrl)
	ON_BN_CLICKED(IDC_CHECK_ADULT, &CEdemChannelEditorDlg::OnChanges)
	ON_BN_CLICKED(IDC_CHECK_ARCHIVE, &CEdemChannelEditorDlg::OnChanges)
	ON_BN_CLICKED(IDC_CHECK_CUSTOMIZE, &CEdemChannelEditorDlg::OnBnClickedCheckCustomize)
	ON_EN_CHANGE(IDC_EDIT_CHANNEL_NAME, &CEdemChannelEditorDlg::OnChanges)
	ON_EN_CHANGE(IDC_EDIT_NEXT_EPG, &CEdemChannelEditorDlg::OnEnChangeEditNum)
	ON_EN_CHANGE(IDC_EDIT_PL_SEARCH, &CEdemChannelEditorDlg::OnEnChangeEditPlSearch)
	ON_EN_CHANGE(IDC_EDIT_PREV_EPG, &CEdemChannelEditorDlg::OnEnChangeEditNum)
	ON_EN_CHANGE(IDC_EDIT_SEARCH, &CEdemChannelEditorDlg::OnEnChangeEditSearch)
	ON_EN_CHANGE(IDC_MFCEDITBROWSE_PLAYER, &CEdemChannelEditorDlg::OnEnChangeMfceditbrowsePlayer)
	ON_EN_KILLFOCUS(IDC_EDIT_CHANNEL_NAME, &CEdemChannelEditorDlg::OnEnKillfocusEditChannelName)
	ON_EN_KILLFOCUS(IDC_EDIT_DOMAIN, &CEdemChannelEditorDlg::OnEnKillfocusEditDomain)
	ON_EN_KILLFOCUS(IDC_EDIT_KEY, &CEdemChannelEditorDlg::OnEnKillfocusEditKey)
	ON_EN_KILLFOCUS(IDC_EDIT_STREAM_URL, &CEdemChannelEditorDlg::OnEnKillfocusEditStreamUrl)
	ON_EN_KILLFOCUS(IDC_EDIT_TVG_ID, &CEdemChannelEditorDlg::OnEnKillfocusEditTvgId)
	ON_EN_KILLFOCUS(IDC_EDIT_URL_ID, &CEdemChannelEditorDlg::OnEnKillfocusEditUrlId)
	ON_LBN_DBLCLK(IDC_LIST_PLAYLIST, &CEdemChannelEditorDlg::OnLbnDblclkListPlaylist)
	ON_LBN_SELCHANGE(IDC_LIST_CHANNELS, &CEdemChannelEditorDlg::OnLbnSelchangeListChannels)
	ON_LBN_SELCHANGE(IDC_LIST_PLAYLIST, &CEdemChannelEditorDlg::OnLbnSelchangeListPlaylist)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_NEXT, &CEdemChannelEditorDlg::OnDeltaposSpinNext)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_PREV, &CEdemChannelEditorDlg::OnDeltaposSpinPrev)
	ON_STN_CLICKED(IDC_STATIC_ICON, &CEdemChannelEditorDlg::OnStnClickedStaticIcon)
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
	DDX_Control(pDX, IDC_SPIN_PREV, m_wndSpinPrev);
	DDX_Control(pDX, IDC_SPIN_NEXT, m_wndSpinNext);
	DDX_Control(pDX, IDC_CHECK_CUSTOMIZE, m_wndCustom);
	DDX_Text(pDX, IDC_EDIT_CHANNEL_NAME, m_channelName);
	DDX_Text(pDX, IDC_EDIT_URL_ID, m_streamID);
	DDX_Text(pDX, IDC_EDIT_TVG_ID, m_TVGID);
	DDX_Control(pDX, IDC_EDIT_URL_ID, m_wndStreamID);
	DDX_Control(pDX, IDC_EDIT_STREAM_URL, m_wndStreamUrl);
	DDX_Text(pDX, IDC_EDIT_STREAM_URL, m_streamUrl);
	DDX_Check(pDX, IDC_CHECK_ARCHIVE, m_hasArchive);
	DDX_Check(pDX, IDC_CHECK_ADULT, m_isAdult);
	DDX_Text(pDX, IDC_EDIT_PREV_EPG, m_prevDays);
	DDX_Text(pDX, IDC_EDIT_NEXT_EPG, m_nextDays);
	DDX_Control(pDX, IDC_STATIC_ICON, m_wndIcon);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_PLAYER, m_player);
	DDX_Text(pDX, IDC_EDIT_KEY, m_accessKey);
	DDX_Text(pDX, IDC_EDIT_DOMAIN, m_domain);
	DDX_Control(pDX, IDC_BUTTON_SAVE, m_wndSave);
	DDX_Control(pDX, IDC_BUTTON_PACK, m_wndPack);
	DDX_Control(pDX, IDC_LIST_CHANNELS, m_wndChannelsList);
	DDX_Text(pDX, IDC_EDIT_SEARCH, m_search);
	DDX_Control(pDX, IDC_LIST_PLAYLIST, m_wndPlaylist);
	DDX_Text(pDX, IDC_EDIT_PL_SEARCH, m_plSearch);
	DDX_Text(pDX, IDC_STATIC_ICON_NAME, m_iconUrl);
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

	LOGFONT lfDlg;
	GetFont()->GetLogFont(&lfDlg);
	CDC* dc = GetDC();
	lfDlg.lfHeight = -MulDiv(160, GetDeviceCaps(dc->m_hDC, LOGPIXELSY), 720);
	lfDlg.lfWeight = FW_BOLD;
	m_largeFont.CreateFontIndirect(&lfDlg);

	GetDlgItem(IDC_STATIC_TITLE)->SetFont(&m_largeFont);

	LoadSetting();

	m_player = theApp.GetProfileString(_T("Setting"), _T("Player"));
	m_accessKey = theApp.GetProfileString(_T("Setting"), _T("AccessKey"));
	m_domain = theApp.GetProfileString(_T("Setting"), _T("Domain"));

	LoadPlaylist(theApp.GetProfileString(_T("Setting"), _T("Playlist")));

	UpdateControls();

	UpdateData(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CEdemChannelEditorDlg::OnOK()
{
	// Prevent exit on Enter
}

void CEdemChannelEditorDlg::OnCancel()
{
	if (is_allow_save() && AfxMessageBox(_T("You have unsaved changes.\nAre you sure?"), MB_YESNO | MB_ICONWARNING) != IDYES)
	{
		return;
	}

	EndDialog(IDCANCEL);
}

void CEdemChannelEditorDlg::UpdateControls()
{
	GetDlgItem(IDC_BUTTON_ADD)->EnableWindow(m_wndCategories.GetCount());
	GetDlgItem(IDC_BUTTON_REMOVE)->EnableWindow(m_wndCategoriesList.GetCurSel() != LB_ERR);
	GetDlgItem(IDC_BUTTON_SEARCH_NEXT)->EnableWindow(!m_search.IsEmpty());
	GetDlgItem(IDC_BUTTON_PL_SEARCH_NEXT)->EnableWindow(!m_plSearch.IsEmpty());
}

void CEdemChannelEditorDlg::set_allow_save(BOOL val)
{
	m_allow_save = val;
	if (m_wndSave.GetSafeHwnd())
		m_wndSave.EnableWindow(val);

	UpdateControls();
}

BOOL CEdemChannelEditorDlg::LoadSetting()
{
	std::wstring path = theApp.GetAppPath().GetString();
	path += CHANNELS_CONFIG;

	if (!m_channels.LoadFromFile(path))
		return FALSE;

	FillCategories();
	LoadChannels();

	if (m_current == CB_ERR && !m_channels.get_channels().empty())
	{
		m_current = 0;
		m_wndChannelsList.SetCurSel(m_current);
	}

	LoadChannelInfo();

	set_allow_save(FALSE);

	return TRUE;
}

void CEdemChannelEditorDlg::LoadChannels()
{
	m_wndChannelsList.ResetContent();
	const auto& channels = m_channels.get_channels();
	for (const auto& channel : channels)
	{
		int idx = m_wndChannelsList.AddString(channel->get_name().c_str());
		m_wndChannelsList.SetItemData(idx, (DWORD_PTR)channel.get());
	}
}

void CEdemChannelEditorDlg::FillCategories()
{
	m_wndCategories.ResetContent();
	const auto& categories = m_channels.get_categories();
	for (const auto& category : categories)
	{
		int idx = m_wndCategories.AddString(category.second->get_caption().c_str());
		m_wndCategories.SetItemData(idx, (DWORD_PTR)category.second.get());
	}

	if (!categories.empty())
		m_wndCategories.SetCurSel(0);
}

void CEdemChannelEditorDlg::LoadChannelInfo()
{
	auto channel = GetChannel(m_current);
	if (!channel)
		return;

	m_channelName = channel->get_name().c_str();
	m_TVGID = channel->get_tvguide_id().c_str();
	m_prevDays = channel->get_prev_epg_days();
	m_nextDays = channel->get_next_epg_days();
	m_hasArchive = channel->get_has_archive();
	m_isAdult = channel->get_adult();
	m_iconUrl = channel->GetIconRelativePath().c_str();
	m_streamUrl = channel->get_streaming_url().c_str();
	m_streamID = channel->get_edem_channel_id();

	CString path = theApp.GetAppPath() + PLUGIN_ROOT + m_iconUrl;
	theApp.LoadImage(m_wndIcon, path);

	m_wndCategoriesList.ResetContent();
	for (const auto& id : channel->get_categores())
	{
		auto category = m_channels.get_categories().at(id).get();
		int pos = m_wndCategoriesList.AddString(category->get_caption().c_str());
		m_wndCategoriesList.SetItemData(pos, (DWORD_PTR)category);
	}

	m_wndCustom.SetCheck(m_streamID == 0);
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

	channel->set_name(m_channelName.GetString());
	channel->set_tvguide_id(CStringA(m_TVGID).GetString());
	channel->set_prev_epg_days(m_prevDays);
	channel->set_next_epg_days(m_nextDays);
	channel->set_has_archive(m_hasArchive);
	channel->set_adult(m_isAdult);
	channel->SetIconPluginPath(CStringA(m_iconUrl).GetString());
	channel->set_streaming_url(CStringA(m_streamUrl).GetString());

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

PlaylistEntry* CEdemChannelEditorDlg::GetPlaylistEntry(int idx)
{
	if (idx == LB_ERR)
		return nullptr;

	return (PlaylistEntry*)m_wndPlaylist.GetItemData(idx);
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
		CDialog::OnSysCommand(nID, lParam);
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
		CDialog::OnPaint();
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

	NewChannel dlg;

	if (dlg.DoModal() == IDOK)
	{
		auto channel = m_channels.CreateChannel();
		channel->set_name(dlg.m_name.GetString());

		int idx = m_wndChannelsList.AddString(dlg.m_name);
		m_wndChannelsList.SetItemData(idx, (DWORD_PTR)channel);
		m_wndChannelsList.SetCurSel(idx);
		m_current = idx;
		LoadChannelInfo();
		set_allow_save();
	}
}

void CEdemChannelEditorDlg::OnBnClickedButtonRemoveChannel()
{
	auto channel = GetChannel(m_current);
	if (channel && AfxMessageBox(_T("Delete channel. Are your sure?"), MB_YESNO | MB_ICONWARNING) == IDYES)
	{
		int count = m_wndChannelsList.DeleteString(m_current);
		if (m_current >= count)
			m_current = count - 1;

		auto& channels = m_channels.get_channels();
		auto found = std::find_if(channels.begin(), channels.end(), [channel](const auto& item)
								  {
									  return item.get() == channel;
								  });

		ASSERT(found != channels.end());
		channels.erase(found);
		LoadChannelInfo();
		set_allow_save();
	}
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

void CEdemChannelEditorDlg::OnBnClickedButtonRemoveFromShowIn()
{
	int idx = m_wndCategoriesList.GetCurSel();
	if (idx != CB_ERR)
	{
		m_wndCategoriesList.DeleteString(idx);
		set_allow_save();
	}
	GetDlgItem(IDC_BUTTON_ADD)->EnableWindow(m_wndCategoriesList.GetCount());
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
	static LPCSTR url = "http://www.teleguide.info/kanal%s.html";

	UpdateData(TRUE);

	auto channel = GetChannel(m_wndChannelsList.GetCurSel());
	if (channel)
	{
		CStringA tvg_url;
		tvg_url.Format(url, channel->get_tvguide_id().c_str());
		ShellExecuteA(nullptr, "open", tvg_url.GetString(), nullptr, nullptr, SW_SHOWNORMAL);
	}
}

void CEdemChannelEditorDlg::OnBnClickedButtonTestUrl()
{
	UpdateData(TRUE);

	SaveChannelInfo();
	auto channel = GetChannel(m_wndChannelsList.GetCurSel());
	if (channel)
	{
		CString tvg_url;
		const auto& url = channel->get_streaming_url();
		if (m_wndCustom.GetCheck())
		{
			tvg_url = ChannelInfo::TranslateStreamingUrl(url).c_str();
		}
		else
		{
			tvg_url = channel->CombineEdemStreamingUrl(CStringA(m_domain).GetString(), CStringA(m_accessKey).GetString()).c_str();
		}

		TRACE(_T("Test URL: %s"), tvg_url.GetString());
		ShellExecute(nullptr, _T("open"), _T("C:\\Program Files (x86)\\K-Lite Codec Pack\\MPC-HC64\\mpc-hc64.exe"), tvg_url.GetString(), nullptr, SW_SHOWNORMAL);
	}
}

void CEdemChannelEditorDlg::OnEnChangeMfceditbrowsePlayer()
{
	UpdateData(TRUE);

	if (m_player.IsEmpty())
		return;

	theApp.WriteProfileString(_T("Setting"), _T("Player"), m_player);
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

	std::ifstream is(CStringA(file).GetString());
	if (!is.good())
		return;

	int step = 0;
	PlaylistEntry entry;

	std::string line;
	while (std::getline(is, line))
	{
		if (!strncmp(line.c_str(), "#EXTM3U", 7)) continue;

		if (step == 3)
		{
			auto res = m_playlist.emplace(entry.id, std::make_unique<PlaylistEntry>(entry));
			if (!res.second)
			{
				TRACE("Duplicate channel: %d\n", entry.id);
			}
			entry.Clear();
			step = 0;
		}

		if (!strncmp(line.c_str(), "#EXTINF", 7))
		{
			// #EXTINF:0 tvg-rec="3",Первый HD
			std::regex re_name(R"(#EXTINF:\d+\stvg-rec=\"(\d+)\",(.*))");
			std::smatch m_name;
			if (std::regex_match(line, m_name, re_name))
			{
				entry.archive = utils::char_to_int(m_name[1].str().c_str());
				entry.name = utils::utf8_to_utf16(m_name[2].str());
				step++;
			}
			continue;
		}

		if (!strncmp(line.c_str(), "#EXTGRP", 7))
		{
			std::regex re_name("#EXTGRP:(.*)");
			std::smatch m_name;
			if (std::regex_match(line, m_name, re_name))
			{
				entry.category = utils::utf8_to_utf16(m_name[1].str());
				step++;
			}
			continue;
		}

		if (step == 2)
		{
			if (m_accessKey.IsEmpty() && m_domain.IsEmpty())
			{
				std::regex re_url(R"(http[s]{0,1}:\/\/([0-9a-z\.]+)\/iptv\/([0-9A-Za-z\.]+)\/\d+\/index.m3u8)");
				std::smatch m_url;
				if (std::regex_match(line, m_url, re_url))
				{
					m_domain = m_url[1].str().c_str();
					m_accessKey = m_url[2].str().c_str();
					theApp.WriteProfileString(_T("Setting"), _T("AccessKey"), m_accessKey);
					theApp.WriteProfileString(_T("Setting"), _T("Domain"), m_domain);
				}
			}

			std::regex re_url(R"(http[s]{0,1}:\/\/[0-9a-z\.]+\/iptv\/[0-9A-Za-z\.]+\/(\d+)\/index.m3u8)");
			std::smatch m_url;
			if (std::regex_match(line, m_url, re_url))
			{
				entry.id = atoi(m_url[1].str().c_str());
				entry.url = line;
				step++;
				continue;
			}
		}
	}

	for (const auto& pair : m_playlist)
	{
		const auto& entry = pair.second;
		CString name;
		name.Format(_T("%s (%d) - %s%s"),
					entry->name.c_str(),
					entry->id,
					entry->category.c_str(),
					entry->archive ? _T(" (R)") : _T(""));

		int idx = m_wndPlaylist.AddString(name);
		m_wndPlaylist.SetItemData(idx, (DWORD_PTR)entry.get());
	}
}

void CEdemChannelEditorDlg::OnEnKillfocusEditKey()
{
	UpdateData(TRUE);
	theApp.WriteProfileString(_T("Setting"), _T("AccessKey"), m_accessKey);
}

void CEdemChannelEditorDlg::OnEnKillfocusEditDomain()
{
	UpdateData(TRUE);
	m_domain.MakeLower();
	theApp.WriteProfileString(_T("Setting"), _T("Domain"), m_domain);

	UpdateData(FALSE);
}

void CEdemChannelEditorDlg::OnBnClickedButtonSave()
{
	SaveChannelInfo();

	std::wstring path = theApp.GetAppPath().GetString();
	path += CHANNELS_CONFIG;

	BOOL res = m_channels.SaveToFile(path);
	set_allow_save(!res);
	if (res)
	{
		LoadSetting();
		m_wndChannelsList.SetCurSel(m_current);
	}
}

void CEdemChannelEditorDlg::OnBnClickedButtonAddCategory()
{
	UpdateData(TRUE);

	NewCategory dlg;

	if (dlg.DoModal() == IDOK)
	{
		auto& categories = m_channels.get_categories();
		auto last_id = m_channels.GetFreeCategoryID();
		auto newCategory = std::make_unique<ChannelCategory>();
		newCategory->set_id(last_id);
		newCategory->set_caption(dlg.m_name.GetString());
		newCategory->set_icon_url(CStringA(dlg.m_iconUrl).GetString());
		categories.emplace(last_id, std::move(newCategory));

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

	for (const auto& channel : m_channels.get_channels())
	{
		if (auto found = channel->get_categores().find(id); found != channel->get_categores().cend())
		{
			CString msg;
			msg.Format(_T("Category is assigned to the channel '%s'.\nPlease remove it first."), m_channels.get_categories().at(id)->get_caption().c_str());
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

	NewCategory dlg;

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
	CString path = theApp.GetAppPath() + CHANNELS_LOGO_PATH;
	CString file = theApp.GetAppPath() + PLUGIN_ROOT + m_iconUrl;
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

	CString dllFile = theApp.GetAppPath();
#ifdef _DEBUG
	dllFile += _T("..\\dll\\");
#endif // _DEBUG
	dllFile += _T("7za.dll");

	CString plugin_folder = theApp.GetAppPath() + PLUGIN_ROOT;

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
		channel->set_streaming_url(CStringA(m_streamUrl).GetString());
		m_streamID = channel->get_edem_channel_id();

		UpdateData(FALSE);

		m_wndCustom.SetCheck(m_streamID == 0);
		OnBnClickedCheckCustomize();
		SaveChannelInfo();
	}
}

void CEdemChannelEditorDlg::OnEnKillfocusEditUrlId()
{
	UpdateData(TRUE);

	auto channel = GetChannel(m_wndChannelsList.GetCurSel());
	if (channel)
	{
		m_streamUrl = channel->SetChannelIdForStreamingUrl(m_streamID).c_str();
		UpdateData(FALSE);
		set_allow_save();
		SaveChannelInfo();
	}
}

void CEdemChannelEditorDlg::OnEnChangeEditSearch()
{
	UpdateData(TRUE);

	if (m_search.IsEmpty())
		return;

	if (m_search.GetAt(0) == '\\')
	{
		CString num = m_search.Mid(1);
		if (!num.IsEmpty())
		{
			int id = _tstoi(m_search.Mid(1).GetString());
			for (int i = 0; i < m_wndChannelsList.GetCount(); i++)
			{
				auto channel = GetChannel(i);
				if (channel && channel->get_edem_channel_id() == id)
				{
					m_wndChannelsList.SetCurSel(i);
					m_current = i;
					LoadChannelInfo();
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
			m_current = idx;
			LoadChannelInfo();
		}
	}

	UpdateControls();
}

void CEdemChannelEditorDlg::OnLbnSelchangeListPlaylist()
{
	UpdateData(TRUE);

	auto entry = GetPlaylistEntry(m_wndPlaylist.GetCurSel());
	if (entry)
	{
		m_search.Format(_T("\\%d"), entry->id);
		const auto& channels = m_channels.get_channels();
		auto found = std::find_if(channels.begin(), channels.end(), [entry](const auto& item)
					 {
						 return entry->id == item->get_edem_channel_id();
					 });
		GetDlgItem(IDC_BUTTON_IMPORT)->EnableWindow(found == channels.end());

		UpdateData(FALSE);
		OnEnChangeEditSearch();
	}

	UpdateControls();
}

void CEdemChannelEditorDlg::OnEnKillfocusEditTvgId()
{
	SaveChannelInfo();
}

void CEdemChannelEditorDlg::OnLbnDblclkListPlaylist()
{
	auto entry = GetPlaylistEntry(m_wndPlaylist.GetCurSel());
	if (entry)
	{
		TRACE(_T("Test URL: %hs"), entry->url.c_str());
		ShellExecute(nullptr,
					 _T("open"),
					 _T("C:\\Program Files (x86)\\K-Lite Codec Pack\\MPC-HC64\\mpc-hc64.exe"),
					 CString(entry->url.c_str()).GetString(),
					 nullptr,
					 SW_SHOWNORMAL);
	}

	UpdateControls();
}

void CEdemChannelEditorDlg::OnEnChangeEditPlSearch()
{
	UpdateData(TRUE);

	if (m_plSearch.IsEmpty())
		return;

	if (m_plSearch.GetAt(0) == '\\')
	{
		CString num = m_plSearch.Mid(1);
		if (!num.IsEmpty())
		{
			int id = _tstoi(m_plSearch.Mid(1).GetString());
			for (int i = 0; i < m_wndPlaylist.GetCount(); i++)
			{
				auto entry = (PlaylistEntry*)m_wndPlaylist.GetItemData(i);
				if (entry && entry->id == id)
				{
					m_wndPlaylist.SetCurSel(i);
					break;
				}
			}
		}
	}
	else
	{
		int idx = m_wndPlaylist.SelectString(m_pl_current, m_plSearch);
		if (idx != CB_ERR)
		{
			m_pl_current = idx;
		}
	}

	UpdateControls();
}

void CEdemChannelEditorDlg::OnBnClickedButtonImport()
{
	auto entry = GetPlaylistEntry(m_wndPlaylist.GetCurSel());
	if (!entry)
		return;

	auto channel = m_channels.CreateChannel();
	channel->set_name(entry->name);
	channel->set_has_archive(entry->archive != 0);
	channel->set_streaming_url(ChannelInfo::ConvertPlainUrlToStreamingUrl(entry->url));
	if (auto id = m_channels.FindCategory(entry->category); id != -1)
	{
		channel->get_categores().emplace(id);
	}

	int idx = m_wndChannelsList.AddString(entry->name.c_str());
	m_wndChannelsList.SetItemData(idx, (DWORD_PTR)channel);
	m_wndChannelsList.SetCurSel(idx);
	m_current = idx;

	LoadChannelInfo();
	set_allow_save();
}
