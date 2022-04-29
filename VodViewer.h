#pragma once
#include "afxdialogex.h"
#include "Config.h"
#include "vod_movie.h"
#include "uri_stream.h"
#include "vod_category.h"
#include "PlayListEntry.h"
#include "UtilsLib/vectormap.h"


// CVodViewer dialog

class CVodViewer : public CDialogEx
{
	DECLARE_DYNAMIC(CVodViewer)

public:
	CVodViewer(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CVodViewer();

public:
	BOOL OnInitDialog() override;
	void OnCancel() override;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_VOD };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg LRESULT OnEndLoadPlaylist(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg LRESULT OnInitProgress(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg LRESULT OnUpdateProgress(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetDispinfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCacheHint(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCbnSelchangeComboCategories();
	afx_msg void OnCbnSelchangeComboGenres();
	afx_msg void OnNMDblclkListMovies(NMHDR* pNMHDR, LRESULT* pResult);

private:
	void LoadPlaylist();
	void LoadM3U8Playlist(const std::wstring& url);
	void FillCategories();
	void LoadMovieInfo(int idx);


public:
	StreamType m_plugin_type = StreamType::enBase;

protected:
	CComboBox m_wndCategories;
	CComboBox m_wndGenres;
	CListCtrl m_wndMoviesList;
	CComboBox m_wndFilterYear;
	CComboBox m_wndFilterGenre;
	CButton m_wndFilter;
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
	utils::vectormap<std::wstring, std::shared_ptr<vod_category>> m_categories;
	utils::vectormap<std::wstring, std::shared_ptr<vod_movie>> m_all_movies;
	std::unique_ptr<uri_stream> m_plugin;
	// all entries loaded from playlist, filled when parse playlist
	std::unique_ptr<Playlist> m_playlistEntries;
public:
};
