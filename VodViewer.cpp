// VodViewer.cpp : implementation file
//

#include "pch.h"
#include <afxdialogex.h>
#include "resource.h"
#include "VodViewer.h"
#include "StreamContainer.h"

#include "UtilsLib\inet_utils.h"
#include "PlayListEntry.h"
#include "PlaylistParseM3U8Thread.h"
#include "IPTVChannelEditor.h"

// CVodViewer dialog

IMPLEMENT_DYNAMIC(CVodViewer, CDialogEx)

BEGIN_MESSAGE_MAP(CVodViewer, CDialogEx)
	ON_WM_GETMINMAXINFO()
	ON_MESSAGE(WM_END_LOAD_PLAYLIST, &CVodViewer::OnEndLoadPlaylist)
	ON_MESSAGE(WM_INIT_PROGRESS, &CVodViewer::OnInitProgress)
	ON_MESSAGE(WM_UPDATE_PROGRESS, &CVodViewer::OnUpdateProgress)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_MOVIES, &CVodViewer::OnItemChanged)
	ON_NOTIFY(LVN_ODCACHEHINT, IDC_LIST_MOVIES, &CVodViewer::OnCacheHint)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_MOVIES, &CVodViewer::OnGetDispinfo)
	ON_CBN_SELCHANGE(IDC_COMBO_CATEGORIES, &CVodViewer::OnCbnSelchangeComboCategories)
	ON_CBN_SELCHANGE(IDC_COMBO_GENRES, &CVodViewer::OnCbnSelchangeComboGenres)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_MOVIES, &CVodViewer::OnNMDblclkListMovies)
END_MESSAGE_MAP()

CVodViewer::CVodViewer(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_VOD, pParent)
	, m_evtStop(FALSE, TRUE)
	, m_evtFinished(TRUE, TRUE)
	, m_category_idx(0)
	, m_description(_T(""))
{

}

CVodViewer::~CVodViewer()
{
}

void CVodViewer::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_BUTTON_FILTER, m_wndFilter);
	DDX_Control(pDX, IDC_COMBO_FILTER_YEAR, m_wndFilterYear);
	DDX_Control(pDX, IDC_COMBO_FILTER_GENRE, m_wndFilterGenre);
	DDX_Control(pDX, IDC_STATIC_PROGRESS_INFO, m_wndProgressInfo);
	DDX_Control(pDX, IDC_LIST_MOVIES, m_wndMoviesList);
	DDX_Control(pDX, IDC_EDIT_SEARCH, m_wndSearchText);
	DDX_Control(pDX, IDC_BUTTON_SEARCH, m_wndSearch);
	DDX_Control(pDX, IDC_EDIT_TOTAL_MOVIES, m_wndTotal);
	DDX_Control(pDX, IDC_STATIC_ICON, m_wndPoster);
	DDX_Control(pDX, IDC_PROGRESS_LOAD, m_wndProgress);
	DDX_Control(pDX, IDC_COMBO_CATEGORIES, m_wndCategories);
	DDX_CBIndex(pDX, IDC_COMBO_CATEGORIES, m_category_idx);
	DDX_Control(pDX, IDC_COMBO_GENRES, m_wndGenres);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, m_description);
}

// CVodViewer message handlers

