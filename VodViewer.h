#pragma once
#include "afxdialogex.h"
#include "Config.h"
#include "vod_movie.h"
#include "uri_stream.h"
#include "PlayListEntry.h"
#include "UtilsLib/vectormap.h"


// CVodViewer dialog
using vod_category_storage = utils::vectormap<std::wstring, std::shared_ptr<vod_category>>;
using vod_movie_storage = utils::vectormap<std::wstring, std::shared_ptr<vod_movie>>;
using vod_genre_storage = utils::vectormap<std::wstring, std::wstring>;

class CVodViewer : public CDialogEx
{
	DECLARE_DYNAMIC(CVodViewer)

public:
	CVodViewer(vod_category_storage* categories, CWnd* pParent = nullptr);   // standard constructor
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
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetDispinfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCbnSelchangeComboCategories();
	afx_msg void OnCbnSelchangeComboGenres();
	afx_msg void OnCbnSelchangeComboYears();
	afx_msg void OnNMDblclkListMovies(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedButtonRefresh();
	afx_msg void OnBnClickedButtonSearch();

private:
	void LoadPlaylist(bool use_cache = true);
	void LoadM3U8Playlist(bool use_cache = true);
	void LoadJsonPlaylist(bool use_cache = true);
	void FillCategories();
	void FillGenres();
	void FillYears();
	void LoadMovieInfo(int idx);
	void FilterList();
	void FetchMovie(vod_movie& movie);

public:
	StreamType m_plugin_type = StreamType::enBase;
	std::wstring m_domain;
	std::wstring m_login;
	std::wstring m_password;
	std::wstring m_token;

protected:
	CComboBox m_wndCategories;
	CComboBox m_wndGenres;
	CComboBox m_wndYears;
	CComboBox m_wndSeason;
	CComboBox m_wndEpisode;
	CComboBox m_wndQuality;
	CListCtrl m_wndMoviesList;
	CButton m_wndSearch;
	CEdit m_wndTotal;
	CStatic m_wndProgressInfo;
	CStatic m_wndPoster;
	CProgressCtrl m_wndProgress;
	CRichEditCtrl m_wndDescription;

private:
	// Event to signal for load playlist thread
	CEvent m_evtStop;
	CEvent m_evtFinished;
	int m_category_idx = -1;
	int m_genre_idx = -1;
	int m_year_idx = -1;
	int m_season_idx = -1;
	int m_episode_idx = -1;
	vod_category_storage* m_vod_categories = nullptr;
	vod_movie_storage m_filtered_movies;
	vod_genre_storage m_genres;
	std::set<std::wstring> m_years;
	std::unique_ptr<uri_stream> m_plugin;
	// all entries loaded from playlist, filled when parse playlist
	std::unique_ptr<Playlist> m_playlistEntries;
	CString m_SearchText;
};