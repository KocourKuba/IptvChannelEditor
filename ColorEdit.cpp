#include "pch.h"
#include "ColorEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
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
