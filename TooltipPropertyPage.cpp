/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2023): sharky72 (https://github.com/KocourKuba)

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
#include "TooltipPropertyPage.h"

IMPLEMENT_DYNAMIC(CTooltipPropertyPage, CMFCPropertyPage)

BEGIN_MESSAGE_MAP(CTooltipPropertyPage, CMFCPropertyPage)
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, &CTooltipPropertyPage::OnToolTipText)
END_MESSAGE_MAP()

BOOL CTooltipPropertyPage::OnInitDialog()
{
	__super::OnInitDialog();

	if (!m_wndToolTipCtrl.Create(this, TTS_ALWAYSTIP))
	{
		TRACE(_T("Unable To create ToolTip\n"));
		return FALSE;
	}

	m_wndToolTipCtrl.SetDelayTime(TTDT_AUTOPOP, 10000);
	m_wndToolTipCtrl.SetDelayTime(TTDT_INITIAL, 500);
	m_wndToolTipCtrl.SetMaxTipWidth(300);
	m_wndToolTipCtrl.Activate(TRUE);

	return TRUE;
}

BOOL CTooltipPropertyPage::PreTranslateMessage(MSG* pMsg)
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

BOOL CTooltipPropertyPage::OnApply()
{
	return TRUE;
}

BOOL CTooltipPropertyPage::OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult)
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

void CTooltipPropertyPage::AddTooltip(UINT ctrlID, UINT textID)
{
	CWnd* wnd = GetDlgItem(ctrlID);
	if (wnd)
	{
		m_tooltips_info.emplace(wnd, load_string_resource(textID));
		m_wndToolTipCtrl.AddTool(wnd, LPSTR_TEXTCALLBACK);
	}
}

void CTooltipPropertyPage::AllowSave(bool val /*= true*/)
{
	GetPropertySheet()->AllowSave(val);
}

CPluginConfigPropertySheet* CTooltipPropertyPage::GetPropertySheet()
{
	return DYNAMIC_DOWNCAST(CPluginConfigPropertySheet, GetParent());
}
