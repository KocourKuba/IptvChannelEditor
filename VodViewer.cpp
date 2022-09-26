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
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CVodViewer::OnBnClickedButtonStop)
	ON_CBN_SELCHANGE(IDC_COMBO_EPISODE, &CVodViewer::OnCbnSelchangeComboEpisode)
	ON_CBN_SELCHANGE(IDC_COMBO_QUALITY, &CVodViewer::OnCbnSelchangeComboQuality)
END_MESSAGE_MAP()

CVodViewer::CVodViewer(vod_category_storage* categories, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_VOD, pParent)
	, m_vod_categories(categories)
	, m_evtStop(FALSE, TRUE)
	, m_evtFinished(TRUE, TRUE)
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
	DDX_CBIndex(pDX, IDC_COMBO_QUALITY, m_quality_idx);
	DDX_Control(pDX, IDC_LIST_MOVIES, m_wndMoviesList);
	DDX_Control(pDX, IDC_BUTTON_SEARCH, m_wndSearch);
	DDX_Control(pDX, IDC_EDIT_TOTAL_MOVIES, m_wndTotal);
	DDX_Control(pDX, IDC_STATIC_ICON, m_wndPoster);
	DDX_Control(pDX, IDC_RICHEDIT_DESCRIPTION, m_wndDescription);
	DDX_Control(pDX, IDC_PROGRESS_LOAD, m_wndProgress);
	DDX_Text(pDX, IDC_EDIT_SEARCH, m_SearchText);
	DDX_Control(pDX, IDC_BUTTON_STOP, m_wndStop);
	DDX_Control(pDX, IDC_BUTTON_REFRESH, m_wndReload);
	DDX_Control(pDX, IDC_EDIT_STREAM_URL, m_wndStreamUrl);
	DDX_Text(pDX, IDC_EDIT_STREAM_URL, m_streamUrl);
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

	m_wndMoviesList.InsertColumn(0, load_string_resource(IDS_STRING_COL_INFO).c_str(), LVCFMT_LEFT, vWidth, 0);

	m_plugin = StreamContainer::get_instance(m_plugin_type);

	if (auto pos = m_account.subdomain.find(':'); pos != std::string::npos)
	{
		m_account.subdomain = m_account.subdomain.substr(0, pos);
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
	m_wndGenres.EnableWindow(FALSE);
	m_wndYears.EnableWindow(FALSE);
	m_wndSeason.EnableWindow(FALSE);
	m_wndEpisode.EnableWindow(FALSE);
	m_wndQuality.EnableWindow(FALSE);
	m_wndReload.EnableWindow(FALSE);

	if (m_plugin->is_vod_m3u())
		LoadM3U8Playlist(use_cache);
	else
		LoadJsonPlaylist(use_cache);
}

void CVodViewer::LoadJsonPlaylist(bool use_cache /*= true*/)
{
	CWaitCursor cur;

	if (!m_vod_categories->empty())
	{
		FillCategories();
		FillGenres();
		FillYears();
		FilterList();
		return;
	}

	m_wndTotal.SetWindowText(load_string_resource(IDS_STRING_LOADING).c_str());
	m_evtStop.ResetEvent();
	m_evtFinished.ResetEvent();

	TemplateParams params;
	if (m_plugin_type == PluginType::enEdem)
	{
		params.subdomain = m_account.get_portal();
	}
	else
	{
		params.login = m_account.get_login();
		params.password = m_account.get_password();
		params.subdomain = GetConfig().get_string(false, REG_LIST_DOMAIN);
	}

	auto& url = m_plugin->get_vod_url(params);

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
	cfg.m_url = std::move(url);
	cfg.m_use_cache = use_cache;

	pThread->SetData(cfg);
	pThread->ResumeThread();

	m_wndProgress.ShowWindow(SW_HIDE);
	m_wndStop.ShowWindow(SW_HIDE);
}

