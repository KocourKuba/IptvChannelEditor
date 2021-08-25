#pragma once


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

	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CString m_player;
	CString m_probe;
	BOOL m_bAutoSync;

protected:
	CMFCEditBrowseCtrl m_wndProbe;
	CMFCEditBrowseCtrl m_wndPlayer;

};
