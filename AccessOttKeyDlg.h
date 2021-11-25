#pragma once
#include "Config.h"


// CAccessDlg dialog

class CAccessOttKeyDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAccessOttKeyDlg)

public:
	CAccessOttKeyDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CAccessOttKeyDlg() = default;

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ACCESS_INFO };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	BOOL OnInitDialog() override;

	DECLARE_MESSAGE_MAP()

	afx_msg void OnEnChangeEditPlaylistUrl();
	afx_msg void OnBnClickedBtnGet();

public:
	CString m_accessKey;
	CString m_domain;
	CString m_url;
	CString m_status;
	StreamType m_streamType;
	BOOL m_bEmbed = FALSE;

protected:
	CButton m_wndGet;
	CMFCEditBrowseCtrl m_wndUrl;
};
