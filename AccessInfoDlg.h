/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2022): sharky72 (https://github.com/KocourKuba)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

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
	CMFCLinkCtrl m_wndProviderLink;
};

