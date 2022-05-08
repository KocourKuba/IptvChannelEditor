#pragma once
#include "afxdialogex.h"
#include "Config.h"
#include "vod_movie.h"
#include "uri_stream.h"
#include "vod_category.h"
#include "PlayListEntry.h"
#include "UtilsLib/vectormap.h"


// CVodViewer dialog
using vod_category_storage = utils::vectormap<std::wstring, std::shared_ptr<vod_category>>;

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

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_VOD };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg LRESULT OnEndLoadPlaylist(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg LRESULT OnEndLoadJsonPlaylist(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg LRESULT OnInitProgress(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg LRESULT OnUpdateProgress(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetDispinfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCacheHint(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCbnSelchangeComboCategories();
	afx_msg void OnCbnSelchangeComboGenres();
	afx_msg void OnCbnSelchangeComboYears();
	afx_msg void OnNMDblclkListMovies(NMHDR* pNMHDR, LRESULT* pResult);

private:
	void LoadJsonCategories(const std::wstring& url);
	void LoadM3U8Playlist(const std::wstring& url);
	void LoadJsonPlaylist(const std::wstring& url);
	void FillCategories();
	void FillGenres();
	void FillYears();
	void LoadMovieInfo(int idx);
	void FilterList();

public:
	StreamType m_plugin_type = StreamType::enBase;

protected:
	CComboBox m_wndCategories;
	CListCtrl m_wndMoviesList;
	CComboBox m_wndYears;
	CComboBox m_wndGenres;
	CComboBox m_wndQuality;
	CStatic m_wndQualityText;
	CButton m_wndSearch;
	CEdit m_wndTotal;
	CEdit m_wndSearchText;
	CStatic m_wndProgressInfo;
	CStatic m_wndPoster;
	CProgressCtrl m_wndProgress;

private:
	// Event to signal for load playlist thread
	CEvent m_evtStop;
	CEvent m_evtFinished;
	CString m_description;
	int m_category_idx;
	vod_category_storage* m_vod_categories = nullptr;
	utils::vectormap<std::wstring, std::shared_ptr<vod_movie>> m_filtered_movies;
	std::set<std::wstring> m_genres;
	std::set<std::wstring> m_years;
	std::unique_ptr<uri_stream> m_plugin;
	// all entries loaded from playlist, filled when parse playlist
	std::unique_ptr<Playlist> m_playlistEntries;
};
