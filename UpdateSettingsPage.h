#pragma once
#include "Config.h"

// CUpdateSettingsPage dialog

class CUpdateSettingsPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CUpdateSettingsPage)

public:
	CUpdateSettingsPage();   // standard constructor
	virtual ~CUpdateSettingsPage() = default;

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_UPDATE_SETTINGS_PAGE };
#endif

protected:
	BOOL OnInitDialog() override;
	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	afx_msg void OnEnChangeEditUpdateFreq();
	afx_msg void OnDeltaposSpinStreamThreads(NMHDR* pNMHDR, LRESULT* pResult);

private:
	BOOL m_bUpdateChannels = FALSE;
	int m_UpdateFreq = 3;
};
