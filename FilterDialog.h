#pragma once
#include "HistoryCombo.h"


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
	std::array<CButton, 2> m_wndFilter;
	std::array<CButton, 2> m_wndFilterRegex;
	std::array<CButton, 2> m_wndFilterCase;
	std::array<CHistoryCombo, 2> m_wndFilterString;

	std::array<BOOL, 2> m_filterState; // m_wndFilter
	std::array<BOOL, 2> m_filterRegex; // m_wndFilterRegex
	std::array<BOOL, 2> m_filterCase; // m_wndFilterCase
};
