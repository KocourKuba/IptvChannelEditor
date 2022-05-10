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
#include "IconCache.h"

// CVodViewer dialog

enum class jsonParserType
{
	enUnknown = 0,
	enJsonSharaClub,
	enJsonCbilling,
	enJsonEdem,
};

IMPLEMENT_DYNAMIC(CVodViewer, CDialogEx)

BEGIN_MESSAGE_MAP(CVodViewer, CDialogEx)
	ON_WM_GETMINMAXINFO()
	ON_MESSAGE(WM_END_LOAD_PLAYLIST, &CVodViewer::OnEndLoadM3U8Playlist)
	ON_MESSAGE(WM_END_LOAD_JSON_PLAYLIST, &CVodViewer::OnEndLoadJsonPlaylist)
	ON_MESSAGE(WM_INIT_PROGRESS, &CVodViewer::OnInitProgress)
	ON_MESSAGE(WM_UPDATE_PROGRESS, &CVodViewer::OnUpdateProgress)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_MOVIES, &CVodViewer::OnItemChanged)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_MOVIES, &CVodViewer::OnGetDispinfo)
	ON_CBN_SELCHANGE(IDC_COMBO_CATEGORIES, &CVodViewer::OnCbnSelchangeComboCategories)
	ON_CBN_SELCHANGE(IDC_COMBO_GENRES, &CVodViewer::OnCbnSelchangeComboGenres)
	ON_CBN_SELCHANGE(IDC_COMBO_YEARS, &CVodViewer::OnCbnSelchangeComboYears)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_MOVIES, &CVodViewer::OnNMDblclkListMovies)
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, &CVodViewer::OnBnClickedButtonRefresh)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, &CVodViewer::OnBnClickedButtonSearch)
END_MESSAGE_MAP()

CVodViewer::CVodViewer(vod_category_storage* categories, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_VOD, pParent)
	, m_vod_categories(categories)
	, m_evtStop(FALSE, TRUE)
	, m_evtFinished(TRUE, TRUE)
	, m_SearchText(_T(""))
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
	DDX_Control(pDX, IDC_COMBO_GENRES, m_wndGenres);
	DDX_CBIndex(pDX, IDC_COMBO_GENRES, m_genre_idx);
	DDX_Control(pDX, IDC_COMBO_YEARS, m_wndYears);
	DDX_CBIndex(pDX, IDC_COMBO_YEARS, m_year_idx);
	DDX_Control(pDX, IDC_COMBO_SEASON, m_wndSeason);
	DDX_CBIndex(pDX, IDC_COMBO_SEASON, m_season_idx);
	DDX_Control(pDX, IDC_COMBO_EPISODE, m_wndEpisode);
	DDX_CBIndex(pDX, IDC_COMBO_EPISODE, m_episode_idx);
	DDX_Control(pDX, IDC_COMBO_QUALITY, m_wndQuality);
	DDX_Control(pDX, IDC_LIST_MOVIES, m_wndMoviesList);
	DDX_Control(pDX, IDC_BUTTON_SEARCH, m_wndSearch);
	DDX_Control(pDX, IDC_EDIT_TOTAL_MOVIES, m_wndTotal);
	DDX_Control(pDX, IDC_STATIC_ICON, m_wndPoster);
	DDX_Control(pDX, IDC_RICHEDIT_DESCRIPTION, m_wndDescription);
	DDX_Control(pDX, IDC_STATIC_PROGRESS_INFO, m_wndProgressInfo);
	DDX_Control(pDX, IDC_PROGRESS_LOAD, m_wndProgress);
	DDX_Text(pDX, IDC_EDIT_SEARCH, m_SearchText);
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

	m_plugin = StreamContainer::get_instance(m_plugin_type);

	if (auto pos = m_domain.find(':'); pos != std::wstring::npos)
	{
		m_domain = m_domain.substr(0, pos);
	}

	LoadPlaylist();

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

