// VodViewer.cpp : implementation file
//

#include "pch.h"
#include <afxdialogex.h>
#include "resource.h"
#include "VodViewer.h"
#include "StreamContainer.h"

#include "UtilsLib\inet_utils.h"
#include "PlayListEntry.h"
#include "PlaylistParseM3U8Thread.h"
#include "IPTVChannelEditor.h"
#include "PlaylistParseJsonThread.h"

// CVodViewer dialog

enum class loadType
{
	enUnknown,
	enM3U8,
	enJsonFull,
	enJsonLazy,
};

IMPLEMENT_DYNAMIC(CVodViewer, CDialogEx)

BEGIN_MESSAGE_MAP(CVodViewer, CDialogEx)
	ON_WM_GETMINMAXINFO()
	ON_MESSAGE(WM_END_LOAD_PLAYLIST, &CVodViewer::OnEndLoadPlaylist)
	ON_MESSAGE(WM_END_LOAD_JSON_PLAYLIST, &CVodViewer::OnEndLoadJsonPlaylist)
	ON_MESSAGE(WM_INIT_PROGRESS, &CVodViewer::OnInitProgress)
	ON_MESSAGE(WM_UPDATE_PROGRESS, &CVodViewer::OnUpdateProgress)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_MOVIES, &CVodViewer::OnItemChanged)
	ON_NOTIFY(LVN_ODCACHEHINT, IDC_LIST_MOVIES, &CVodViewer::OnCacheHint)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_MOVIES, &CVodViewer::OnGetDispinfo)
	ON_CBN_SELCHANGE(IDC_COMBO_CATEGORIES, &CVodViewer::OnCbnSelchangeComboCategories)
	ON_CBN_SELCHANGE(IDC_COMBO_GENRES, &CVodViewer::OnCbnSelchangeComboGenres)
	ON_CBN_SELCHANGE(IDC_COMBO_YEARS, &CVodViewer::OnCbnSelchangeComboYears)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_MOVIES, &CVodViewer::OnNMDblclkListMovies)
END_MESSAGE_MAP()

CVodViewer::CVodViewer(vod_category_storage* categories, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_VOD, pParent)
	, m_vod_categories(categories)
	, m_evtStop(FALSE, TRUE)
	, m_evtFinished(TRUE, TRUE)
	, m_category_idx(0)
	, m_description(_T(""))
{
	ASSERT(m_vod_categories);
}

CVodViewer::~CVodViewer()
{
}

void CVodViewer::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_COMBO_CATEGORIES, m_wndCategories);
	DDX_CBIndex(pDX, IDC_COMBO_CATEGORIES, m_category_idx);
	DDX_Control(pDX, IDC_COMBO_YEARS, m_wndYears);
	DDX_Control(pDX, IDC_COMBO_GENRES, m_wndGenres);
	DDX_Control(pDX, IDC_STATIC_PROGRESS_INFO, m_wndProgressInfo);
	DDX_Control(pDX, IDC_LIST_MOVIES, m_wndMoviesList);
	DDX_Control(pDX, IDC_EDIT_SEARCH, m_wndSearchText);
	DDX_Control(pDX, IDC_BUTTON_SEARCH, m_wndSearch);
	DDX_Control(pDX, IDC_EDIT_TOTAL_MOVIES, m_wndTotal);
	DDX_Control(pDX, IDC_STATIC_ICON, m_wndPoster);
	DDX_Control(pDX, IDC_PROGRESS_LOAD, m_wndProgress);
	DDX_Control(pDX, IDC_COMBO_QUALITY, m_wndQuality);
	DDX_Control(pDX, IDC_STATIC_QUALITY, m_wndQualityText);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, m_description);
}

// CVodViewer message handlers

