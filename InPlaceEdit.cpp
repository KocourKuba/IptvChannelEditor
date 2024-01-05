/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2024): sharky72 (https://github.com/KocourKuba)

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
#include "InPlaceEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CInPlaceEdit, CEdit)
	ON_WM_KILLFOCUS()
	ON_WM_NCDESTROY()
	ON_WM_CHAR()
	ON_WM_CREATE()
END_MESSAGE_MAP()

CInPlaceEdit::CInPlaceEdit(int iItem, int iSubItem, LPCTSTR initText)
	: m_initText(initText)
	, m_iItem(iItem)
	, m_iSubItem(iSubItem)
{
}

// Translate window messages before they are dispatched to the TranslateMessage and DispatchMessage Windows functions.
BOOL CInPlaceEdit::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_DELETE || pMsg->wParam == VK_ESCAPE || GetKeyState(VK_CONTROL))
		{
			::TranslateMessage(pMsg);
			::DispatchMessage(pMsg);
			return TRUE;
			// DO NOT process further
		}
	}

	return CEdit::PreTranslateMessage(pMsg);
}

// Called immediately before losing the input focus
void CInPlaceEdit::OnKillFocus(CWnd* pNewWnd)
{
	CEdit::OnKillFocus(pNewWnd);

	CWnd* pParent = GetParent();

	CString str;
	GetWindowText(str);
	// Construct list control item data
	NMLVDISPINFO dispinfo = {};
	dispinfo.hdr.hwndFrom = pParent->m_hWnd;
	dispinfo.hdr.idFrom = GetDlgCtrlID();
	dispinfo.hdr.code = LVN_ENDLABELEDIT;
	dispinfo.item.mask = LVIF_TEXT;
	dispinfo.item.iItem = m_iItem;
	dispinfo.item.iSubItem = m_iSubItem;
	dispinfo.item.pszText = str.GetBuffer();
	dispinfo.item.cchTextMax = str.GetLength();

	// Send this Notification to parent of ListView ctrl
	if (pParent->GetParent())
	{
		pParent->GetParent()->SendMessage(WM_NOTIFY_END_EDIT, pParent->GetDlgCtrlID(), (LPARAM)&dispinfo);
	}

	DestroyWindow();
}

// Called when non client area is being destroyed
void CInPlaceEdit::OnNcDestroy()
{
	CEdit::OnNcDestroy();
	delete this;
}

// Called for nonsystem character keystrokes
void CInPlaceEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CWnd* pParent = GetParent();

	if (nChar == VK_ESCAPE)
	{
		SetWindowText(m_initText);
		pParent->SetFocus();
		return;
	}

	if (nChar == VK_RETURN)
	{

		CString str;
		GetWindowText(str);
		// Construct list control item data
		NMLVDISPINFO dispinfo = {};
		dispinfo.hdr.hwndFrom = pParent->m_hWnd;
		dispinfo.hdr.idFrom = GetDlgCtrlID();
		dispinfo.hdr.code = LVN_ENDLABELEDIT;
		dispinfo.item.mask = LVIF_TEXT;
		dispinfo.item.iItem = m_iItem;
		dispinfo.item.iSubItem = m_iSubItem;
		dispinfo.item.pszText = str.GetBuffer();
		dispinfo.item.cchTextMax = str.GetLength();

		// Send this Notification to parent of ListView ctrl
		if (pParent->GetParent())
		{
			pParent->GetParent()->SendMessage(WM_NOTIFY_END_EDIT, pParent->GetDlgCtrlID(), (LPARAM)&dispinfo);
		}
		pParent->SetFocus();
		return;
	}

	CEdit::OnChar(nChar, nRepCnt, nFlags);

	// Resize edit control if needed
	CString str;
	GetWindowText(str);
	CWindowDC dc(this);
	CFont* pFont = pParent->GetFont();
	CFont* pFontDC = dc.SelectObject(pFont);
	CSize size = dc.GetTextExtent(str);
	dc.SelectObject(pFontDC);
	size.cx += 5;

	// Get the client rectangle
	CRect rect;
	GetClientRect(&rect);
	// Transform rectangle to parent coordinates
	ClientToScreen(&rect);
	pParent->ScreenToClient(&rect);

	// Check whether control needs resizing and if sufficient space to grow
	if (size.cx > rect.Width())
	{
		CRect parentRect;
		pParent->GetClientRect(&parentRect);

		if (size.cx + rect.left < parentRect.right)
		{
			rect.right = rect.left + size.cx;
		}
		else
		{
			rect.right = parentRect.right;
		}

		MoveWindow(&rect);
	}
}

// Called when application requests the Windows window be created by calling the Create/CreateEx member function.
int CInPlaceEdit::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CEdit::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}

	// Set the proper font
	CFont* font = GetParent()->GetFont();
	SetFont(font);
	SetWindowText(m_initText);
	SetFocus();
	SetSel(0, -1);
	return 0;
}