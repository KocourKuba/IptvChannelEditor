/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2025): sharky72 (https://github.com/KocourKuba)

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

#pragma once
#include <afxdialogex.h>
#include "vod_movie.h"
#include "base_plugin.h"
#include "PlayListEntry.h"
#include "ListCtrlEx.h"


// CVodViewer dialog
using vod_category_storage = utils::vectormap<std::wstring, std::shared_ptr<vod_category>>;

class CVodViewer : public CDialogEx
{
	DECLARE_DYNAMIC(CVodViewer)

public:
	CVodViewer(std::map<std::wstring, vod_category_storage>& categories, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CVodViewer();

public:
	BOOL OnInitDialog() override;
	void OnCancel() override;
	BOOL DestroyWindow() override;
	BOOL PreTranslateMessage(MSG* pMsg) override;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_VOD };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg LRESULT OnEndLoadM3U8Playlist(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg LRESULT OnEndLoadJsonPlaylist(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg LRESULT OnInitProgress(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg LRESULT OnUpdateProgress(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg void OnCbnSelchangeComboPlaylist();
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetDispinfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCbnSelchangeComboCategories();
	afx_msg void OnCbnSelchangeComboGenres();
	afx_msg void OnCbnSelchangeComboYears();
	afx_msg void OnCbnSelchangeComboSeason();
	afx_msg void OnCbnSelchangeComboEpisode();
	afx_msg void OnCbnSelchangeComboQuality();
	afx_msg void OnCbnSelchangeComboAudio();
	afx_msg void OnNMDblclkListMovies(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedButtonRefresh();
	afx_msg void OnBnClickedButtonSearch();
	afx_msg void OnBnClickedButtonStop();

private:
	std::shared_ptr<vod_movie> GetFilteredMovie(int idx);

	void LoadPlaylist(bool use_cache = true);
	void LoadM3U8Playlist(bool use_cache = true);
	void LoadJsonPlaylist(bool use_cache = true);
	void FillCategories();
	void FillSeasons(const std::shared_ptr<vod_movie>& movie);
	void FillEpisodes(const std::shared_ptr<vod_movie>& movie);
	void FillQuality(const vod_variants_storage& qualities);
	void FillAudio(const vod_variants_storage& audios);
	void FillGenres();
	void FillYears();
	void LoadMovieInfo(int idx);
	void FilterList();
	void GetUrl(int idx);

public:
	Credentials m_account;
	std::shared_ptr<base_plugin> m_plugin;

protected:
	CComboBox m_wndPlaylist;
	CComboBox m_wndCategories;
	CComboBox m_wndGenres;
	CComboBox m_wndYears;
	CComboBox m_wndSeason;
	CComboBox m_wndEpisode;
	CComboBox m_wndQuality;
	CComboBox m_wndAudio;
	CListCtrlEx m_wndMoviesList;
	CButton m_wndSearch;
	CEdit m_wndTotal;
	CEdit m_wndStreamUrl;
	CEdit m_wndSIconUrl;
	CStatic m_wndPoster;
	CProgressCtrl m_wndProgress;
	CRichEditCtrl m_wndDescription;
	CButton m_wndStop;
	CButton m_wndBtnReload;

private:
	utils::CUrlDownload m_dl;
	// Event to signal for load playlist thread
	CEvent m_evtStop;
	CEvent m_evtFinished;
	int m_category_idx = -1;
	int m_genre_idx = -1;
	int m_year_idx = -1;
	int m_season_idx = -1;
	int m_episode_idx = -1;
	int m_quality_idx = -1;
	int m_audio_idx = -1;
	size_t m_total = 0;
	vod_category_storage m_current_vod;
	std::map<std::wstring, vod_category_storage>& m_vod_storages;
	vod_movie_storage m_filtered_movies;
	vod_genre_storage m_genres;
	utils::vectormap<std::wstring, std::wstring> m_years;
	// all entries loaded from playlist, filled when parse playlist
	std::unique_ptr<Playlist> m_playlistEntries;
	CString m_SearchText;
	CString m_streamUrl;
	CString m_iconUrl;
};