BOOL CVodViewer::OnInitDialog()
{
	__super::OnInitDialog();

	RestoreWindowPos(GetSafeHwnd(), REG_VOD_WINDOW_POS);

	m_wndMoviesList.SetExtendedStyle(m_wndMoviesList.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	CRect rect;
	m_wndMoviesList.GetClientRect(&rect);
	int vWidth = rect.Width() - GetSystemMetrics(SM_CXVSCROLL) - 1;

	CString str;
	str.LoadString(IDS_STRING_COL_INFO);
	m_wndMoviesList.InsertColumn(0, str, LVCFMT_LEFT, vWidth, 0);

	m_wndCategories.EnableWindow(FALSE);
	BOOL enableFilter = FALSE;
	BOOL enableGenre = FALSE;
	BOOL enableQuality = FALSE;

	const auto& plugin = StreamContainer::get_instance(m_plugin_type);
	loadType lt = loadType::enUnknown;
	switch (m_plugin_type)
	{
		case StreamType::enGlanz:
		case StreamType::enFox:
			lt = loadType::enM3U8;
			break;

		case StreamType::enAntifriz:
		case StreamType::enCbilling:
			enableGenre = TRUE;
			lt = loadType::enJsonLazy;
			break;

		case StreamType::enSharaclub:
			lt = loadType::enJsonFull;
			break;

		case StreamType::enEdem:
			enableFilter = TRUE;
			enableQuality = TRUE;
			lt = loadType::enJsonLazy;
			break;

		default:
			ASSERT(false);
			break;
	}

	m_wndYears.EnableWindow(enableFilter);
	m_wndGenres.EnableWindow(enableFilter);
	m_wndQuality.EnableWindow(enableQuality);
	m_wndQualityText.EnableWindow(enableQuality);

	const auto& url = fmt::format(plugin->get_vod_url(), GetConfig().get_string(false, REG_LOGIN), GetConfig().get_string(false, REG_PASSWORD));
	switch (lt)
	{
		case loadType::enM3U8:
			LoadM3U8Playlist(url);
			break;
		case loadType::enJsonFull:
			LoadJsonPlaylist(url);
			break;
		case loadType::enJsonLazy:
			LoadJsonCategories(url);
			break;
		default:
			ASSERT(false);
			break;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CVodViewer::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
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

void CVodViewer::OnCancel()
{
	m_evtStop.SetEvent();
	Sleep(1000);
	EndDialog(IDCANCEL);
}

BOOL CVodViewer::DestroyWindow()
{
	SaveWindowPos(GetSafeHwnd(), REG_VOD_WINDOW_POS);

	return __super::DestroyWindow();
}

void CVodViewer::LoadJsonCategories(const std::wstring& url)
{
	std::vector<BYTE> data;
	if (!utils::DownloadFile(url, data, true) || data.empty())
	{
		return;
	}

	JSON_ALL_TRY
	{
		nlohmann::json parsed_json = nlohmann::json::parse(data);

		switch (m_plugin_type)
		{
			case StreamType::enAntifriz:
				break;
			case StreamType::enEdem:
				break;
			case StreamType::enSharaclub:
				break;
			case StreamType::enCbilling:
				break;
			default:
				return;
		}
	}
	JSON_ALL_CATCH;
}

void CVodViewer::LoadJsonPlaylist(const std::wstring& url)
{
	if (!m_vod_categories->empty())
	{
		FillCategories();
		FillGenres();
		FillYears();
		FilterList();
		return;
	}

	m_evtStop.ResetEvent();
	m_evtFinished.ResetEvent();
	auto data = std::make_unique<std::vector<BYTE>>();

	if (!utils::DownloadFile(url, *data) || data->empty())
	{
		AfxMessageBox(IDS_STRING_ERR_THREAD_NOT_START, MB_OK | MB_ICONERROR);
		OnEndLoadPlaylist(0);
		return;
	}

	auto pThread = (CPlaylistParseJsonThread*)AfxBeginThread(RUNTIME_CLASS(CPlaylistParseJsonThread), THREAD_PRIORITY_HIGHEST, 0, CREATE_SUSPENDED);
	if (!pThread)
	{
		AfxMessageBox(IDS_STRING_ERR_THREAD_NOT_START, MB_OK | MB_ICONERROR);
		OnEndLoadPlaylist(0);
		return;
	}

	ThreadConfig cfg;
	cfg.m_parent = this;
	cfg.m_data = data.release();
	cfg.m_hStop = m_evtStop;
	cfg.m_pluginType = m_plugin_type;

	pThread->SetData(cfg);
	pThread->ResumeThread();

	m_wndProgress.ShowWindow(SW_HIDE);
	m_wndProgressInfo.ShowWindow(SW_HIDE);
}

void CVodViewer::LoadM3U8Playlist(const std::wstring& url)
{
	m_evtStop.ResetEvent();
	m_evtFinished.ResetEvent();
	if (!m_vod_categories->empty())
	{
		FillCategories();
		FillGenres();
		FillYears();
		FilterList();
		return;
	}

	auto data = std::make_unique<std::vector<BYTE>>();

	if (!utils::DownloadFile(url, *data) || data->empty())
	{
		AfxMessageBox(IDS_STRING_ERR_CANT_DOWNLOAD_PLAYLIST, MB_OK | MB_ICONERROR);
		OnEndLoadPlaylist(0);
		return;
	}

	auto pThread = (CPlaylistParseM3U8Thread*)AfxBeginThread(RUNTIME_CLASS(CPlaylistParseM3U8Thread), THREAD_PRIORITY_HIGHEST, 0, CREATE_SUSPENDED);
	if (!pThread)
	{
		AfxMessageBox(IDS_STRING_ERR_THREAD_NOT_START, MB_OK | MB_ICONERROR);
		OnEndLoadPlaylist(0);
		return;
	}

	ThreadConfig cfg;
	cfg.m_parent = this;
	cfg.m_data = data.release();
	cfg.m_hStop = m_evtStop;
	cfg.m_pluginType = m_plugin_type;

	pThread->SetData(cfg);
	pThread->ResumeThread();
}

LRESULT CVodViewer::OnEndLoadPlaylist(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	m_evtStop.ResetEvent();
	m_evtFinished.SetEvent();

	m_wndProgress.ShowWindow(SW_HIDE);
	m_wndProgressInfo.ShowWindow(SW_HIDE);

	m_playlistEntries.reset((Playlist*)wParam);
	m_vod_categories->clear();

	if (m_playlistEntries)
	{
		for (const auto& entry : m_playlistEntries->m_entries)
		{
			std::shared_ptr<vod_category> category;
			if (!m_vod_categories->tryGet(entry->get_category(), category))
			{
				category = std::make_shared<vod_category>(entry->get_category());
				m_vod_categories->set(entry->get_category(), category);
			}

			auto movie = std::make_shared<vod_movie>();
			movie->id = entry->get_epg_id();
			movie->title = entry->get_title();
			movie->poster_url = entry->get_icon_uri();
			movie->url = entry->get_uri_stream()->get_uri();
			category->movies.set(movie->id, movie);
		}
	}

	FillCategories();
	FillGenres();
	FillYears();
	FilterList();

	return 0;
}

LRESULT CVodViewer::OnEndLoadJsonPlaylist(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	m_evtStop.ResetEvent();
	m_evtFinished.SetEvent();

	m_wndProgress.ShowWindow(SW_HIDE);
	m_wndProgressInfo.ShowWindow(SW_HIDE);

	// make copy content not pointers!
	std::unique_ptr<vod_category_storage> deleter((vod_category_storage*)wParam);
	*m_vod_categories = *deleter;

	FillCategories();
	FillGenres();
	FillYears();
	FilterList();

	return 0;
}
LRESULT CVodViewer::OnInitProgress(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	m_wndProgress.SetRange32(0, (int)wParam);
	m_wndProgress.SetPos(0);
	m_wndProgress.ShowWindow(lParam ? SW_SHOW : SW_HIDE);
	m_wndProgressInfo.ShowWindow(lParam ? SW_SHOW : SW_HIDE);

	return 0;
}

LRESULT CVodViewer::OnUpdateProgress(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	CString str;
	str.Format(IDS_STRING_FMT_MOVIES_READED, wParam);
	m_wndProgressInfo.SetWindowText(str);
	m_wndProgress.SetPos(lParam);

	return 0;
}

void CVodViewer::OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	auto* pNMLV = (NMLISTVIEW*)pNMHDR;
	*pResult = 0;
	if (pNMLV->iItem == -1 || pNMLV->uOldState) return;

	return LoadMovieInfo(pNMLV->iItem);
}

void CVodViewer::OnCacheHint(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (!pNMHDR) return;

	auto* pCacheHint = (NMLVCACHEHINT*)pNMHDR;
	*pResult = 0;
}

void CVodViewer::OnGetDispinfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVDISPINFO* pDispInfo = (NMLVDISPINFO*)pNMHDR;
	LVITEM& lvItem = pDispInfo->item;
	*pResult = 0;

	if (0 == (lvItem.mask & LVIF_TEXT)) return; //valid text buffer?

	StrCpyN(lvItem.pszText, m_filtered_movies.getAt(lvItem.iItem)->title.c_str(), lvItem.cchTextMax - 1);
}

void CVodViewer::FillCategories()
{
	m_wndCategories.ResetContent();

	for (const auto& pair : m_vod_categories->vec())
	{
		for (const auto& movie_pair : pair.second->movies.vec())
		{
			for (const auto& genre : movie_pair.second->genres)
			{
				m_genres.emplace(genre);
			}
			m_years.emplace(movie_pair.second->year);
		}
	}

	for (const auto& pair : m_vod_categories->vec())
	{
		m_wndCategories.AddString(pair.first.c_str());
	}

	if (!m_vod_categories->empty())
	{
		m_wndCategories.EnableWindow(TRUE);
		m_category_idx = 0;
	}
}

void CVodViewer::FillGenres()
{
	m_wndGenres.ResetContent();

	for (const auto& genre : m_genres)
	{
		m_wndGenres.AddString(genre.c_str());
	}

	if (m_wndGenres.GetCount())
	{
		m_wndGenres.InsertString(0, L"All");
		m_wndGenres.EnableWindow(TRUE);
		m_wndGenres.SetCurSel(0);
	}
	UpdateData(FALSE);
}

void CVodViewer::FillYears()
{
	m_wndYears.ResetContent();

	for (const auto& year : m_years)
	{
		if (!year.empty())
		{
			m_wndYears.AddString(year.c_str());
		}
	}

	if (m_wndYears.GetCount())
	{
		m_wndYears.InsertString(0, L"All");
		m_wndYears.EnableWindow(TRUE);
		m_wndYears.SetCurSel(0);
	}
	UpdateData(FALSE);
}

void CVodViewer::LoadMovieInfo(int idx)
{
	UpdateData(TRUE);
	if (idx >= (int)m_filtered_movies.size()) return;

	const auto& movie = m_filtered_movies.getAt(idx);
	CImage img;
	if (LoadImageFromUrl(movie->poster_url.get_uri(), img))
	{
		SetImageControl(img, m_wndPoster);
	}

	m_description = movie->title.c_str();
	m_description += L"\r\n";
	m_description += movie->description.c_str();
	m_description += (movie->description.empty() ? L"" : L"\r\n");
	m_description += movie->year.c_str();
	m_description += L" ";
	m_description += movie->country.c_str();

	UpdateData(FALSE);
}

void CVodViewer::FilterList()
{
	UpdateData(TRUE);
	m_filtered_movies.clear();

	if (m_category_idx == LB_ERR || m_category_idx >= (int)m_vod_categories->size())
	{
		m_wndMoviesList.SetItemCount(0);
		m_wndMoviesList.Invalidate();
		return;
	}


	CString selectedGenre;
	if (m_wndGenres.IsWindowEnabled())
		m_wndGenres.GetLBText(m_wndGenres.GetCurSel(), selectedGenre);

	CString selectedYear;
	if (m_wndYears.IsWindowEnabled())
		m_wndYears.GetLBText(m_wndYears.GetCurSel(), selectedYear);

	bool genre = false;
	bool year = false;
	for (const auto& movie_pair : m_vod_categories->getAt(m_category_idx)->movies.vec())
	{
		const auto& movie = movie_pair.second;
		bool add = false;
		if (!m_wndGenres.IsWindowEnabled())
		{
			genre = true;
		}
		else if (!selectedGenre.IsEmpty())
		{
			if (m_wndGenres.GetCurSel() == 0)
			{
				genre = true;
			}
			else
			{
				genre = std::find_if(movie->genres.begin(), movie->genres.end(), [&selectedGenre](const auto& genre)
									 {
										 return (selectedGenre.CompareNoCase(genre.c_str()) == 0);

									 }) != movie->genres.end();
			}
		}

		if (!m_wndYears.IsWindowEnabled())
		{
			year = true;
		}
		else
		{
			year = m_wndYears.GetCurSel() == 0 || (!selectedYear.IsEmpty() && selectedYear.CompareNoCase(movie->year.c_str()) == 0);
		}

		if (genre && year)
		{
			m_filtered_movies.set(movie_pair.first, movie_pair.second);
		}
	}

	m_wndTotal.SetWindowText(fmt::format(L"{:d}", m_filtered_movies.size()).c_str());
	m_wndMoviesList.SetItemCount(m_filtered_movies.size());
	m_wndMoviesList.Invalidate();
	LoadMovieInfo(0);
}

void CVodViewer::OnCbnSelchangeComboCategories()
{
	FilterList();
}

void CVodViewer::OnCbnSelchangeComboGenres()
{
	FilterList();
}

void CVodViewer::OnCbnSelchangeComboYears()
{
	FilterList();
}

void CVodViewer::OnNMDblclkListMovies(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	UpdateData(TRUE);
	if (m_category_idx == LB_ERR || m_category_idx >= (int)m_vod_categories->size()) return;

	const auto& movies = m_vod_categories->getAt(m_category_idx)->movies;

	if (!pNMItemActivate || pNMItemActivate->iItem >= (int)movies.size()) return;

	const auto& url = movies.getAt(pNMItemActivate->iItem)->url;
	ShellExecuteW(nullptr, L"open", GetConfig().get_string(true, REG_PLAYER).c_str(), url.c_str(), nullptr, SW_SHOWNORMAL);
}
