#pragma once

constexpr auto CATEGORY_LOGO_PATH = "icons\\";

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

private:
	void LoadImage(CStatic& wnd, const CString& fullPath);

public:
	CStatic m_categoryIcon;
	CString m_name;
	CString m_iconUrl;
	CString m_pluginRoot;
};
