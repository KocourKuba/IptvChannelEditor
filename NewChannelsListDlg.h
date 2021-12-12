#pragma once

// CNewChannelsListDlg dialog

class CNewChannelsListDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CNewChannelsListDlg)

public:
	CNewChannelsListDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CNewChannelsListDlg() = default;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_NEW_CHANNELS_LIST };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	void OnOK() override;

	DECLARE_MESSAGE_MAP()

public:
	CString m_name;
};
