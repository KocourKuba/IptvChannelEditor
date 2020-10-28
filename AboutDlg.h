// AboutDlg.h : header file
//

#pragma once

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	BOOL OnInitDialog() override;
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()

protected:
	CStatic m_QR;
	virtual void OnOK();
};

