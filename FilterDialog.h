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
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()

private:
	CString m_filterString;
	BOOL m_filterRegex = FALSE;
	BOOL m_filterCase = FALSE;
public:
	virtual BOOL OnInitDialog();
};