BOOL CVodViewer::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->wParam == VK_RETURN)
	{
		m_wndSearch.SendMessage(BM_CLICK, 0, 0);
		return TRUE;
	}

	return __super::PreTranslateMessage(pMsg);
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

void CVodViewer::LoadPlaylist(bool use_cache /*= true*/)
{
	m_wndCategories.EnableWindow(FALSE);
	BOOL enableFilter = FALSE;
	BOOL enableGenre = FALSE;
	BOOL enableQuality = FALSE;

	switch (m_plugin_type)
	{
		case StreamType::enAntifriz:
		case StreamType::enCbilling:
			enableGenre = TRUE;
			break;

		case StreamType::enEdem:
			enableFilter = TRUE;
			enableQuality = TRUE;
			break;

		default:
			break;
	}

	m_wndYears.EnableWindow(enableFilter);
	m_wndGenres.EnableWindow(enableFilter);
	m_wndQuality.EnableWindow(enableQuality);

	switch (m_plugin_type)
	{
		case StreamType::enGlanz:
		case StreamType::enFox:
			LoadM3U8Playlist(use_cache);
			break;

		case StreamType::enAntifriz:
		case StreamType::enCbilling:
		case StreamType::enSharaclub:
		case StreamType::enEdem:
			LoadJsonPlaylist(use_cache);
			break;

		default:
			ASSERT(false);
			break;
	}
}

void CVodViewer::LoadJsonPlaylist(bool use_cache /*= true*/)
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

	std::wstring url;
	jsonParserType parserType = jsonParserType::enUnknown;
	switch (m_plugin_type)
	{
		case StreamType::enAntifriz:
		case StreamType::enCbilling:
			parserType = jsonParserType::enJsonCbilling;
			url = m_plugin->get_vod_url();
			break;

		case StreamType::enSharaclub:
			parserType = jsonParserType::enJsonSharaClub;
			url = fmt::format(m_plugin->get_vod_url(), m_login, m_password);
			break;

		case StreamType::enEdem:
			parserType = jsonParserType::enJsonEdem;
			break;

		default:
			ASSERT(false);
			break;
	}

	auto pThread = (CPlaylistParseJsonThread*)AfxBeginThread(RUNTIME_CLASS(CPlaylistParseJsonThread), THREAD_PRIORITY_HIGHEST, 0, CREATE_SUSPENDED);
	if (!pThread)
	{
		AfxMessageBox(IDS_STRING_ERR_THREAD_NOT_START, MB_OK | MB_ICONERROR);
		OnEndLoadJsonPlaylist(0);
		return;
	}

	ThreadConfig cfg;
	cfg.m_parent = this;
	cfg.m_hStop = m_evtStop;
	cfg.m_pluginType = m_plugin_type;
	cfg.m_url = url;
	cfg.m_parser = (int)parserType;
	cfg.m_use_cache = use_cache;

	pThread->SetData(cfg);
	pThread->ResumeThread();

	m_wndProgress.ShowWindow(SW_HIDE);
	m_wndProgressInfo.ShowWindow(SW_HIDE);
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

	for (const auto& pair : m_vod_categories->vec())
	{
		for (const auto& movie_pair : pair.second->movies.vec())
		{
			for (const auto& genre : movie_pair.second->genres)
			{
				m_genres.set(genre, genre);
			}
			m_years.emplace(movie_pair.second->year);
		}
	}

	FillCategories();
	FillGenres();
	FillYears();
	FilterList();

	return 0;
}

