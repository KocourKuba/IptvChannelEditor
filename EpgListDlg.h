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

	void FillList(const COleDateTime& now);

	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	BOOL DestroyWindow() override;


	DECLARE_MESSAGE_MAP()

	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDtnDatetimechangeDatetimepicker(NMHDR* pNMHDR, LRESULT* pResult);

public:
	int m_epg_idx = 0;
	BaseInfo* m_info = nullptr;
	std::array<std::map<std::wstring, std::map<time_t, EpgInfo>>, 2>* m_epg_cache = nullptr;
	std::map<std::wstring, std::wstring>* m_epg_mapper = nullptr;

protected:
	CListCtrl m_wndEpgList;
	CRichEditCtrl m_wndEpg;
	CDateTimeCtrl m_day;

	CString m_csEpgUrl;

	std::map<time_t, EpgInfo>* m_pEpgChannelMap = nullptr;
	std::map<int, time_t> m_idx_map;
};
