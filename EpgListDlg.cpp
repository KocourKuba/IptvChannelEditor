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

#include "pch.h"
#include <afxdialogex.h>
#include "EpgListDlg.h"
#include "IPTVChannelEditor.h"

#include "UtilsLib\inet_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CIconsList dialog

IMPLEMENT_DYNAMIC(CEpgListDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CEpgListDlg, CDialogEx)
	ON_WM_GETMINMAXINFO()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_EPG, &CEpgListDlg::OnItemchangedList)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER, &CEpgListDlg::OnDtnDatetimechangeDatetimepicker)
END_MESSAGE_MAP()

CEpgListDlg::CEpgListDlg(CWnd* pParent /*=nullptr*/) : CDialogEx(IDD_DIALOG_EPG_LIST, pParent)
{
}

void CEpgListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_EPG_URL, m_csEpgUrl);
	DDX_Control(pDX, IDC_LIST_EPG, m_wndEpgList);
	DDX_Control(pDX, IDC_RICHEDIT_EPG, m_wndEpg);
	DDX_Control(pDX, IDC_DATETIMEPICKER, m_day);
}

BOOL CEpgListDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	RestoreWindowPos(GetSafeHwnd(), REG_EPG_WINDOW_POS);

	m_wndEpgList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_UNDERLINECOLD | LVS_EX_UNDERLINEHOT);

	// Set up list control
	// Nothing special here.  Just some columns for the report view.
	CString str;
	str.LoadString(IDS_STRING_COL_START);
	m_wndEpgList.InsertColumn(0, str, LVCFMT_LEFT, 120);
	str.LoadString(IDS_STRING_COL_END);
	m_wndEpgList.InsertColumn(1, str, LVCFMT_LEFT, 120);
	str.LoadString(IDS_STRING_COL_TITLE);
	m_wndEpgList.InsertColumn(2, str, LVCFMT_LEFT, 500);

	m_day.SetTime(COleDateTime::GetCurrentTime());

	FillList(COleDateTime::GetCurrentTime());

	m_wndEpgList.SetFocus();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CEpgListDlg::FillList(const COleDateTime& sel_time)
{
	if (!m_epg_cache || !m_info)
		return;

	m_wndEpgList.DeleteAllItems();

	CTime nt(sel_time.GetYear(), sel_time.GetMonth(), sel_time.GetDay(), sel_time.GetHour(), sel_time.GetMinute(), sel_time.GetSecond());
	time_t now = nt.GetTime();

	CTime st(sel_time.GetYear(), sel_time.GetMonth(), sel_time.GetDay(), 0, 0, 0);
	time_t start_time = st.GetTime();

	CTime et(sel_time.GetYear(), sel_time.GetMonth(), sel_time.GetDay(), 23, 59, 59);
	time_t end_time = et.GetTime();

	int i = 0;
	int current_idx = -1;
	const auto& epg_id = m_info->get_epg_id(m_epg_idx);
	m_pEpgChannelMap = &((*m_epg_cache)[m_epg_idx][epg_id]);
	m_csEpgUrl = m_info->stream_uri->compile_epg_url(m_epg_idx, epg_id, start_time).c_str();

	bool need_load = true;
	while (need_load)
	{
		for (auto& epg_pair : *m_pEpgChannelMap)
		{
			if (epg_pair.second.time_start <= now && now <= epg_pair.second.time_end)
			{
				need_load = false;
				break;
			}
		}

		if (need_load && !m_info->stream_uri->parse_epg(m_epg_idx, epg_id, *m_pEpgChannelMap, now))
		{
			need_load = false;
		}
	}

	for (const auto& epg : *m_pEpgChannelMap)
	{
		if (epg.second.time_start < start_time || epg.second.time_start > end_time) continue;

		COleDateTime start(epg.first);
		COleDateTime end(epg.second.time_end);
		int idx = m_wndEpgList.InsertItem(i++, start.Format(_T("%d.%m.%Y %H:%M:%S")));
		m_wndEpgList.SetItemText(idx, 1, end.Format(_T("%d.%m.%Y %H:%M:%S")));
		m_wndEpgList.SetItemText(idx, 2, utils::utf8_to_utf16(epg.second.name).c_str());
		m_idx_map.emplace(idx, epg.first);

		if (now >= epg.first && now <= epg.second.time_end)
		{
			current_idx = idx;
		}
	}

	if (current_idx != -1)
	{
		m_wndEpgList.SetItemState(-1, 0, LVIS_SELECTED);
		m_wndEpgList.SetItemState(current_idx, LVIS_SELECTED, LVIS_SELECTED);
		m_wndEpgList.EnsureVisible(m_wndEpgList.GetItemCount() - 1, FALSE); // Scroll down to the bottom
		m_wndEpgList.EnsureVisible(current_idx, TRUE);
	}

	UpdateData(FALSE);
}

void CEpgListDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	CMFCDynamicLayout* layout = GetDynamicLayout();

	if (layout)
	{
		CSize size = layout->GetMinSize();
		CRect rect(0, 0, size.cx, size.cy);
		AdjustWindowRect(&rect, GetStyle(), FALSE);
		lpMMI->ptMinTrackSize.x = rect.Width();
		lpMMI->ptMinTrackSize.y = rect.Height();
	}

	__super::OnGetMinMaxInfo(lpMMI);
}

void CEpgListDlg::OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult)
{
	do
	{
		if (!m_pEpgChannelMap) break;

		NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
		if (!(pNMListView->uChanged & LVIF_STATE) || !(pNMListView->uNewState & LVIS_SELECTED)) break;

		LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

		m_wndEpg.SetWindowText(L"");

		auto& start_pair = m_idx_map.find(pNMItemActivate->iItem);
		if (start_pair == m_idx_map.end()) break;

		auto& epg_pair = m_pEpgChannelMap->find(start_pair->second);
		if (epg_pair == m_pEpgChannelMap->end()) break;

		CStringA text;
		text.Format(R"({\rtf1 %s})", epg_pair->second.desc.c_str());

		SETTEXTEX set_text_ex = { ST_SELECTION, CP_UTF8 };
		m_wndEpg.SendMessage(EM_SETTEXTEX, (WPARAM)&set_text_ex, (LPARAM)text.GetString());
	} while (false);

	*pResult = 0;
}

BOOL CEpgListDlg::DestroyWindow()
{
	SaveWindowPos(GetSafeHwnd(), REG_EPG_WINDOW_POS);

	return __super::DestroyWindow();
}


void CEpgListDlg::OnDtnDatetimechangeDatetimepicker(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	if (pDTChange && pDTChange->dwFlags == 0)
	{
		FillList(pDTChange->st);
	}

	*pResult = 0;
}