void CVodViewer::LoadM3U8Playlist(bool use_cache /*= true*/)
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

	std::wstring url;
	switch (m_plugin_type)
	{
		case StreamType::enGlanz:
		case StreamType::enFox:
			url = fmt::format(m_plugin->get_vod_url(), m_login, m_password);
			break;
	}
	auto data = std::make_unique<std::vector<BYTE>>();

	if (!utils::DownloadFile(url, *data) || data->empty())
	{
		AfxMessageBox(IDS_STRING_ERR_CANT_DOWNLOAD_PLAYLIST, MB_OK | MB_ICONERROR);
		OnEndLoadM3U8Playlist(0);
		return;
	}

	auto pThread = (CPlaylistParseM3U8Thread*)AfxBeginThread(RUNTIME_CLASS(CPlaylistParseM3U8Thread), THREAD_PRIORITY_HIGHEST, 0, CREATE_SUSPENDED);
	if (!pThread)
	{
		AfxMessageBox(IDS_STRING_ERR_THREAD_NOT_START, MB_OK | MB_ICONERROR);
		OnEndLoadM3U8Playlist(0);
		return;
	}

	ThreadConfig cfg;
	cfg.m_parent = this;
	cfg.m_data = data.release();
	cfg.m_hStop = m_evtStop;
	cfg.m_pluginType = m_plugin_type;
	cfg.m_use_cache = use_cache;

	pThread->SetData(cfg);
	pThread->ResumeThread();
}

