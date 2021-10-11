#pragma once
#include "PlayListEntry.h"

class CAccessInfoPinDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAccessInfoPinDlg)

public:
	CAccessInfoPinDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CAccessInfoPinDlg() = default;

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ACCESS_INFO_PIN};
#endif

protected:
	void DoDataExchange(CDataExchange * pDX) override;    // DDX/DDV support
	BOOL OnInitDialog() override;
	void OnOK() override;

	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedBtnGet();

public:
	std::shared_ptr<PlaylistEntry> m_entry;
	BOOL m_bEmbed = FALSE;
	CString m_status;

protected:
	CButton m_wndGet;
	CEdit m_wndPassword;
	CString m_password;
	CString m_subscription;
	CString m_balance;
};

