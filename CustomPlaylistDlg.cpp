// CustomPlaylist.cpp : implementation file
//

#include "StdAfx.h"
#include "afxdialogex.h"
#include "EdemChannelEditor.h"
#include "CustomPlaylistDlg.h"


// CCustomPlaylist dialog

IMPLEMENT_DYNAMIC(CCustomPlaylistDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CCustomPlaylistDlg, CDialogEx)
END_MESSAGE_MAP()

CCustomPlaylistDlg::CCustomPlaylistDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CUSTOM_PLAYLIST, pParent)
{
}

void CCustomPlaylistDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_CUSTOM_PLAYLIST, m_wndUrl);
	DDX_Text(pDX, IDC_CUSTOM_PLAYLIST, m_url);
}

BOOL CCustomPlaylistDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	if (m_isFile)
	{
		m_wndUrl.EnableFileBrowseButton(_T("*.m3u8"),
										_T("Playlist m3u(*.m3u8, *.m3u)|*.m3u8;*.m3u|All Files (*.*)|*.*||"),
										OFN_EXPLORER | OFN_ENABLESIZING | OFN_LONGNAMES | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NONETWORKBUTTON);
	}
	else
	{
		m_wndUrl.EnableBrowseButton(FALSE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}
