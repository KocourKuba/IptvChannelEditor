#pragma once


// CFilterDialog dialog

class CFilterDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CFilterDialog)

public:
	CFilterDialog(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CFilterDialog() = default;

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_FILTER };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	BOOL OnInitDialog() override;
	void OnOK() override;

	DECLARE_MESSAGE_MAP()

	afx_msg void OnUpdateControls();

public:
	CButton m_wndFilterRegex;
	CButton m_wndFilterCase;

	CString m_filterString;
	BOOL m_filterRegex = FALSE; // m_wndFilterRegex
	BOOL m_filterCase = FALSE; // m_wndFilterCase
};