LRESULT CVodViewer::OnEndLoadM3U8Playlist(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	m_evtStop.ResetEvent();
	m_evtFinished.SetEvent();

	m_wndProgress.ShowWindow(SW_HIDE);
	m_wndProgressInfo.ShowWindow(SW_HIDE);

	m_playlistEntries.reset((Playlist*)wParam);

	if (m_playlistEntries)
	{
		for (const auto& entry : m_playlistEntries->m_entries)
		{
			std::shared_ptr<vod_category> category;
			if (!m_vod_categories->tryGet(entry->get_category(), category))
			{
				category = std::make_shared<vod_category>(entry->get_category());
				category->name = entry->get_category();
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
	m_wndProgress.SetPos((int)lParam);

	return 0;
}

void CVodViewer::OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	auto* pNMLV = (NMLISTVIEW*)pNMHDR;
	*pResult = 0;
	if (pNMLV->iItem == -1 || pNMLV->uOldState) return;

	return LoadMovieInfo(pNMLV->iItem);
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
	m_category_idx = -1;

	if (!m_vod_categories) return;

	for (const auto& pair : m_vod_categories->vec())
	{
		m_wndCategories.AddString(pair.second->name.c_str());

		for (const auto& movie_pair : pair.second->movies.vec())
		{
			for (const auto& genre : movie_pair.second->genres)
			{
				m_genres.set(genre, genre);
			}
			m_years.emplace(movie_pair.second->year);
		}
	}

	if (!m_vod_categories->empty())
	{
		m_wndCategories.EnableWindow(TRUE);
		m_category_idx = 0;
		UpdateData(FALSE);
	}
}

void CVodViewer::FillGenres()
{
	m_wndGenres.ResetContent();
	m_genre_idx = -1;
	for (const auto& genre : m_genres.vec())
	{
		m_wndGenres.AddString(genre.first.c_str());
	}

	if (m_wndGenres.GetCount())
	{
		CString str;
		str.LoadString(IDS_STRING_ALL);
		m_wndGenres.InsertString(0, str);
		m_wndGenres.EnableWindow(TRUE);
		m_genre_idx = 0;
	}

	UpdateData(FALSE);
}

void CVodViewer::FillYears()
{
	m_wndYears.ResetContent();
	m_year_idx = -1;
	for (const auto& year : m_years)
	{
		if (!year.empty())
		{
			m_wndYears.AddString(year.c_str());
		}
	}

	if (m_wndYears.GetCount())
	{
		CString str;
		str.LoadString(IDS_STRING_ALL);
		m_wndYears.InsertString(0, str);
		m_wndYears.EnableWindow(TRUE);
		m_year_idx = 0;
	}

	UpdateData(FALSE);
}

void CVodViewer::LoadMovieInfo(int idx)
{
	UpdateData(TRUE);
	m_wndDescription.SetWindowText(L"");
	if (idx >= (int)m_filtered_movies.size()) return;

	const auto& movie = m_filtered_movies.getAt(idx);
	if (movie->url.empty() && movie->seasons.empty())
	{
		FetchMovie(*movie);
	}

	m_wndSeason.ResetContent();
	m_wndEpisode.ResetContent();

	BOOL enableSeason = FALSE;
	BOOL enableEpisode = FALSE;
	if (!movie->seasons.empty())
	{
		CString str;
		str.LoadString(IDS_STRING_SEASON);

		enableSeason = TRUE;
		m_season_idx = 0;

		for (const auto& season_it : movie->seasons.vec())
		{
			const auto& season = season_it.second;
			if (season.title.empty())
			{
				m_wndSeason.AddString(fmt::format(L"{:s} {:s}", str.GetString(), season.season_id).c_str());
			}
			else
			{
				m_wndSeason.AddString(season.title.c_str());
			}
		}

		const auto& episodes = movie->seasons[0].episodes;
		if (!episodes.empty())
		{
			str.LoadString(IDS_STRING_EPISODE);

			enableEpisode = TRUE;
			m_episode_idx = 0;

			for (const auto& episode_it : episodes.vec())
			{
				const auto& episode = episode_it.second;
				if (episode.title.empty())
				{
					m_wndEpisode.AddString(fmt::format(L"{:s} {:s}", str.GetString(), episode.episode_id).c_str());
				}
				else
				{
					m_wndEpisode.AddString(episode.title.c_str());
				}
			}
		}
	}

	m_wndSeason.EnableWindow(enableSeason);
	m_wndEpisode.EnableWindow(enableEpisode);

	SetImageControl(GetIconCache().get_icon(movie->poster_url.get_uri()), m_wndPoster);
	CStringA desc;
	desc.LoadString(IDS_STRING_VOD_DESC);
	const auto& text = fmt::format(desc.GetString(),
								   utils::utf16_to_utf8(movie->title),
								   utils::utf16_to_utf8(movie->description),
								   utils::utf16_to_utf8(movie->year),
								   utils::utf16_to_utf8(movie->country),
								   utils::utf16_to_utf8(movie->director),
								   utils::utf16_to_utf8(movie->casting),
								   utils::utf16_to_utf8(movie->age),
								   utils::utf16_to_utf8(movie->movie_time)
								   );

	SETTEXTEX set_text_ex = { ST_SELECTION, CP_UTF8 };
	m_wndDescription.SendMessage(EM_SETTEXTEX, (WPARAM)&set_text_ex, (LPARAM)text.c_str());

	UpdateData(FALSE);
}

void CVodViewer::FilterList()
{
	UpdateData(TRUE);

	if (m_category_idx == LB_ERR || m_category_idx >= (int)m_vod_categories->size())
	{
		m_filtered_movies.clear();
		m_wndMoviesList.SetItemCount(0);
		m_wndMoviesList.Invalidate();
		return;
	}

	vod_movie_storage filtered_movies;
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
			year = (m_year_idx == 0) || (!selectedYear.IsEmpty() && selectedYear.CompareNoCase(movie->year.c_str()) == 0);
		}

		if (genre && year)
		{
			filtered_movies.set(movie_pair.first, movie_pair.second);
		}
	}

	std::swap(m_filtered_movies, filtered_movies);

	m_wndTotal.SetWindowText(fmt::format(L"{:d}", m_filtered_movies.size()).c_str());
	m_wndMoviesList.SetItemCount((int)m_filtered_movies.size());
	m_wndMoviesList.Invalidate();
	LoadMovieInfo(0);
}

