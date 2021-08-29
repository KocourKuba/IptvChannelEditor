#pragma once
#include "StreamContainer.h"


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
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	BOOL OnInitDialog() override;
	void OnOK() override;

	DECLARE_MESSAGE_MAP()

	afx_msg void OnEnChangeEditPlaylistUrl();
	afx_msg void OnBnClickedBtnGet();
	afx_msg void OnCbnSelchangeComboType();

public:
	CString m_accessKeyGlobal;
	CString m_domainGlobal;
	CString m_accessKeyEmbedded;
	CString m_domainEmbedded;
	CString m_url;
	StreamType m_streamType = StreamType::enEdem;
	int m_type;

protected:
	CButton m_wndGet;
	CMFCEditBrowseCtrl m_wndUrl;
	CString m_accessKey;
	CString m_domain;
};
