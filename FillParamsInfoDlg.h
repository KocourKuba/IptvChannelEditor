#pragma once
#include "afxdialogex.h"
#include "plugin_config.h"
#include "EditableListCtrl.h"


// CFillParamsInfo dialog

class CFillParamsInfoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFillParamsInfoDlg)

public:
	CFillParamsInfoDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CFillParamsInfoDlg() = default;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_FILL_PARAMS };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	BOOL OnInitDialog() override;
	void OnOK() override;

	DECLARE_MESSAGE_MAP()

	afx_msg void OnNMDblclkListInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnNotifyEndEdit(WPARAM, LPARAM);
	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnBnClickedButtonRemove();
	afx_msg void OnLvnItemchangedListInfo(NMHDR* pNMHDR, LRESULT* pResult);

public:
	int m_type = 0;
	bool m_readonly = true;
	std::vector<DynamicParamsInfo> m_paramsList;

protected:
	CEditableListCtrl m_wndListParams;
	CButton m_wndAdd;
	CButton m_wndRemove;
};
