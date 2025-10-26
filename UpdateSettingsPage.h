/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2025): sharky72 (https://github.com/KocourKuba)

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
#include "TooltipPropertyPage.h"

// CUpdateSettingsPage dialog

class CUpdateSettingsPage : public CTooltipPropertyPage
{
	DECLARE_DYNAMIC(CUpdateSettingsPage)

public:
	CUpdateSettingsPage();   // standard constructor
	virtual ~CUpdateSettingsPage() = default;

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_UPDATE_SETTINGS_PAGE };
#endif

protected:
	BOOL OnInitDialog() override;
	BOOL OnApply() override;
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	afx_msg void OnEnChangeEditUpdateFreq();
	afx_msg void OnDeltaposSpinStreamThreads(NMHDR* pNMHDR, LRESULT* pResult);

private:
	CComboBox m_wndUpdateServer;
	BOOL m_bUpdateChannels = FALSE;
	int m_UpdateFreq = 3;
	int m_UpdateServer = 0;
};
