
// EdemChannelEditorDlg.h : header file
//

#pragma once
#include "ChannelList.h"


// CEdemChannelEditorDlg dialog
class CEdemChannelEditorDlg : public CDialog
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

	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedButtonAbout();
	afx_msg void OnBnClickedButtonLoadPlaylist();
	afx_msg void OnBnClickedCheckCustomize();
	afx_msg void OnBnClickedButtonTestEpg();
	afx_msg void OnBnClickedButtonTestUrl();
	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnBnClickedButtonRemove();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonAddCategory();
	afx_msg void OnBnClickedButtonRemoveCategory();
	afx_msg void OnStnClickedStaticIcon();
	afx_msg void OnCbnSelchangeComboChannel();
	afx_msg void OnEnChangeMfceditbrowsePluginRoot();
	afx_msg void OnEnChangeMfceditbrowsePlayer();
	afx_msg void OnEnChangeEditNum();
	afx_msg void OnEnChangeEditKey();
	afx_msg void OnEnChangeEditDomain();
	afx_msg void OnDeltaposSpinPrev(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinNext(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChanges();

	DECLARE_MESSAGE_MAP()

private:
	BOOL is_changed() const { return m_changed; }
	void set_changed(BOOL val) { m_changed = val; }

	BOOL is_allow_save() const { return m_allow_save; }
	void set_allow_save(BOOL val) { m_allow_save = val; if (m_wndSave.GetSafeHwnd()) m_wndSave.EnableWindow(val); }

	BOOL LoadSetting();
	void CheckLimits();
	void ReloadChannels();
	void ReloadCategories();
	void LoadImage(CStatic& wnd, const CString& fullPath);

protected:
	CComboBox m_wndChannelList;
	CComboBox m_wndCategoriesList;
	CListBox m_wndShowIn;
	CEdit m_wndStreamUrl;
	CSpinButtonCtrl m_wndSpinPrev;
	CSpinButtonCtrl m_wndSpinNext;
	CButton m_wndCustom;
	CButton m_wndSave;
	CStatic m_wndIcon;
	CFont m_largeFont;

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
	BOOL m_changed = FALSE;
	BOOL m_allow_save = FALSE;
	int m_current = CB_ERR;

	CString m_pluginRoot;
	CString m_player;
	CString m_iconPath;

	ChannelList m_channels;
};
