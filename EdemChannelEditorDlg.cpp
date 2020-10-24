
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

#include "SevenZip/7zip/SevenZipWrapper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace SevenZip;

constexpr auto CHANNELS_CONFIG = L"edem_channel_list.xml";
constexpr auto CHANNELS_LOGO_PATH = "icons\\channels\\";

// CEdemChannelEditorDlg dialog

BEGIN_MESSAGE_MAP(CEdemChannelEditorDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_LOAD_PLAYLIST, &CEdemChannelEditorDlg::OnBnClickedButtonLoadPlaylist)
	ON_BN_CLICKED(IDC_CHECK_CUSTOMIZE, &CEdemChannelEditorDlg::OnBnClickedCheckCustomize)
	ON_CBN_SELCHANGE(IDC_COMBO_CHANNEL, &CEdemChannelEditorDlg::OnCbnSelchangeComboChannel)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CEdemChannelEditorDlg::OnBnClickedButtonAdd)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_PREV, &CEdemChannelEditorDlg::OnDeltaposSpinPrev)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_NEXT, &CEdemChannelEditorDlg::OnDeltaposSpinNext)
	ON_EN_CHANGE(IDC_EDIT_PREV_EPG, &CEdemChannelEditorDlg::OnEnChangeEditNum)
	ON_EN_CHANGE(IDC_EDIT_NEXT_EPG, &CEdemChannelEditorDlg::OnEnChangeEditNum)
	ON_BN_CLICKED(IDC_BUTTON_TEST_EPG, &CEdemChannelEditorDlg::OnBnClickedButtonTestEpg)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, &CEdemChannelEditorDlg::OnBnClickedButtonRemove)
	ON_BN_CLICKED(IDC_BUTTON_TEST_URL, &CEdemChannelEditorDlg::OnBnClickedButtonTestUrl)
	ON_EN_CHANGE(IDC_EDIT_TVG_ID, &CEdemChannelEditorDlg::OnChanges)
	ON_EN_CHANGE(IDC_EDIT_URL_ID, &CEdemChannelEditorDlg::OnChanges)
	ON_BN_CLICKED(IDC_CHECK_ARCHIVE, &CEdemChannelEditorDlg::OnChanges)
	ON_BN_CLICKED(IDC_CHECK_ADULT, &CEdemChannelEditorDlg::OnChanges)
	ON_EN_CHANGE(IDC_MFCEDITBROWSE_PLUGIN_ROOT, &CEdemChannelEditorDlg::OnEnChangeMfceditbrowsePluginRoot)
	ON_EN_CHANGE(IDC_MFCEDITBROWSE_PLAYER, &CEdemChannelEditorDlg::OnEnChangeMfceditbrowsePlayer)
	ON_EN_CHANGE(IDC_EDIT_KEY, &CEdemChannelEditorDlg::OnEnChangeEditKey)
	ON_EN_CHANGE(IDC_EDIT_DOMAIN, &CEdemChannelEditorDlg::OnEnChangeEditDomain)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CEdemChannelEditorDlg::OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_ADD_CATEGORY, &CEdemChannelEditorDlg::OnBnClickedButtonAddCategory)
	ON_STN_CLICKED(IDC_STATIC_ICON, &CEdemChannelEditorDlg::OnStnClickedStaticIcon)
	ON_BN_CLICKED(IDC_BUTTON_ABOUT, &CEdemChannelEditorDlg::OnBnClickedButtonAbout)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_CATEGORY, &CEdemChannelEditorDlg::OnBnClickedButtonRemoveCategory)
	ON_BN_CLICKED(IDC_BUTTON_PACK, &CEdemChannelEditorDlg::OnBnClickedButtonPack)
END_MESSAGE_MAP()


