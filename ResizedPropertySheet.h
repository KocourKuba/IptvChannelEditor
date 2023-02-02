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

class CResizedPropertySheet : public CMFCPropertySheet
{
	DECLARE_DYNAMIC(CResizedPropertySheet)

public:
	CResizedPropertySheet() = default;
	CResizedPropertySheet(LPCTSTR pszSection,
						  CWnd* pParentWnd = nullptr,
						  UINT iSelectPage = 0)
		: CMFCPropertySheet(_T(""), pParentWnd, iSelectPage)
		, m_posKey(pszSection) {}

	CResizedPropertySheet(LPCTSTR pszCaption,
						  LPCTSTR pszSection,
						  CWnd* pParentWnd = nullptr,
						  UINT iSelectPage = 0)
		: CMFCPropertySheet(pszCaption, pParentWnd, iSelectPage), m_posKey(pszSection) {}

	~CResizedPropertySheet() override = default;

public:
	BOOL OnInitDialog() override;
	INT_PTR DoModal() override;

protected:
	DECLARE_MESSAGE_MAP()

	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();

protected:
	void SetupDynamicLayout();

protected:
	CRect m_min_rc;
	CString m_posKey;
};
