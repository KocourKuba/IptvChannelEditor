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
#include "PluginConfigPropertySheet.h"
#include "IPTVChannelEditor.h"
#include "NewConfigDlg.h"
#include "TooltipPropertyPage.h"

int CALLBACK fnPropSheetCallback(HWND hWnd, UINT message, LPARAM lParam)
{
	extern int CALLBACK AfxPropSheetCallback(HWND, UINT message, LPARAM lParam);
	// XMN: Call MFC's callback
	int nRes = AfxPropSheetCallback(hWnd, message, lParam);

	switch (message)
	{
		case PSCB_PRECREATE:
			// Set our own window styles
			((LPDLGTEMPLATE)lParam)->style |= (DS_3DLOOK | DS_SETFONT | WS_THICKFRAME | WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
			break;
	}
	return nRes;
}

IMPLEMENT_DYNAMIC(CPluginConfigPropertySheet, CMFCPropertySheet)

BEGIN_MESSAGE_MAP(CPluginConfigPropertySheet, CMFCPropertySheet)
	ON_WM_CREATE()
	ON_WM_GETMINMAXINFO()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, &CPluginConfigPropertySheet::OnToolTipText)
	ON_CBN_SELCHANGE(IDC_COMBO_PLUGIN_CONFIG, &CPluginConfigPropertySheet::OnCbnSelchangeComboPluginConfig)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_CONFIG, &CPluginConfigPropertySheet::OnBnClickedButtonSaveConfig)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_AS_CONFIG, &CPluginConfigPropertySheet::OnBnClickedButtonSaveAsConfig)
END_MESSAGE_MAP()

BOOL CPluginConfigPropertySheet::OnInitDialog()
{
	BOOL bResult = __super::OnInitDialog();

	m_wndToolTipCtrl.SetDelayTime(TTDT_AUTOPOP, 10000);
	m_wndToolTipCtrl.SetDelayTime(TTDT_INITIAL, 500);
	m_wndToolTipCtrl.SetMaxTipWidth(300);
	m_wndToolTipCtrl.Activate(TRUE);

	GetWindowRect(m_min_rc);

	m_min_rc.InflateRect(0, 0, 0, m_gapHeight);
	MoveWindow(m_min_rc);

	CRect rcCombo(CPoint(7, 5), CSize(115, 30));
	MapDialogRect(rcCombo);
	::SetWindowPos(m_wndPluginConfigs.GetSafeHwnd(), nullptr,
				   rcCombo.left, rcCombo.top, rcCombo.Width(), rcCombo.Height(),
				   SWP_NOZORDER | SWP_NOACTIVATE);

	CRect rcSave(CPoint(132, 4), CSize(19, 14));
	MapDialogRect(rcSave);
	::SetWindowPos(m_wndBtnSaveConf.GetSafeHwnd(), nullptr,
				   rcSave.left, rcSave.top, rcSave.Width(), rcSave.Height(),
				   SWP_NOZORDER | SWP_NOACTIVATE);

	CRect rcSaveAs(CPoint(155, 4), CSize(19, 14));
	MapDialogRect(rcSaveAs);
	::SetWindowPos(m_wndBtnSaveAsConf.GetSafeHwnd(), nullptr,
				   rcSaveAs.left, rcSaveAs.top, rcSaveAs.Width(), rcSaveAs.Height(),
				   SWP_NOZORDER | SWP_NOACTIVATE);

	// ... move tab control down here ...
	HWND hWnd = (HWND)GetTabControl()->m_hWnd;
	ASSERT(hWnd != nullptr);

	CRect rectOld;
	::GetWindowRect(hWnd, &rectOld);
	ScreenToClient(&rectOld);
	::SetWindowPos(hWnd, nullptr,
				   rectOld.left, rectOld.top + m_gapHeight, rectOld.Width(), rectOld.Height(),
				   SWP_NOZORDER | SWP_NOACTIVATE);

	hWnd = (HWND)GetActivePage()->m_hWnd; // m_Page1 is assumed to be the first page
	// in your property sheet. de corrections accordingly

	ASSERT(hWnd != nullptr);
	::GetWindowRect(hWnd, &rectOld);
	ScreenToClient(&rectOld);
	::SetWindowPos(hWnd, nullptr,
				   rectOld.left, rectOld.top + m_gapHeight, rectOld.Width(), rectOld.Height(),
				   SWP_NOZORDER | SWP_NOACTIVATE);

	int _PropSheetButtons[] = { IDOK, IDCANCEL };
	for (int PropSheetButton : _PropSheetButtons)
	{
		hWnd = ::GetDlgItem(m_hWnd, PropSheetButton);
		if (hWnd != nullptr)
		{
			::GetWindowRect(hWnd, &rectOld);
			ScreenToClient(&rectOld);
			::SetWindowPos(hWnd, nullptr,
						   rectOld.left, rectOld.top + m_gapHeight, rectOld.Width(), rectOld.Height(),
						   SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	m_wndPluginConfigs.SetFont(GetFont());

	SetButtonImage(IDB_PNG_SAVE, m_wndBtnSaveConf);
	SetButtonImage(IDB_PNG_SAVE_AS, m_wndBtnSaveAsConf);

	AddTooltip(IDC_COMBO_PLUGIN_CONFIG, IDS_STRING_COMBO_CONFIG);
	AddTooltip(IDC_BUTTON_SAVE_CONFIG, IDS_STRING_BUTTON_SAVE_CONFIG);
	AddTooltip(IDC_BUTTON_SAVE_AS_CONFIG, IDS_STRING_BUTTON_SAVE_AS_CONFIG);

	FillConfigs();

	SetupDynamicLayout();

	RestoreWindowPos(GetSafeHwnd(), m_posKey);

	AllowSave(false);

	UpdateControls();

	return bResult;
}

BOOL CPluginConfigPropertySheet::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_MOUSEMOVE)
	{
		HWND hWnd = pMsg->hwnd;
		LPARAM lParam = pMsg->lParam;

		POINT pt{};
		pt.x = LOWORD(pMsg->lParam);  // horizontal position of cursor
		pt.y = HIWORD(pMsg->lParam);  // vertical position of cursor

		for (auto& pair : m_tooltips_info)
		{
			auto& wnd = pair.first;
			if (!wnd->IsWindowVisible() || wnd->IsWindowEnabled()) continue;

			CRect rect;
			wnd->GetWindowRect(&rect);
			ScreenToClient(&rect);

			if (rect.PtInRect(pt))
			{
				pMsg->hwnd = wnd->m_hWnd;

				ClientToScreen(&pt);
				wnd->ScreenToClient(&pt);
				pMsg->lParam = MAKELPARAM(pt.x, pt.y);
				break;
			}
		}

		m_wndToolTipCtrl.RelayEvent(pMsg);

		pMsg->hwnd = hWnd;
		pMsg->lParam = lParam;
	}

	return __super::PreTranslateMessage(pMsg);
}


void CPluginConfigPropertySheet::OnDestroy()
{
	SaveWindowPos(GetSafeHwnd(), m_posKey);

	__super::OnDestroy();
}

void CPluginConfigPropertySheet::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	__super::OnGetMinMaxInfo(lpMMI); // Will set your modified values

	if (!m_min_rc.IsRectEmpty())
	{
		lpMMI->ptMinTrackSize.x = m_min_rc.Width();
		lpMMI->ptMinTrackSize.y = m_min_rc.Height();
	}
}

