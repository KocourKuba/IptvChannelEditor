#pragma once
#include "ResizedPropertySheet.h"

class CTooltipPropertyPage : public CMFCPropertyPage
{
	DECLARE_DYNAMIC(CTooltipPropertyPage)

public:
	CTooltipPropertyPage() = default;
	CTooltipPropertyPage(UINT nIDTemplate, UINT nIDCaption = 0) : CMFCPropertyPage(nIDTemplate, nIDCaption) {}
	CTooltipPropertyPage(LPCTSTR lpszTemplateName, UINT nIDCaption = 0) : CMFCPropertyPage(lpszTemplateName, nIDCaption) {}
	virtual ~CTooltipPropertyPage() = default;

public:
	BOOL OnInitDialog() override;
	BOOL PreTranslateMessage(MSG* pMsg) override;
	BOOL OnApply() override;

	DECLARE_MESSAGE_MAP()

	afx_msg BOOL OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult);

protected:

	CResizedPropertySheet* GetPropertySheet();
	void AddTooltip(UINT ctrlID, UINT textID);
	void AllowSave(bool val = true);

protected:
	CButton* m_pSaveButton = nullptr;
	CToolTipCtrl m_wndToolTipCtrl;
	std::map<CWnd*, std::wstring> m_tooltips_info;
};