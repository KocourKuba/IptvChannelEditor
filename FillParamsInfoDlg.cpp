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
#include "IPTVChannelEditor.h"
#include "FillParamsInfoDlg.h"
#include "Constants.h"

// CFillParamsInfo dialog

IMPLEMENT_DYNAMIC(CFillParamsInfoDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CFillParamsInfoDlg, CDialogEx)
	ON_WM_GETMINMAXINFO()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_INFO, &CFillParamsInfoDlg::OnNMDblclkListInfo)
	ON_MESSAGE(WM_NOTIFY_END_EDIT, &CFillParamsInfoDlg::OnNotifyEndEdit)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CFillParamsInfoDlg::OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, &CFillParamsInfoDlg::OnBnClickedButtonRemove)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_INFO, &CFillParamsInfoDlg::OnLvnItemchangedListInfo)
	ON_BN_CLICKED(IDC_BUTTON_COPY, &CFillParamsInfoDlg::OnBnClickedButtonCopy)
END_MESSAGE_MAP()


CFillParamsInfoDlg::CFillParamsInfoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_FILL_PARAMS, pParent)
{
}

void CFillParamsInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LIST_INFO, m_wndListParams);
	DDX_Control(pDX, IDC_BUTTON_ADD, m_wndAdd);
	DDX_Control(pDX, IDC_BUTTON_REMOVE, m_wndRemove);
	DDX_Control(pDX, IDC_BUTTON_COPY, m_wndCopy);
}

// FillParamsInfo message handlers

BOOL CFillParamsInfoDlg::OnInitDialog()
{
	__super::OnInitDialog();

	RestoreWindowPos(GetSafeHwnd(), REG_FILL_INFO_WINDOW_POS);

	m_wndListParams.SetExtendedStyle(m_wndListParams.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	CRect rect;
	m_wndListParams.GetClientRect(&rect);
	int vWidth = rect.Width() - GetSystemMetrics(SM_CXVSCROLL);

	CString csID;
	CString csName;
	switch (m_type)
	{
		case 0:
			csID = L"{SERVER_ID}";
			csName = L"{SERVER}";
			break;
		case 1:
			csID = L"{DEVICE_ID}";
			csName = L"{DEVICE}";
			break;
		case 2:
			csID = L"{QUALITY_ID}";
			csName = L"{QUALITY}";
			break;
		case 3:
			csID = L"{PROFILE_ID}";
			csName = L"{PROFILE}";
			break;
		case 4:
			m_isFirstColEditable = false;
			csID.LoadString(IDS_STRING_CURRENT);
			csName.LoadString(IDS_STRING_NAME);
			break;
		default:
			break;
	}

	m_wndListParams.InsertColumn(0, csID, LVCFMT_LEFT, 80, 0);
	m_wndListParams.InsertColumn(1, csName, LVCFMT_LEFT, vWidth - 80, 0);

	int idx = 0;
	for (const auto& info : m_paramsList)
	{
		m_wndListParams.InsertItem(idx, info.get_id().c_str(), 0);
		m_wndListParams.SetItemText(idx, 1, info.get_name().c_str());
		idx++;
	}

	m_wndAdd.EnableWindow(!m_readonly);
	m_wndRemove.EnableWindow(FALSE);
	m_wndCopy.EnableWindow(FALSE);

	GetDlgItem(IDOK)->EnableWindow(!m_readonly);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFillParamsInfoDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
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

void CFillParamsInfoDlg::OnNMDblclkListInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	//LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	*pResult = 0;

	DWORD pos = GetMessagePos();
	CPoint pt(LOWORD(pos), HIWORD(pos));
	ScreenToClient(&pt);

	CRect rect;
	m_wndListParams.GetWindowRect(&rect);
	ScreenToClient(&rect);

	pt.x -= rect.left;
	pt.y -= rect.top;

	int col = 0;
	m_wndListParams.GetRowFromPoint(pt, &col);
	if (!m_isFirstColEditable && col == 0)
		return;

	m_wndListParams.OnLButtonDown(MK_LBUTTON, pt);
}

LRESULT CFillParamsInfoDlg::OnNotifyEndEdit(WPARAM wParam, LPARAM lParam)
{
	// Get the changed field text via the callback
	NMLVDISPINFO* dispinfo = reinterpret_cast<NMLVDISPINFO*>(lParam);
	if (m_readonly || dispinfo->item.pszText[0] == '\0')
		return 1;

	m_wndListParams.SetItemText(dispinfo->item.iItem, dispinfo->item.iSubItem, dispinfo->item.pszText);

	return 0;
}

void CFillParamsInfoDlg::OnBnClickedButtonAdd()
{
	int cnt = m_wndListParams.GetItemCount();
	m_wndListParams.InsertItem(cnt, std::to_wstring(cnt + 1).c_str(), 0);
	m_wndListParams.SetItemText(cnt, 1, fmt::format(L"param{:d}", cnt + 1).c_str());
}

void CFillParamsInfoDlg::OnBnClickedButtonRemove()
{
	POSITION pos = m_wndListParams.GetFirstSelectedItemPosition();
	if (pos != nullptr)
	{
		m_wndListParams.DeleteItem(m_wndListParams.GetNextSelectedItem(pos));
	}
}

void CFillParamsInfoDlg::OnBnClickedButtonCopy()
{
	POSITION pos = m_wndListParams.GetFirstSelectedItemPosition();
	if (pos != nullptr)
	{
		int selected = m_wndListParams.GetNextSelectedItem(pos);
		const CString& data = m_wndListParams.GetItemText(selected, 1);
		int cnt = m_wndListParams.GetItemCount();
		m_wndListParams.InsertItem(cnt, std::to_wstring(cnt + 1).c_str(), 0);
		m_wndListParams.SetItemText(cnt, 1, data.GetString());
	}
}

void CFillParamsInfoDlg::OnLvnItemchangedListInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if ((pNMLV->uChanged & LVIF_STATE))
	{
		BOOL enable = (!m_readonly && pNMLV->uNewState & LVIS_SELECTED);
		m_wndRemove.EnableWindow(enable);
		m_wndCopy.EnableWindow(enable);
	}

	*pResult = 0;
}

void CFillParamsInfoDlg::OnOK()
{
	int cnt = m_wndListParams.GetItemCount();
	m_paramsList.clear();
	for (int i = 0; i < cnt; i++)
	{
		DynamicParamsInfo info;
		info.set_id(m_wndListParams.GetItemText(i, 0).GetString());
		info.set_name(m_wndListParams.GetItemText(i, 1).GetString());
		m_paramsList.emplace_back(info);
	}

	__super::OnOK();
}

BOOL CFillParamsInfoDlg::DestroyWindow()
{
	SaveWindowPos(GetSafeHwnd(), REG_FILL_INFO_WINDOW_POS);

	return __super::DestroyWindow();
}
