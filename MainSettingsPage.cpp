/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2022): sharky72 (https://github.com/KocourKuba)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "pch.h"
#include <afxdialogex.h>
#include "IPTVChannelEditor.h"
#include "MainSettingsPage.h"
#include "AccountSettings.h"
#include "Constants.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CMainSettingsPage dialog

IMPLEMENT_DYNAMIC(CMainSettingsPage, CPropertyPage)

BEGIN_MESSAGE_MAP(CMainSettingsPage, CPropertyPage)
	ON_EN_CHANGE(IDC_EDIT_STREAM_THREADS, &CMainSettingsPage::OnEnChangeEditStreamThreads)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_STREAM_THREADS, &CMainSettingsPage::OnDeltaposSpinStreamThreads)
	ON_CBN_SELCHANGE(IDC_COMBO_LANG, &CMainSettingsPage::OnCbnSelchangeComboLang)
	ON_BN_CLICKED(IDC_BUTTON_RESET, &CMainSettingsPage::OnBnClickedButtonReset)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_CACHE, &CMainSettingsPage::OnBnClickedButtonClearCache)
END_MESSAGE_MAP()


CMainSettingsPage::CMainSettingsPage() : CPropertyPage(IDD_MAIN_SETTINGS_PAGE)
{
}

void CMainSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_COMBO_LANG, m_wndLanguage);
	DDX_Text(pDX, IDC_EDIT_STREAM_THREADS, m_MaxThreads);
	DDX_Control(pDX, IDC_EDIT_STREAM_THREADS, m_wndMaxThreads);
	DDX_Control(pDX, IDC_SPIN_STREAM_THREADS, m_wndSpinMaxThreads);
	DDX_Check(pDX, IDC_CHECK_AUTO_SYNC_CHANNELS, m_bAutoSync);
	DDX_Check(pDX, IDC_CHECK_AUTO_HIDE, m_bAutoHide);
	DDX_Check(pDX, IDC_CHECK_PORTABLE, m_bPortable);

	DDX_Check(pDX, IDC_CHECK_CMP_TITLE, m_bCmpTitle);
	DDX_Check(pDX, IDC_CHECK_CMP_ICON, m_bCmpIcon);
	DDX_Check(pDX, IDC_CHECK_CMP_ARCHIVE, m_bCmpArchive);
	DDX_Check(pDX, IDC_CHECK_CMP_EPG1, m_bCmpEpg1);
	DDX_Check(pDX, IDC_CHECK_CMP_EPG2, m_bCmpEpg2);
	DDX_Control(pDX, IDC_BUTTON_ADDED, m_wndAdded);
	DDX_Control(pDX, IDC_BUTTON_NOT_ADDED, m_wndNotAdded);
	DDX_Control(pDX, IDC_BUTTON_CHANGED, m_wndChanged);
	DDX_Control(pDX, IDC_BUTTON_HEVC, m_wndHEVC);
	DDX_Control(pDX, IDC_BUTTON_CLEAR_CACHE, m_wndClearCache);
}

