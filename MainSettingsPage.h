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

// CMainSettingsPage dialog

class CMainSettingsPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CMainSettingsPage)

public:
	CMainSettingsPage();   // standard constructor
	virtual ~CMainSettingsPage() = default;

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MAIN_SETTINGS_PAGE };
#endif

protected:
	BOOL OnInitDialog() override;
	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	afx_msg void OnEnChangeEditStreamThreads();
	afx_msg void OnDeltaposSpinStreamThreads(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnChangeEditCacheTTL();
	afx_msg void OnDeltaposSpinCacheTTL(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCbnSelchangeComboLang();
	afx_msg void OnBnClickedButtonReset();
	afx_msg void OnBnClickedButtonClearCache();

protected:
	CComboBox m_wndLanguage;
	CMFCColorButton m_wndAdded;
	CMFCColorButton m_wndNotAdded;
	CMFCColorButton m_wndUnknown;
	CMFCColorButton m_wndChanged;
	CMFCColorButton m_wndHEVC;
	CButton m_wndClearCache;

private:
	BOOL m_bAutoSync = FALSE;
	BOOL m_bAutoHide = FALSE;
	BOOL m_bPortable = FALSE;

	BOOL m_bCmpTitle = TRUE;
	BOOL m_bCmpIcon = TRUE;
	BOOL m_bCmpArchive = TRUE;
	BOOL m_bCmpEpg1 = TRUE;
	BOOL m_bCmpEpg2 = TRUE;
	int m_MaxThreads = 1;
	int m_MaxCacheTTL = 24;
	WORD m_nLang = 0;
};
