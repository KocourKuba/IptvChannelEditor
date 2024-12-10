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
#include "ColorEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CColorEdit

BEGIN_MESSAGE_MAP(CColorEdit, CEdit)
	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()

CColorEdit::CColorEdit()
{
	m_crBkColor = ::GetSysColor(COLOR_WINDOW);
	m_crTextColor = RGB(0, 0, 0);
	m_brBkgnd.CreateSolidBrush(m_crBkColor);
}

void CColorEdit::SetTextColor(COLORREF crColor)
{
	m_crTextColor = crColor;
	RedrawWindow();
}

void CColorEdit::SetBkColor(COLORREF crColor)
{
	m_crBkColor = crColor;
	m_brBkgnd.DeleteObject();
	m_brBkgnd.CreateSolidBrush(crColor);
	RedrawWindow();
}

HBRUSH CColorEdit::CtlColor(CDC* pDC, UINT /*nCtlColor*/)
{
	HBRUSH hbr = (HBRUSH)m_brBkgnd;
	pDC->SetBkColor(m_crBkColor);
	pDC->SetTextColor(m_crTextColor);

	return hbr;
}

BOOL CColorEdit::SetReadOnly(BOOL flag)
{
   if (flag)
	  SetBkColor(m_crBkColor);
   else
	  SetBkColor(RGB(255, 255, 255));

   return CEdit::SetReadOnly(flag);
}