void CPluginConfigPropertySheet::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	Invalidate(TRUE);
}

int CPluginConfigPropertySheet::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CRect rcCombo(0, 0, 115, 30);
	m_wndPluginConfigs.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
							  rcCombo, this, IDC_COMBO_PLUGIN_CONFIG);

	CRect rcSave(0, 0, 19, 14);
	m_wndBtnSaveConf.Create(_T("S"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_BITMAP,
							rcSave, this, IDC_BUTTON_SAVE_CONFIG);

	CRect rcSaveAs(0, 0, 19, 14);
	m_wndBtnSaveAsConf.Create(_T("SAS"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_BITMAP,
							  rcSaveAs, this, IDC_BUTTON_SAVE_AS_CONFIG);

	if (!m_wndToolTipCtrl.Create(this, TTS_ALWAYSTIP))
	{
		TRACE(_T("Unable To create ToolTip\n"));
		return 1;
	}

	return 0;
}

INT_PTR CPluginConfigPropertySheet::DoModal()
{
	// Hook into property sheet creation code
	m_psh.dwFlags |= PSH_USECALLBACK;
	m_psh.pfnCallback = fnPropSheetCallback;

	return __super::DoModal();
}

void CPluginConfigPropertySheet::FillConfigs()
{
	m_wndPluginConfigs.ResetContent();
	int cur_idx = 0;
	for (const auto& entry : m_configs)
	{
		if (!m_selected_cred.config.empty() && entry == m_selected_cred.get_config())
		{
			const auto& name = fmt::format(L"{:s} ({:s})", entry, load_string_resource(IDS_STRING_CURRENT));
			int idx = m_wndPluginConfigs.AddString(name.c_str());
			cur_idx = idx;
		}
		else
		{
			m_wndPluginConfigs.AddString(entry.c_str());
		}
	}

	m_wndPluginConfigs.SetCurSel(cur_idx);
}

std::wstring CPluginConfigPropertySheet::GetSelectedConfig()
{
	size_t idx = (size_t)m_wndPluginConfigs.GetCurSel();
	if (idx < 1 || idx >= m_configs.size()) return L"";

	return m_configs[idx];
}

void CPluginConfigPropertySheet::UpdateControls()
{
	m_wndBtnSaveConf.EnableWindow(m_allow_save && !GetSelectedConfig().empty());

	auto activePage = DYNAMIC_DOWNCAST(CTooltipPropertyPage, GetActivePage());
	if (activePage)
		activePage->FillControls();
}

void CPluginConfigPropertySheet::AllowSave(bool val /*= true*/)
{
	m_allow_save = val;
	if (m_wndBtnSaveConf.GetSafeHwnd())
		m_wndBtnSaveConf.EnableWindow(val);

	GetDlgItem(IDOK)->EnableWindow(!val && !GetSelectedConfig().empty());
}

void CPluginConfigPropertySheet::OnCbnSelchangeComboPluginConfig()
{
	AllowSave(false);
	m_plugin->load_plugin_parameters(GetSelectedConfig(), m_plugin->get_internal_name());
	auto activePage = DYNAMIC_DOWNCAST(CTooltipPropertyPage, GetActivePage());
	if (activePage)
		activePage->FillControls();
}

void CPluginConfigPropertySheet::OnBnClickedButtonSaveConfig()
{
	auto name = GetSelectedConfig();
	if (name.empty()) return;

	if (m_plugin->save_plugin_parameters(name, m_plugin->get_internal_name()))
	{
		AllowSave(false);
	}
	else
	{
		AfxMessageBox(IDS_STRING_ERR_SAVE_CONFIG, MB_ICONERROR | MB_OK);
	}
}

void CPluginConfigPropertySheet::OnBnClickedButtonSaveAsConfig()
{
	CNewConfigDlg dlg;
	if (dlg.DoModal() != IDOK || dlg.m_name.IsEmpty())
		return;

	std::filesystem::path new_conf = dlg.m_name.GetString();
	if (new_conf.extension() != L".json")
		new_conf += (L".json");

	if (!m_plugin->save_plugin_parameters(new_conf, m_plugin->get_internal_name()))
	{
		AfxMessageBox(IDS_STRING_ERR_SAVE_CONFIG, MB_ICONERROR | MB_OK);
	}
	else
	{
		AllowSave(false);
		m_configs.emplace_back(new_conf);
		FillConfigs();
		m_wndPluginConfigs.SetCurSel((int)m_configs.size() - 1);
		UpdateControls();
	}
}

void CPluginConfigPropertySheet::SetupDynamicLayout()
{
	EnableDynamicLayout(TRUE);
	auto pManager = GetDynamicLayout();
	if (pManager != nullptr)
	{
		pManager->Create(this);

		for (CWnd* child = GetWindow(GW_CHILD); child != nullptr; child = child->GetWindow(GW_HWNDNEXT))
		{
			UINT id = child->GetDlgCtrlID();
			if (id == IDC_COMBO_PLUGIN_CONFIG || id == IDC_BUTTON_EDIT_CONFIG || id == IDC_BUTTON_SAVE_CONFIG || id == IDC_BUTTON_SAVE_AS_CONFIG)
			{
				pManager->AddItem(child->GetSafeHwnd(), CMFCDynamicLayout::MoveNone(), CMFCDynamicLayout::SizeNone());
			}
			else if (child->SendMessage(WM_GETDLGCODE) & DLGC_BUTTON)
			{
				// All buttons need to be moved 100% in all directions
				pManager->AddItem(child->GetSafeHwnd(), CMFCDynamicLayout::MoveHorizontalAndVertical(100, 100), CMFCDynamicLayout::SizeNone());
			}
			else // This will be the main tab control which needs to be stretched in both directions
			{
				pManager->AddItem(child->GetSafeHwnd(), CMFCDynamicLayout::MoveNone(), CMFCDynamicLayout::SizeHorizontalAndVertical(100, 100));
			}
		}
	}
}

BOOL CPluginConfigPropertySheet::OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult)
{
	// if there is a top level routing frame then let it handle the message
	if (GetRoutingFrame() != nullptr)
		return FALSE;

	// to be thorough we will need to handle UNICODE versions of the message also !!

	UINT_PTR nID = pNMHDR->idFrom;
	TOOLTIPTEXT* pTTT = (TOOLTIPTEXT*)pNMHDR;

	if (pNMHDR->code == TTN_NEEDTEXT && (pTTT->uFlags & TTF_IDISHWND))
	{
		// idFrom is actually the HWND of the tool
		nID = ::GetDlgCtrlID((HWND)nID);
	}

	if (nID != 0) // will be zero on a separator
	{
		const auto& pair = std::find_if(m_tooltips_info.begin(), m_tooltips_info.end(), [nID](auto& pair)
										{
											return pair.first->GetDlgCtrlID() == nID;
										});

		if (pair != m_tooltips_info.end())
		{
			pTTT->lpszText = pair->second.data();
			*pResult = 0;
			return TRUE;
		}
	}

	return FALSE;
}

void CPluginConfigPropertySheet::AddTooltip(UINT ctrlID, UINT textID)
{
	CWnd* wnd = GetDlgItem(ctrlID);
	if (wnd)
	{
		m_tooltips_info.emplace(wnd, load_string_resource(textID));
		m_wndToolTipCtrl.AddTool(wnd, LPSTR_TEXTCALLBACK);
	}
}
