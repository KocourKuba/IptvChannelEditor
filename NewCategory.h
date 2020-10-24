#pragma once

// NewCategory dialog

class NewCategory : public CDialogEx
{
	DECLARE_DYNAMIC(NewCategory)

public:
	NewCategory(CWnd* pParent = nullptr);   // standard constructor
	~NewCategory() = default;

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
	int m_type = 0;
	CStatic m_wndIcon;
	CString m_name;
	CString m_iconUrl;
};
