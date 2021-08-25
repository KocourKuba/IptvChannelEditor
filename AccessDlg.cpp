// SettingsDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "EdemChannelEditor.h"
#include "AccessDlg.h"
#include "afxdialogex.h"
#include "PlayListEntry.h"

// CAccessDlg dialog

IMPLEMENT_DYNAMIC(CAccessDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CAccessDlg, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT_PLAYLIST_URL, &CAccessDlg::OnEnChangeEditPlaylistUrl)
	ON_BN_CLICKED(IDC_BUTTON_GET, &CAccessDlg::OnBnClickedBtnGet)
END_MESSAGE_MAP()


CAccessDlg::CAccessDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_ACCESS_INFO, pParent)
	, m_bEmbedded(FALSE)
{

}

void CAccessDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_KEY, m_accessKey);
	DDX_Text(pDX, IDC_EDIT_DOMAIN, m_domain);
	DDX_Check(pDX, IDC_CHECK_GLOBAL, m_bEmbedded);
	DDX_Text(pDX, IDC_EDIT_PLAYLIST_URL, m_url);
	DDX_Control(pDX, IDC_EDIT_PLAYLIST_URL, m_wndUrl);
	DDX_Control(pDX, IDC_BUTTON_GET, m_wndGet);
}

void CAccessDlg::OnEnChangeEditPlaylistUrl()
{
	UpdateData(TRUE);

	m_wndGet.EnableWindow(!m_url.IsEmpty());
}


void CAccessDlg::OnBnClickedBtnGet()
{
	UpdateData(TRUE);

	// Short (OTTPplay.es) format
	// #EXTM3U
	// #EXTINF:0 tvg-rec="3",������ HD
	// #EXTGRP:�����
	// http://6646b6bc.akadatel.com/iptv/PWXQ2KD5G2VNSK/204/index.m3u8

	std::vector<BYTE> data;
	std::unique_ptr<std::istream> pl_stream;
	if (utils::CrackUrl(utils::utf16_to_utf8(m_url.GetString())))
	{
		if (utils::DownloadFile(utils::utf16_to_utf8(m_url.GetString()), data))
		{
			utils::vector_to_streambuf<char> buf(data);
			pl_stream = std::make_unique<std::istream>(&buf);
		}
	}
	else
	{
		pl_stream = std::make_unique<std::ifstream>(m_url.GetString());
	}

	if (!pl_stream || !pl_stream->good()) return;

	int step = 0;
	std::string line;
	auto entry = std::make_unique<PlaylistEntry>(m_streamType);
	while (std::getline(*pl_stream, line))
	{
		utils::string_rtrim(line, "\r");
		if (!entry->Parse(line)) continue;

		const auto& access_key = entry->get_access_key();
		const auto& domain = entry->get_domain();
		if (!access_key.empty() && !domain.empty() && access_key != "00000000000000" && domain != "localhost")
		{
			m_accessKey = access_key.c_str();
			m_domain = domain.c_str();
			break;
		}
	}

	UpdateData(FALSE);
}
