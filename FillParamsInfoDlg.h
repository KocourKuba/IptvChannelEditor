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
#include <afxdialogex.h>
#include <variant>
#include "DynamicParamsInfo.h"
#include "PlaylistTemplateInfo.h"
#include "PluginEnums.h"
#include "EditableListCtrl.h"
#include "base_plugin.h"


// CFillParamsInfo dialog
template <class... Fs> struct get_id_overload : Fs... { using Fs::operator()...; };
template <class... Fs> get_id_overload(Fs...) -> get_id_overload<Fs...>;

class CFillParamsInfoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFillParamsInfoDlg)

public:
	CFillParamsInfoDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CFillParamsInfoDlg() = default;

	using variantInfo = std::variant<DynamicParamsInfo, PlaylistTemplateInfo>;
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_FILL_PARAMS };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	BOOL OnInitDialog() override;
	BOOL DestroyWindow() override;
	void OnOK() override;

	DECLARE_MESSAGE_MAP()

	afx_msg void OnNMDblclkListInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnNotifyEndEdit(WPARAM, LPARAM);
	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnBnClickedButtonRemove();
	afx_msg void OnBnClickedButtonCopy();
	afx_msg void OnBnClickedButtonFromPlaylist();
	afx_msg void OnLvnItemchangedListInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);

public:
	DynamicParamsType m_type = DynamicParamsType::enUnknown;
	bool m_readonly = true;
	bool m_fixed = false;
	bool m_isFirstColEditable = true;
	std::vector<variantInfo> m_paramsList;
	std::shared_ptr<base_plugin> m_plugin = nullptr;

protected:
	std::wstring GetParamId(const CFillParamsInfoDlg::variantInfo& info);
	std::wstring GetParamName(const variantInfo& info);

	CEditableListCtrl m_wndListParams;
	CButton m_wndAdd;
	CButton m_wndRemove;
	CButton m_wndCopy;
	CButton m_wndPlaylist;
};