#pragma once


// CSettingsPage dialog

class CSettingsPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CSettingsPage)

public:
	CSettingsPage();
	virtual ~CSettingsPage();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CSettingsPage };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