BOOL CVodViewer::OnInitDialog()
{
	__super::OnInitDialog();

	BOOL enableFilter = FALSE;
	BOOL enableGenre = FALSE;
	switch (m_plugin_type)
	{
		case StreamType::enGlanz:
		case StreamType::enFox:
			// load m3u8 file
			LoadPlaylist();
			break;

		case StreamType::enAntifriz:
		case StreamType::enCbilling:
			enableGenre = TRUE;
			// make http request
			break;
		case StreamType::enSharaclub:
			// make http request
			break;

		case StreamType::enEdem:
			// make http request
			enableFilter = TRUE;
			break;

		default:
			ASSERT(false);
			break;
	}

	m_wndCategories.EnableWindow(FALSE);
	m_wndFilter.EnableWindow(enableFilter);
	m_wndFilterYear.EnableWindow(enableFilter);
	m_wndFilterGenre.EnableWindow(enableFilter);
	m_wndGenres.EnableWindow(enableGenre);

	m_wndMoviesList.SetExtendedStyle(m_wndMoviesList.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	CRect rect;
	m_wndMoviesList.GetClientRect(&rect);
	int vWidth = rect.Width() - GetSystemMetrics(SM_CXVSCROLL) - 1;

	CString str;
	str.LoadString(IDS_STRING_COL_INFO);
	m_wndMoviesList.InsertColumn(0, str, LVCFMT_LEFT, vWidth, 0);

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CVodViewer::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
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

void CVodViewer::OnCancel()
{
	m_evtStop.SetEvent();
	Sleep(1000);
	EndDialog(IDCANCEL);
}

void CVodViewer::LoadPlaylist()
{
	const auto& plugin = StreamContainer::get_instance(m_plugin_type);
	const auto& vod_url = plugin->get_vod_url();
	std::wstring url;
	bool isJson = false;
	switch (m_plugin_type)
	{
		case StreamType::enGlanz:
		case StreamType::enFox:
			url = fmt::format(vod_url, GetConfig().get_string(false, REG_LOGIN), GetConfig().get_string(false, REG_PASSWORD));
			break;

		case StreamType::enAntifriz:
		case StreamType::enCbilling:
		case StreamType::enEdem:
		case StreamType::enSharaclub:
			isJson = true;
			break;
	}

	if (isJson)
	{

	}
	else
	{
		LoadM3U8Playlist(url);
	}
}

void CVodViewer::LoadM3U8Playlist(const std::wstring& url)
{
	m_evtStop.ResetEvent();
	m_evtFinished.ResetEvent();
	auto data = std::make_unique<std::vector<BYTE>>();

	if (!utils::DownloadFile(url, *data) || data->empty())
	{
		AfxMessageBox(IDS_STRING_ERR_THREAD_NOT_START, MB_OK | MB_ICONERROR);
		OnEndLoadPlaylist(0);
		return;
	}

	auto pThread = (CPlaylistParseM3U8Thread*)AfxBeginThread(RUNTIME_CLASS(CPlaylistParseM3U8Thread), THREAD_PRIORITY_HIGHEST, 0, CREATE_SUSPENDED);
	if (!pThread)
	{
		AfxMessageBox(IDS_STRING_ERR_THREAD_NOT_START, MB_OK | MB_ICONERROR);
		OnEndLoadPlaylist(0);
		return;
	}

	ThreadConfig cfg;
	cfg.m_parent = this;
	cfg.m_data = data.release();
	cfg.m_hStop = m_evtStop;
	cfg.m_pluginType = m_plugin_type;

	pThread->SetData(cfg);
	pThread->ResumeThread();
}

LRESULT CVodViewer::OnEndLoadPlaylist(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	m_evtStop.ResetEvent();
	m_evtFinished.SetEvent();

	m_wndProgress.ShowWindow(SW_HIDE);
	m_wndProgressInfo.ShowWindow(SW_HIDE);

	m_playlistEntries.reset((Playlist*)wParam);
	if (m_playlistEntries)
	{
		for (const auto& entry : m_playlistEntries->m_entries)
		{
			std::shared_ptr<vod_category> category;
			if (!m_categories.tryGet(entry->get_category(), category))
			{
				category = std::make_shared<vod_category>(entry->get_category());
				m_categories.set(entry->get_category(), category);
			}

			auto movie = std::make_shared<vod_movie>();
			movie->id = entry->get_epg_id();
			movie->title = entry->get_title();
			movie->poster_url = entry->get_icon_uri();
			movie->url = entry->get_uri_stream()->get_uri();
			m_all_movies.set(movie->id, movie);
			category->movies.set(movie->id, movie);
		}
	}

	FillCategories();

	return 0;
}

LRESULT CVodViewer::OnInitProgress(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	m_wndProgress.SetRange32(0, (int)wParam);
	m_wndProgress.SetPos(0);
	m_wndProgress.ShowWindow(lParam ? SW_SHOW : SW_HIDE);
	m_wndProgressInfo.ShowWindow(lParam ? SW_SHOW : SW_HIDE);

	return 0;
}

LRESULT CVodViewer::OnUpdateProgress(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	CString str;
	str.Format(IDS_STRING_FMT_MOVIES_READED, wParam);
	m_wndProgressInfo.SetWindowText(str);
	m_wndProgress.SetPos(lParam);

	return 0;
}

void CVodViewer::OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	auto* pNMLV = (NMLISTVIEW*)pNMHDR;
	*pResult = 0;
	if (pNMLV->iItem == -1 || pNMLV->uOldState) return;

	return LoadMovieInfo(pNMLV->iItem);
}

void CVodViewer::OnCacheHint(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (!pNMHDR) return;

	auto* pCacheHint = (NMLVCACHEHINT*)pNMHDR;
	*pResult = 0;

	//FillCache(pCacheHint->iFrom, pCacheHint->iTo);
}

void CVodViewer::OnGetDispinfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVDISPINFO* pDispInfo = (NMLVDISPINFO*)pNMHDR;
	LVITEM& lvItem = pDispInfo->item;
	*pResult = 0;

	if (0 == (lvItem.mask & LVIF_TEXT)) return; //valid text buffer?

	UpdateData(TRUE);
	if (m_category_idx == LB_ERR || m_category_idx >= (int)m_categories.size()) return;

	StrCpyN(lvItem.pszText, m_categories.getAt(m_category_idx)->movies.getAt(lvItem.iItem)->title.c_str(), lvItem.cchTextMax - 1);

	//auto* pContext = GetContextRow(lvItem.iItem);

	//FillRow(m_columns, lvItem, pContext);
	//FillIcon(lvItem, pContext);
}

