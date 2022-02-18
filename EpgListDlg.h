#pragma once
#include "PlayListEntry.h"
#include "IPTVChannelEditorDlg.h"

// Data object handling class
class CEpgListDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CEpgListDlg)

public:
	CEpgListDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CEpgListDlg() = default;

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_EPG_LIST };
#endif

protected:
	BOOL OnInitDialog() override;
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	BOOL DestroyWindow() override;


	DECLARE_MESSAGE_MAP()

	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult);

public:
	CString m_epg_url;
	std::map < time_t, EpgInfo> m_epg_map;

protected:
	CListCtrl m_wndEpgList;
	CRichEditCtrl m_wndEpg;

	std::map<int, time_t> m_idx_map;
};
