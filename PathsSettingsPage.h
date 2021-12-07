#pragma once
#include "CMFCEditBrowsCtrlEx.h"


// CPathsSettingsPage dialog

class CPathsSettingsPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CPathsSettingsPage)

public:
	CPathsSettingsPage();
	virtual ~CPathsSettingsPage() = default;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PATHS_SETTINGS_PAGE };
#endif

protected:
	BOOL OnInitDialog() override;
	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

protected:
	CMFCEditBrowseCtrlEx m_wndProbe;
	CMFCEditBrowseCtrlEx m_wndPlayer;
	CMFCEditBrowseCtrlEx m_wndListsPath;
	CMFCEditBrowseCtrlEx m_wndPluginsPath;

private:
	CString m_player;
	CString m_probe;
	CString m_lists_path;
	CString m_plugins_path;
};