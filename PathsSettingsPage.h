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
#include "CMFCEditBrowsCtrlEx.h"


// CPathsSettingsPage dialog

class CPathsSettingsPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CPathsSettingsPage)

public:
	CPathsSettingsPage();
	virtual ~CPathsSettingsPage() = default;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PATHS_SETTINGS_PAGE };
#endif

protected:
	BOOL OnInitDialog() override;
	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

protected:
	CMFCEditBrowseCtrlEx m_wndProbe;
	CMFCEditBrowseCtrlEx m_wndPlayer;
	CMFCEditBrowseCtrlEx m_wndListsPath;
	CMFCEditBrowseCtrlEx m_wndPluginsPath;
	CMFCEditBrowseCtrlEx m_wndPluginsWebUpdatePath;

private:
	CString m_player;
	CString m_probe;
	CString m_lists_path;
	CString m_plugins_path;
	CString m_plugins_web_update_path;
};
