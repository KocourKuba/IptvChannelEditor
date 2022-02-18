// IconsList.cpp : implementation file
//

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
END_MESSAGE_MAP()

CEpgListDlg::CEpgListDlg(CWnd* pParent /*=nullptr*/) : CDialogEx(IDD_DIALOG_EPG_LIST, pParent)
{
}

void CEpgListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_EPG_URL, m_epg_url);
	DDX_Control(pDX, IDC_LIST_EPG, m_wndEpgList);
	DDX_Control(pDX, IDC_RICHEDIT_EPG, m_wndEpg);
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

	time_t now = time(nullptr);

	int i = 0;
	int current_idx = -1;
	for (const auto& epg : m_epg_map)
	{
		COleDateTime start(epg.first);
		COleDateTime end(epg.second.time_end);
		int idx = m_wndEpgList.InsertItem(i++, start.Format(_T("%d.%m.%Y %H:%M:%S")));
		m_wndEpgList.SetItemText(idx, 1, end.Format(_T("%d.%m.%Y %H:%M:%S")));
		m_wndEpgList.SetItemText(idx, 2, utils::utf8_to_utf16(epg.second.name).c_str());
		m_idx_map.emplace(idx, epg.first);

		if (now > epg.first && now < epg.second.time_end)
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

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
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
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if ((pNMListView->uChanged & LVIF_STATE) && (pNMListView->uNewState & LVIS_SELECTED))
	{
		LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

		m_wndEpg.SetWindowText(L"");

		if (auto& start_pair = m_idx_map.find(pNMItemActivate->iItem); start_pair != m_idx_map.end())
		{
			if(auto& epg_pair = m_epg_map.find(start_pair->second); epg_pair != m_epg_map.end())
			{
				CStringA text;
				text.Format(R"({\rtf1 %s})", epg_pair->second.desc.c_str());

				SETTEXTEX set_text_ex = { ST_SELECTION, CP_UTF8 };
				m_wndEpg.SendMessage(EM_SETTEXTEX, (WPARAM)&set_text_ex, (LPARAM)text.GetString());
			}
		}
	}

	*pResult = 0;
}

BOOL CEpgListDlg::DestroyWindow()
{
	SaveWindowPos(GetSafeHwnd(), REG_EPG_WINDOW_POS);

	return __super::DestroyWindow();
}
