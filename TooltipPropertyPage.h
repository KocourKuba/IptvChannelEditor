/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2023): sharky72 (https://github.com/KocourKuba)

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
#include "PluginConfigPropertySheet.h"

class CTooltipPropertyPage : public CMFCPropertyPage
{
	DECLARE_DYNAMIC(CTooltipPropertyPage)

public:
	CTooltipPropertyPage() = default;
	CTooltipPropertyPage(UINT nIDTemplate, UINT nIDCaption = 0) : CMFCPropertyPage(nIDTemplate, nIDCaption) {}
	CTooltipPropertyPage(LPCTSTR lpszTemplateName, UINT nIDCaption = 0) : CMFCPropertyPage(lpszTemplateName, nIDCaption) {}
	virtual ~CTooltipPropertyPage() = default;

public:
	BOOL OnInitDialog() override;
	BOOL PreTranslateMessage(MSG* pMsg) override;
	BOOL OnApply() override;

	virtual void FillControls() {}

	DECLARE_MESSAGE_MAP()

	afx_msg BOOL OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult);

protected:

	void AddTooltip(UINT ctrlID, UINT textID);
	void AllowSave(bool val = true);
	CPluginConfigPropertySheet* GetPropertySheet();

protected:
	CToolTipCtrl m_wndToolTipCtrl;
	std::map<CWnd*, std::wstring> m_tooltips_info;
};