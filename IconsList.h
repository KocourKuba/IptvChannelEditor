#pragma once


// CIconsList dialog

class CIconsList : public CDialogEx
{
	DECLARE_DYNAMIC(CIconsList)

public:
	CIconsList(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CIconsList() {};

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ICONS_LIST };
#endif

public:
	BOOL OnInitDialog() override;

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	CListCtrl m_wndIconsList;
public:
	afx_msg void OnHdnGetdispinfoListIcons(NMHDR* pNMHDR, LRESULT* pResult);
};
