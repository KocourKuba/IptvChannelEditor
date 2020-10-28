#pragma once

// CNewChannelDlg dialog

class CNewChannelDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CNewChannelDlg)

public:
	CNewChannelDlg(CWnd* pParent = nullptr);   // standard constructor
	~CNewChannelDlg() = default;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_NEW_CHANNEL };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	BOOL OnInitDialog() override;

	DECLARE_MESSAGE_MAP()

public:
	CString m_name;
};
