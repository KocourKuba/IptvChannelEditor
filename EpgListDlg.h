/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2026): sharky72 (https://github.com/KocourKuba)

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
#include "uri_stream.h"
#include "ListCtrlEx.h"
#include "base_plugin.h"

// Data object handling class
class CEpgListDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CEpgListDlg)

public:
	CEpgListDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CEpgListDlg() = default;

	CEpgListDlg(const CEpgListDlg&) = delete;
	CEpgListDlg(CEpgListDlg&&) = delete;
	CEpgListDlg& operator=(const CEpgListDlg&) = delete;
	CEpgListDlg& operator=(CEpgListDlg&&) = delete;

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_EPG_LIST };
#endif

protected:
	BOOL OnInitDialog() override;

	void FillList(const COleDateTime& now);

	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	BOOL DestroyWindow() override;


	DECLARE_MESSAGE_MAP()

	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDtnDatetimechangeDatetimepicker(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclkListEpg(NMHDR* pNMHDR, LRESULT* pResult);

public:
	TemplateParams m_params;
	const uri_stream* m_info = nullptr;
	const EpgAliases* m_epg_aliases = nullptr;

	std::wstring m_xmltv_source;

protected:
	CListCtrlEx m_wndEpgList;
	CRichEditCtrl m_wndEpg;
	CDateTimeCtrl m_day;

	CString m_csEpgUrl;
	CString m_csArchiveUrl;

	const std::map<time_t, std::shared_ptr<EpgInfo>>* m_pEpgChannelMap = nullptr;
	std::map<int, std::pair<time_t, time_t>> m_idx_map;
};
