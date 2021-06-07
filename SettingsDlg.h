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
	void OnOK() override;

	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

protected:
	CMFCEditBrowseCtrl m_wndProbe;
	CMFCEditBrowseCtrl m_wndPlayer;

	CString m_playlistURL;
	CString m_player;
	CString m_probe;
};
