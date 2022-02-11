#pragma once
#include "Config.h"

// CMainSettingsPage dialog

class CMainSettingsPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CMainSettingsPage)

public:
	CMainSettingsPage();   // standard constructor
	virtual ~CMainSettingsPage() = default;

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MAIN_SETTINGS_PAGE };
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
	CEdit m_wndMaxThreads;
	CSpinButtonCtrl m_wndSpinMaxThreads;
	CComboBox m_wndLanguage;

private:
	BOOL m_bAutoSync = FALSE;
	BOOL m_bAutoHide = FALSE;
	BOOL m_bPortable = FALSE;

	BOOL m_bCmpTitle = TRUE;
	BOOL m_bCmpIcon = TRUE;
	BOOL m_bCmpArchive = TRUE;
	BOOL m_bCmpEpg1 = TRUE;
	BOOL m_bCmpEpg2 = TRUE;
	int m_MaxThreads = 1;
	WORD m_nLang = 0;
};
