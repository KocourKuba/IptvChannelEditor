#pragma once


// CIconLink dialog

class CIconLinkDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CIconLinkDlg)

public:
	CIconLinkDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CIconLinkDlg();

	BOOL OnInitDialog() override;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ICON_LINK };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnBnClickedButtonGet();

public:
	CString m_url;
	CStatic m_wndIcon;
};