LRESULT CVodViewer::OnEndLoadJsonPlaylist(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	m_evtStop.ResetEvent();
	m_evtFinished.SetEvent();

	m_wndProgress.ShowWindow(SW_HIDE);
	m_wndStop.ShowWindow(SW_HIDE);
	m_wndReload.EnableWindow(TRUE);

	// make copy content not pointers!
	std::unique_ptr<vod_category_storage> deleter((vod_category_storage*)wParam);
	if (deleter != nullptr)
	{
		*m_vod_categories = *deleter;
	}
	else
	{
		m_vod_categories->clear();
	}

	FillCategories();
	FillGenres();
	FillYears();
	FilterList();

	return 0;
}

void CVodViewer::LoadM3U8Playlist(bool use_cache /*= true*/)
{
	CWaitCursor cur;

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

	m_wndTotal.SetWindowText(load_string_resource(IDS_STRING_LOADING).c_str());

	TemplateParams params;
	params.login = m_account.get_login();
	params.password = m_account.get_password();
	const auto& url = m_plugin->get_vod_url(params);

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
	m_wndStop.ShowWindow(SW_HIDE);
	m_wndReload.EnableWindow(TRUE);

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
	m_wndProgress.SetPos((int)lParam);
	m_wndProgress.ShowWindow(SW_SHOW);
	m_wndStop.ShowWindow(SW_SHOW);

	return 0;
}

LRESULT CVodViewer::OnUpdateProgress(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	m_wndTotal.SetWindowText(fmt::format(L"{:d}", (int)wParam).c_str());
	m_wndProgress.SetPos((int)wParam);

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

	StrCpyN(lvItem.pszText, m_filtered_movies[lvItem.iItem]->title.c_str(), lvItem.cchTextMax - 1);
}