CEdemChannelEditorDlg::CEdemChannelEditorDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_EDEMCHANNELEDITOR_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CEdemChannelEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_CHANNEL, m_wndChannelList);
	DDX_Control(pDX, IDC_COMBO_CATEGORY, m_wndCategoriesList);
	DDX_Control(pDX, IDC_LIST_CATEGORIES, m_wndShowIn);
	DDX_Control(pDX, IDC_SPIN_PREV, m_wndSpinPrev);
	DDX_Control(pDX, IDC_SPIN_NEXT, m_wndSpinNext);
	DDX_Control(pDX, IDC_CHECK_CUSTOMIZE, m_wndCustom);
	DDX_Text(pDX, IDC_EDIT_CHANNEL_NAME, m_channelName);
	DDX_Text(pDX, IDC_EDIT_URL_ID, m_streamID);
	DDX_Text(pDX, IDC_EDIT_TVG_ID, m_TVGID);
	DDX_Control(pDX, IDC_EDIT_STREAM_URL, m_wndStreamUrl);
	DDX_Text(pDX, IDC_EDIT_STREAM_URL, m_streamUrl);
	DDX_Check(pDX, IDC_CHECK_ARCHIVE, m_hasArchive);
	DDX_Check(pDX, IDC_CHECK_ADULT, m_isAdult);
	DDX_Text(pDX, IDC_EDIT_PREV_EPG, m_prevDays);
	DDX_Text(pDX, IDC_EDIT_NEXT_EPG, m_nextDays);
	DDX_Control(pDX, IDC_STATIC_ICON, m_wndIcon);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_PLUGIN_ROOT, m_pluginRoot);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_PLAYER, m_player);
	DDX_Text(pDX, IDC_EDIT_KEY, m_accessKey);
	DDX_Text(pDX, IDC_EDIT_DOMAIN, m_domain);
	DDX_Control(pDX, IDC_BUTTON_SAVE, m_wndSave);
	DDX_Control(pDX, IDC_BUTTON_PACK, m_wndPack);
}

void CEdemChannelEditorDlg::OnOK()
{
	// Prevent exit on Enter
}

void CEdemChannelEditorDlg::OnCancel()
{
	if (is_allow_save() && AfxMessageBox(_T("You have unsaved changes. Are you sure?"), MB_YESNO | MB_ICONWARNING) != IDYES)
	{
		return;
	}

	EndDialog(IDCANCEL);
}

// CEdemChannelEditorDlg message handlers

BOOL CEdemChannelEditorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

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

	m_pluginRoot = theApp.GetProfileString(_T("Setting"), _T("PluginRoot"));
	m_player = theApp.GetProfileString(_T("Setting"), _T("Player"));
	m_accessKey = theApp.GetProfileString(_T("Setting"), _T("AccessKey"));
	m_domain = theApp.GetProfileString(_T("Setting"), _T("Domain"));

	LoadSetting();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CEdemChannelEditorDlg::set_allow_save(BOOL val)
{
	m_allow_save = val;
	if (m_wndSave.GetSafeHwnd())
		m_wndSave.EnableWindow(val);

	if (m_wndPack.GetSafeHwnd())
		m_wndPack.EnableWindow(val);
}

BOOL CEdemChannelEditorDlg::LoadSetting()
{
	if (m_pluginRoot.IsEmpty())
		return FALSE;

	CStringW listFile = m_pluginRoot + CHANNELS_CONFIG;
	if (!m_channels.LoadFromFile(listFile.GetString()))
		return FALSE;

	ReloadCategories();
	ReloadChannels();
	OnCbnSelchangeComboChannel();
	theApp.WriteProfileString(_T("Setting"), _T("PluginRoot"), m_pluginRoot);
	set_changed(FALSE);
	set_allow_save(FALSE);

	return TRUE;
}

void CEdemChannelEditorDlg::ReloadChannels()
{
	m_wndChannelList.ResetContent();
	const auto& channels = m_channels.get_channels();
	for (const auto& channel : channels)
	{
		int idx = m_wndChannelList.AddString(channel->get_name().c_str());
		m_wndChannelList.SetItemData(idx, (DWORD_PTR)channel.get());
	}

	if (!channels.empty())
		m_wndChannelList.SetCurSel(0);

	set_changed(FALSE);
}

