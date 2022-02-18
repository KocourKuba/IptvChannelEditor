// IconsList.cpp : implementation file
//

#include "pch.h"
#include <afxdialogex.h>
#include "IconsListDlg.h"
#include "IPTVChannelEditor.h"
#include "PlaylistParseM3U8Thread.h"
#include "IconCache.h"

#include "UtilsLib\inet_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CIconsList dialog

IMPLEMENT_DYNAMIC(CIconsListDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CIconsListDlg, CDialogEx)
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED(IDOK, &CIconsListDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH_NEXT, &CIconsListDlg::OnBnClickedButtonSearchNext)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_ICONS, &CIconsListDlg::OnGetdispinfoListIcons)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_ICONS, &CIconsListDlg::OnNMDblclkListIcons)
	ON_NOTIFY(NM_CLICK, IDC_LIST_ICONS, &CIconsListDlg::OnNMClickListIcons)
	ON_MESSAGE(WM_UPDATE_PROGRESS, &CIconsListDlg::OnUpdateProgress)
	ON_MESSAGE(WM_END_LOAD_PLAYLIST, &CIconsListDlg::OnEndLoadPlaylist)
END_MESSAGE_MAP()

CIconsListDlg::CIconsListDlg(std::shared_ptr<Playlist>& icons,
							 const std::wstring& iconSource,
							 CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_ICONS_LIST, pParent)
	, m_Icons(icons)
	, m_iconSource(iconSource)
	, m_evtStop(FALSE, TRUE)
{
}

CIconsListDlg::~CIconsListDlg()
{
	m_imageList.DeleteImageList();
}

void CIconsListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LIST_ICONS, m_wndIconsList);
	DDX_Control(pDX, IDC_PROGRESS_LOAD, m_wndProgress);
	DDX_Text(pDX, IDC_EDIT_SEARCH, m_search);
	DDX_Control(pDX, IDC_EDIT_SEARCH, m_wndSearch);
}

BOOL CIconsListDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	RestoreWindowPos(GetSafeHwnd(), REG_ICON_WINDOW_POS);

	// This image list is only used to hold the place.  The images are loaded
	// dynamically as the list control is scrolled.
	if (m_imageList.GetSafeHandle() == nullptr)
	{
		m_imageList.Create(162, 92, ILC_COLOR32 | ILC_MASK, 8, 1);
		m_wndIconsList.SetImageList(&m_imageList, LVSIL_SMALL);
		m_wndIconsList.SetImageList(&m_imageList, LVSIL_NORMAL);
		m_imageList.SetImageCount(1);
	}

	// Set up list control
	// Nothing special here.  Just some columns for the report view.
	CString str;
	str.LoadString(IDS_STRING_COL_ICON);
	m_wndIconsList.InsertColumn(0, str, LVCFMT_LEFT, 170);
	str.LoadString(IDS_STRING_COL_CHANNEL_NAME);
	m_wndIconsList.InsertColumn(1, str, LVCFMT_LEFT, 200);
	m_wndIconsList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP /*| LVS_EX_FLATSB*/);

	// Short (OTTPplay.es) format
	// #EXTM3U
	// #EXTINF:0 group-title="םמגמסעט" tvg-id="743" tvg-logo="http://epg.it999.ru/img/743.png" tvg-rec="3",ÐÁÊ-ÒÂ
	// http://localhost/iptv/00000000000000/106/index.m3u8

	if (m_Icons)
	{
		UpdateListCtrl();
	}
	else
	{
		auto data = std::make_unique<std::vector<BYTE>>();
		std::unique_ptr<std::istream> pl_stream;
		if (utils::DownloadFile(m_iconSource, *data))
		{
			auto* pThread = (CPlaylistParseM3U8Thread*)AfxBeginThread(RUNTIME_CLASS(CPlaylistParseM3U8Thread), THREAD_PRIORITY_HIGHEST, 0, CREATE_SUSPENDED);
			if (pThread)
			{
				m_evtStop.ResetEvent();

				GetDlgItem(IDOK)->EnableWindow(FALSE);
				GetDlgItem(IDC_EDIT_SEARCH)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_BUTTON_SEARCH_NEXT)->ShowWindow(SW_HIDE);
				m_wndProgress.SetRange32(0, (int)std::count(data->begin(), data->end(), '\n'));
				m_wndProgress.SetPos(0);
				m_wndProgress.ShowWindow(SW_SHOW);

				ThreadConfig cfg;
				cfg.m_parent = this;
				cfg.m_data = data.release();
				cfg.m_hStop = m_evtStop;
				cfg.m_rootPath = GetAppPath(utils::PLUGIN_ROOT);

				pThread->SetData(cfg);
				pThread->ResumeThread();
			}
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CIconsListDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		HWND hFocus = ::GetFocus();
		if (hFocus == m_wndSearch.GetSafeHwnd() && !m_search.IsEmpty())
		{
			OnBnClickedButtonSearchNext();
			return TRUE;
		}
	}

	return __super::PreTranslateMessage(pMsg);
}

void CIconsListDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
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

void CIconsListDlg::OnOK()
{
	UpdateData(TRUE);

	HWND hFocus = ::GetFocus();
	if (hFocus == m_wndSearch.GetSafeHwnd() && !m_search.IsEmpty())
	{
		OnBnClickedButtonSearchNext();
	}
}

