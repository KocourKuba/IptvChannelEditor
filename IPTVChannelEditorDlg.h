
// EdemChannelEditorDlg.h : header file
//

#pragma once
#include "PlayListEntry.h"
#include "TreeCtrlEx.h"
#include "ChannelCategory.h"
#include "ChannelInfo.h"
#include "map_serializer.h"
#include "json.hpp"
#include "RichToolTipCtrl.h"

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

	struct CategoryInfo
	{
		HTREEITEM hItem;
		std::shared_ptr<ChannelCategory> category;
	};

	bool SelectTreeItem(CTreeCtrlEx& ctl, InfoType type, const SearchParams& searchParams);

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
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonPlSearchNext();
	afx_msg void OnBnClickedButtonSearchNext();
	afx_msg void OnBnClickedButtonSettings();
	afx_msg void OnBnClickedButtonTestEpg1();
	afx_msg void OnBnClickedButtonTestEpg2();
	afx_msg void OnUpdateIcon();
	afx_msg void OnUpdateUpdateIcon(CCmdUI* pCmdUI);
	afx_msg void OnBnClickedCheckCustomize();
	afx_msg void OnBnClickedCheckAdult();
	afx_msg void OnBnClickedCheckArchive();
	afx_msg void OnEnChangeEditEpg2ID();
	afx_msg void OnEnChangeEditEpg1ID();
	afx_msg void OnEnChangeEditStreamUrl();
	afx_msg void OnEnChangeEditArchiveDays();
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
	afx_msg BOOL OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult);
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

	afx_msg LRESULT OnUpdateProgress(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg LRESULT OnEndLoadPlaylist(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg LRESULT OnUpdateProgressStream(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg LRESULT OnEndGetStreamInfo(WPARAM wParam = 0, LPARAM lParam = 0);

	DECLARE_MESSAGE_MAP()

private:
	void SetUpToolTips();

	BOOL is_allow_save() const { return m_allow_save; }
	void set_allow_save(BOOL val = TRUE);

	bool LoadChannels(const CString& path);
	void LoadPlaylist(bool saveToFile = false);

	bool AddChannel(HTREEITEM hSelectedItem, int categoryId = -1);

	void FillTreeChannels(LPCSTR select = nullptr);
	void FillTreePlaylist();

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

	std::shared_ptr<ChannelCategory> GetItemCategory(HTREEITEM hItem) const;
	std::shared_ptr<ChannelCategory> GetCategory(HTREEITEM hItem) const;
	HTREEITEM GetCategoryTreeItemById(int id) const;

	std::shared_ptr<ChannelInfo> FindChannel(HTREEITEM hItem) const;
	std::shared_ptr<ChannelCategory> FindCategory(HTREEITEM hItem) const;
	std::shared_ptr<PlaylistEntry> FindEntry(HTREEITEM item) const;
	BaseInfo* GetBaseInfo(const CTreeCtrlEx* pCtl, HTREEITEM item) const;

	bool IsSelectedTheSameType() const;
	bool IsSelectedChannelsOrEntries(bool onlyChannel = false) const;
	bool IsSelectedCategory() const;
	bool IsSelectedNotFavorite() const;
	bool IsChannelSelectionConsistent() const;
	bool IsSelectedInTheSameCategory() const;
	bool IsChannel(HTREEITEM hItem) const;
	bool IsCategory(HTREEITEM hItem) const;
	bool IsPlaylistEntry(HTREEITEM hItem) const;
	bool IsPlaylistCategory(HTREEITEM hItem) const;

	int GetNewCategoryID() const;
	int GetCategoryIdByName(const std::wstring& categoryName);
	void MoveChannels(HTREEITEM hBegin, HTREEITEM hEnd, bool down);
	void SwapCategories(const HTREEITEM hLeft, const HTREEITEM hRight);

	void SwitchPlugin();

	std::wstring GetPluginNameW(bool bCamel = false) const;
	std::string GetPluginNameA(bool bCamel = false) const;

	bool HasEPG2();
	CString GetEpgText(BaseInfo* info, bool first = true);
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

	void UpdateExtToken(BaseInfo* info) const;

protected:
	CFont m_largeFont;
	// GUI controls and variables

	CRichToolTipCtrl m_wndToolTipCtrl;
	CComboBox m_wndPluginType;
	CTreeCtrlEx m_wndChannelsTree;
	CComboBox m_wndPlaylist;
	CTreeCtrlEx m_wndPlaylistTree;
	CComboBox m_wndChannels;
	CComboBox m_wndIconSource;
	CComboBox m_wndStreamType;

	CEdit m_wndStreamID;
	CEdit m_wndStreamUrl;
	CEdit m_wndEpgID1;
	CEdit m_wndEpgID2;
	CEdit m_wndArchiveDays;
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
	CButton m_wndStop;
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
	int m_archiveDays; // m_wndArchiveDays
	int m_timeShiftHours = 0; // m_wndTimeShift
	int m_archiveCheckDays = 0; // m_wndArchiveCheckays
	int m_archiveCheckHours = 0; // m_wndArchiveCheckHours
	int m_archivePlDays = 0; // always read only field
	int m_StreamType = 0; // m_wndStreamType

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

	CString m_probe;
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

	// current plugin type
	StreamType m_pluginType = StreamType::enEdem;

	// Last icon id selected in the icons resource editor
	int m_lastIconSelected = 0;

	// Event to signal for load playlist thread
	CEvent m_evtStop;

	COLORREF m_normal; // channel not present in the current playlist
	COLORREF m_gray; // channel disabled
	COLORREF m_red; // playlist entry not present in the current channels list
	COLORREF m_green; // channel present in the playlist and have not differences
	COLORREF m_brown; // channel has difference with same entry in the playlist

	// Stream info container. Loaded when switch plugin and updates by GetStreamInfo
	serializable_map m_stream_infos;

	// Icons entries
	// loaded when used icon resource list
	std::shared_ptr<std::vector<std::shared_ptr<PlaylistEntry>>> m_Icons;

	//////////////////////////////////////////////////////////////////////////
	// channels part

	// list of all channel lists, filled when switch plugin. Reads from \playlists\plugin-name\*.xml
	std::vector<std::pair<std::wstring, std::wstring>> m_all_channels_lists;

	// map of all categories for fast search to category key (id)
	// Loaded from channels list
	std::map<int, CategoryInfo> m_categoriesMap;

	// map of all channels for fast search
	// Loaded from channels list
	std::map<std::string, std::shared_ptr<ChannelInfo>> m_channelsMap;

	// map HTREE items to categories id
	// Loaded when fill channels tree
	std::map<HTREEITEM, int> m_categoriesTreeMap;

	// map of all channels htree items
	// Loaded when fill channels tree
	std::map<HTREEITEM, std::shared_ptr<ChannelInfo>> m_channelsTreeMap;

	//////////////////////////////////////////////////////////////////////////
	// playlist part

	// all entries loaded from playlist, filled when parse playlist
	std::unique_ptr<std::vector<std::shared_ptr<PlaylistEntry>>> m_playlistEntries;

	// list of playlist id's in the same order as in the playlist
	// Must not contains duplicates!
	// Loaded when fill playlist tree
	std::vector<std::string> m_playlistIds;

	// map of all playlist entries to entry id (channel id)
	// Loaded when fill playlist tree
	std::map<std::string, std::shared_ptr<PlaylistEntry>> m_playlistMap;

	// map HTREE items to entry
	// Loaded when fill playlist tree
	std::map<HTREEITEM, std::shared_ptr<PlaylistEntry>> m_playlistTreeMap;

	// map of category and TREEITEM for fast add to tree
	// Loaded when fill playlist tree
	std::map<std::wstring, HTREEITEM> m_pl_categoriesTreeMap;

	// map of TREEITEM and category for fast search
	// Loaded when fill playlist tree
	std::map<HTREEITEM, std::wstring> m_pl_categoriesMap;

	//////////////////////////////////////////////////////////////////////////
	// map epg to channel id
	std::map<std::string, nlohmann::json> m_epgMap;
};