void CVodViewer::FetchMovie(vod_movie& movie)
{
	std::wstring url;
	switch (m_plugin_type)
	{
		case StreamType::enAntifriz:
		case StreamType::enCbilling:
			url = fmt::format(L"{:s}/video/{:s}", m_plugin->get_vod_url(), movie.id);
		break;

		default:
			break;
	}

	std::vector<BYTE> data;
	if (url.empty() || !utils::DownloadFile(url, data, true) || data.empty())
	{
		return;
	}

	JSON_ALL_TRY
	{
		nlohmann::json parsed_json = nlohmann::json::parse(data.begin(), data.end());
		if (parsed_json.contains("data"))
		{
			const auto& value = parsed_json["data"];

			movie.description = utils::get_json_value("description", value);
			movie.director = utils::get_json_value("director", value);
			movie.casting = utils::get_json_value("actors", value);
			movie.movie_time = utils::get_json_value("time", value);
			const auto& adult = utils::get_json_value("adult", value);
			if (adult == L"1")
				movie.age += L" 18+";

			if (value.contains("seasons"))
			{
				for (const auto& season_it : value["seasons"].items())
				{
					const auto& season_item = season_it.value();
					vod_season season;
					season.id = utils::get_json_value("id", season_item);
					season.title = utils::get_json_value("name", season_item);
					season.season_id = utils::get_json_value("number", season_item);

					for (const auto& episode_it : season_item["series"].items())
					{
						const auto& episode_item = episode_it.value();

						vod_episode episode;
						episode.id = utils::get_json_value("id", episode_item);
						episode.title = utils::get_json_value("name", episode_item);
						episode.episode_id = utils::get_json_value("number", episode_item);

						if (episode_item["files"].is_array())
						{
							episode.url = utils::get_json_value("url", episode_item["files"].front());
						}

						season.episodes.set(episode.id, episode);
					}
					movie.seasons.set(season.id, season);
				}
			}
			else
			{
				movie.url = utils::get_json_value("url", value["files"][0]);
			}
		}
	}
	JSON_ALL_CATCH;
}

void CVodViewer::OnCbnSelchangeComboCategories()
{
	UpdateData(TRUE);
	if (m_category_idx == CB_ERR || m_category_idx >= (int)m_vod_categories->size()) return;

	const auto& category = m_vod_categories->getAt(m_category_idx);
	if (!category->genres.empty())
	{
		m_genres = category->genres;
		FillGenres();
	}
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
	*pResult = 0;

	UpdateData(TRUE);
	if (!pNMItemActivate || pNMItemActivate->iItem >= (int)m_filtered_movies.size()) return;

	const auto& movie = m_filtered_movies[pNMItemActivate->iItem];

	std::wstring url = movie->url;
	if (m_plugin_type == StreamType::enCbilling || m_plugin_type == StreamType::enAntifriz)
	{
		if (movie->url.empty())
		{
			if (m_season_idx != CB_ERR && m_episode_idx != CB_ERR)
			{
				const auto& season = movie->seasons[m_season_idx];
				url = season.episodes[m_episode_idx].url;
			}
		}

		url = fmt::format(L"http://{:s}{:s}?token={:s}", m_domain, url, m_token);
	}
	ShellExecuteW(nullptr, L"open", GetConfig().get_string(true, REG_PLAYER).c_str(), url.c_str(), nullptr, SW_SHOWNORMAL);
}

void CVodViewer::OnBnClickedButtonRefresh()
{
	m_vod_categories->clear();
	LoadPlaylist(false);
}

void CVodViewer::OnBnClickedButtonSearch()
{
	UpdateData(TRUE);
	if (m_vod_categories->empty()) return;
	if (m_SearchText.IsEmpty())
	{
		FilterList();
		return;
	}

	vod_movie_storage searchMovies;
	for (const auto& category : m_vod_categories->vec())
	{
		for (const auto& movies : category.second->movies.vec())
		{
			const auto& movie = movies.second;
			if (StrStrI(movie->title.c_str(), m_SearchText.GetString()) != nullptr)
			{
				searchMovies.set(movie->id, movie);
			}
		}
	}

	std::swap(m_filtered_movies, searchMovies);
	m_wndTotal.SetWindowText(fmt::format(L"{:d}", m_filtered_movies.size()).c_str());
	m_wndMoviesList.SetItemCount((int)m_filtered_movies.size());
	m_wndMoviesList.Invalidate();
	LoadMovieInfo(0);
}