void CIconsListDlg::OnCancel()
{
	m_evtStop.SetEvent();

	EndDialog(IDCANCEL);
}

void CIconsListDlg::OnGetdispinfoListIcons(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVDISPINFO* pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

	//Create a pointer to the item
	LV_ITEM* pItem = &(pDispInfo)->item;

	//Which item number?
	size_t nItem = pItem->iItem;

	if (!m_Icons || nItem >= m_Icons->m_entries.size())
		return; // Just to be safe

	const auto& entry = m_Icons->m_entries[nItem];

	//Do we need text information?
	if (pItem->mask & LVIF_TEXT)
	{
		CString csText;
		//Which column?
		if (pItem->iSubItem == 1) // Column 1
			csText = m_Icons->m_entries[nItem]->get_title().c_str();

		//Copy the text to the LV_ITEM structure
		//Maximum number of characters is in pItem->cchTextMax
		lstrcpyn(pItem->pszText, csText, pItem->cchTextMax);
	}

	//Does the list need image information?
	if (pItem->mask & LVIF_IMAGE)
	{
		const auto& image = GetIconCache().get_icon(entry->get_icon_absolute_path());

		if (image)
		{
			CWnd* pDesktopWindow = CWnd::GetDesktopWindow();
			CDC* pDesktopDC = pDesktopWindow->GetDC();

			CBitmap* bmpOrig = CBitmap::FromHandle(image);

			CDC dcOrig;
			dcOrig.CreateCompatibleDC(pDesktopDC);
			dcOrig.SelectObject(bmpOrig);

			// 2/3 of original size
			CRect rc(0, 0, 162, 92);
			CDC dcDest;
			dcDest.CreateCompatibleDC(pDesktopDC);

			CBitmap bmpDest;
			bmpDest.CreateCompatibleBitmap(pDesktopDC, rc.Width(), rc.Height());
			CBitmap* pOldDestBitmap = dcDest.SelectObject(&bmpDest);
			dcDest.StretchBlt(0, 0, rc.Width(), rc.Height(), &dcOrig, 0, 0, image.GetWidth(), image.GetHeight(), SRCCOPY);
			dcDest.SelectObject(pOldDestBitmap);

			pDesktopWindow->ReleaseDC(pDesktopDC);

			m_imageList.Replace(0, &bmpDest, nullptr);
		}

		pItem->iImage = 0; // Always use 0 since only one element
	}

	*pResult = 0;
}

LRESULT CIconsListDlg::OnUpdateProgress(WPARAM wParam, LPARAM lParam /*= 0*/)
{
	m_wndProgress.SetPos(lParam);

	return 0;
}

LRESULT CIconsListDlg::OnEndLoadPlaylist(WPARAM wParam, LPARAM lParam /*= 0*/)
{
	GetDlgItem(IDOK)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_SEARCH)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_BUTTON_SEARCH_NEXT)->ShowWindow(SW_SHOW);
	m_wndProgress.ShowWindow(SW_HIDE);

	m_wndIconsList.DeleteAllItems();
	m_Icons.reset((Playlist*)wParam);
	if (m_Icons)
	{
		std::sort(m_Icons->m_entries.begin(), m_Icons->m_entries.end(), [](const auto& left, const auto& right)
				  {
					  return left->get_title() < right->get_title();
				  });


		UpdateListCtrl();
	}

	return 0;
}

void CIconsListDlg::OnNMDblclkListIcons(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	m_selected = pNMItemActivate->iItem;

	m_evtStop.SetEvent();

	EndDialog(IDOK);

	*pResult = 0;
}


void CIconsListDlg::OnNMClickListIcons(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	m_selected = pNMItemActivate->iItem;

	*pResult = 0;
}

void CIconsListDlg::OnBnClickedOk()
{
	POSITION pos = m_wndIconsList.GetFirstSelectedItemPosition();
	if (pos == nullptr)
		return;

	m_selected = m_wndIconsList.GetNextSelectedItem(pos);
	m_evtStop.SetEvent();

	CDialogEx::OnOK();
}

void CIconsListDlg::OnBnClickedButtonSearchNext()
{
	UpdateData(TRUE);

	if (m_search.IsEmpty())
		return;

	size_t idx = m_lastFound + 1;
	for (;;)
	{
		if (idx >= m_Icons->m_entries.size())
			idx = 0;

		if (idx == m_lastFound) break;

		if (StrStrI(m_Icons->m_entries[idx]->get_title().c_str(), m_search.GetString()) != nullptr)
		{
			m_wndIconsList.SetItemState(idx, LVIS_SELECTED, LVIS_SELECTED);
			m_wndIconsList.EnsureVisible(idx, TRUE);
			m_lastFound = idx;
			m_selected = idx;
			break;
		}

		idx++;
	}
}

void CIconsListDlg::UpdateListCtrl()
{
	if (m_Icons)
	{
		m_wndIconsList.SetItemCountEx((int)m_Icons->m_entries.size(), LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);
		OnBnClickedButtonSearchNext();
	}
}

BOOL CIconsListDlg::DestroyWindow()
{
	SaveWindowPos(GetSafeHwnd(), REG_ICON_WINDOW_POS);

	return __super::DestroyWindow();
}
