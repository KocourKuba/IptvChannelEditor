#pragma once
#include "CMFCEditBrowsCtrlEx.h"


// CUrlDlg dialog

class CUrlDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CUrlDlg)

public:
	CUrlDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CUrlDlg() = default;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_URL };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CString m_url;

protected:
	CMFCEditBrowseCtrlEx m_wndUrl;
};
