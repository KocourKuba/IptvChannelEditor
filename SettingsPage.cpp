// SettingsPage.cpp : implementation file
//

#include "pch.h"
#include "IPTVChannelEditor.h"
#include "SettingsPage.h"
#include "afxdialogex.h"


// CSettingsPage dialog

IMPLEMENT_DYNAMIC(CSettingsPage, CPropertyPage)

CSettingsPage::CSettingsPage()
	: CPropertyPage(IDD_CSettingsPage)
{

}

CSettingsPage::~CSettingsPage()
{
}

void CSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSettingsPage, CPropertyPage)
END_MESSAGE_MAP()


// CSettingsPage message handlers