void CVodViewer::OnBnClickedButtonStop()
{
	m_evtStop.SetEvent();
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

void CVodViewer::OnCbnSelchangeComboEpisode()
{
	UpdateData(TRUE);

	auto pos = m_wndMoviesList.GetFirstSelectedItemPosition();
	int idx = -1;
	if (pos)
		idx = m_wndMoviesList.GetNextSelectedItem(pos);

	return GetUrl(idx);
}

void CVodViewer::OnCbnSelchangeComboQuality()
{
	UpdateData(TRUE);

	auto pos = m_wndMoviesList.GetFirstSelectedItemPosition();
	int idx = -1;
	if (pos)
		idx = m_wndMoviesList.GetNextSelectedItem(pos);

	return GetUrl(idx);
}

void CVodViewer::OnNMDblclkListMovies(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	UpdateData(TRUE);
	if (!pNMItemActivate || pNMItemActivate->iItem >= (int)m_filtered_movies.size()) return;

	const auto& movie = m_filtered_movies[pNMItemActivate->iItem];

	std::wstring url = movie->url;
	switch (m_plugin_type)
	{
		case PluginType::enAntifriz:
		case PluginType::enCbilling:
		{
			if (movie->url.empty() && m_season_idx != CB_ERR && m_episode_idx != CB_ERR)
			{
				const auto& season = movie->seasons[m_season_idx];
				url = season.episodes[m_episode_idx].url;
			}
			url = fmt::format(L"http://{:s}{:s}?token={:s}", m_account.get_subdomain(), url, m_account.get_token());
			break;
		}
		case PluginType::enEdem:
			if (!movie->quality.empty())
			{
				url = movie->quality[m_quality_idx].url;
			}
			else if (!movie->seasons.empty())
			{
				const auto& episodes = movie->seasons.front().episodes;
				if (!episodes.empty())
				{
					const auto& quality = episodes[m_episode_idx].quality;
					if (quality.empty())
						url = episodes[m_episode_idx].url;
					else
						url = episodes[m_episode_idx].quality[m_quality_idx].url;
				}
			}
			break;
		default:
			break;
	}

	if (!url.empty())
	{
		ShellExecuteW(nullptr, L"open", GetConfig().get_string(true, REG_PLAYER).c_str(), url.c_str(), nullptr, SW_SHOWNORMAL);
	}
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
	m_wndMoviesList.SetItemState(-1, 0, LVIS_SELECTED);
	m_wndMoviesList.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	LoadMovieInfo(0);
}

//////////////////////////////////////////////////////////////////////////

void CVodViewer::FillCategories()
{
	m_wndCategories.ResetContent();
	m_category_idx = -1;

	if (!m_vod_categories) return;

	m_genres.clear();
	m_years.clear();

	for (const auto& pair : m_vod_categories->vec())
	{
		m_wndCategories.AddString(pair.second->name.c_str());
		if (m_plugin_type == PluginType::enEdem)
		{
			for (const auto& filter : pair.second->filters.vec())
			{
				if (filter.first == L"genre")
				{
					for (const auto& filter_item : filter.second.vec())
					{
						vod_genre genre({ filter_item.second.id, filter_item.second.title });
						m_genres.set(filter_item.second.id, genre);
					}
				}
				else if (filter.first == L"years")
				{
					for (const auto& filter_item : filter.second.vec())
					{
						m_years.set(filter_item.second.title, filter_item.second.title);
					}
				}
			}
		}
		else
		{
			for (const auto& pair : m_vod_categories->vec())
			{
				for (const auto& movie_pair : pair.second->movies.vec())
				{
					for (const auto& genre : movie_pair.second->genres.vec())
					{
						m_genres.set(genre.first, genre.second);
					}
					m_years.set(movie_pair.second->year, movie_pair.second->year);
				}
			}
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
		m_wndGenres.AddString(genre.second.title.c_str());
	}

	if (m_wndGenres.GetCount())
	{
		m_wndGenres.InsertString(0, load_string_resource(m_plugin_type == PluginType::enEdem ? IDS_STRING_NONE : IDS_STRING_ALL).c_str());
		m_wndGenres.EnableWindow(TRUE);
		m_genre_idx = 0;
	}
	else
	{
		m_wndGenres.EnableWindow(FALSE);
	}

	UpdateData(FALSE);
}

void CVodViewer::FillYears()
{
	m_wndYears.ResetContent();
	m_year_idx = -1;
	for (const auto& year : m_years.vec())
	{
		if (!year.first.empty())
		{
			m_wndYears.AddString(year.first.c_str());
		}
	}

	if (m_wndYears.GetCount())
	{
		m_wndYears.InsertString(0, load_string_resource(m_plugin_type == PluginType::enEdem ? IDS_STRING_NONE : IDS_STRING_ALL).c_str());
		m_wndYears.EnableWindow(TRUE);
		m_year_idx = 0;
	}
	else
	{
		m_wndYears.EnableWindow(FALSE);
	}

	UpdateData(FALSE);
}

void CVodViewer::LoadMovieInfo(int idx)
{
	UpdateData(TRUE);

	m_wndDescription.SetWindowText(L"");
	m_wndSeason.ResetContent();
	m_wndEpisode.ResetContent();
	m_wndQuality.ResetContent();
	m_wndPoster.SetBitmap(nullptr);

	if (idx >= (int)m_filtered_movies.size())
	{
		m_streamUrl.Empty();
		UpdateData(FALSE);
		return;
	}

	const auto& movie = m_filtered_movies[idx];
	if (movie->url.empty() && movie->seasons.empty())
	{
		switch (m_plugin_type)
		{
			case PluginType::enAntifriz:
			case PluginType::enCbilling:
				FetchMovieCbilling(*movie);
				break;
			case PluginType::enEdem:
				FetchMovieEdem(*movie);
				break;
			default:
				break;
		}
	}

	BOOL enableSeason = FALSE;
	BOOL enableEpisode = FALSE;
	BOOL enableQuality = FALSE;
	if (movie->seasons.empty())
	{
		if (!movie->quality.empty())
		{
			enableQuality = TRUE;
			m_quality_idx = 0;
			for (const auto& q_it : movie->quality.vec())
			{
				m_wndQuality.AddString(q_it.second.title.c_str());
			}
		}
	}
	else
	{

		m_season_idx = 0;

		if (m_plugin_type != PluginType::enEdem)
		{
			const auto& str = load_string_resource(IDS_STRING_SEASON);
			enableSeason = TRUE;
			for (const auto& season_it : movie->seasons.vec())
			{
				const auto& season = season_it.second;
				if (season.title.empty())
				{
					m_wndSeason.AddString(fmt::format(L"{:s} {:s}", str, season.season_id).c_str());
				}
				else
				{
					m_wndSeason.AddString(season.title.c_str());
				}
			}
		}

		const auto& episodes = movie->seasons.front().episodes;
		if (!episodes.empty())
		{
			const auto& str = load_string_resource(IDS_STRING_EPISODE);

			enableEpisode = TRUE;
			m_episode_idx = 0;

			for (const auto& episode_it : episodes.vec())
			{
				const auto& episode = episode_it.second;
				if (episode.title.empty())
				{
					m_wndEpisode.AddString(fmt::format(L"{:s} {:s}", str, episode.episode_id).c_str());
				}
				else
				{
					m_wndEpisode.AddString(episode.title.c_str());
				}

				if (enableQuality) continue;

				if (!episode.quality.empty())
				{
					enableQuality = TRUE;
					m_quality_idx = 0;
					for (const auto& q_it : episode.quality.vec())
					{
						m_wndQuality.AddString(q_it.second.title.c_str());
					}
				}
			}
		}
	}

	m_wndSeason.EnableWindow(enableSeason);
	m_wndEpisode.EnableWindow(enableEpisode);
	m_wndQuality.EnableWindow(enableQuality);

	SetImageControl(GetIconCache().get_icon(movie->poster_url.get_uri()), m_wndPoster);
	const auto& text = fmt::format(utils::utf16_to_utf8(load_string_resource(IDS_STRING_VOD_DESC)),
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

	GetUrl(idx);

	UpdateData(FALSE);
}

void CVodViewer::FilterList()
{
	UpdateData(TRUE);

	if (m_category_idx == LB_ERR || m_category_idx >= (int)m_vod_categories->size())
	{
		m_filtered_movies.clear();
		m_wndProgress.ShowWindow(SW_HIDE);
		m_wndStop.ShowWindow(SW_HIDE);
		m_wndTotal.SetWindowText(fmt::format(L"{:d}", m_filtered_movies.size()).c_str());
		m_wndMoviesList.SetItemCount(0);
		m_wndMoviesList.Invalidate();
		m_wndPoster.SetBitmap(nullptr);
		return;
	}

	m_wndReload.EnableWindow(FALSE);

	bool filterByGenre = false;
	bool filterByYear = false;
	CString selectedGenre;
	if (m_wndGenres.IsWindowEnabled() && m_genre_idx != CB_ERR)
	{
		m_wndGenres.GetLBText(m_genre_idx, selectedGenre);
		filterByGenre = !selectedGenre.IsEmpty();
	}

	CString selectedYear;
	if (m_wndYears.IsWindowEnabled() && m_year_idx != CB_ERR)
	{
		m_wndYears.GetLBText(m_year_idx, selectedYear);
		filterByYear = !selectedYear.IsEmpty();
	}

	vod_movie_storage filtered_movies;

	if (m_plugin_type == PluginType::enEdem)
	{
		do
		{
			if (m_genre_idx == 0 && m_year_idx == 0)
			{
				for (const auto& movie_pair : m_vod_categories->getAt(m_category_idx)->movies.vec())
				{
					filtered_movies.set(movie_pair.first, movie_pair.second);
				}

				break;
			}

			std::wregex re_url(LR"(^portal::\[key:(.+)\](.+)$)");
			std::wsmatch m;
			const auto& vportal = m_account.get_portal();
			if (!std::regex_match(vportal, m, re_url)) break;

			const auto& key = m[1].str();
			const auto& url = m[2].str();

			nlohmann::json json_request;
			json_request["filter"] = "on";
			json_request["limit"] = 300;
			json_request["offset"] = 0;
			json_request["key"] = utils::utf16_to_utf8(key);
			json_request["mac"] = "000000000000";
			json_request["app"] = "IPTV ChannelEditor";
			if (m_genre_idx != 0)
			{
				int id = utils::char_to_int(m_genres[m_genre_idx - 1].id);
				json_request["genre"] = id;
				ATLTRACE("\ngenre id: %d\n", id);
			}

			if (m_year_idx != 0)
			{
				const auto& years = utils::utf16_to_utf8(m_years[m_year_idx - 1]);
				json_request["years"] = years;
				ATLTRACE("\nyears: %s\n", years.c_str());
			}

			std::vector<BYTE> data;
			std::string post = json_request.dump();
			std::wstring header = L"accept: */*\r\nContent-Type: application/json";
			ATLTRACE("\n%s\n", post.c_str());
			if (!utils::DownloadFile(url, data, true, &header, L"POST", &post) || data.empty()) break;

			JSON_ALL_TRY;
			nlohmann::json parsed_json = nlohmann::json::parse(data.begin(), data.end());
			int total = utils::get_json_int("count", parsed_json);
			OnInitProgress(total, 0);
			ATLTRACE("\nfiltered movies: %d\n", total);

			int cnt = 0;
			int offset = 0;
			int prev_offset = -1;
			for (;;)
			{
				for (const auto& item_it : parsed_json["items"].items())
				{
					const auto& movie_item = item_it.value();

					auto movie = std::make_shared<vod_movie>();

					if (utils::get_json_wstring("type", movie_item) == L"next")
					{
						offset = utils::get_json_int("offset", movie_item["request"]);
						continue;
					}

					movie->id = utils::get_json_wstring("fid", movie_item["request"]);
					movie->title = utils::get_json_wstring("title", movie_item);
					movie->description = utils::get_json_wstring("description", movie_item);
					movie->poster_url.set_uri(utils::get_json_wstring("img", movie_item));
					movie->poster_url.set_schema(L"http://");
					movie->rating = utils::get_json_wstring("rating", movie_item);
					movie->country = utils::get_json_wstring("country", movie_item);
					movie->year = utils::get_json_wstring("year", movie_item);
					movie->age = utils::get_json_wstring("agelimit", movie_item);
					movie->movie_time = utils::get_json_wstring("duration", movie_item);

					filtered_movies.set(movie->id, movie);
					cnt++;

					if (cnt % 100 == 0)
					{
						OnUpdateProgress(cnt);
						if (::WaitForSingleObject(m_evtStop, 0) == WAIT_OBJECT_0) break;
					}
				}

				ATLTRACE("\nreaded: %d\n", cnt);

				if (::WaitForSingleObject(m_evtStop, 0) == WAIT_OBJECT_0) break;
				if (offset == prev_offset) break;

				prev_offset = offset;
				json_request["offset"] = offset;
				ATLTRACE("\noffset: %d\n", offset);

				data.clear();
				post = json_request.dump();
				ATLTRACE("\n%s\n", post.c_str());
				if (!utils::DownloadFile(url, data, true, &header, L"POST", &post) || data.empty()) break;

				parsed_json = nlohmann::json::parse(data.begin(), data.end());
			}
			JSON_ALL_CATCH;
		} while (false);

		if (::WaitForSingleObject(m_evtStop, 0) == WAIT_OBJECT_0)
			filtered_movies.clear();
	}
	else
	{
		for (const auto& movie_pair : m_vod_categories->getAt(m_category_idx)->movies.vec())
		{
			bool genre = true;
			bool year = true;

			const auto& movie = movie_pair.second;
			if (filterByGenre && m_genre_idx != 0)
			{
				const auto& genres = movie->genres.vec();
				genre = std::find_if(genres.begin(), genres.end(), [&selectedGenre](const auto& pair)
										{
											return (selectedGenre.CompareNoCase(pair.second.title.c_str()) == 0);

										}) != genres.end();
			}

			if (m_year_idx != 0)
			{
				year = selectedYear.CompareNoCase(movie->year.c_str()) == 0;
			}

			if (genre && year)
			{
				filtered_movies.set(movie_pair.first, movie_pair.second);
			}
		}
	}

	std::swap(m_filtered_movies, filtered_movies);

	m_wndProgress.ShowWindow(SW_HIDE);
	m_wndStop.ShowWindow(SW_HIDE);
	m_wndReload.EnableWindow(TRUE);
	m_wndTotal.SetWindowText(fmt::format(L"{:d}", m_filtered_movies.size()).c_str());
	m_wndMoviesList.SetItemCount((int)m_filtered_movies.size());
	m_wndMoviesList.Invalidate();

	m_wndMoviesList.SetItemState(-1, 0, LVIS_SELECTED);
	m_wndMoviesList.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	LoadMovieInfo(0);
}

void CVodViewer::FetchMovieCbilling(vod_movie& movie) const
{
	CWaitCursor cur;

	const auto& url = m_plugin->get_vod_template() + L"/video/" + movie.id;
	std::vector<BYTE> data;
	if (url.empty() || !utils::DownloadFile(url, data, false) || data.empty())
	{
		return;
	}

	JSON_ALL_TRY;
	const auto& parsed_json = nlohmann::json::parse(data.begin(), data.end());

	if (parsed_json.contains("data"))
	{
		const auto& value = parsed_json["data"];

		movie.description = utils::get_json_wstring("description", value);
		movie.director = utils::get_json_wstring("director", value);
		movie.casting = utils::get_json_wstring("actors", value);
		movie.movie_time = utils::get_json_wstring("time", value);
		const auto& adult = utils::get_json_wstring("adult", value);
		if (adult == L"1")
			movie.age += L" 18+";

		if (value.contains("seasons"))
		{
			for (const auto& season_it : value["seasons"].items())
			{
				const auto& season_item = season_it.value();
				vod_season season;
				season.id = utils::get_json_wstring("id", season_item);
				season.title = utils::get_json_wstring("name", season_item);
				season.season_id = utils::get_json_wstring("number", season_item);

				for (const auto& episode_it : season_item["series"].items())
				{
					const auto& episode_item = episode_it.value();

					vod_episode episode;
					episode.id = utils::get_json_wstring("id", episode_item);
					episode.title = utils::get_json_wstring("name", episode_item);
					episode.episode_id = utils::get_json_wstring("number", episode_item);

					if (episode_item["files"].is_array())
					{
						episode.url = utils::get_json_wstring("url", episode_item["files"].front());
					}

					season.episodes.set(episode.id, episode);
				}
				movie.seasons.set(season.id, season);
			}
		}
		else
		{
			movie.url = utils::get_json_wstring("url", value["files"][0]);
		}
	}
	JSON_ALL_CATCH;
}

void CVodViewer::FetchMovieEdem(vod_movie& movie) const
{
	CWaitCursor cur;
	do
	{
		std::wregex re_url(LR"(^portal::\[key:(.+)\](.+)$)");
		std::wsmatch m;
		const auto& vportal = m_account.get_portal();
		if (!std::regex_match(vportal, m, re_url)) break;

		const auto& key = m[1].str();
		const auto& url = m[2].str();

		nlohmann::json json_request;
		json_request["cmd"] = "flick";
		json_request["limit"] = 300;
		json_request["offset"] = 0;
		json_request["key"] = utils::utf16_to_utf8(key);
		json_request["mac"] = "000000000000";
		json_request["app"] = "IPTV ChannelEditor";
		json_request["fid"] = utils::char_to_int(movie.id);

		std::string post = json_request.dump();
		std::wstring header = L"accept: */*\r\nContent-Type: application/json";
		ATLTRACE("\n%s\n", post.c_str());

		std::vector<BYTE> data;
		if (!utils::DownloadFile(url, data, false, &header, L"POST", &post) || data.empty()) break;

		JSON_ALL_TRY;

		const auto& json_data = nlohmann::json::parse(data.begin(), data.end());
		const auto& type = json_data["type"];
		if (type == "multistream")
		{
			if (movie.seasons.empty())
			{
				movie.seasons.set(L"season", vod_season());
			}
			auto& season = movie.seasons.get(L"season");

			for (const auto& items_it : json_data["items"].items())
			{

				const auto& item = items_it.value();
				vod_episode episode;
				episode.title = utils::get_json_wstring("title", item);
				episode.url = utils::get_json_wstring("url", item);
				episode.id = utils::get_json_wstring("fid", item);
				if (episode.id.empty())
				{
					episode.id = utils::get_json_wstring("fid", item["request"]);
				}

				json_request["fid"] = utils::char_to_int(episode.id);
				post = json_request.dump();
				ATLTRACE("\n%s\n", post.c_str());
				data.clear();
				if (utils::DownloadFile(url, data, false, &header, L"POST", &post) && !data.empty())
				{
					const auto& variants_data = nlohmann::json::parse(data.begin(), data.end());
					if (variants_data.contains("variants"))
					{
						for (const auto& variant_it : variants_data["variants"].items())
						{
							const auto& title = utils::utf8_to_utf16(variant_it.key());
							const auto& q_url = utils::utf8_to_utf16(variant_it.value().get<std::string>());

							episode.quality.set(title, vod_quality({ title, q_url }));
						}
					}
				}

				season.episodes.set(episode.id, episode);
			}
		}
		else
		{
			movie.url = utils::get_json_wstring("url", json_data);
			if (json_data.contains("variants"))
			{
				for (const auto& variant_it : json_data["variants"].items())
				{
					const auto& title = utils::utf8_to_utf16(variant_it.key());
					const auto& q_url = utils::utf8_to_utf16(variant_it.value().get<std::string>());

					movie.quality.set(title, vod_quality({ title, q_url }));
				}
			}
		}
		JSON_ALL_CATCH;
	} while (false);
}

void CVodViewer::GetUrl(int idx)
{
	if (idx == CB_ERR || idx >= (int)m_filtered_movies.size()) return;

	const auto& movie = m_filtered_movies[idx];

	std::wstring url = movie->url;
	switch (m_plugin_type)
	{
		case PluginType::enAntifriz:
		case PluginType::enCbilling:
		{
			if (movie->url.empty() && m_season_idx != CB_ERR && m_episode_idx != CB_ERR)
			{
				const auto& season = movie->seasons[m_season_idx];
				url = season.episodes[m_episode_idx].url;
			}
			url = fmt::format(L"http://{:s}{:s}?token={:s}", m_account.get_subdomain(), url, m_account.get_token());
			break;
		}
		case PluginType::enEdem:
			if (!movie->quality.empty())
			{
				url = movie->quality[m_quality_idx].url;
			}
			else if (!movie->seasons.empty())
			{
				const auto& episodes = movie->seasons.front().episodes;
				if (!episodes.empty())
				{
					const auto& quality = episodes[m_episode_idx].quality;
					if (quality.empty())
						url = episodes[m_episode_idx].url;
					else
						url = episodes[m_episode_idx].quality[m_quality_idx].url;
				}
			}
			break;
		default:
			break;
	}

	m_streamUrl = url.c_str();

	UpdateData(FALSE);
}
