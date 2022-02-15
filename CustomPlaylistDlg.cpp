// CustomPlaylist.cpp : implementation file
//

#include "pch.h"
#include <afxdialogex.h>
#include "IPTVChannelEditor.h"
#include "CustomPlaylistDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CCustomPlaylist dialog

IMPLEMENT_DYNAMIC(CCustomPlaylistDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CCustomPlaylistDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK_FILE, &CCustomPlaylistDlg::OnBnClickedCheckFile)
END_MESSAGE_MAP()

CCustomPlaylistDlg::CCustomPlaylistDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CUSTOM_PLAYLIST, pParent)
{
}

void CCustomPlaylistDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_CUSTOM_PLAYLIST, m_wndUrl);
	DDX_Text(pDX, IDC_CUSTOM_PLAYLIST, m_url);
	DDX_Check(pDX, IDC_CHECK_FILE, m_isFile);
}

BOOL CCustomPlaylistDlg::OnInitDialog()
{
	__super::OnInitDialog();

	OnBnClickedCheckFile();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CCustomPlaylistDlg::OnBnClickedCheckFile()
{
	UpdateData(TRUE);

	if (m_isFile)
	{
		CString filter;
		filter.LoadString(IDS_STRING_LOAD_PLAYLIST);
		m_wndUrl.EnableFileBrowseButton(_T("*.m3u8"),
										filter,
										OFN_EXPLORER | OFN_ENABLESIZING | OFN_LONGNAMES | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NONETWORKBUTTON);
	}
	else
	{
		m_wndUrl.EnableBrowseButton(FALSE);
	}
}
