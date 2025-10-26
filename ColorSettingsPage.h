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

const auto DEFAULT_COLOR_ADDED = RGB(0, 128, 0);
const auto DEFAULT_COLOR_NOT_ADDED = RGB(200, 0, 0);
const auto DEFAULT_COLOR_CHANGED = RGB(226, 135, 67);

const auto DEFAULT_COLOR_HEVC = RGB(158, 255, 250);
const auto DEFAULT_COLOR_HD = RGB(255, 255, 157);
const auto DEFAULT_COLOR_FHD = RGB(243, 193, 254);

// CColorSettingsPage dialog

class CColorSettingsPage : public CTooltipPropertyPage
{
	DECLARE_DYNAMIC(CColorSettingsPage)

public:
	CColorSettingsPage();   // standard constructor
	virtual ~CColorSettingsPage() = default;

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_COLOR_SETTINGS_PAGE };
#endif

protected:
	BOOL OnInitDialog() override;
	BOOL OnApply() override;
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedButtonReset();

protected:
	CMFCColorButton m_wndAdded;
	CMFCColorButton m_wndNotAdded;
	CMFCColorButton m_wndUnknown;
	CMFCColorButton m_wndChanged;
	CMFCColorButton m_wndHEVC;
	CMFCColorButton m_wndHD;
	CMFCColorButton m_wndFHD;
	CMFCColorButton m_wndDuplicated;
};
