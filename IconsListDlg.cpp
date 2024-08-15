/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2024): sharky72 (https://github.com/KocourKuba)

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
#include "IconsListDlg.h"
#include "IPTVChannelEditor.h"
#include "PlaylistParseM3U8Thread.h"
#include "IconsSourceParseThread.h"
#include "PlayListEntry.h"
#include "Constants.h"

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

CIconsListDlg::CIconsListDlg(std::shared_ptr<std::vector<CIconSourceData>>& icons,
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
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LIST_ICONS, m_wndIconsList);
	DDX_Control(pDX, IDC_PROGRESS_LOAD, m_wndProgress);
	DDX_Text(pDX, IDC_EDIT_SEARCH, m_search);
	DDX_Control(pDX, IDC_EDIT_SEARCH, m_wndSearch);
	DDX_Control(pDX, IDC_EDIT_ICON_PATH, m_wndIconPath);
}

BOOL CIconsListDlg::OnInitDialog()
{
	__super::OnInitDialog();

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
	m_wndIconsList.SetExtendedStyle(m_wndIconsList.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	int nColumns[] = { IDS_STRING_COL_ICON, IDS_STRING_COL_CHANNEL_NAME };
	int nWidths[] = { 170, 280 };

	// Set up list control
	// Nothing special here.  Just some columns for the report view.
	m_wndIconsList.BuildColumns(_countof(nWidths), nWidths, nColumns);
	m_wndIconsList.AutoSaveColumns(REG_ICON_COLUMNS_WIDTH);

	// https://epg.drm-play.com/?prov=iptvx.one
	// /<img src='(?<link>[^']+)'.+><\/td><td>(?<name>[^<].+)<\/td><td>(?<id>[^<].+)<\/td><td>/gm

	if (m_Icons)
	{
		UpdateListCtrl();
	}
	else
	{
		CWaitCursor cur;
		std::unique_ptr<std::istream> pl_stream;
		std::stringstream data;
		utils::CUrlDownload dl;
		dl.SetUrl(m_iconSource);
		if (dl.DownloadFile(data))
		{
			const auto& str = data.str();
			int lines = (int)std::count(str.begin(), str.end(), '\n');

			CThreadConfig cfg;
			cfg.m_parent = this;
			cfg.m_data = std::move(data);
			cfg.m_hStop = m_evtStop;

			CBaseThread* pThread = nullptr;
			if (m_isHtmlParser)
			{
				cfg.nparam = R"(^<tr><td><img src='(?<link>[^']+)'.+><\/td><td>(?<name>[^<].+)<\/td><td>(?<id>[^<].+)<\/td><td>.*$)";
				pThread = (CBaseThread*)AfxBeginThread(RUNTIME_CLASS(CIconsSourceParseThread), THREAD_PRIORITY_HIGHEST, 0, CREATE_SUSPENDED);
			}
			else
			{
				cfg.m_rootPath = GetAppPath(utils::PLUGIN_ROOT);
				pThread = (CBaseThread*)AfxBeginThread(RUNTIME_CLASS(CPlaylistParseM3U8Thread), THREAD_PRIORITY_HIGHEST, 0, CREATE_SUSPENDED);
				lines /= 2;
			}

			if (pThread)
			{
				m_evtStop.ResetEvent();

				GetDlgItem(IDOK)->EnableWindow(FALSE);
				GetDlgItem(IDC_EDIT_SEARCH)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_BUTTON_SEARCH_NEXT)->ShowWindow(SW_HIDE);
				m_wndProgress.SetRange32(0, lines);
				m_wndProgress.SetPos(0);
				m_wndProgress.ShowWindow(SW_SHOW);
				pThread->SetData(cfg);
				pThread->SetPlugin(m_parent_plugin);
				pThread->ResumeThread();
			}
		}
		else
		{
			AfxMessageBox(dl.GetLastErrorMessage().c_str(), MB_ICONERROR | MB_OK);
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

	if (!m_Icons || nItem >= m_Icons->size())
		return; // Just to be safe

	const auto& entry = m_Icons->at(nItem);

	//Do we need text information?
	if (pItem->mask & LVIF_TEXT)
	{
		CString csText;
		//Which column?
		if (pItem->iSubItem == 1) // Column 1
			csText = m_Icons->at(nItem).logo_name.c_str();

		//Copy the text to the LV_ITEM structure
		//Maximum number of characters is in pItem->cchTextMax
		lstrcpyn(pItem->pszText, csText, pItem->cchTextMax);
	}

	//Does the list need image information?
	if (pItem->mask & LVIF_IMAGE)
	{
		const auto& image = GetIconCache().get_icon(entry.logo_path);

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
			CRect rc_img(rc);
			if ((image.GetWidth() - image.GetHeight()) < 10)
				rc_img.right = 92;

			CDC dcDest;
			dcDest.CreateCompatibleDC(pDesktopDC);

			CBitmap bmpDest;
			bmpDest.CreateCompatibleBitmap(pDesktopDC, rc.Width(), rc.Height());
			CBitmap* pOldDestBitmap = dcDest.SelectObject(&bmpDest);
			dcDest.StretchBlt((rc.Width() - rc_img.Width()) / 2, 0, rc_img.Width(), rc_img.Height(), &dcOrig, 0, 0, image.GetWidth(), image.GetHeight(), SRCCOPY);
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
	m_wndProgress.SetPos((int)lParam);

	return 0;
}

LRESULT CIconsListDlg::OnEndLoadPlaylist(WPARAM wParam, LPARAM lParam /*= 0*/)
{
	GetDlgItem(IDOK)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_SEARCH)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_BUTTON_SEARCH_NEXT)->ShowWindow(SW_SHOW);
	m_wndProgress.ShowWindow(SW_HIDE);

	m_wndIconsList.DeleteAllItems();
	if (m_isHtmlParser)
	{
		m_Icons.reset((std::vector<CIconSourceData>*)wParam);
	}
	else
	{
		std::unique_ptr<Playlist> playlist((Playlist*)wParam);
		if (playlist)
		{
			m_Icons = std::make_shared<std::vector<CIconSourceData>>();

			for (const auto& pl_entry : playlist->m_entries)
			{
				CIconSourceData entry;
				if (m_force_square)
					entry.logo_path = utils::string_replace<wchar_t>(pl_entry->get_icon_absolute_path(), L"/img2/", L"/img/");
				else
					entry.logo_path = pl_entry->get_icon_absolute_path();

				entry.logo_name = pl_entry->get_title();
				entry.logo_id = pl_entry->get_id();
				m_Icons->emplace_back(entry);
			}
		}
	}

	if (m_Icons)
	{
		std::sort(m_Icons->begin(), m_Icons->end(), [](const auto& left, const auto& right)
				  {
					  return left.logo_name < right.logo_name;
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
	if (m_selected != -1 && m_selected < (int)m_Icons->size())
	{
		m_wndIconPath.SetWindowText(m_Icons->at(m_selected).logo_path.c_str());
	}
	else
	{
		m_wndIconPath.SetWindowText(L"");
	}

	*pResult = 0;
}

void CIconsListDlg::OnBnClickedOk()
{
	POSITION pos = m_wndIconsList.GetFirstSelectedItemPosition();
	if (pos == nullptr)
		return;

	m_selected = m_wndIconsList.GetNextSelectedItem(pos);
	m_evtStop.SetEvent();

	__super::OnOK();
}

void CIconsListDlg::OnBnClickedButtonSearchNext()
{
	UpdateData(TRUE);

	if (m_search.IsEmpty())
		return;

	int idx = m_lastFound + 1;
	for (;;)
	{
		if (idx >= (int)m_Icons->size())
			idx = 0;

		if (idx == m_lastFound) break;

		if (StrStrI(m_Icons->at(idx).logo_name.c_str(), m_search.GetString()) != nullptr)
		{
			m_wndIconPath.SetWindowText(m_Icons->at(idx).logo_path.c_str());
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
		m_wndIconsList.SetItemCountEx((int)m_Icons->size(), LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);
		OnBnClickedButtonSearchNext();
	}
}

BOOL CIconsListDlg::DestroyWindow()
{
	SaveWindowPos(GetSafeHwnd(), REG_ICON_WINDOW_POS);

	return __super::DestroyWindow();
}
