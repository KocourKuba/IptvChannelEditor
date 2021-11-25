#pragma once
#include "Config.h"


// CSettingsDlg dialog

class CSettingsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSettingsDlg)

public:
	CSettingsDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CSettingsDlg() = default;

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SETTINGS };
#endif

protected:
	BOOL OnInitDialog() override;
	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	afx_msg void OnEnChangeEditStreamThreads();
	afx_msg void OnDeltaposSpinStreamThreads(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCbnSelchangeComboLang();

protected:
	CMFCEditBrowseCtrl m_wndProbe;
	CMFCEditBrowseCtrl m_wndPlayer;
	CMFCEditBrowseCtrl m_wndListsPath;
	CMFCEditBrowseCtrl m_wndPluginsPath;
	CEdit m_wndMaxThreads;
	CSpinButtonCtrl m_wndSpinMaxThreads;
	CComboBox m_wndLanguage;

private:
	CString m_player;
	CString m_probe;
	CString m_lists_path;
	CString m_plugins_path;
	BOOL m_bAutoSync = FALSE;
	BOOL m_bAutoHide = FALSE;
	int m_MaxThreads = 1;
	WORD m_nLang = 0;
};
