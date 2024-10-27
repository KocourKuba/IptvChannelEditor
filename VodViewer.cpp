/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2024): sharky72 (https://github.com/KocourKuba)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "pch.h"
#include <afxdialogex.h>
#include "resource.h"
#include "VodViewer.h"
#include "IPTVChannelEditor.h"
#include "PlayListEntry.h"
#include "PlaylistParseM3U8Thread.h"
#include "PlaylistParseJsonThread.h"
#include "IconCache.h"
#include "AccountSettings.h"
#include "Constants.h"

#include "UtilsLib\inet_utils.h"

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
	ON_CBN_SELCHANGE(IDC_COMBO_SEASON, &CVodViewer::OnCbnSelchangeComboSeason)
	ON_CBN_SELCHANGE(IDC_COMBO_EPISODE, &CVodViewer::OnCbnSelchangeComboEpisode)
	ON_CBN_SELCHANGE(IDC_COMBO_QUALITY, &CVodViewer::OnCbnSelchangeComboQuality)
	ON_CBN_SELCHANGE(IDC_COMBO_AUDIO, &CVodViewer::OnCbnSelchangeComboAudio)
	ON_CBN_SELCHANGE(IDC_COMBO_PLAYLIST, &CVodViewer::OnCbnSelchangeComboPlaylist)
END_MESSAGE_MAP()

CVodViewer::CVodViewer(std::map<std::wstring, vod_category_storage>& categories, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_VOD, pParent)
	, m_vod_storages(categories)
	, m_evtStop(FALSE, TRUE)
	, m_evtFinished(TRUE, TRUE)
{
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
	DDX_Control(pDX, IDC_COMBO_AUDIO, m_wndAudio);
	DDX_CBIndex(pDX, IDC_COMBO_AUDIO, m_audio_idx);
	DDX_Control(pDX, IDC_LIST_MOVIES, m_wndMoviesList);
	DDX_Control(pDX, IDC_BUTTON_SEARCH, m_wndSearch);
	DDX_Control(pDX, IDC_EDIT_TOTAL_MOVIES, m_wndTotal);
	DDX_Control(pDX, IDC_STATIC_ICON, m_wndPoster);
	DDX_Control(pDX, IDC_RICHEDIT_DESCRIPTION, m_wndDescription);
	DDX_Control(pDX, IDC_PROGRESS_LOAD, m_wndProgress);
	DDX_Text(pDX, IDC_EDIT_SEARCH, m_SearchText);
	DDX_Control(pDX, IDC_BUTTON_STOP, m_wndStop);
	DDX_Control(pDX, IDC_BUTTON_REFRESH, m_wndBtnReload);
	DDX_Control(pDX, IDC_EDIT_STREAM_URL, m_wndStreamUrl);
	DDX_Text(pDX, IDC_EDIT_STREAM_URL, m_streamUrl);
	DDX_Control(pDX, IDC_EDIT_ICON_URL, m_wndSIconUrl);
	DDX_Text(pDX, IDC_EDIT_ICON_URL, m_iconUrl);
	DDX_Control(pDX, IDC_COMBO_PLAYLIST, m_wndPlaylist);
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

	TemplateParams params;
	params.creds = m_account;
	m_plugin->get_api_token(params);

	SetButtonImage(IDB_PNG_RELOAD, m_wndBtnReload);

	const auto& vods = m_plugin->get_vod_infos();
	for (const auto& vod : vods)
	{
		int idx = m_wndPlaylist.AddString(vod.get_name().c_str());
	}

	int idx = (int)m_plugin->get_vod_info_idx();
	m_wndPlaylist.SetCurSel(idx);
	m_wndPlaylist.EnableWindow(m_wndPlaylist.GetCount() > 1);
	m_current_vod = m_vod_storages[m_plugin->get_vod_info(idx).get_pl_template()];

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
	if (!m_evtFinished.Lock(0))
	{
		m_evtStop.SetEvent();
		Sleep(1000);
	}

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
	m_wndAudio.EnableWindow(FALSE);
	m_wndBtnReload.EnableWindow(FALSE);

	switch (m_plugin->get_vod_engine())
	{
		case VodEngine::enNone:
			break;

		case VodEngine::enM3U:
			LoadM3U8Playlist(use_cache);
			break;

		case VodEngine::enJson:
		case VodEngine::enXC:
			LoadJsonPlaylist(use_cache);
			break;
	}
}

void CVodViewer::LoadJsonPlaylist(bool use_cache /*= true*/)
{
	CWaitCursor cur;
	m_total = 0;

	if (!m_current_vod.empty())
	{
		m_total = m_current_vod.get(load_string_resource(IDS_STRING_ALL))->movies.size();

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
	params.creds = m_account;

	m_plugin->get_api_token(params);
	m_plugin->update_provider_params(params);

	auto& url = m_plugin->get_vod_url(m_wndPlaylist.GetCurSel(), params);

	auto pThread = (CPlaylistParseJsonThread*)AfxBeginThread(RUNTIME_CLASS(CPlaylistParseJsonThread), THREAD_PRIORITY_HIGHEST, 0, CREATE_SUSPENDED);
	if (!pThread)
	{
		AfxMessageBox(IDS_STRING_ERR_THREAD_NOT_START, MB_OK | MB_ICONERROR);
		OnEndLoadJsonPlaylist(0);
		return;
	}

	CThreadConfig cfg;
	cfg.m_parent = this;
	cfg.m_hStop = m_evtStop;
	cfg.m_url = std::move(url);
	cfg.m_params = params;

	pThread->SetData(cfg);
	pThread->SetPlugin(m_plugin);
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
	m_wndBtnReload.EnableWindow(TRUE);

	// make copy content not pointers!
	std::unique_ptr<vod_category_storage> deleter((vod_category_storage*)wParam);
	if (deleter != nullptr)
	{
		m_current_vod = *deleter;
		auto& all_category = m_current_vod.get(load_string_resource(IDS_STRING_ALL));
		for (const auto& category : m_current_vod.vec())
		{
			for (const auto& pair : category.second->movies.vec())
			{
				all_category->movies.set_back(pair.second->id, pair.second);
			}

		}
		m_total += all_category->movies.size();
	}
	else
	{
		m_current_vod.clear();
	}

	FillCategories();
	FillGenres();
	FillYears();
	FilterList();

	return 0;
}

void CVodViewer::LoadM3U8Playlist(bool use_cache /*= true*/)
{
	m_total = 0;
	m_evtStop.ResetEvent();
	m_evtFinished.ResetEvent();
	if (!m_current_vod.empty())
	{
		m_total = m_current_vod.get(load_string_resource(IDS_STRING_ALL))->movies.size();

		FillCategories();
		FillGenres();
		FillYears();
		FilterList();
		return;
	}

	m_wndTotal.SetWindowText(load_string_resource(IDS_STRING_LOADING).c_str());

	TemplateParams params;
	params.creds = m_account;

	m_plugin->update_provider_params(params);

	m_dl.SetUrl(m_plugin->get_vod_url(m_wndPlaylist.GetCurSel(), params));
	m_dl.SetCacheTtl(0);
	m_dl.SetUserAgent(m_plugin->get_user_agent());

	CWaitCursor cur;
	std::stringstream data;
	if (!m_dl.DownloadFile(data))
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

	CThreadConfig cfg;
	cfg.m_parent = this;
	cfg.m_data = std::move(data);
	cfg.m_hStop = m_evtStop;

	pThread->SetData(cfg);
	pThread->SetPlugin(m_plugin);
	pThread->ResumeThread();
}

LRESULT CVodViewer::OnEndLoadM3U8Playlist(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	static vod_movie default_vod;

	m_evtStop.ResetEvent();
	m_evtFinished.SetEvent();

	m_wndProgress.ShowWindow(SW_HIDE);
	m_wndStop.ShowWindow(SW_HIDE);
	m_wndBtnReload.EnableWindow(TRUE);

	m_playlistEntries.reset((Playlist*)wParam);

	// clear named group
	std::vector<std::wstring> regex_named_groups;

	boost::wregex re;
	try
	{
		const auto& info = m_plugin->get_vod_info(m_wndPlaylist.GetCurSel());
		const auto& pattern = info.get_parse_regex_title();
		if (!pattern.empty())
		{
			re = pattern;

			boost::wregex re_group(L"(\\?<([^>]+)>)");
			boost::match_results<std::wstring::const_iterator> what;
			std::wstring::const_iterator start = pattern.begin();
			std::wstring::const_iterator end = pattern.end();
			auto flags = boost::match_default;
			while (boost::regex_search(start, end, what, re_group, flags))
			{
				if (default_vod.parser_mapper.find(what[2]) != default_vod.parser_mapper.end())
				{
					// add only known group!
					regex_named_groups.emplace_back(what[2]);
				}
				start = what[0].second;
				flags |= boost::match_prev_avail;
				flags |= boost::match_not_bob;
			}
		}
	}
	catch (...)
	{

	}

	bool parseTitle = !re.empty();
	if (m_playlistEntries)
	{
		m_total = m_playlistEntries->m_entries.size();

		const auto& all_name = load_string_resource(IDS_STRING_ALL);
		auto all_category = std::make_shared<vod_category>(all_name);
		all_category->name = all_name;
		m_current_vod.set_back(all_name, all_category);

		for (const auto& entry : m_playlistEntries->m_entries)
		{
			std::shared_ptr<vod_category> category;
			const auto& category_name = entry->get_category_w();
			if (!m_current_vod.tryGet(category_name, category))
			{
				category = std::make_shared<vod_category>(category_name);
				category->name = category_name;
				m_current_vod.set_back(category_name, category);
			}

			auto movie = std::make_shared<vod_movie>();
			movie->id = entry->get_epg_id();
			movie->title = entry->get_title();
			movie->poster_url = entry->get_icon_uri();
			movie->url = entry->get_uri();

			if (parseTitle)
			{
				boost::wsmatch m;
				if (boost::regex_match(entry->get_title(), m, re))
				{
					// map groups to parser members
					size_t pos = 1;
					for (const auto& group : regex_named_groups)
					{
						*movie->parser_mapper[group] = std::move(m[(int)(pos++)].str());
					}
				}
			}

			all_category->movies.set_back(movie->id, movie);
			category->movies.set_back(movie->id, movie);
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

	StrCpyN(lvItem.pszText, m_filtered_movies[lvItem.iItem]->title.c_str(), lvItem.cchTextMax - 1);
}

void CVodViewer::OnBnClickedButtonStop()
{
	m_evtStop.SetEvent();
}


void CVodViewer::OnCbnSelchangeComboCategories()
{
	UpdateData(TRUE);
	if (m_category_idx == CB_ERR || m_category_idx >= (int)m_current_vod.size()) return;

	const auto& category = m_current_vod.getAt(m_category_idx);
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

void CVodViewer::OnCbnSelchangeComboSeason()
{
	UpdateData(TRUE);

	auto pos = m_wndMoviesList.GetFirstSelectedItemPosition();
	int idx = -1;
	if (pos)
	{
		idx = m_wndMoviesList.GetNextSelectedItem(pos);
	}

	FillEpisodes(GetFilteredMovie(idx));

	return GetUrl(idx);
}

void CVodViewer::OnCbnSelchangeComboEpisode()
{
	UpdateData(TRUE);

	auto pos = m_wndMoviesList.GetFirstSelectedItemPosition();
	int idx = -1;
	if (pos)
	{
		idx = m_wndMoviesList.GetNextSelectedItem(pos);
	}

	return GetUrl(idx);
}

void CVodViewer::OnCbnSelchangeComboQuality()
{
	UpdateData(TRUE);

	auto pos = m_wndMoviesList.GetFirstSelectedItemPosition();
	int idx = -1;
	if (pos)
	{
		idx = m_wndMoviesList.GetNextSelectedItem(pos);
	}

	return GetUrl(idx);
}

void CVodViewer::OnCbnSelchangeComboAudio()
{
	UpdateData(TRUE);

	auto pos = m_wndMoviesList.GetFirstSelectedItemPosition();
	int idx = -1;
	if (pos)
	{
		idx = m_wndMoviesList.GetNextSelectedItem(pos);
	}

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
	if (m_plugin->get_plugin_type() == PluginType::enAntifriz || m_plugin->get_plugin_type() == PluginType::enCbilling)
	{
		if (movie->url.empty() && m_season_idx != CB_ERR && m_episode_idx != CB_ERR)
		{
			const auto& season = movie->seasons[m_season_idx];
			url = season.episodes[m_episode_idx].url;
		}
		url = fmt::format(L"http://{:s}{:s}?token={:s}", m_account.get_subdomain(), url, m_account.get_s_token());
	}
	else
	{
		if (!movie->quality.empty() && m_quality_idx != CB_ERR)
		{
			url = movie->quality[m_quality_idx].url;
		}
		else if (!movie->audios.empty() && m_audio_idx != CB_ERR)
		{
			url = movie->audios[m_audio_idx].url;
		}
		else if (!movie->seasons.empty())
		{
			const auto& episodes = movie->seasons.front().episodes;
			if (!episodes.empty())
			{
				if (!episodes[m_episode_idx].qualities.empty() && m_quality_idx != CB_ERR)
				{
					url = episodes[m_episode_idx].qualities[m_quality_idx].url;
				}
				else if (!episodes[m_episode_idx].audios.empty() && m_audio_idx != CB_ERR)
				{
					url = episodes[m_episode_idx].audios[m_audio_idx].url;
				}
				else
				{
					url = episodes[m_episode_idx].url;
				}
			}
		}
	}

	if (!url.empty())
	{
		ShellExecuteW(nullptr, L"open", GetConfig().get_string(true, REG_PLAYER).c_str(), url.c_str(), nullptr, SW_SHOWNORMAL);
	}
}

void CVodViewer::OnBnClickedButtonRefresh()
{
	m_current_vod.clear();
	LoadPlaylist(false);
}

void CVodViewer::OnBnClickedButtonSearch()
{
	UpdateData(TRUE);

	if (m_current_vod.empty()) return;

	if (m_SearchText.IsEmpty())
	{
		FilterList();
		return;
	}

	vod_movie_storage searchMovies;
	for (const auto& category : m_current_vod.vec())
	{
		for (const auto& movies : category.second->movies.vec())
		{
			const auto& movie = movies.second;
			if (StrStrI(movie->title.c_str(), m_SearchText.GetString()) != nullptr)
			{
				searchMovies.set_back(movie->id, movie);
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

	if (m_current_vod.empty()) return;

	m_genres.clear();
	m_years.clear();
	for (const auto& pair : m_current_vod.vec())
	{
		m_wndCategories.AddString(pair.second->name.c_str());
		if (m_plugin->get_plugin_type() == PluginType::enEdem)
		{
			for (const auto& filter : pair.second->filters.vec())
			{
				if (filter.first == L"genre")
				{
					for (const auto& filter_item : filter.second.vec())
					{
						vod_genre_def genre({ filter_item.second.id, filter_item.second.title });
						m_genres.set_back(filter_item.second.id, genre);
					}
				}
				else if (filter.first == L"years")
				{
					for (const auto& filter_item : filter.second.vec())
					{
						m_years.set_back(filter_item.second.title, filter_item.second.title);
					}
				}
			}
		}
		else
		{
			for (const auto& pair : m_current_vod.vec())
			{
				for (const auto& movie_pair : pair.second->movies.vec())
				{
					for (const auto& genre : movie_pair.second->genres.vec())
					{
						m_genres.set_back(genre.first, genre.second);
					}
					m_years.set_back(movie_pair.second->year, movie_pair.second->year);
				}
			}
		}
	}

	m_category_idx = m_current_vod.empty() ? -1 : 0;
	m_wndCategories.EnableWindow(!m_current_vod.empty());

	UpdateData(FALSE);
}

void CVodViewer::FillGenres()
{
	m_genre_idx = -1;
	m_wndGenres.ResetContent();

	for (const auto& genre : m_genres.vec())
	{
		m_wndGenres.AddString(genre.second.title.c_str());
	}

	if (m_wndGenres.GetCount())
	{
		m_wndGenres.InsertString(0, load_string_resource(m_plugin->get_plugin_type() == PluginType::enEdem ? IDS_STRING_NONE : IDS_STRING_ALL).c_str());
	}

	m_genre_idx = m_genres.empty() ? -1 : 0;
	m_wndGenres.EnableWindow(!m_genres.empty());

	UpdateData(FALSE);
}

void CVodViewer::FillYears()
{
	m_year_idx = -1;
	m_wndYears.ResetContent();

	std::vector<std::wstring> sortedYears;
	for (const auto& year : m_years.vec())
	{
		if (!year.first.empty())
		{
			sortedYears.emplace_back(year.first);
		}
	}
	std::sort(sortedYears.rbegin(), sortedYears.rend());

	for (const auto& year : sortedYears)
	{
		m_wndYears.AddString(year.c_str());
	}

	if (m_wndYears.GetCount())
	{
		m_wndYears.InsertString(0, load_string_resource(m_plugin->get_plugin_type() == PluginType::enEdem ? IDS_STRING_NONE : IDS_STRING_ALL).c_str());
	}

	m_year_idx = sortedYears.empty() ? -1 : 0;
	m_wndYears.EnableWindow(!sortedYears.empty());

	UpdateData(FALSE);
}

void CVodViewer::FillSeasons(const std::shared_ptr<vod_movie>& movie)
{
	m_season_idx = -1;
	m_wndSeason.ResetContent();
	m_wndSeason.EnableWindow(FALSE);

	const auto& str = load_string_resource(IDS_STRING_SEASON);

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

	m_season_idx = movie->seasons.empty() ? -1 : 0;
	m_wndSeason.EnableWindow(!movie->seasons.empty());

	FillEpisodes(movie);
}

void CVodViewer::FillEpisodes(const std::shared_ptr<vod_movie>& movie)
{
	m_episode_idx = -1;
	m_wndEpisode.ResetContent();
	m_wndEpisode.EnableWindow(FALSE);

	if (movie->seasons.empty() || m_season_idx < 0 || m_season_idx >= (int)movie->seasons.size())
	{
		return;
	}

	const auto& episodes = movie->seasons[m_season_idx].episodes;
	const auto& str = load_string_resource(IDS_STRING_EPISODE);

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

		FillQuality(episode.qualities);
		FillAudio(episode.audios);
	}

	m_episode_idx = episodes.empty() ? -1 : 0;
	m_wndEpisode.EnableWindow(!episodes.empty());
}

void CVodViewer::FillQuality(const vod_variants_storage& qualities)
{
	m_quality_idx = 0;
	m_wndQuality.ResetContent();

	for (const auto& q_it : qualities.vec())
	{
		m_wndQuality.AddString(q_it.second.title.c_str());
	}

	m_quality_idx = qualities.empty() ? -1 : 0;
	m_wndQuality.EnableWindow(!qualities.empty());
}

void CVodViewer::FillAudio(const vod_variants_storage& audios)
{
	m_audio_idx = 0;
	m_wndAudio.ResetContent();

	for (const auto& q_it : audios.vec())
	{
		m_wndAudio.AddString(q_it.second.title.c_str());
	}

	m_audio_idx = audios.empty() ? -1 : 0;
	m_wndAudio.EnableWindow(!audios.empty());
}

std::shared_ptr<vod_movie> CVodViewer::GetFilteredMovie(int idx)
{
	if (idx < 0 || idx >= (int)m_filtered_movies.size())
	{
		m_streamUrl.Empty();
		m_iconUrl.Empty();
		UpdateData(FALSE);
		return nullptr;
	}

	std::shared_ptr<vod_movie> movie = m_filtered_movies[idx];
	if (movie->url.empty() && movie->seasons.empty())
	{
		m_plugin->fetch_movie_info(m_account, *movie);
	}

	return movie;
}

void CVodViewer::LoadMovieInfo(int idx)
{
	UpdateData(TRUE);

	m_wndDescription.SetWindowText(L"");
	m_wndPoster.SetBitmap(nullptr);

	auto movie = GetFilteredMovie(idx);
	if (movie == nullptr)
	{
		m_wndSeason.ResetContent();
		m_wndEpisode.ResetContent();
		m_wndQuality.ResetContent();
		m_wndAudio.ResetContent();

		m_wndSeason.EnableWindow(FALSE);
		m_wndEpisode.EnableWindow(FALSE);
		m_wndQuality.EnableWindow(FALSE);
		m_wndAudio.EnableWindow(FALSE);
		return;
	}

	FillQuality(movie->quality);
	FillAudio(movie->audios);
	FillSeasons(movie);


	SetImageControl(GetIconCache().get_icon(movie->poster_url.get_uri()), m_wndPoster);

	std::string text = fmt::format(R"({{\rtf1 \b {:s}\b0\par )", utils::utf16_to_utf8(movie->title));

	if (!movie->title_orig.empty())
		text += fmt::format(R"({:s}\par )", utils::utf16_to_utf8(movie->title_orig));

	if (!movie->description.empty())
		text += fmt::format(R"({:s}\par )", utils::utf16_to_utf8(movie->description));

	utils::string_rtrim(text);
	text += R"(\par )";

	if (!movie->year.empty())
		text += fmt::format((load_string_resource_a(IDS_STRING_VOD_YEAR)), utils::utf16_to_utf8(movie->year));

	if (!movie->country.empty())
		text += fmt::format(load_string_resource_a(IDS_STRING_VOD_COUNTRY), utils::utf16_to_utf8(movie->country));

	if (!movie->director.empty())
		text += fmt::format(load_string_resource_a(IDS_STRING_VOD_DIRECTOR), utils::utf16_to_utf8(movie->director));

	if (!movie->casting.empty())
		text += fmt::format(load_string_resource_a(IDS_STRING_VOD_ACTORS), utils::utf16_to_utf8(movie->casting));

	if (!movie->age.empty())
		text += fmt::format(load_string_resource_a(IDS_STRING_VOD_AGE), utils::utf16_to_utf8(movie->age));

	if (movie->movie_time != 0)
	{
		int mins = movie->movie_time;
		int hours = movie->movie_time / 60;
		if (hours)
		{
			mins %= 60;
			text += fmt::format(load_string_resource_a(IDS_STRING_VOD_HOUR_MIN), hours, mins);
		}
		else
		{
			text += fmt::format(load_string_resource_a(IDS_STRING_VOD_MIN), mins);
		}
	}

	utils::string_rtrim(text);
	text += "}";

	SETTEXTEX set_text_ex = { ST_SELECTION, CP_UTF8 };
	m_wndDescription.SendMessage(EM_SETTEXTEX, (WPARAM)&set_text_ex, (LPARAM)text.c_str());

	GetUrl(idx);

	UpdateData(FALSE);
}

void CVodViewer::FilterList()
{
	UpdateData(TRUE);
	CWaitCursor cur;

	if (m_category_idx == LB_ERR || m_category_idx >= (int)m_current_vod.size())
	{
		m_filtered_movies.clear();
		m_wndProgress.ShowWindow(SW_HIDE);
		m_wndStop.ShowWindow(SW_HIDE);
		m_wndTotal.SetWindowText(fmt::format(L"{:d} / {:d}", m_filtered_movies.size(), m_total).c_str());
		m_wndMoviesList.SetItemCount(0);
		m_wndMoviesList.Invalidate();
		m_wndPoster.SetBitmap(nullptr);
		return;
	}

	m_wndBtnReload.EnableWindow(FALSE);

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

	if (m_plugin->get_plugin_type() == PluginType::enEdem)
	{
		do
		{
			if (m_genre_idx <= 0 && m_year_idx <= 0)
			{
				for (const auto& movie_pair : m_current_vod.getAt(m_category_idx)->movies.vec())
				{
					filtered_movies.set_back(movie_pair.first, movie_pair.second);
				}

				break;
			}

			boost::wregex re_url(LR"(^portal::\[key:(.+)\](.+)$)");
			boost::wsmatch m;
			const auto& vportal = m_account.get_portal();
			if (!boost::regex_match(vportal, m, re_url)) break;

			const auto& key = m[1].str();
			const auto& url = m[2].str();

			nlohmann::json json_request;
			json_request["filter"] = "on";
			json_request["limit"] = 300;
			json_request["offset"] = 0;
			json_request["key"] = utils::utf16_to_utf8(key);
			json_request["mac"] = "000000000000";
			json_request["app"] = "IPTV ChannelEditor";
			if (m_genre_idx > 0)
			{
				int id = utils::char_to_int(m_genres[m_genre_idx - 1].id);
				json_request["genre"] = id;
				ATLTRACE("\ngenre id: %d\n", id);
			}

			if (m_year_idx > 0)
			{
				const auto& years = utils::utf16_to_utf8(m_years[m_year_idx - 1]);
				json_request["years"] = years;
				ATLTRACE("\nyears: %s\n", years.c_str());
			}

			std::vector<std::string> headers;
			headers.emplace_back("accept: */*");
			headers.emplace_back("Content-Type: application/json");

			const auto& post = json_request.dump();
			ATLTRACE("\n%s\n", post.c_str());

			m_dl.SetUrl(url);
			m_dl.SetCacheTtl(GetConfig().get_int(true, REG_MAX_CACHE_TTL));
			m_dl.SetUserAgent(m_plugin->get_user_agent());

			CWaitCursor cur;
			std::stringstream data;
			if (!m_dl.DownloadFile(data, &headers, true, post.c_str())) break;

			JSON_ALL_TRY;
			nlohmann::json parsed_json = nlohmann::json::parse(data.str());
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
					movie->poster_url.set_scheme(L"http://");
					movie->rating = utils::get_json_wstring("rating", movie_item);
					movie->country = utils::get_json_wstring("country", movie_item);
					movie->year = utils::get_json_wstring("year", movie_item);
					movie->age = utils::get_json_wstring("agelimit", movie_item);
					movie->movie_time = utils::get_json_int("duration", movie_item);

					filtered_movies.set_back(movie->id, movie);
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

				std::stringstream next_data;
				const auto& next_post = json_request.dump();
				ATLTRACE("\n%s\n", post.c_str());
				if (!m_dl.DownloadFile(next_data, &headers, true, next_post.c_str())) break;

				parsed_json = nlohmann::json::parse(data.str());
			}
			JSON_ALL_CATCH;
		} while (false);

		if (::WaitForSingleObject(m_evtStop, 0) == WAIT_OBJECT_0)
			filtered_movies.clear();
	}
	else
	{
		for (const auto& movie_pair : m_current_vod.getAt(m_category_idx)->movies.vec())
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

			if (m_year_idx > 0)
			{
				year = selectedYear.CompareNoCase(movie->year.c_str()) == 0;
			}

			if (genre && year)
			{
				filtered_movies.set_back(movie_pair.first, movie_pair.second);
			}
		}
	}

	std::swap(m_filtered_movies, filtered_movies);

	m_wndProgress.ShowWindow(SW_HIDE);
	m_wndStop.ShowWindow(SW_HIDE);
	m_wndBtnReload.EnableWindow(TRUE);
	m_wndTotal.SetWindowText(fmt::format(L"{:d} / {:d}", m_filtered_movies.size(), m_total).c_str());
	m_wndMoviesList.SetItemCount((int)m_filtered_movies.size());
	m_wndMoviesList.Invalidate();

	m_wndMoviesList.SetItemState(-1, 0, LVIS_SELECTED);
	m_wndMoviesList.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	LoadMovieInfo(0);
}

void CVodViewer::GetUrl(int idx)
{
	if (idx == CB_ERR || idx >= (int)m_filtered_movies.size()) return;

	const auto& movie = m_filtered_movies[idx];

	base_plugin::movie_request request{ m_season_idx, m_episode_idx, m_quality_idx, m_audio_idx };

	m_streamUrl = m_plugin->get_movie_url(m_account, request, *movie).c_str();
	m_iconUrl = movie->poster_url.get_uri().c_str();

	UpdateData(FALSE);
}

void CVodViewer::OnCbnSelchangeComboPlaylist()
{
	CString selText;
	m_wndPlaylist.GetLBText(m_wndPlaylist.GetCurSel(), selText);
	m_current_vod = m_vod_storages[selText.GetString()];
	LoadPlaylist();
}