void CVodViewer::FillCategories()
{
	m_wndCategories.ResetContent();
	for (const auto& pair : m_categories.vec())
	{
		m_wndCategories.AddString(pair.first.c_str());
	}

	if (!m_categories.empty())
	{
		m_wndCategories.EnableWindow(TRUE);
		m_category_idx = 0;
		UpdateData(FALSE);
		OnCbnSelchangeComboCategories();
	}
}

void CVodViewer::LoadMovieInfo(int idx)
{
	UpdateData(TRUE);
	if (m_category_idx == LB_ERR || m_category_idx >= (int)m_categories.size()) return;

	const auto& movies = m_categories.getAt(m_category_idx)->movies;
	m_wndTotal.SetWindowText(fmt::format(L"{:d}", movies.size()).c_str());

	const auto& movie = movies.getAt(idx);
	CImage img;
	if (LoadImageFromUrl(movie->poster_url.get_uri(), img))
	{
		SetImageControl(img, m_wndPoster);
	}

	m_description = movie->title.c_str();
	m_description += L"\n";
	m_description += movie->description.c_str();
	m_description += (movie->description.empty() ? L"" : L"\n");
	m_description += movie->year.c_str();
	m_description += L" ";
	m_description += movie->country.c_str();

	UpdateData(FALSE);
}

void CVodViewer::OnCbnSelchangeComboCategories()
{
	UpdateData(TRUE);
	if (m_category_idx == LB_ERR) return;

	const auto& movies = m_categories.getAt(m_category_idx)->movies;
	m_wndMoviesList.SetItemCount(movies.size());
	m_wndMoviesList.Invalidate();
	m_wndTotal.SetWindowText(fmt::format(L"{:d}", m_all_movies.size()).c_str());
	LoadMovieInfo(0);

	// TODO: Add your control notification handler code here
}


void CVodViewer::OnCbnSelchangeComboGenres()
{
	// TODO: Add your control notification handler code here
}


void CVodViewer::OnNMDblclkListMovies(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	UpdateData(TRUE);
	if (m_category_idx == LB_ERR || m_category_idx >= (int)m_categories.size()) return;

	const auto& movies = m_categories.getAt(m_category_idx)->movies;

	if (!pNMItemActivate || pNMItemActivate->iItem >= (int)movies.size()) return;

	const auto& url = movies.getAt(pNMItemActivate->iItem)->url;
	ShellExecuteW(nullptr, L"open", GetConfig().get_string(true, REG_PLAYER).c_str(), url.c_str(), nullptr, SW_SHOWNORMAL);

}
