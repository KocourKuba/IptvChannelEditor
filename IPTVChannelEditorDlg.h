/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2022): sharky72 (https://github.com/KocourKuba)

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


// EdemChannelEditorDlg.h : header file
//

#pragma once
#include "PlayListEntry.h"
#include "TreeCtrlEx.h"
#include "ChannelCategory.h"
#include "ChannelInfo.h"
#include "map_serializer.h"
#include "TrayIcon.h"
#include "MySplitButton.h"
#include "ColorEdit.h"
#include "MenuEdit.h"
#include "vod_movie.h"

#include "UtilsLib\json_wrapper.h"

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
		std::wstring id;
		int hash = 0;
		CString searchString;
		InfoType type = InfoType::enChannel;
		bool select = true;
		bool next = false;
	};

	struct CategoryInfo
	{
		HTREEITEM hItem = nullptr;
		std::shared_ptr<ChannelCategory> category;
	};

	HTREEITEM SelectTreeItem(CTreeCtrlEx* ctl, const SearchParams& searchParams);
	void ReloadConfigs();

	// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	BOOL OnInitDialog() override;
	void DoDataExchange(CDataExchange* pDX) override;	// DDX/DDV support
	void OnOK() override;
	void OnCancel() override;
	BOOL PreTranslateMessage(MSG* pMsg) override;

	void AddTooltip(UINT ctrlID, UINT textID);

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
	afx_msg void OnRemove();
	afx_msg void OnUpdateRemove(CCmdUI* pCmdUI);
	afx_msg void OnAddCategory();
	afx_msg void OnUpdateAddCategory(CCmdUI* pCmdUI);
	afx_msg void OnChannelUp();
	afx_msg void OnUpdateChannelUp(CCmdUI* pCmdUI);
	afx_msg void OnChannelDown();
	afx_msg void OnUpdateChannelDown(CCmdUI* pCmdUI);
	afx_msg void OnToggleChannel();
	afx_msg void OnUpdateToggleChannel(CCmdUI* pCmdUI);
	afx_msg void OnToggleCategory();
	afx_msg void OnUpdateToggleCategory(CCmdUI* pCmdUI);
	afx_msg void OnRenameChannel();
	afx_msg void OnUpdateRenameChannel(CCmdUI* pCmdUI);
	afx_msg void OnNewCategory();
	afx_msg void OnUpdateNewCategory(CCmdUI* pCmdUI);
	afx_msg void OnSortCategory();
	afx_msg void OnUpdateSortCategory(CCmdUI* pCmdUI);
	afx_msg void OnGetStreamInfo();
	afx_msg void OnUpdateClearStreamInfo(CCmdUI* pCmdUI);
	afx_msg void OnClearStreamInfo();
	afx_msg void OnUpdateGetStreamInfo(CCmdUI* pCmdUI);
	afx_msg void OnPlayStream();
	afx_msg void OnUpdatePlayStream(CCmdUI* pCmdUI);
	afx_msg void OnSyncTreeItem();
	afx_msg void OnUpdateSyncTreeItem(CCmdUI* pCmdUI);

	afx_msg void OnBnClickedButtonEditConfig();
	afx_msg void OnBnClickedButtonCreateNewChannelsList();
	afx_msg void OnBnClickedCheckShowChangedCh();
	afx_msg void OnBnClickedButtonPlFilter();
	afx_msg void OnBnClickedButtonAddPlaylist();
	afx_msg void OnBnClickedButtonAbout();
	afx_msg void OnBnClickedButtonCacheIcon();
	afx_msg void OnUpdateButtonCacheIcon(CCmdUI* pCmdUI);
	afx_msg void OnBnClickedButtonAccountSettings();
	afx_msg void OnBnClickedButtonPack();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonPlSearchNext();
	afx_msg void OnBnClickedButtonSearchNext();
	afx_msg void OnBnClickedButtonSettings();
	afx_msg void OnBnClickedButtonViewEpg();
	afx_msg void OnBnClickedButtonEpg();
	afx_msg void OnBnClickedButtonUpdateChanged();
	afx_msg void OnBnClickedCheckShowChanged();
	afx_msg void OnBnClickedCheckNotAdded();
	afx_msg void OnBnClickedCheckShowUnknown();
	afx_msg void OnBnClickedCheckShowUrl();

	afx_msg void OnBnClickedCheckCustomUrl();
	afx_msg void OnBnClickedCheckCustomArchive();
	afx_msg void OnBnClickedCheckAdult();
	afx_msg void OnBnClickedCheckArchive();
	afx_msg void OnEnChangeEditEpg2ID();
	afx_msg void OnEnChangeEditEpg1ID();
	afx_msg void OnEnChangeEditStreamUrl();
	afx_msg void OnEnChangeEditStreamArchiveUrl();
	afx_msg void OnEnChangeEditArchiveDays();
	afx_msg void OnEnChangeEditUrlID();
	afx_msg void OnDeltaposSpinTimeShiftHours(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnChangeEditTimeShiftHours();
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
	afx_msg void OnCbnSelchangeComboCustomStreamType();
	afx_msg void OnCbnSelchangeComboCustomArcStreamType();
	afx_msg void OnAddToFavorite();
	afx_msg void OnUpdateAddToFavorite(CCmdUI* pCmdUI);
	afx_msg void OnCopyTo(UINT id);
	afx_msg void OnMoveTo(UINT id);
	afx_msg void OnAddTo(UINT id);
	afx_msg void OnMakeAccount(UINT id);
	afx_msg void OnMakeAll();
	afx_msg void OnMakeAllAccounts();
	afx_msg void OnRemoveUnknownChannels();
	afx_msg void OnRestore();
	afx_msg void OnAppExit();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonVod();
	afx_msg void OnBnDropDownSplitButtonUpdateChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedCheckShowEpg();
	afx_msg void OnBnClickedButtonReloadIcon();

	afx_msg LRESULT OnInitProgress(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg LRESULT OnSwitchPlugin(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg LRESULT OnUpdateProgress(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg LRESULT OnEndLoadPlaylist(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg LRESULT OnUpdateProgressStream(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg LRESULT OnEndGetStreamInfo(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg LRESULT OnTrayIconNotify(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnExit(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLoadChannelImage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLoadPlaylistImage(WPARAM wParam, LPARAM lParam);

	afx_msg BOOL OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()

private:
	bool is_allow_save() const { return m_allow_save; }
	void set_allow_save(bool val = true);

	bool LoadChannels();
	void LoadPlaylist(bool saveToFile = false);

	bool AddChannel(const std::shared_ptr<PlaylistEntry>& entry, int categoryId = -1);
	void CopyMoveChannelTo(int category_id, bool move);
	void FillTreeChannels(LPCWSTR select = nullptr);
	void FillTreePlaylist();
	std::vector<std::wstring> FilterPlaylist();

	void LoadChannelInfo(std::shared_ptr<ChannelInfo> channel = nullptr);
	void LoadPlayListInfo(HTREEITEM hItem = nullptr);

	void PlayItem(HTREEITEM hItem, int archive_hour = 0, int archiveHour = 0) const;

	void SearchTreeItem(InfoType type, bool next = false);

	void RemoveOrphanChannels();
	void UpdateChannelsTreeColors(HTREEITEM root = nullptr);
	void CheckForExistingPlaylist();

	std::shared_ptr<ChannelCategory> GetItemCategory(HTREEITEM hItem) const;
	std::shared_ptr<ChannelCategory> GetCategory(HTREEITEM hItem) const;

	std::shared_ptr<ChannelInfo> FindChannel(HTREEITEM hItem) const;
	std::shared_ptr<ChannelCategory> FindCategory(HTREEITEM hItem) const;
	std::shared_ptr<PlaylistEntry> FindEntry(HTREEITEM item) const;
	uri_stream* GetBaseInfo(const CTreeCtrlEx* pCtl, HTREEITEM item) const;

	bool IsSelectedTheSameType(const CTreeCtrlEx* pTreeCtl) const;
	bool IsSelectedChannelsOrEntries(bool onlyChannel = false) const;
	bool IsSelectedCategory() const;
	bool IsSelectedNotFavorite() const;
	bool IsSelectedNotInFavorite() const;
	bool IsChannelSelectionConsistent() const;
	bool IsSelectedInTheSameCategory() const;
	bool IsChannel(HTREEITEM hItem) const;
	bool IsCategory(HTREEITEM hItem) const;
	bool IsPlaylistEntry(HTREEITEM hItem) const;
	bool IsPlaylistCategory(HTREEITEM hItem) const;

	int GetNewCategoryID() const;
	void MoveChannels(HTREEITEM hBegin, HTREEITEM hEnd, bool down);
	void SwapCategories(const HTREEITEM hLeft, const HTREEITEM hRight);

	void SwitchPlugin();
	void CollectCredentials();

	void LoadTimerEPG();
	void FillEPG();
	void UpdateExtToken(uri_stream* uri) const;
	void UpdateControlsForItem(HTREEITEM hSelected = nullptr);
	bool CheckForSave();
	void SaveStreamInfo();
	void UpdateWindowTitle();

protected:
	CFont m_largeFont;
	// GUI controls and variables

	CToolTipCtrl m_wndToolTipCtrl;
	CComboBox m_wndPluginType;
	CButton m_wndBtnVod;
	CButton m_wndBtnEditConfig;
	CTreeCtrlEx m_wndChannelsTree;
	CComboBox m_wndPlaylist;
	CTreeCtrlEx m_wndPlaylistTree;
	CComboBox m_wndChannels;
	CComboBox m_wndIconSource;
	CComboBox m_wndStreamType;
	CComboBox m_wndCustomStreamType;
	CComboBox m_wndCustomArcStreamType;
	CRichEditCtrl m_wndEpg;
	CMySplitButton m_wndUpdateChanged;
	CSplitButton m_wndPack;

	CEdit m_wndStreamID;
	CMenuEdit m_wndStreamUrl;
	CMenuEdit m_wndCustomArchiveUrl;
	CColorEdit m_wndEpgID1;
	CColorEdit m_wndEpgID2;
	CColorEdit m_wndArchiveDays;
	CColorEdit m_wndIconUrl;
	CEdit m_wndInfoVideo;
	CEdit m_wndInfoAudio;
	CEdit m_wndTimeShift;
	CEdit m_wndSearch;
	CEdit m_wndPlSearch;
	CSpinButtonCtrl m_wndSpinTimeShift;
	CButton m_wndBtnSearchNext;
	CButton m_wndBtnPlSearchNext;
	CButton m_wndBtnAddNewChannelsList;
	CButton m_wndShowUrl;
	CButton m_wndBtnFilter;
	CButton m_wndShowUnknown;
	CButton m_wndShowChanged;
	CButton m_wndShowChangedCh;
	CButton m_wndBtnAddPlaylist;
	CButton m_wndNotAdded;
	CButton m_wndArchive;
	CButton m_wndAdult;
	CButton m_wndBtnCustomUrl;
	CButton m_wndBtnCustomArchiveUrl;
	CButton m_wndPlArchive;
	CButton m_wndBtnViewEPG;
	CButton m_wndBtnAccountSetting;
	CButton m_wndBtnDownloadPlaylist;
	CButton m_wndBtnCacheIcon;
	CButton m_wndBtnSave;
	CButton m_wndBtnStop;
	CStatic m_wndChannelIcon;
	CStatic m_wndPlIcon;
	CStatic m_wndChInfo;
	CStatic m_wndPlInfo;
	CStatic m_wndProgressInfo;
	CButton m_wndShowEPG;
	CButton m_wndEpg1;
	CButton m_wndEpg2;
	CButton m_wndBtnSettings;
	CButton m_wndMakeWebUpdate;
	CProgressCtrl m_wndProgress;
	CProgressCtrl m_wndProgressTime;
	CTrayIcon m_wndTrayIcon;

	CString m_search; // m_wndSearch
	CString m_streamUrl; // m_wndStreamUrl
	CString m_streamArchiveUrl; // m_wndStreamArchiveUrl
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
	int m_archiveDays = 0; // m_wndArchiveDays
	int m_timeShiftHours = 0; // m_wndTimeShift
	int m_archivePlDays = 0; // always read only field

private:
	Credentials m_cur_account;

	PluginType m_plugin_type = PluginType::enCustom;
	std::shared_ptr<base_plugin> m_plugin;

	HACCEL m_hAccel = nullptr;
	CTreeCtrlEx* m_lastTree = nullptr;

	CString m_toolTipText;
	std::wstring m_plFileName;

	bool m_allow_save = false;
	bool m_loading = false;
	bool m_inStreamInfo = false;
	bool m_inSync = false;
	bool m_bInFillTree = false;
	bool m_blockChecking = false;
	bool m_menu_enable_channel = false;
	bool m_menu_enable_category = false;

	std::map<CWnd*, std::wstring> m_tooltips_info;

	// Last icon id selected in the icons resource editor
	int m_lastIconSelected = 0;
	UINT_PTR m_update_epg_timer = 0;
	UINT_PTR m_load_epg_timer = 0;
	UINT_PTR m_switch_plugin_timer = 0;
	std::map<std::wstring, int> m_changedChannels;
	std::set<std::wstring> m_unknownChannels;

	// Event to signal for load playlist thread
	CEvent m_evtStop;
	CEvent m_evtThreadExit;

	COLORREF m_normal; // channel not present in the current playlist
	COLORREF m_gray; // channel disabled
	COLORREF m_colorNotAdded; // playlist entry not present in the current channels list
	COLORREF m_colorNotChanged; // playlist entry not present in the current channels list
	COLORREF m_colorAdded; // channel present in the playlist and have not differences
	COLORREF m_colorHEVC; // channel HEVC
	COLORREF m_colorChanged; // channel has difference with same entry in the playlist

	// Stream info container. Loaded when switch plugin and updates by GetStreamInfo
	serializable_map m_stream_infos;

	// Icons entries
	// loaded when used icon resource list
	std::shared_ptr<Playlist> m_Icons;

	// Accounts
	std::vector<Credentials> m_all_credentials;

	//////////////////////////////////////////////////////////////////////////
	// channels part

	// list of all channel lists, filled when switch plugin. Reads from \playlists\plugin-name\*.xml
	std::vector<std::wstring> m_all_channels_lists;

	// list of all configs
	std::vector<std::wstring> m_all_configs_lists;

	// map of all categories for fast search to category key (id)
	// Loaded from channels list
	std::map<int, CategoryInfo> m_categoriesMap;

	// map of all channels for fast search
	// Loaded from channels list
	std::unordered_map<std::wstring, std::shared_ptr<ChannelInfo>> m_channelsMap;

	// map HTREE items to categories id
	// Loaded when fill channels tree
	std::unordered_map<HTREEITEM, int> m_categoriesTreeMap;

	// map of all channels htree items
	// Loaded when fill channels tree
	std::unordered_map<HTREEITEM, std::shared_ptr<ChannelInfo>> m_channelsTreeMap;

	//////////////////////////////////////////////////////////////////////////
	// playlist part

	std::vector<PlaylistTemplateInfo> m_playlist_info;
	// all entries loaded from playlist, filled when parse playlist
	std::unique_ptr<Playlist> m_playlistEntries;

	// list of playlist id's in the same order as in the playlist
	// Must not contains duplicates!
	// Loaded when fill playlist tree
	std::vector<std::wstring> m_playlistIds;

	// map of all playlist entries to entry id (channel id)
	// Loaded when fill playlist tree
	std::unordered_map<std::wstring, std::shared_ptr<PlaylistEntry>> m_playlistMap;

	// map HTREE items to entry
	// Loaded when fill playlist tree
	std::unordered_map<HTREEITEM, std::shared_ptr<PlaylistEntry>> m_playlistTreeMap;

	// map of category and TREEITEM for fast add to tree
	// Loaded when fill playlist tree
	std::unordered_map<std::wstring, HTREEITEM> m_pl_categoriesTreeMap;

	// map of TREEITEM and category for fast search
	// Loaded when fill playlist tree
	std::unordered_map<HTREEITEM, std::wstring> m_pl_categoriesMap;

	//////////////////////////////////////////////////////////////////////////
	// map epg to channel id
	std::array<std::unordered_map<std::wstring, std::map<time_t, EpgInfo>>, 2> m_epg_cache;

	//////////////////////////////////////////////////////////////////////////
	// vod
	std::map<PluginType, std::map<std::wstring, vod_category_storage>> m_vod_categories;
public:
	afx_msg void OnBeginlabeleditTreeChannels(NMHDR* pNMHDR, LRESULT* pResult);
};
