
// EdemChannelEditorDlg.h : header file
//

#pragma once
#include <memory>
#include "PlayListEntry.h"
#include "TreeCtrlEx.h"
#include "ChannelCategory.h"
#include "ChannelInfo.h"

// CEdemChannelEditorDlg dialog
class CEdemChannelEditorDlg : public CDialogEx
{
// Construction
public:
	CEdemChannelEditorDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EDEMCHANNELEDITOR_DIALOG };
#endif

public:
	static HTREEITEM FindTreeItem(CTreeCtrl& ctl, DWORD_PTR entry);
	static HTREEITEM FindTreeNextItem(CTreeCtrl& ctl, HTREEITEM hItem, DWORD_PTR entry);
	static HTREEITEM FindTreeSubItem(CTreeCtrl& ctl, HTREEITEM hItem, DWORD_PTR entry);
	static std::wstring TranslateStreamUri(const std::wstring& stream_uri);
	static void GetChannelStreamInfo(const std::wstring& url, std::string& audio, std::string& video);

	static CString GetAccessKey() { return m_embedded_info ? m_ch_access_key : m_gl_access_key; }
	static CString GetAccessDomain() { return m_embedded_info ? m_ch_domain : m_gl_domain; }
	static void SetAccessKey(const CString& access_key)
	{
		auto& target = m_embedded_info ? m_ch_access_key : m_gl_access_key;
		target = access_key;
	}

	static void SetDomain(const CString& domain)
	{
		auto& target = m_embedded_info ? m_ch_domain : m_gl_domain;
		target = domain;
	}

	// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	BOOL OnInitDialog() override;
	void DoDataExchange(CDataExchange* pDX) override;	// DDX/DDV support
	void OnOK() override;
	void OnCancel() override;
	BOOL PreTranslateMessage(MSG* pMsg) override;

	afx_msg void OnKickIdle();
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);

	afx_msg void OnSave();
	afx_msg void OnUpdateSave(CCmdUI* pCmdUI);
	afx_msg void OnNewChannel();
	afx_msg void OnAddUpdateChannel();
	afx_msg void OnRemoveChannel();
	afx_msg void OnUpdateRemoveChannel(CCmdUI* pCmdUI);
	afx_msg void OnAddCategory();
	afx_msg void OnUpdateAddCategory(CCmdUI* pCmdUI);
	afx_msg void OnChannelUp();
	afx_msg void OnUpdateChannelUp(CCmdUI* pCmdUI);
	afx_msg void OnChannelDown();
	afx_msg void OnUpdateChannelDown(CCmdUI* pCmdUI);
	afx_msg void OnToggleChannel();
	afx_msg void OnUpdateToggleChannel(CCmdUI* pCmdUI);
	afx_msg void OnRenameChannel();
	afx_msg void OnUpdateRenameChannel(CCmdUI* pCmdUI);
	afx_msg void OnNewCategory();
	afx_msg void OnUpdateNewCategory(CCmdUI* pCmdUI);
	afx_msg void OnRemoveCategory();
	afx_msg void OnUpdateRemoveCategory(CCmdUI* pCmdUI);
	afx_msg void OnGetStreamInfo();
	afx_msg void OnUpdateGetStreamInfo(CCmdUI* pCmdUI);
	afx_msg void OnGetStreamInfoAll();
	afx_msg void OnUpdateGetStreamInfoAll(CCmdUI* pCmdUI);
	afx_msg void OnPlayChannelStream();
	afx_msg void OnUpdatePlayChannelStream(CCmdUI* pCmdUI);
	afx_msg void OnPlayPlaylistStream();
	afx_msg void OnUpdatePlayPlaylistStream(CCmdUI* pCmdUI);
	afx_msg void OnPlayChannelStreamArchive();
	afx_msg void OnSyncEntry();
	afx_msg void OnUpdateSyncEntry(CCmdUI* pCmdUI);

	afx_msg void OnBnClickedButtonAddNewChannelsList();
	afx_msg void OnBnClickedButtonPlFilter();
	afx_msg void OnBnClickedButtonAbout();
	afx_msg void OnBnClickedButtonCacheIcon();
	afx_msg void OnUpdateButtonCacheIcon(CCmdUI* pCmdUI);
	afx_msg void OnBnClickedButtonCustomPlaylist();
	afx_msg void OnBnClickedButtonPack();
	afx_msg void OnBnClickedButtonPlSearchNext();
	afx_msg void OnBnClickedButtonAccessInfo();
	afx_msg void OnBnClickedButtonSearchNext();
	afx_msg void OnBnClickedButtonSettings();
	afx_msg void OnBnClickedButtonTestEpg();
	afx_msg void OnBnClickedButtonTestTvg();
	afx_msg void OnUpdateIcon();
	afx_msg void OnUpdateUpdateIcon(CCmdUI* pCmdUI);
	afx_msg void OnBnClickedCheckCustomize();
	afx_msg void OnEditChangeTvIdd();
	afx_msg void OnBnClickedCheckAdult();
	afx_msg void OnBnClickedCheckArchive();
	afx_msg void OnEnChangeEditTvgID();
	afx_msg void OnEnChangeEditEpgID();
	afx_msg void OnEnChangeEditStreamUrl();
	afx_msg void OnEnChangeEditUrlID();
	afx_msg void OnDeltaposSpinTimeShiftHours(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinArchiveCheck(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnChangeEditTimeShiftHours();
	afx_msg void OnEnChangeEditArchiveCheck();
	afx_msg void OnTvnSelchangedTreeChannels(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclkTreeChannels(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMRclickTreeChannel(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTvnEndlabeleditTreeChannels(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTvnSelchangedTreePaylist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclkTreePaylist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMRclickTreePlaylist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMSetfocusTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnStnClickedStaticIcon();
	afx_msg void OnUpdateAddUpdateChannel(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonPlSearchNext(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonSearchNext(CCmdUI* pCmdUI);
	afx_msg void OnBnClickedButtonDownloadPlaylist();
	afx_msg void OnCbnSelchangeComboPlaylist();
	afx_msg void OnCbnSelchangeComboChannels();
	afx_msg void OnCopyTo(UINT id);
	afx_msg void OnMoveTo(UINT id);
	afx_msg void OnAddTo(UINT id);
	afx_msg LRESULT OnStartLoadPlaylist(WPARAM wParam = 0, LPARAM lParam = 0);

	DECLARE_MESSAGE_MAP()

private:
	BOOL is_allow_save() const { return m_allow_save; }
	void set_allow_save(BOOL val = TRUE);

	bool LoadChannels(const CString& path);
	bool AddChannel(HTREEITEM hSelectedItem, int categoryId = -1);
	bool AddPlaylistEntry(std::unique_ptr<PlaylistEntry>& entry, BOOL bRegex, BOOL bCase);

	void FillChannels();
	void FillPlaylist();

	template <class T>
	void GetStreamInfo(const std::vector<T*>& channels, CStatic& staticCtrl);

	void GetInfoPlaylist(const std::vector<PlaylistEntry*>& playlist);

	void LoadChannelInfo(HTREEITEM hItem);
	void LoadPlayListInfo(HTREEITEM hItem);

	void PlayChannel(HTREEITEM hItem, int archive_hour = 0) const;
	void PlayPlaylistEntry(HTREEITEM hItem, int archive_hour = 0) const;
	void PlayStream(const std::wstring& stream_url, int archive_hour = 0) const;
	void UpdateChannelsCount();
	void UpdatePlaylistCount();

	void CheckForExisting();

	ChannelCategory* GetItemCategory(HTREEITEM hItem) const;
	ChannelCategory* GetCategory(HTREEITEM hItem) const;
	HTREEITEM GetCategoryItem(int id) const;
	std::map<int, HTREEITEM> GetCategoriesTreeMap() const;

	ChannelInfo* GetChannel(HTREEITEM hItem) const;
	PlaylistEntry* GetPlaylistEntry(HTREEITEM item) const;

	bool IsSelectedTheSameType() const;
	bool IsSelectedChannelsOrEntries(bool onlyChannel = false) const;
	bool IsSelectedCategory() const;
	bool IsChannelSelectionConsistent() const;
	bool IsSelectedTheSameCategory() const;
	bool IsChannel(HTREEITEM hItem) const;
	bool IsCategory(HTREEITEM hItem) const;
	bool IsPlaylistEntry(HTREEITEM hItem) const;
	bool IsPlaylistCategory(HTREEITEM hItem) const;
	bool IsCategoryInChannels(const ChannelCategory* category) const;

	ChannelInfo* FindChannelByEntry(const PlaylistEntry* entry) const;
	int GetNewCategoryID() const;
	int GetCategoryByName(const std::wstring& categoryName);
	void MoveChannels(HTREEITEM hBegin, HTREEITEM hEnd, bool down);
	void SwapCategories(const HTREEITEM hCur, const HTREEITEM hNext);

public:
	static CString m_probe;

protected:
	CToolTipCtrl m_pToolTipCtrl;
	CTreeCtrlEx m_wndChannelsTree;
	CComboBox m_wndPlaylistType;
	CTreeCtrlEx m_wndPlaylistTree;
	CComboBox m_wndChannels;
	CEdit m_wndStreamID;
	CEdit m_wndStreamUrl;
	CEdit m_wndTvgID;
	CEdit m_wndEpgID;
	CEdit m_wndInfoVideo;
	CEdit m_wndInfoAudio;
	CEdit m_wndTimeShift;
	CSpinButtonCtrl m_wndSpinTimeShift;
	CProgressCtrl m_wndProgress;
	CButton m_wndArchive;
	CButton m_wndAdult;
	CButton m_wndCustom;
	CButton m_wndPlArchive;
	CButton m_wndTestTVG;
	CButton m_wndTestEPG;
	CButton m_wndChooseUrl;
	CButton m_wndDownloadUrl;
	CButton m_wndGetInfo;
	CButton m_wndCheckArchive;
	CButton m_wndCacheIcon;
	CButton m_wndUpdateIcon;
	CButton m_wndSave;
	CStatic m_wndIcon;
	CStatic m_wndPlIcon;
	CStatic m_wndChInfo;
	CStatic m_wndPlInfo;
	CFont m_largeFont;

	CString m_search;
	CString m_streamUrl;
	CString m_iconUrl;

	CString m_plSearch;
	CString m_plIconName;
	CString m_plID;
	CString m_plEPG;
	CString m_infoVideo;
	CString m_infoAudio;

	HWND m_lastTree = nullptr;
	BOOL m_hasArchive = FALSE;
	BOOL m_isAdult = FALSE;
	int m_streamID = 0;
	int m_tvgID = 0;
	int m_epgID = 0;
	int m_archiveCheck = 0;
	int m_archiveDays = 0;
	int m_timeShiftHours = 0;

private:
	static CString m_gl_domain;
	static CString m_gl_access_key;
	static CString m_ch_access_key;
	static CString m_ch_domain;
	static BOOL m_embedded_info;

	CString m_chFileName;
	CString m_plFileName;
	CString m_player;
	CString m_filterString;
	BOOL m_bAutoSync = FALSE;
	BOOL m_filterRegex = FALSE;
	BOOL m_filterCase = FALSE;
	BOOL m_allow_save = FALSE;
	bool m_menu_enable_channel = false;
	HACCEL m_hAccel = nullptr;

	std::vector<std::unique_ptr<PlaylistEntry>>::iterator m_pl_cur_it;
	std::vector<std::unique_ptr<ChannelInfo>>::iterator m_cur_it;

	std::vector<std::unique_ptr<ChannelInfo>> m_channels;
	std::map<int, std::unique_ptr<ChannelCategory>> m_categories;

	std::set<int> m_playlistIds;
	std::vector<std::unique_ptr<PlaylistEntry>> m_playlist;
	std::vector<std::pair<std::wstring, HTREEITEM>> m_pl_categories;

	std::vector<std::pair<CString, CString>> m_all_playlists;
};

template <class T>
void CEdemChannelEditorDlg::GetStreamInfo(const std::vector<T*>& container, CStatic& staticCtrl)
{
	m_wndProgress.SetRange32(0, container.size());
	auto it = container.begin();
	while (it != container.end())
	{
		std::array<std::thread, 5> workers;
		std::array<std::string, 5> audio;
		std::array<std::string, 5> video;
		auto pool = it;
		int j = 0;
		while (j < 5 && pool != container.end())
		{
			const auto& url = (*pool)->get_stream_uri().get_ts_translated_url();
			workers[j] = std::thread(GetChannelStreamInfo, url, std::ref(audio[j]), std::ref(video[j]));
			j++;
			++pool;
		}

		j = 0;
		for (auto& w : workers)
		{
			if (!w.joinable()) continue;

			w.join();
			(*it)->set_audio(audio[j]);
			(*it)->set_video(video[j]);
			++it;
			j++;

			auto step = std::distance(container.begin(), it);
			CString str;
			str.Format(_T("Get Stream Info: %d from %d"), step, container.size());
			staticCtrl.SetWindowText(str);
			m_wndProgress.SetPos(step);
		}
	}
}
