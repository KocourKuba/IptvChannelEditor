
// EdemChannelEditorDlg.h : header file
//

#pragma once
#include <memory>
#include "PlayListEntry.h"
#include "ColorTreeCtrl.h"
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

	afx_msg void OnAccelRemoveChannel();
	afx_msg void OnAccelChannelUp();
	afx_msg void OnAccelChannelDown();
	afx_msg void OnBnClickedButtonAbout();
	afx_msg void OnBnClickedButtonAddCategory();
	afx_msg void OnBnClickedButtonAddToShowIn();
	afx_msg void OnBnClickedButtonCacheIcon();
	afx_msg void OnBnClickedButtonEditCategory();
	afx_msg void OnBnClickedButtonGetInfo();
	afx_msg void OnBnClickedButtonGetInfoPl();
	afx_msg void OnBnClickedButtonImport();
	afx_msg void OnBnClickedButtonLoadPlaylist();
	afx_msg void OnBnClickedButtonPack();
	afx_msg void OnBnClickedButtonPlSearchNext();
	afx_msg void OnBnClickedButtonRemoveCategory();
	afx_msg void OnBnClickedButtonRemoveChannel();
	afx_msg void OnBnClickedButtonRemoveFromShowIn();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonSearchNext();
	afx_msg void OnBnClickedButtonSettings();
	afx_msg void OnBnClickedButtonSort();
	afx_msg void OnBnClickedButtonTestEpg();
	afx_msg void OnBnClickedButtonTestTvg();
	afx_msg void OnBnClickedButtonTestUrl();
	afx_msg void OnBnClickedButtonUpdateIcon();
	afx_msg void OnBnClickedCheckCustomize();
	afx_msg void OnChanges();
	afx_msg void OnDeltaposSpinNext(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinPrev(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnChangeEditNum();
	afx_msg void OnEnChangeEditChannelName();
	afx_msg void OnTvnSelchangedTreeChannels(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclkTreeChannels(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTvnSelchangedTreePaylist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclkTreePaylist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnStnClickedStaticIcon();
	afx_msg void OnUpdateButtonAddToShowIn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonCacheIcon(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonGetInfo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonGetInfoPl(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonImport(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonPlSearchNext(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonRemoveChannel(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonRemoveFromShow(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonSave(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonSearchNext(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonTestEpg(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonTestUrl(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonUpdateIcon(CCmdUI* pCmdUI);
	afx_msg void OnBnClickedButtonChannelUp();
	afx_msg void OnUpdateButtonChannelUp(CCmdUI* pCmdUI);
	afx_msg void OnBnClickedButtonChannelDown();
	afx_msg void OnBnClickedButtonDownloadPlaylist();
	afx_msg void OnUpdateButtonChannelDown(CCmdUI* pCmdUI);
	afx_msg void OnTvnSelchangingTreeChannels(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCbnSelchangeComboPlaylist();

	DECLARE_MESSAGE_MAP()

private:
	BOOL is_allow_save() const { return m_allow_save; }
	void set_allow_save(BOOL val = TRUE);

	bool LoadChannels(const std::wstring& path);
	void SaveChannels();
	void LoadPlaylist(const CString& file);

	void FillCategories();
	void FillChannels();
	void FillPlaylist();

	void LoadChannelInfo();
	void SaveChannelInfo();

	void PlayStream(const std::wstring& stream_url);
	void UpdateChannelsCount();

	void CheckLimits();
	void CheckForExisting();

	void SetCurrentChannel(HTREEITEM hCur);
	ChannelInfo* GetChannel(HTREEITEM hItem);
	ChannelInfo* GetCurrentChannel();
	PlaylistEntry* GetPlaylistEntry(HTREEITEM item);
	PlaylistEntry* GetCurrentPlaylistEntry();
	ChannelCategory* GetCategory(int hItem);

	HTREEITEM FindTreeItem(CTreeCtrl& ctl, DWORD_PTR entry);
	HTREEITEM FindTreeNextItem(CTreeCtrl& ctl, HTREEITEM hItem, DWORD_PTR entry);
	HTREEITEM FindTreeSubItem(CTreeCtrl& ctl, HTREEITEM hItem, DWORD_PTR entry);

	void ChangeControlsState(BOOL enable);
	int FindCategory(const std::wstring& name);
	int GetNewCategoryID();
	ChannelInfo* CreateChannel();
	void SwapChannels(HTREEITEM hCur, HTREEITEM hNext);

public:
	static CString m_domain;
	static CString m_accessKey;
	static CString m_probe;

protected:
	CColorTreeCtrl m_wndChannelsTree;
	CComboBox m_wndCategories;
	CListBox m_wndCategoriesList;
	CComboBox m_wndPlaylistType;
	CColorTreeCtrl m_wndPlaylistTree;
	CEdit m_wndStreamID;
	CEdit m_wndStreamUrl;
	CButton m_wndArchive;
	CButton m_wndAdult;
	CButton m_wndCustom;
	CButton m_wndPlArchive;
	CButton m_wndTestTVG;
	CButton m_wndTestEPG;
	CButton m_wndTestUrl;
	CButton m_wndAddToShow;
	CButton m_wndRemoveFromShow;
	CButton m_wndEditCategory;
	CButton m_wndAddCategory;
	CButton m_wndRemoveCategory;
	CButton m_wndGetInfo;
	CStatic m_wndIcon;
	CStatic m_wndPlIcon;
	CFont m_largeFont;

	CString m_channelName;
	CString m_search;
	CString m_streamUrl;
	CString m_iconUrl;

	CString m_plSearch;
	CString m_plFileName;
	CString m_plInfo;
	CString m_plIconName;
	CString m_plID;
	CString m_plEPG;
	CString m_infoVideo;
	CString m_infoAudio;
	CString m_channelsInfo;

	BOOL m_hasArchive = FALSE;
	BOOL m_isAdult = FALSE;
	BOOL m_isDisabled = FALSE;
	int m_streamID = 0;
	int m_tvgID = 0;
	int m_epgID = 0;
	int m_prevDays = 0;
	int m_nextDays = 0;
	int m_sortType = 0;

private:
	CString m_player;
	BOOL m_allow_save = FALSE;
	HACCEL m_hAccel = nullptr;
	HTREEITEM m_current = nullptr;
	HTREEITEM m_pl_current = nullptr;

	std::map<int, std::unique_ptr<PlaylistEntry>>::iterator m_pl_cur_it;
	std::vector<std::unique_ptr<ChannelInfo>>::iterator m_cur_it;

	std::map<int, std::unique_ptr<ChannelCategory>> m_channels_categories;
	std::map<int, HTREEITEM> m_tree_categories;
	std::vector<std::unique_ptr<ChannelInfo>> m_channels;
	std::map<int, std::unique_ptr<PlaylistEntry>> m_playlist;
	std::vector<std::pair<std::wstring, HTREEITEM>> m_playlist_categories;
};

std::wstring TranslateStreamUri(const std::string& stream_uri);
void GetChannelStreamInfo(const std::string& url, std::string& audio, std::string& video);