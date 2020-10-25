
// EdemChannelEditorDlg.h : header file
//

#pragma once
#include "ChannelList.h"
#include "PlayListEntry.h"
#include "ColorListBox.h"

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

	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedButtonAbout();
	afx_msg void OnBnClickedButtonAddCategory();
	afx_msg void OnBnClickedButtonAddChannel();
	afx_msg void OnBnClickedButtonAddToShowIn();
	afx_msg void OnBnClickedButtonEditCategory();
	afx_msg void OnBnClickedButtonImport();
	afx_msg void OnBnClickedButtonLoadPlaylist();
	afx_msg void OnBnClickedButtonPack();
	afx_msg void OnBnClickedButtonRemoveCategory();
	afx_msg void OnBnClickedButtonRemoveChannel();
	afx_msg void OnBnClickedButtonRemoveFromShowIn();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonTestEpg();
	afx_msg void OnBnClickedButtonTestUrl();
	afx_msg void OnBnClickedCheckCustomize();
	afx_msg void OnChanges();
	afx_msg void OnDeltaposSpinNext(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinPrev(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnChangeEditNum();
	afx_msg void OnEnChangeEditPlSearch();
	afx_msg void OnEnChangeEditSearch();
	afx_msg void OnEnChangeMfceditbrowsePlayer();
	afx_msg void OnEnKillfocusEditChannelName();
	afx_msg void OnEnKillfocusEditDomain();
	afx_msg void OnEnKillfocusEditKey();
	afx_msg void OnEnKillfocusEditStreamUrl();
	afx_msg void OnEnKillfocusEditTvgId();
	afx_msg void OnEnKillfocusEditUrlId();
	afx_msg void OnLbnDblclkListPlaylist();
	afx_msg void OnLbnSelchangeListChannels();
	afx_msg void OnLbnSelchangeListPlaylist();
	afx_msg void OnLbnSelchangeListCategories();
	afx_msg void OnPaint();
	afx_msg void OnStnClickedStaticIcon();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

private:
	BOOL is_allow_save() const { return m_allow_save; }
	void set_allow_save(BOOL val = TRUE);

	void LoadChannelInfo();
	BOOL LoadSetting();
	void LoadPlaylist(const CString& file);
	void CheckLimits();
	void LoadChannels();
	void FillCategories();
	void SaveChannelInfo();
	void UpdateControls();
	ChannelInfo* GetChannel(int idx);
	ChannelCategory* GetCategory(int idx);
	PlaylistEntry* GetPlaylistEntry(int idx);

protected:
	CListBox m_wndChannelsList;
	CComboBox m_wndCategories;
	CListBox m_wndCategoriesList;
	CColorListBox m_wndPlaylist;
	CEdit m_wndStreamID;
	CEdit m_wndStreamUrl;
	CSpinButtonCtrl m_wndSpinPrev;
	CSpinButtonCtrl m_wndSpinNext;
	CButton m_wndCustom;
	CButton m_wndSave;
	CButton m_wndPack;
	CStatic m_wndIcon;
	CFont m_largeFont;

	CString m_search;
	CString m_plSearch;
	CString m_channelName;
	CString m_streamUrl;

	CString m_accessKey;
	CString m_domain;

	BOOL m_hasArchive = FALSE;
	BOOL m_isAdult = FALSE;
	int m_streamID;
	CString m_TVGID;
	int m_prevDays = 0;
	int m_nextDays = 0;

private:
	BOOL m_allow_save = FALSE;
	int m_current = CB_ERR;
	int m_pl_current = CB_ERR;

	CString m_player;
	CString m_iconUrl;

	ChannelList m_channels;
	std::map<int, std::unique_ptr<PlaylistEntry>> m_playlist;
	std::map<int, std::wstring> m_allChannels;
};
