#pragma once
#include "EditableListCtrl.h"

class CAccessInfoDlg : public CPropertyPage
{
	DECLARE_DYNAMIC(CAccessInfoDlg)

public:
	CAccessInfoDlg();   // standard constructor
	virtual ~CAccessInfoDlg() = default;

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ACCESS_INFO };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	BOOL OnInitDialog() override;
	void OnOK() override;

	DECLARE_MESSAGE_MAP()

	afx_msg void OnNMDblClickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnItemchangedListAccounts(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedButtonNew();
	afx_msg void OnBnClickedButtonRemove();
	afx_msg void OnBnClickedButtonNewFromUrl();
	afx_msg LRESULT OnNotifyDescriptionEdited(WPARAM, LPARAM);

private:
	int GetChecked();
	void GetAccountInfo();

public:
	BOOL m_bEmbed = FALSE;
	BOOL m_bEmbed_vp = FALSE;
	CString m_status;

	std::wstring m_login;
	std::wstring m_password;
	std::wstring m_token;
	std::wstring m_domain;
	std::wstring m_host;
	std::wstring m_portal;

protected:
	CButton m_wndGet;
	CButton m_wndRemove;
	CButton m_wndNewFromUrl;
	CButton m_wndEmbedPortal;
	CComboBox m_wndDeviceID;
	CEditableListCtrl m_wndAccounts;
	CListCtrl m_wndInfo;
};