BOOL CMainSettingsPage::OnInitDialog()
{
	__super::OnInitDialog();

	m_bAutoSync = GetConfig().get_int(true, REG_AUTO_SYNC);
	m_bAutoHide = GetConfig().get_int(true, REG_AUTO_HIDE);
	m_bPortable = GetConfig().IsPortable();
	m_MaxThreads = GetConfig().get_int(true, REG_MAX_THREADS, 3);
	m_nLang = GetConfig().get_int(true, REG_LANGUAGE);
	m_wndAdded.SetColor(GetConfig().get_int(true, REG_COLOR_ADDED, RGB(0, 200, 0)));
	m_wndNotAdded.SetColor(GetConfig().get_int(true, REG_COLOR_NOT_ADDED, RGB(200, 0, 0)));
	m_wndChanged.SetColor(GetConfig().get_int(true, REG_COLOR_CHANGED, RGB(226, 135, 67)));
	m_wndHEVC.SetColor(GetConfig().get_int(true, REG_COLOR_HEVC, RGB(158, 255, 250)));

	int flags = GetConfig().get_int(true, REG_CMP_FLAGS, CMP_FLAG_ALL);

	m_bCmpTitle   = (flags & CMP_FLAG_TITLE) ? TRUE : FALSE;
	m_bCmpIcon    = (flags & CMP_FLAG_ICON) ? TRUE : FALSE;
	m_bCmpArchive = (flags & CMP_FLAG_ARCHIVE) ? TRUE : FALSE;
	m_bCmpEpg1    = (flags & CMP_FLAG_EPG1) ? TRUE : FALSE;
	m_bCmpEpg2    = (flags & CMP_FLAG_EPG2) ? TRUE : FALSE;

	int nCurrent = 0;
	for (const auto& pair : theApp.m_LangMap)
	{
		int nIdx = m_wndLanguage.AddString(pair.second.csLang);
		m_wndLanguage.SetItemData(nIdx, pair.first);
		if (pair.first == m_nLang)
			nCurrent = nIdx;
	}

	if (m_wndLanguage.GetCount())
	{
		m_wndLanguage.SetCurSel(nCurrent);
	}

	m_wndClearCache.EnableWindow(std::filesystem::exists(std::filesystem::temp_directory_path().append(L"iptv_cache")));

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CMainSettingsPage::OnOK()
{
	UpdateData(TRUE);

	GetConfig().set_int(true, REG_AUTO_SYNC, m_bAutoSync);
	GetConfig().set_int(true, REG_AUTO_HIDE, m_bAutoHide);
	GetConfig().set_int(true, REG_MAX_THREADS, m_MaxThreads);
	GetConfig().set_int(true, REG_LANGUAGE, m_nLang);
	GetConfig().set_int(true, REG_COLOR_ADDED, m_wndAdded.GetColor());
	GetConfig().set_int(true, REG_COLOR_NOT_ADDED, m_wndNotAdded.GetColor());
	GetConfig().set_int(true, REG_COLOR_CHANGED, m_wndChanged.GetColor());
	GetConfig().set_int(true, REG_COLOR_HEVC, m_wndHEVC.GetColor());

	int flags = 0;
	flags |= (m_bCmpTitle ? CMP_FLAG_TITLE : 0);
	flags |= (m_bCmpIcon ? CMP_FLAG_ICON : 0);
	flags |= (m_bCmpArchive ? CMP_FLAG_ARCHIVE : 0);
	flags |= (m_bCmpEpg1 ? CMP_FLAG_EPG1 : 0);
	flags |= (m_bCmpEpg2 ? CMP_FLAG_EPG2 : 0);
	GetConfig().set_int(true, REG_CMP_FLAGS, flags);

	GetConfig().SetPortable(m_bPortable);

	__super::OnOK();
}

void CMainSettingsPage::OnEnChangeEditStreamThreads()
{
	UpdateData(TRUE);

	if (m_MaxThreads < 1)
		m_MaxThreads = 1;

	if (m_MaxThreads > 10)
		m_MaxThreads = 10;

	UpdateData(FALSE);
}

void CMainSettingsPage::OnDeltaposSpinStreamThreads(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateData(TRUE);
	m_MaxThreads -= reinterpret_cast<LPNMUPDOWN>(pNMHDR)->iDelta;
	UpdateData(FALSE);
	OnEnChangeEditStreamThreads();
	*pResult = 0;
}

void CMainSettingsPage::OnCbnSelchangeComboLang()
{
	AfxMessageBox(IDS_STRING_INFO_RESTART_NEED, MB_OK | MB_ICONINFORMATION);
	m_nLang = (WORD)m_wndLanguage.GetItemData(m_wndLanguage.GetCurSel());
}

void CMainSettingsPage::OnBnClickedButtonReset()
{
	m_wndAdded.SetColor(RGB(0, 200, 0));
	m_wndNotAdded.SetColor(RGB(200, 0, 0));
	m_wndChanged.SetColor(RGB(226, 135, 67));
	m_wndHEVC.SetColor(RGB(158, 255, 250));
}

void CMainSettingsPage::OnBnClickedButtonClearCache()
{
	std::filesystem::path cache_file = std::filesystem::temp_directory_path().append(L"iptv_cache");
	std::error_code err;
	std::filesystem::remove_all(cache_file, err);
	m_wndClearCache.EnableWindow(FALSE);
}
