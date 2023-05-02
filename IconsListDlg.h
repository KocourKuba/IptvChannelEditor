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
#include "IconCache.h"
#include "ListCtrlEx.h"
#include "base_plugin.h"

// Data object handling class
class CIconsListDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CIconsListDlg)

public:
	CIconsListDlg(std::shared_ptr<std::vector<CIconSourceData>>& icons,
				  const std::wstring& iconSource,
				  CWnd* pParent = nullptr);   // standard constructor
	virtual ~CIconsListDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ICONS_LIST };
#endif

protected:
	BOOL OnInitDialog() override;
	void OnOK() override;
	void OnCancel() override;
	BOOL PreTranslateMessage(MSG* pMsg) override;
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	BOOL DestroyWindow() override;


	DECLARE_MESSAGE_MAP()

	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnGetdispinfoListIcons(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclkListIcons(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMClickListIcons(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnEndLoadPlaylist(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg LRESULT OnUpdateProgress(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonSearchNext();

private:
	void UpdateListCtrl();

public:
	int m_selected = 0;
	int m_lastFound = 0;
	bool m_force_square = false;
	bool m_isHtmlParser = false;
	CString m_search; // m_wndSearch
	std::shared_ptr<base_plugin> m_parent_plugin;

protected:
	std::wstring m_iconSource;
	std::shared_ptr<std::vector<CIconSourceData>>& m_Icons;

	CProgressCtrl m_wndProgress;
	CListCtrlEx m_wndIconsList;
	CImageList	m_imageList;
	CEdit m_wndIconPath;
	CEdit m_wndSearch;

	// Event to signal for load playlist thread
	CEvent m_evtStop;
};
