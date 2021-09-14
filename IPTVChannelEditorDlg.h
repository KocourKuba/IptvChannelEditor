
// EdemChannelEditorDlg.h : header file
//

#pragma once
#include "PlayListEntry.h"
#include "TreeCtrlEx.h"
#include "ChannelCategory.h"
#include "ChannelInfo.h"
#include "map_serializer.h"

// CEdemChannelEditorDlg dialog
class CIPTVChannelEditorDlg : public CDialogEx
{
// Construction
public:
	CIPTVChannelEditorDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EDEMCHANNELEDITOR_DIALOG };
#endif

public:
	struct SearchParams
	{
		std::string id;
		int hash = 0;
		CString searchString;
	};

	void SelectTreeItem(CTreeCtrlEx& ctl, const SearchParams& searchParams);
	static HTREEITEM FindTreeItem(CTreeCtrlEx& ctl, DWORD_PTR entry);
	static HTREEITEM FindTreeNextItem(CTreeCtrlEx& ctl, HTREEITEM hItem, DWORD_PTR entry);
	static HTREEITEM FindTreeSubItem(CTreeCtrlEx& ctl, HTREEITEM hItem, DWORD_PTR entry);
	static BaseInfo* GetBaseInfo(const CTreeCtrlEx* pTreeCtrl, HTREEITEM hItem);
	static void GetChannelStreamInfo(const std::string& url, std::string& audio, std::string& video);

	// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	BOOL OnInitDialog() override;
	void DoDataExchange(CDataExchange* pDX) override;	// DDX/DDV support
	void OnOK() override;
	void OnCancel() override;
	BOOL PreTranslateMessage(MSG* pMsg) override;
	BOOL DestroyWindow() override;

	afx_msg void OnKickIdle();
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);

	afx_msg void OnSave();
	afx_msg void OnUpdateSave(CCmdUI* pCmdUI);
	afx_msg void OnNewChannel();
	afx_msg void OnUpdateNewChannel(CCmdUI* pCmdUI);
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
	afx_msg void OnSortCategory();
	afx_msg void OnUpdateSortCategory(CCmdUI* pCmdUI);
	afx_msg void OnGetStreamInfo();
	afx_msg void OnUpdateGetStreamInfo(CCmdUI* pCmdUI);
	afx_msg void OnGetStreamInfoAll();
	afx_msg void OnUpdateGetStreamInfoAll(CCmdUI* pCmdUI);
	afx_msg void OnPlayStream();
	afx_msg void OnUpdatePlayStream(CCmdUI* pCmdUI);
	afx_msg void OnBnClickCheckArchive();
	afx_msg void OnSyncTreeItem();
	afx_msg void OnUpdateSyncTreeItem(CCmdUI* pCmdUI);

	afx_msg void OnBnClickedButtonAddNewChannelsList();
	afx_msg void OnBnClickedButtonPlFilter();
	afx_msg void OnBnClickedButtonAbout();
	afx_msg void OnBnClickedButtonCacheIcon();
	afx_msg void OnUpdateButtonCacheIcon(CCmdUI* pCmdUI);
	afx_msg void OnBnClickedButtonCustomPlaylist();
	afx_msg void OnBnClickedButtonPack();
	afx_msg void OnBnClickedButtonPlSearchNext();
	afx_msg void OnBnClickedButtonSearchNext();
	afx_msg void OnBnClickedButtonSettings();
	afx_msg void OnBnClickedButtonTestEpg1();
	afx_msg void OnBnClickedButtonTestEpg2();
	afx_msg void OnUpdateIcon();
	afx_msg void OnUpdateUpdateIcon(CCmdUI* pCmdUI);
	afx_msg void OnBnClickedCheckCustomize();
	afx_msg void OnEditChangeTvIdd();
	afx_msg void OnBnClickedCheckAdult();
	afx_msg void OnBnClickedCheckArchive();
	afx_msg void OnEnChangeEditEpg2ID();
	afx_msg void OnEnChangeEditEpg1ID();
	afx_msg void OnEnChangeEditStreamUrl();
	afx_msg void OnEnChangeEditUrlID();
	afx_msg void OnDeltaposSpinTimeShiftHours(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinArchiveCheckDay(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinArchiveCheckHour(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnChangeEditTimeShiftHours();
	afx_msg void OnEnChangeEditArchiveCheckDays();
	afx_msg void OnEnChangeEditArchiveCheckHours();
	afx_msg void OnTvnSelchangedTreeChannels(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclkTreeChannels(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMRclickTreeChannel(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTvnEndlabeleditTreeChannels(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTvnSelchangedTreePaylist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclkTreePaylist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMRclickTreePlaylist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMSetfocusTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTvnChannelsGetInfoTip(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTvnPlaylistGetInfoTip(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnStnClickedStaticIcon();
	afx_msg void OnUpdateAddUpdateChannel(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonPlSearchNext(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonSearchNext(CCmdUI* pCmdUI);
	afx_msg void OnBnClickedButtonDownloadPlaylist();
	afx_msg void OnCbnSelchangeComboIconSource();
	afx_msg void OnCbnSelchangeComboPluginType();
	afx_msg void OnCbnSelchangeComboPlaylist();
	afx_msg void OnCbnSelchangeComboChannels();
	afx_msg void OnCbnSelchangeComboStreamType();
	afx_msg void OnAddToFavorite();
	afx_msg void OnUpdateAddToFavorite(CCmdUI* pCmdUI);
	afx_msg void OnCopyTo(UINT id);
	afx_msg void OnMoveTo(UINT id);
	afx_msg void OnAddTo(UINT id);
	afx_msg LRESULT OnEndLoadPlaylist(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg LRESULT OnUpdateProgress(WPARAM wParam = 0, LPARAM lParam = 0);

	DECLARE_MESSAGE_MAP()

private:
	BOOL is_allow_save() const { return m_allow_save; }
	void set_allow_save(BOOL val = TRUE);

	bool LoadChannels(const CString& path);
	void LoadPlaylist(bool saveToFile = false);

	bool AddChannel(HTREEITEM hSelectedItem, int categoryId = -1);

	void FillTreeChannels(LPCSTR select = nullptr);
	void FillTreePlaylist();

	void GetStreamInfo(std::vector<uri_stream*>& container);

	void LoadChannelInfo(HTREEITEM hItem);
	void LoadPlayListInfo(HTREEITEM hItem);

	void PlayItem(HTREEITEM hItem, int archive_hour = 0, int archiveHour = 0) const;
	void UpdateChannelsCount();
	void UpdatePlaylistCount();

	void RemoveOrphanChannels();
	void CheckForExistingChannels(HTREEITEM root = nullptr);
	void CheckForExistingPlaylist();

	bool SetupCustomPlaylist(bool loaded);
	bool SetupOttKey(bool loaded);
	bool SetupLogin(bool loaded);
	bool SetupPin(bool loaded);

	ChannelCategory* GetItemCategory(HTREEITEM hItem) const;
	ChannelCategory* GetCategory(HTREEITEM hItem) const;
	HTREEITEM GetCategoryTreeItemById(int id) const;

	ChannelInfo* GetChannel(HTREEITEM hItem) const;
	std::shared_ptr<ChannelInfo> FindChannel(HTREEITEM hItem) const;
	PlaylistEntry* GetPlaylistEntry(HTREEITEM item) const;

	bool IsSelectedTheSameType() const;
	bool IsSelectedChannelsOrEntries(bool onlyChannel = false) const;
	bool IsSelectedCategory() const;
	bool IsSelectedNotFavorite() const;
	bool IsChannelSelectionConsistent() const;
	bool IsSelectedTheSameCategory() const;
	bool IsChannel(HTREEITEM hItem) const;
	bool IsCategory(HTREEITEM hItem) const;
	bool IsPlaylistEntry(HTREEITEM hItem) const;
	bool IsPlaylistCategory(HTREEITEM hItem) const;

	int GetNewCategoryID() const;
	int GetCategoryIdByName(const std::wstring& categoryName);
	void MoveChannels(HTREEITEM hBegin, HTREEITEM hEnd, bool down);
	void SwapCategories(const HTREEITEM hCur, const HTREEITEM hNext);

	void SwitchPlugin();

	std::wstring GetPluginNameW(bool bCamel = false) const;
	std::string GetPluginNameA(bool bCamel = false) const;
	void SaveStreamInfo();

	std::wstring GetAbsPath(LPCTSTR rel_path) { return theApp.GetAppPath(rel_path); };
	std::wstring GetPluginRegPath() const;

	void SaveReg(LPCTSTR path, LPCSTR szValue);
	void SaveReg(LPCTSTR path, LPCWSTR szValue);
	void SaveReg(LPCTSTR path, int value);

	CString ReadRegStringT(LPCTSTR path) const;
	std::string ReadRegStringA(LPCTSTR path) const;
	std::wstring ReadRegStringW(LPCTSTR path) const;
	int ReadRegInt(LPCTSTR path, int default = 0) const;

	void SaveRegPlugin(LPCTSTR path, LPCSTR szValue);
	void SaveRegPlugin(LPCTSTR path, LPCWSTR szValue);
	void SaveRegPlugin(LPCTSTR path, int value);

	CString ReadRegStringPluginT(LPCTSTR path) const;
	std::string ReadRegStringPluginA(LPCTSTR path) const;
	std::wstring ReadRegStringPluginW(LPCTSTR path) const;
	int ReadRegIntPlugin(LPCTSTR path, int default = 0) const;

protected:
	static CString m_probe;

	CFont m_largeFont;

	// GUI controls and variables

	CToolTipCtrl m_pToolTipCtrl;
	CComboBox m_wndPluginType;
	CTreeCtrlEx m_wndChannelsTree;
	CComboBox m_wndPlaylist;
	CTreeCtrlEx m_wndPlaylistTree;
	CComboBox m_wndChannels;
	CComboBox m_wndIconSource;
	CComboBox m_wndStreamType;

	CEdit m_wndStreamID;
	CEdit m_wndStreamUrl;
	CEdit m_wndEpgID2;
	CEdit m_wndEpgID1;
	CEdit m_wndInfoVideo;
	CEdit m_wndInfoAudio;
	CEdit m_wndTimeShift;
	CEdit m_wndArchiveCheckDays;
	CEdit m_wndArchiveCheckHours;
	CEdit m_wndSearch;
	CEdit m_wndPlSearch;
	CSpinButtonCtrl m_wndSpinTimeShift;
	CButton m_wndFilter;
	CButton m_wndArchive;
	CButton m_wndAdult;
	CButton m_wndCustom;
	CButton m_wndPlArchive;
	CButton m_wndTestEPG2;
	CButton m_wndTestEPG1;
	CButton m_wndChooseUrl;
	CButton m_wndDownloadUrl;
	CButton m_wndCheckArchive;
	CButton m_wndCacheIcon;
	CButton m_wndUpdateIcon;
	CButton m_wndSave;
	CStatic m_wndChannelIcon;
	CStatic m_wndPlIcon;
	CStatic m_wndChInfo;
	CStatic m_wndPlInfo;
	CStatic m_wndProgressInfo;
	CProgressCtrl m_wndProgress;

	CString m_search; // m_wndSearch
	CString m_streamUrl; // m_wndStreamUrl
	CString m_iconUrl;

	CString m_plSearch; // m_wndPlSearch
	CString m_plIconName;
	CString m_plID;
	CString m_plEPG;
	CString m_infoVideo; // m_wndInfoVideo
	CString m_infoAudio; // m_wndInfoAudio

	BOOL m_isArchive = FALSE; // m_wndArchive
	BOOL m_isAdult = FALSE; // m_wndAdult
	CString m_streamID; // m_wndStreamID
	CString m_epgID1; // Primary EPG source m_wndEpg1ID
	CString m_epgID2; // Secondary EPG source m_wndEpg2ID
	int m_timeShiftHours = 0; // m_wndTimeShift
	int m_archiveCheckDays = 0; // m_wndArchiveCheckays
	int m_archiveCheckHours = 0; // m_wndArchiveCheckHours
	int m_archivePlDays = 0; // m_wndArchiveDays
	int m_StreamType = 0;

private:
	BOOL m_embedded_info;
	std::string m_token;
	std::string m_domain;
	std::string m_login;
	std::string m_password;
	std::string m_host;

	HACCEL m_hAccel = nullptr;
	CTreeCtrlEx* m_lastTree = nullptr;

	CString m_toolTipText;

	CString m_chFileName;
	CString m_plFileName;
	CString m_player;
	BOOL m_bAutoSync = FALSE;
	BOOL m_allow_save = FALSE;
	bool m_menu_enable_channel = false;
	bool m_loading = false;
	bool m_inSync = false;
	bool m_bInFillTree = false;
	bool m_blockChecking = false;

	StreamType m_pluginType = StreamType::enEdem;
	CString m_pluginName;
	int m_lastIconSelected = 0;
	// Event to signal for load playlist thread
	CEvent m_evtStop;

	COLORREF m_normal;
	COLORREF m_gray;
	COLORREF m_red;
	COLORREF m_green;
	COLORREF m_brown;

	// Icons entries
	std::shared_ptr<std::vector<std::shared_ptr<PlaylistEntry>>> m_Icons;

	// all entries loaded from playlist
	std::unique_ptr<std::vector<std::shared_ptr<PlaylistEntry>>> m_playlistEntries;

	// map of all channels for fast search
	std::map<std::string, std::shared_ptr<ChannelInfo>> m_channelsMap;
	// map of all categories for fast search
	std::map<int, std::shared_ptr<ChannelCategory>> m_categoriesMap;
	std::map<int, HTREEITEM> m_categoriesTreeMap;

	// list of playlist id's in the same order as in the playlist
	// Must not contains duplicates!
	std::vector<std::string> m_playlistIds;
	std::map<std::string, std::shared_ptr<PlaylistEntry>> m_playlistMap;

	// map of category and TREEITEM for fast add to tree
	std::map<std::wstring, HTREEITEM> m_pl_categoriesTreeMap;

	// list of all channel lists
	std::vector<std::pair<std::wstring, std::wstring>> m_all_channels_lists;

	serializable_map m_stream_infos;
};

