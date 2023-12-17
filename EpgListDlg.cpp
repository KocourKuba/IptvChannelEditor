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

#include "pch.h"
#include <afxdialogex.h>
#include "EpgListDlg.h"
#include "IPTVChannelEditor.h"
#include "AccountSettings.h"
#include "Constants.h"

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
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_EPG, &CEpgListDlg::OnNMDblclkListEpg)
END_MESSAGE_MAP()

CEpgListDlg::CEpgListDlg(CWnd* pParent /*=nullptr*/) : CDialogEx(IDD_DIALOG_EPG_LIST, pParent)
{
}

void CEpgListDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_EPG_URL, m_csEpgUrl);
	DDX_Text(pDX, IDC_EDIT_ARCHIVE_URL, m_csArchiveUrl);
	DDX_Control(pDX, IDC_LIST_EPG, m_wndEpgList);
	DDX_Control(pDX, IDC_RICHEDIT_EPG, m_wndEpg);
	DDX_Control(pDX, IDC_DATETIMEPICKER, m_day);
}

BOOL CEpgListDlg::OnInitDialog()
{
	__super::OnInitDialog();

	RestoreWindowPos(GetSafeHwnd(), REG_EPG_WINDOW_POS);

	m_wndEpgList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_UNDERLINECOLD | LVS_EX_UNDERLINEHOT);

	int nColumns[] = { IDS_STRING_COL_ARCHIVE, IDS_STRING_COL_START, IDS_STRING_COL_END, IDS_STRING_COL_TITLE };
	int nWidths[] = { 20, 140, 140, 500 };

	// Set up list control
	// Nothing special here.  Just some columns for the report view.
	m_wndEpgList.BuildColumns(_countof(nWidths), nWidths, nColumns);
	m_wndEpgList.AutoSaveColumns(REG_EPG_COLUMNS_WIDTH);

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
	m_idx_map.clear();

	int time_shift = m_info->get_time_shift_hours() * 3600;

	CTime nt(sel_time.GetYear(), sel_time.GetMonth(), sel_time.GetDay(), sel_time.GetHour(), sel_time.GetMinute(), sel_time.GetSecond());
	time_t now = nt.GetTime() + time_shift;

	CTime st(sel_time.GetYear(), sel_time.GetMonth(), sel_time.GetDay(), 0, 0, 0);
	time_t start_time = st.GetTime();

	CTime et(sel_time.GetYear(), sel_time.GetMonth(), sel_time.GetDay(), 23, 59, 59);
	time_t end_time = et.GetTime();

	int i = 0;
	int current_idx = -1;
	if (m_epg_idx == 2)
	{
		m_csEpgUrl = m_xmltv_source.c_str();
	}
	else
	{
		m_csEpgUrl = m_plugin->compile_epg_url(m_epg_idx, m_info->get_epg_id(m_epg_idx), start_time, m_info).c_str();
	}

	auto epg_ids = m_info->get_epg_ids();
	bool need_load = true;
	while (need_load)
	{
		if (m_epg_idx != 2)
		{
			auto epg_id = epg_ids[m_epg_idx];
			epg_ids[0] = L"";
			epg_ids[1] = L"";
			epg_ids[m_epg_idx] = epg_id;
			bool res = m_plugin->parse_json_epg(m_epg_idx, epg_ids, *m_epg_cache, now, m_info);
			if (!res)
			{
				need_load = false;
			}
		}
		else
		{
			auto& epg_cache = m_epg_cache->at(m_epg_idx);
			if (epg_cache.find(L"file already parsed") == epg_cache.end())
			{
				bool res = m_plugin->parse_xml_epg(m_xmltv_source, epg_cache);
				if (res)
				{
					epg_cache[L"file already parsed"] = std::map<time_t, EpgInfo>();
				}
			}
			need_load = false;
		}
	}

	std::wstring found_id;
	for (const auto& epg_id : epg_ids)
	{
		if (!found_id.empty() || epg_id.empty()) continue;

		if (auto& it = m_epg_cache->at(m_epg_idx).find(epg_id); it != m_epg_cache->at(m_epg_idx).end())
		{
			m_pEpgChannelMap = &(m_epg_cache->at(m_epg_idx)[epg_id]);
			for (auto& epg_pair : it->second)
			{
				if (epg_pair.second.time_start <= now && now <= epg_pair.second.time_end)
				{
					found_id = epg_id;
					break;
				}
			}
		}
	}

	if (!found_id.empty())
	{
		for (const auto& epg : m_epg_cache->at(m_epg_idx)[found_id])
		{
			time_t shifted_start = epg.first - time_shift;
			time_t shifted_end = epg.second.time_end - time_shift;

			if (shifted_start < start_time || shifted_start > end_time) continue;

			bool isArchive = (_time32(nullptr) - epg.second.time_end) > 0 && epg.second.time_start > (_time32(nullptr) - m_info->get_archive_days() * 84600);
			COleDateTime start(shifted_start);
			COleDateTime end(shifted_end);
			int idx = m_wndEpgList.InsertItem(i++, isArchive ? L"R" : L"");
			m_wndEpgList.SetItemText(idx, 1, start.Format(_T("%d.%m.%Y %H:%M:%S")));
			m_wndEpgList.SetItemText(idx, 2, end.Format(_T("%d.%m.%Y %H:%M:%S")));
			m_wndEpgList.SetItemText(idx, 3, utils::utf8_to_utf16(epg.second.name).c_str());
			m_idx_map.emplace(idx, std::pair<time_t, time_t>(epg.second.time_start, epg.second.time_end));

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

		auto& epg_pair = m_pEpgChannelMap->find(start_pair->second.first);
		if (epg_pair == m_pEpgChannelMap->end()) break;

		const auto& text = fmt::format(R"({{\rtf1 {:s}}})", epg_pair->second.desc);

		SETTEXTEX set_text_ex = { ST_SELECTION, CP_UTF8 };
		m_wndEpg.SendMessage(EM_SETTEXTEX, (WPARAM)&set_text_ex, (LPARAM)text.c_str());

		m_params.shift_back = (int)start_pair->second.first;
		m_csArchiveUrl = m_plugin->get_play_stream(m_params, m_info).c_str();
		UpdateData(FALSE);
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

void CEpgListDlg::OnNMDblclkListEpg(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	if (const auto& start_pair = m_idx_map.find(pNMItemActivate->iItem); start_pair != m_idx_map.end())
	{
		m_params.shift_back = (int)start_pair->second.first;
		bool isArchive = (_time32(nullptr) - start_pair->second.first) > 0 && start_pair->second.second > (_time32(nullptr) - m_info->get_archive_days() * 84600);
		if (isArchive)
		{
			const auto& url = m_plugin->get_play_stream(m_params, m_info);

			TRACE(L"\nTest URL: %s\n", url.c_str());

			ShellExecuteW(nullptr, L"open", GetConfig().get_string(true, REG_PLAYER).c_str(), url.c_str(), nullptr, SW_SHOWNORMAL);
		}
	}
}
