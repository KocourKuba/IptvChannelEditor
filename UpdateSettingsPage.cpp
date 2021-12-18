// SettingsDlg.cpp : implementation file
//

#include "pch.h"
#include <afxdialogex.h>
#include "IPTVChannelEditor.h"
#include "UpdateSettingsPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CUpdateSettingsPage dialog

IMPLEMENT_DYNAMIC(CUpdateSettingsPage, CPropertyPage)

BEGIN_MESSAGE_MAP(CUpdateSettingsPage, CPropertyPage)
	ON_EN_CHANGE(IDC_EDIT_UPDATE_FREQ, &CUpdateSettingsPage::OnEnChangeEditUpdateFreq)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CHECK_UPDATE, &CUpdateSettingsPage::OnDeltaposSpinStreamThreads)
END_MESSAGE_MAP()


CUpdateSettingsPage::CUpdateSettingsPage() : CPropertyPage(IDD_UPDATE_SETTINGS_PAGE)
{
}

void CUpdateSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_UPDATE_FREQ, m_UpdateFreq);
	DDX_Check(pDX, IDC_CHECK_CHANNELS, m_bUpdateChannels);
}

BOOL CUpdateSettingsPage::OnInitDialog()
{
	__super::OnInitDialog();

	m_UpdateFreq = GetConfig().get_int(true, REG_UPDATE_FREQ, 3);
	m_bUpdateChannels = GetConfig().get_int(true, REG_UPDATE_PL);

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CUpdateSettingsPage::OnOK()
{
	UpdateData(TRUE);

	GetConfig().set_int(true, REG_UPDATE_FREQ, m_UpdateFreq);
	GetConfig().set_int(true, REG_UPDATE_PL, m_bUpdateChannels);

	__super::OnOK();
}

void CUpdateSettingsPage::OnEnChangeEditUpdateFreq()
{
	UpdateData(TRUE);

	if (m_UpdateFreq < 0)
		m_UpdateFreq = 0;

	if (m_UpdateFreq > 14)
		m_UpdateFreq = 14;

	UpdateData(FALSE);
}

void CUpdateSettingsPage::OnDeltaposSpinStreamThreads(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateData(TRUE);
	m_UpdateFreq -= reinterpret_cast<LPNMUPDOWN>(pNMHDR)->iDelta;
	UpdateData(FALSE);
	OnEnChangeEditUpdateFreq();
	*pResult = 0;
}
