#pragma once
#include "PlayListEntry.h"

// Data object handling class
class CIconsListDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CIconsListDlg)

public:
	CIconsListDlg(std::shared_ptr<std::vector<std::shared_ptr<PlaylistEntry>>>& icons,
				  const std::wstring& iconSource,
				  CWnd* pParent = nullptr);   // standard constructor
	virtual ~CIconsListDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ICONS_LIST };
#endif

protected:
	BOOL OnInitDialog() override;
	void OnOK() override;
	void OnCancel() override;
	BOOL PreTranslateMessage(MSG* pMsg) override;
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	BOOL DestroyWindow() override;


	DECLARE_MESSAGE_MAP()

	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnGetdispinfoListIcons(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclkListIcons(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMClickListIcons(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnEndLoadPlaylist(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg LRESULT OnUpdateProgress(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonSearchNext();

private:
	void UpdateListCtrl();

public:
	int m_selected = 0;
	int m_lastFound = 0;
	CString m_search; // m_wndSearch

protected:
	std::wstring m_iconSource;
	std::shared_ptr<std::vector<std::shared_ptr<PlaylistEntry>>>& m_Icons;

	CProgressCtrl m_wndProgress;
	CListCtrl m_wndIconsList;
	CImageList	m_imageList;
	CEdit m_wndSearch;


	// Event to signal for load playlist thread
	CEvent m_evtStop;
};
