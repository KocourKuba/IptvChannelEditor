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
#include "ResizedPropertySheet.h"
#include "IPTVChannelEditor.h"

int CALLBACK XmnPropSheetCallback(HWND hWnd, UINT message, LPARAM lParam)
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

IMPLEMENT_DYNAMIC(CResizedPropertySheet, CMFCPropertySheet)

BEGIN_MESSAGE_MAP(CResizedPropertySheet, CMFCPropertySheet)
	ON_WM_GETMINMAXINFO()
	ON_WM_DESTROY()
	ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CResizedPropertySheet::OnInitDialog()
{
	BOOL bResult = __super::OnInitDialog();

	GetWindowRect(m_min_rc);

	SetupDynamicLayout();

	RestoreWindowPos(GetSafeHwnd(), m_posKey);

	return bResult;
}

void CResizedPropertySheet::OnDestroy()
{
	SaveWindowPos(GetSafeHwnd(), m_posKey);

	__super::OnDestroy();
}

void CResizedPropertySheet::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	__super::OnGetMinMaxInfo(lpMMI); // Will set your modified values

	if (!m_min_rc.IsRectEmpty())
	{
		lpMMI->ptMinTrackSize.x = m_min_rc.Width();
		lpMMI->ptMinTrackSize.y = m_min_rc.Height();
	}
}

void CResizedPropertySheet::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	Invalidate(TRUE);
}

INT_PTR CResizedPropertySheet::DoModal()
{
	// Hook into property sheet creation code
	m_psh.dwFlags |= PSH_USECALLBACK;
	m_psh.pfnCallback = XmnPropSheetCallback;

	return __super::DoModal();
}

void CResizedPropertySheet::SetupDynamicLayout()
{
	EnableDynamicLayout(TRUE);
	auto pManager = GetDynamicLayout();
	if (pManager != nullptr)
	{
		pManager->Create(this);

		for (CWnd* child = GetWindow(GW_CHILD); child != nullptr; child = child->GetWindow(GW_HWNDNEXT))
		{
			if (child->SendMessage(WM_GETDLGCODE) & DLGC_BUTTON)
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
