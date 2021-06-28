#pragma once

// CCustomPlaylist dialog

class CCustomPlaylistDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCustomPlaylistDlg)

public:
	CCustomPlaylistDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CCustomPlaylistDlg() = default;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CUSTOM_PLAYLIST };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	BOOL OnInitDialog() override;

	DECLARE_MESSAGE_MAP()

protected:
	CMFCEditBrowseCtrl m_wndUrl;

public:
	CString m_url;
	BOOL m_isFile = FALSE;
};
