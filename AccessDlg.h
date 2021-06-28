#pragma once


// CAccessDlg dialog

class CAccessDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAccessDlg)

public:
	CAccessDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CAccessDlg() = default;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ACCESS_INFO };
#endif

protected:
	BOOL OnInitDialog() override;
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedCheckGlobal();
	afx_msg void OnEnChangeEditPlaylistUrl();
	afx_msg void OnBnClickedBtnGet();


public:
	CButton m_wndGet;
	CMFCEditBrowseCtrl m_wndUrl;
	CString m_accessKey;
	CString m_domain;
	BOOL m_bEmbedded;
	CString m_url;
};