void CEdemChannelEditorDlg::ReloadCategories()
{
	m_wndCategoriesList.ResetContent();
	const auto& categories = m_channels.get_categories();
	for (const auto& category : categories)
	{
		int idx = m_wndCategoriesList.AddString(category.second->get_caption().c_str());
		m_wndCategoriesList.SetItemData(idx, (DWORD_PTR)category.second.get());
	}

	if (!categories.empty())
		m_wndCategoriesList.SetCurSel(0);

	set_changed(FALSE);
}

void CEdemChannelEditorDlg::LoadImage(CStatic& wnd, const CString& fullPath)
{
	HBITMAP hImg = nullptr;
	CImage image;
	if (SUCCEEDED(image.Load(fullPath)))
	{
		CDC* screenDC = GetDC();

		CRect rc;
		wnd.GetClientRect(rc);

		CImage resized;
		resized.Create(rc.Width(), rc.Height(), 32);
		image.StretchBlt(resized.GetDC(), rc, SRCCOPY);

		resized.ReleaseDC();
		ReleaseDC(screenDC);

		hImg = (HBITMAP)resized.Detach();
	}

	HBITMAP hOld = wnd.SetBitmap(hImg);
	if (hOld)
		::DeleteObject(hOld);
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

void CEdemChannelEditorDlg::OnBnClickedCheckCustomize()
{
	m_wndStreamUrl.EnableWindow(m_wndCustom.GetCheck());
	OnChanges();
}

void CEdemChannelEditorDlg::OnCbnSelchangeComboChannel()
{
	int idx = m_wndChannelList.GetCurSel();
	if (idx != CB_ERR && idx != m_current)
	{
		if (is_changed())
		{
			// Save changes
			UpdateData(TRUE);

			auto channel = (ChannelInfo*)m_wndChannelList.GetItemData(m_current);
			channel->set_name(m_channelName.GetString());
			channel->set_tvguide_id(CStringA(m_TVGID).GetString());
			channel->set_prev_epg_days(m_prevDays);
			channel->set_next_epg_days(m_nextDays);
			channel->set_has_archive(m_hasArchive);
			channel->set_adult(m_isAdult);
			channel->SetIconPluginPath(CStringA(m_iconPath).GetString());
			channel->set_streaming_url(CStringA(m_streamUrl).GetString());

			std::set<int> newCategories;
			for (int i = 0; i < m_wndShowIn.GetCount(); i++)
			{
				auto category = (ChannelCategory*)m_wndShowIn.GetItemData(i);
				newCategories.emplace(category->get_id());
			}
			channel->set_categores(newCategories);
		}

		auto channel = (ChannelInfo*)m_wndChannelList.GetItemData(idx);
		m_channelName = channel->get_name().c_str();
		m_TVGID = channel->get_tvguide_id().c_str();
		m_prevDays = channel->get_prev_epg_days();
		m_nextDays = channel->get_next_epg_days();
		m_hasArchive = channel->get_has_archive();
		m_isAdult = channel->get_adult();
		m_iconPath = channel->GetIconRelativePath().c_str();
		m_streamUrl = channel->get_streaming_url().c_str();

		LoadImage(m_wndIcon, m_pluginRoot + m_iconPath);

		m_wndShowIn.ResetContent();
		for (const auto& id : channel->get_categores())
		{
			auto category = m_channels.get_categories().at(id).get();
			int pos = m_wndShowIn.AddString(category->get_caption().c_str());
			m_wndShowIn.SetItemData(pos, (DWORD_PTR)category);
		}

		m_streamID = channel->GetChannelIdFromStreamingUrl();

		m_wndCustom.SetCheck(m_streamID == 0);
		m_wndStreamUrl.EnableWindow(m_streamID == 0);
		m_current = idx;

		UpdateData(FALSE);

		set_changed(FALSE);
	}
}

void CEdemChannelEditorDlg::OnBnClickedButtonAdd()
{
	int idx = m_wndCategoriesList.GetCurSel();
	if (idx != CB_ERR)
	{
		auto toAdd = (ChannelCategory*)m_wndCategoriesList.GetItemData(idx);
		for (int i = 0; i < m_wndShowIn.GetCount(); i++)
		{
			auto category = (ChannelCategory*)m_wndShowIn.GetItemData(i);
			if(category->get_id() == toAdd->get_id()) return;
		}
		int added = m_wndShowIn.AddString(toAdd->get_caption().c_str());
		m_wndShowIn.SetItemData(added, (DWORD_PTR)toAdd);
		OnChanges();
	}
}

void CEdemChannelEditorDlg::OnBnClickedButtonRemove()
{
	int idx = m_wndShowIn.GetCurSel();
	if (idx != CB_ERR)
	{
		m_wndShowIn.DeleteString(idx);
		OnChanges();
	}
}

void CEdemChannelEditorDlg::OnChanges()
{
	set_changed(TRUE);
	set_allow_save(TRUE);
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

	OnChanges();

	UpdateData(FALSE);
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

void CEdemChannelEditorDlg::OnBnClickedButtonTestEpg()
{
	static LPCSTR url = "http://www.teleguide.info/kanal%s.html";

	UpdateData(TRUE);

	int idx = m_wndChannelList.GetCurSel();
	if (idx != CB_ERR)
	{
		auto channel = (ChannelInfo*)m_wndChannelList.GetItemData(idx);
		CStringA tvg_url;
		tvg_url.Format(url, channel->get_tvguide_id().c_str());
		ShellExecuteA(nullptr, "open", tvg_url.GetString(), nullptr, nullptr, SW_SHOWNORMAL);
	}
}

void CEdemChannelEditorDlg::OnBnClickedButtonTestUrl()
{
	UpdateData(TRUE);

	int idx = m_wndChannelList.GetCurSel();
	if (idx != CB_ERR)
	{
		CString tvg_url;
		auto channel = (ChannelInfo*)m_wndChannelList.GetItemData(idx);
		const auto& url = channel->get_streaming_url();
		if (m_wndCustom.GetCheck())
		{
			tvg_url = channel->TranslateStreamingUrl(url).c_str();
		}
		else
		{
			tvg_url = channel->CombineEdemStreamingUrl(CStringA(m_domain).GetString(), CStringA(m_accessKey).GetString()).c_str();
		}

		TRACE(_T("URL: %s"), tvg_url.GetString());
		ShellExecute(nullptr, _T("open"), _T("C:\\Program Files (x86)\\K-Lite Codec Pack\\MPC-HC64\\mpc-hc64.exe"), tvg_url.GetString(), nullptr, SW_SHOWNORMAL);
	}
}

void CEdemChannelEditorDlg::OnEnChangeMfceditbrowsePluginRoot()
{
	UpdateData(TRUE);

	if (m_pluginRoot.IsEmpty())
		return;
	if (m_pluginRoot.Right(1) != '\\')
		m_pluginRoot += '\\';

	CStringW listFile = m_pluginRoot + CHANNELS_CONFIG;
	if (!LoadSetting())
	{
		AfxMessageBox(_T("Can't find edem_channel_list.xml in selected folder"), MB_OK | MB_ICONERROR);
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
		// http://456sdf879.akadatel.com/iptv/SDASGREEWEGEGWE/2402/index.m3u8

		std::ifstream is(CStringA(file).GetString());
		std::string line;
		while (std::getline(is, line))
		{
			std::regex re(R"(http[s]{0,1}:\/\/([0-9a-z\.]+)\/iptv\/([0-9A-Za-z\.]+)\/(\d+)\/index.m3u8)");
			std::smatch m;
			if (std::regex_match(line, m, re))
			{
				m_domain = m[1].str().c_str();
				m_accessKey = m[2].str().c_str();
				theApp.WriteProfileString(_T("Setting"), _T("AccessKey"), m_accessKey);
				theApp.WriteProfileString(_T("Setting"), _T("Domain"), m_domain);
				break;
			}
		}

		UpdateData(FALSE);
	}
}

void CEdemChannelEditorDlg::OnEnChangeEditKey()
{
	UpdateData(TRUE);
	theApp.WriteProfileString(_T("Setting"), _T("AccessKey"), m_accessKey);
}

void CEdemChannelEditorDlg::OnEnChangeEditDomain()
{
	UpdateData(TRUE);
	m_domain.MakeLower();
	theApp.WriteProfileString(_T("Setting"), _T("Domain"), m_domain);

	UpdateData(FALSE);
}

void CEdemChannelEditorDlg::OnBnClickedButtonSave()
{
	CString path = m_pluginRoot + L"edem_channel_list_new.xml";
	set_allow_save(!m_channels.SaveToFile(path.GetString()));
}

void CEdemChannelEditorDlg::OnBnClickedButtonAddCategory()
{
	NewCategory dlg;
	dlg.m_pluginRoot = m_pluginRoot;

	if (dlg.DoModal() == IDOK)
	{
		auto& categories = m_channels.get_categories();
		auto last_id = m_channels.GetFreeID();
		auto newCategory = std::make_unique<ChannelCategory>();
		newCategory->set_id(last_id);
		newCategory->set_caption(dlg.m_name.GetString());
		newCategory->set_icon_url(CStringA(dlg.m_iconUrl).GetString());
		categories.emplace(last_id, std::move(newCategory));

		set_changed(TRUE);
		set_allow_save(TRUE);
		ReloadCategories();
	}
}

void CEdemChannelEditorDlg::OnBnClickedButtonRemoveCategory()
{
	int idx = m_wndCategoriesList.GetCurSel();
	if (idx == CB_ERR)
		return;
	auto category = (ChannelCategory*)m_wndCategoriesList.GetItemData(idx);
	auto id = category->get_id();

	for (const auto& channel : m_channels.get_channels())
	{
		if (auto found = channel->get_categores().find(id); found != channel->get_categores().cend())
		{
			CString msg;
			msg.Format(_T("Category is assigned to the channel '%s'. Please remove it first."), m_channels.get_categories().at(id)->get_caption().c_str());
			AfxMessageBox(msg);
			break;
		}
	}
}

void CEdemChannelEditorDlg::OnStnClickedStaticIcon()
{
	const CString logo_path = m_pluginRoot + CHANNELS_LOGO_PATH;

	CFileDialog dlg(TRUE);
	CString file(logo_path);
	CString filter(_T("PNG file(*.png)#*.png#All Files (*.*)#*.*#"));
	filter.Replace('#', '\0');

	OPENFILENAME& oFN = dlg.GetOFN();
	oFN.lpstrFilter = filter.GetString();
	oFN.nMaxFile = MAX_PATH;
	oFN.nFilterIndex = 0;
	oFN.lpstrFile = file.GetBuffer(MAX_PATH);
	oFN.lpstrTitle = _T("Load channel logo");
	oFN.Flags |= OFN_EXPLORER | OFN_NOREADONLYRETURN | OFN_ENABLESIZING | OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_PATHMUSTEXIST;
	oFN.Flags |= OFN_FILEMUSTEXIST | OFN_NONETWORKBUTTON;

	INT_PTR nResult = dlg.DoModal();
	file.ReleaseBuffer();

	if (nResult == IDOK)
	{
		LoadImage(m_wndIcon, logo_path + oFN.lpstrFileTitle);
		CString icon_url(CHANNELS_LOGO_PATH);
		icon_url += oFN.lpstrFileTitle;
		m_iconPath = icon_url;
		set_changed(TRUE);
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
	SevenZipWrapper archiver(_T("7z.dll"));
	archiver.GetCompressor().SetCompressionFormat(CompressionFormat::Zip);
	bool res = archiver.GetCompressor().AddFiles(_T(".\\edem\\"), _T("*.*"), true);
	if (!res)
		return;

	res = archiver.CreateArchive(_T("dune_plugin_edem_free4_mod.zip"));
	if (!res)
	{
		::DeleteFile(_T("dune_plugin_edem_free4_mod.zip"));
	}
}
