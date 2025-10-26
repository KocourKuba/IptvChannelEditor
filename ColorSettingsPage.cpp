/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2025): sharky72 (https://github.com/KocourKuba)

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
#include "ColorSettingsPage.h"
#include "AccountSettings.h"
#include "Constants.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CColorSettingsPage dialog

IMPLEMENT_DYNAMIC(CColorSettingsPage, CTooltipPropertyPage)

BEGIN_MESSAGE_MAP(CColorSettingsPage, CTooltipPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_RESET, &CColorSettingsPage::OnBnClickedButtonReset)
END_MESSAGE_MAP()

CColorSettingsPage::CColorSettingsPage() : CTooltipPropertyPage(IDD_COLOR_SETTINGS_PAGE)
{
}

void CColorSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_BUTTON_ADDED, m_wndAdded);
	DDX_Control(pDX, IDC_BUTTON_NOT_ADDED, m_wndNotAdded);
	DDX_Control(pDX, IDC_BUTTON_CHANGED, m_wndChanged);
	DDX_Control(pDX, IDC_BUTTON_UNKNOWN, m_wndUnknown);
	DDX_Control(pDX, IDC_BUTTON_HEVC, m_wndHEVC);
	DDX_Control(pDX, IDC_BUTTON_HD, m_wndHD);
	DDX_Control(pDX, IDC_BUTTON_FHD, m_wndFHD);
	DDX_Control(pDX, IDC_BUTTON_DUPLICATED, m_wndDuplicated);
}

BOOL CColorSettingsPage::OnInitDialog()
{
	__super::OnInitDialog();

	AddTooltip(IDC_BUTTON_ADDED, IDS_STRING_BUTTON_COLORS);
	AddTooltip(IDC_BUTTON_CHANGED, IDS_STRING_BUTTON_COLORS);
	AddTooltip(IDC_BUTTON_NOT_ADDED, IDS_STRING_BUTTON_COLORS);
	AddTooltip(IDC_BUTTON_UNKNOWN, IDS_STRING_BUTTON_COLORS);
	AddTooltip(IDC_BUTTON_DUPLICATED, IDS_STRING_BUTTON_DUPLICATED);
	AddTooltip(IDC_BUTTON_HEVC, IDS_STRING_BUTTON_COLORS);
	AddTooltip(IDC_BUTTON_HD, IDS_STRING_BUTTON_COLORS);
	AddTooltip(IDC_BUTTON_FHD, IDS_STRING_BUTTON_COLORS);
	AddTooltip(IDC_BUTTON_RESET, IDS_STRING_BUTTON_RESET);

	m_wndAdded.SetColor(GetConfig().get_int(true, REG_COLOR_ADDED, DEFAULT_COLOR_ADDED));
	m_wndNotAdded.SetColor(GetConfig().get_int(true, REG_COLOR_NOT_ADDED, DEFAULT_COLOR_NOT_ADDED));
	m_wndChanged.SetColor(GetConfig().get_int(true, REG_COLOR_CHANGED, DEFAULT_COLOR_CHANGED));
	m_wndUnknown.SetColor(GetConfig().get_int(true, REG_COLOR_UNKNOWN, ::GetSysColor(COLOR_WINDOWTEXT)));
	m_wndDuplicated.SetColor(GetConfig().get_int(true, REG_COLOR_DUPLICATED, ::GetSysColor(COLOR_GRAYTEXT)));

	m_wndHEVC.SetColor(GetConfig().get_int(true, REG_COLOR_HEVC, DEFAULT_COLOR_HEVC));
	m_wndHD.SetColor(GetConfig().get_int(true, REG_COLOR_HD, DEFAULT_COLOR_HD));
	m_wndFHD.SetColor(GetConfig().get_int(true, REG_COLOR_FHD, DEFAULT_COLOR_FHD));

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CColorSettingsPage::OnApply()
{
	UpdateData(TRUE);

	GetConfig().set_int(true, REG_COLOR_ADDED, m_wndAdded.GetColor());
	GetConfig().set_int(true, REG_COLOR_NOT_ADDED, m_wndNotAdded.GetColor());
	GetConfig().set_int(true, REG_COLOR_CHANGED, m_wndChanged.GetColor());
	GetConfig().set_int(true, REG_COLOR_UNKNOWN, m_wndUnknown.GetColor());
	GetConfig().set_int(true, REG_COLOR_HEVC, m_wndHEVC.GetColor());
	GetConfig().set_int(true, REG_COLOR_HD, m_wndHD.GetColor());
	GetConfig().set_int(true, REG_COLOR_FHD, m_wndFHD.GetColor());
	GetConfig().set_int(true, REG_COLOR_DUPLICATED, m_wndDuplicated.GetColor());

	return __super::OnApply();
}

void CColorSettingsPage::OnBnClickedButtonReset()
{
	m_wndAdded.SetColor(DEFAULT_COLOR_ADDED);
	m_wndNotAdded.SetColor(DEFAULT_COLOR_NOT_ADDED);
	m_wndChanged.SetColor(DEFAULT_COLOR_CHANGED);
	m_wndUnknown.SetColor(::GetSysColor(COLOR_WINDOWTEXT));

	m_wndHEVC.SetColor(DEFAULT_COLOR_HEVC);
	m_wndHD.SetColor(DEFAULT_COLOR_HD);
	m_wndFHD.SetColor(DEFAULT_COLOR_FHD);
	m_wndDuplicated.SetColor(::GetSysColor(COLOR_GRAYTEXT));
}
