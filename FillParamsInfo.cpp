// FillParamsInfo.cpp : implementation file
//

#include "pch.h"
#include "IPTVChannelEditor.h"
#include "FillParamsInfo.h"

// CFillParamsInfo dialog

IMPLEMENT_DYNAMIC(CFillParamsInfo, CDialogEx)

BEGIN_MESSAGE_MAP(CFillParamsInfo, CDialogEx)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_INFO, &CFillParamsInfo::OnNMDblclkListInfo)
	ON_MESSAGE(WM_NOTIFY_END_EDIT, &CFillParamsInfo::OnNotifyEndEdit)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CFillParamsInfo::OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, &CFillParamsInfo::OnBnClickedButtonRemove)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_INFO, &CFillParamsInfo::OnLvnItemchangedListInfo)
END_MESSAGE_MAP()


CFillParamsInfo::CFillParamsInfo(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_FILL_PARAMS, pParent)
{
}

void CFillParamsInfo::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LIST_INFO, m_wndListParams);
	DDX_Control(pDX, IDC_BUTTON_ADD, m_wndAdd);
	DDX_Control(pDX, IDC_BUTTON_REMOVE, m_wndRemove);
}

// FillParamsInfo message handlers

BOOL CFillParamsInfo::OnInitDialog()
{
	__super::OnInitDialog();

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

	GetDlgItem(IDOK)->EnableWindow(!m_readonly);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFillParamsInfo::OnNMDblclkListInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	//LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	DWORD pos = GetMessagePos();
	CPoint pt(LOWORD(pos), HIWORD(pos));
	ScreenToClient(&pt);

	CRect rect;
	m_wndListParams.GetWindowRect(&rect);
	ScreenToClient(&rect);

	pt.x -= rect.left;
	pt.y -= rect.top;

	m_wndListParams.OnLButtonDown(MK_LBUTTON, pt);

	*pResult = 0;
}

LRESULT CFillParamsInfo::OnNotifyEndEdit(WPARAM wParam, LPARAM lParam)
{
	// Get the changed field text via the callback
	NMLVDISPINFO* dispinfo = reinterpret_cast<NMLVDISPINFO*>(lParam);
	if (m_readonly || dispinfo->item.pszText[0] == '\0')
		return 1;

	m_wndListParams.SetItemText(dispinfo->item.iItem, dispinfo->item.iSubItem, dispinfo->item.pszText);

	return 0;
}

void CFillParamsInfo::OnBnClickedButtonAdd()
{
	int cnt = m_wndListParams.GetItemCount();
	m_wndListParams.InsertItem(cnt, std::to_wstring(cnt).c_str(), 0);
	m_wndListParams.SetItemText(cnt, 1, fmt::format(L"param{:d}", cnt).c_str());
}

void CFillParamsInfo::OnBnClickedButtonRemove()
{
	POSITION pos = m_wndListParams.GetFirstSelectedItemPosition();
	if (pos != nullptr)
	{
		m_wndListParams.DeleteItem(m_wndListParams.GetNextSelectedItem(pos));
	}
}

void CFillParamsInfo::OnLvnItemchangedListInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if ((pNMLV->uChanged & LVIF_STATE))
	{
		m_wndRemove.EnableWindow(!m_readonly && pNMLV->uNewState & LVIS_SELECTED);
	}

	*pResult = 0;
}

void CFillParamsInfo::OnOK()
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
