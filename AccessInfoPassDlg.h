#pragma once
#include "StreamContainer.h"
#include "PlayListEntry.h"

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
	std::shared_ptr<PlaylistEntry> m_entry;

protected:
	CButton m_wndGet;
	CString m_status;
	CString m_subscription;
	CString m_balance;
	CString m_packages_price;
	CString m_login;
	CString m_password;
};

