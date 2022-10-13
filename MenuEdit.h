#pragma once

class CMenuEdit : public CEdit
{
// Construction
public:
	CMenuEdit() = default;
	virtual ~CMenuEdit() = default;

	void SetTemplateParams(const std::vector<std::wstring>& items) { m_items = items; };
	bool IsTextSelected();
	bool IsReadOnly();

protected:
	DECLARE_MESSAGE_MAP()

	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnProcessMenuItemCommand(UINT nID);

private:
	std::vector<std::wstring> m_items;
};
