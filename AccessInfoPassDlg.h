#pragma once
#include "StreamContainer.h"

class CAccessInfoPassDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAccessInfoPassDlg)

public:
	CAccessInfoPassDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CAccessInfoPassDlg() = default;

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ACCESS_INFO_PASS };
#endif

protected:
	void DoDataExchange(CDataExchange * pDX) override;    // DDX/DDV support
	BOOL OnInitDialog() override;
	void OnOK() override;

	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedBtnGet();

public:
	CString m_token;
	CString m_accessKey;
	CString m_domain;
	StreamType m_streamType = StreamType::enEdem;

protected:
	CButton m_wndGet;
	CString m_status;
	CString m_subscription;
	CString m_balance;
	CString m_forecast;
	CString m_login;
	CString m_password;
};

