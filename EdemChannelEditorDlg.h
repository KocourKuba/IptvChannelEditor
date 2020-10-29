
// EdemChannelEditorDlg.h : header file
//

#pragma once
#include <memory>
#include <vector>
#include "PlayListEntry.h"
#include "ColorListBox.h"
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

	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedButtonAbout();
	afx_msg void OnBnClickedButtonAddCategory();
	afx_msg void OnBnClickedButtonAddChannel();
	afx_msg void OnUpdateButtonAddChannel(CCmdUI* pCmdUI);
	afx_msg void OnBnClickedButtonAddToShowIn();
	afx_msg void OnUpdateButtonAddToShowIn(CCmdUI* pCmdUI);
	afx_msg void OnBnClickedButtonEditCategory();
	afx_msg void OnBnClickedButtonImport();
	afx_msg void OnUpdateButtonImport(CCmdUI* pCmdUI);
	afx_msg void OnBnClickedButtonLoadPlaylist();
	afx_msg void OnBnClickedButtonPack();
	afx_msg void OnBnClickedButtonRemoveCategory();
	afx_msg void OnBnClickedButtonRemoveChannel();
	afx_msg void OnUpdateButtonRemoveChannel(CCmdUI* pCmdUI);
	afx_msg void OnBnClickedButtonRemoveFromShowIn();
	afx_msg void OnUpdateButtonRemoveFromShow(CCmdUI* pCmdUI);
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnUpdateButtonSave(CCmdUI* pCmdUI);
	afx_msg void OnBnClickedButtonTestEpg();
	afx_msg void OnBnClickedButtonTestUrl();
	afx_msg void OnBnClickedCheckCustomize();
	afx_msg void OnBnClickedButtonSettings();
	afx_msg void OnBnClickedButtonUpdateIcon();
	afx_msg void OnUpdateButtonUpdateIcon(CCmdUI* pCmdUI);
	afx_msg void OnBnClickedButtonCacheIcon();
	afx_msg void OnUpdateButtonCacheIcon(CCmdUI* pCmdUI);
	afx_msg void OnChanges();
	afx_msg void OnDeltaposSpinNext(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinPrev(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnChangeEditNum();
	afx_msg void OnEnChangeEditPlSearch();
	afx_msg void OnUpdateButtonPlSearchNext(CCmdUI* pCmdUI);
	afx_msg void OnEnChangeEditSearch();
	afx_msg void OnUpdateButtonSearchNext(CCmdUI* pCmdUI);
	afx_msg void OnEnKillfocusEditChannelName();
	afx_msg void OnEnKillfocusEditStreamUrl();
	afx_msg void OnEnKillfocusEditTvgId();
	afx_msg void OnEnKillfocusEditUrlId();
	afx_msg void OnLbnDblclkListPlaylist();
	afx_msg void OnLbnSelchangeListChannels();
	afx_msg void OnLbnSelchangeListPlaylist();
	afx_msg void OnPaint();
	afx_msg void OnStnClickedStaticIcon();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnKickIdle();
	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()

private:
	BOOL is_allow_save() const { return m_allow_save; }
	void set_allow_save(BOOL val = TRUE);

	BOOL LoadSetting();
	void LoadPlaylist(const CString& file);
	void LoadChannels();
	void SaveChannels();
	void FillCategories();
	void LoadChannelInfo();
	void SaveChannelInfo();
	void PlayStream(const std::string& stream_uri);

	void CheckLimits();
	void CheckForExisting();
	ChannelInfo* GetChannel(int idx);
	ChannelCategory* GetCategory(int idx);
	PlaylistEntry* GetPlaylistEntry(int idx);

	int FindCategory(const std::wstring& name);
	int GetFreeCategoryID();
	ChannelInfo* CreateChannel();
	bool LoadFromFile(const std::wstring & path);

protected:
	CColorListBox<ChannelInfo> m_wndChannelsList;
	CComboBox m_wndCategories;
	CListBox m_wndCategoriesList;
	CColorListBox<PlaylistEntry> m_wndPlaylist;
	CEdit m_wndStreamID;
	CEdit m_wndStreamUrl;
	CSpinButtonCtrl m_wndSpinPrev;
	CSpinButtonCtrl m_wndSpinNext;
	CButton m_wndCustom;
	CButton m_wndSave;
	CButton m_wndImport;
	CButton m_wndPack;
	CStatic m_wndIcon;
	CStatic m_wndPlIcon;
	CFont m_largeFont;

	CString m_search;
	CString m_plSearch;
	CString m_channelName;
	CString m_streamUrl;

	CString m_plFileName;
	CString m_plChannelName;
	CString m_plIconName;

	BOOL m_hasArchive = FALSE;
	BOOL m_isAdult = FALSE;
	int m_streamID;
	int m_tvgID;
	int m_prevDays = 0;
	int m_nextDays = 0;

private:
	HACCEL m_hAccel = nullptr;
	BOOL m_allow_save = FALSE;
	int m_current = LB_ERR;
	int m_pl_current = LB_ERR;

	CString m_accessKey;
	CString m_domain;
	CString m_player;
	CString m_iconUrl;

	std::map<int, std::shared_ptr<ChannelCategory>> m_categories;
	std::vector<std::shared_ptr<ChannelInfo>> m_channels;
	std::map<int, std::unique_ptr<PlaylistEntry>> m_playlist;
public:
};
