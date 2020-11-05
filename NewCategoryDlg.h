#pragma once
#include "uri.h"

// CNewCategoryDlg dialog

class CNewCategoryDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CNewCategoryDlg)

public:
	CNewCategoryDlg(BOOL bNew = TRUE, CWnd* pParent = nullptr);   // standard constructor
	~CNewCategoryDlg() = default;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_NEW_CATEGORY };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	BOOL OnInitDialog() override;

	afx_msg void OnStnClickedStaticCategoryIcon();
	DECLARE_MESSAGE_MAP()

public:
	BOOL m_bNew;
	CStatic m_wndIcon;
	CString m_name;
	uri m_iconUri;
};
