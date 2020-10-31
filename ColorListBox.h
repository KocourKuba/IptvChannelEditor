#pragma once

class CColorListBox : public CListBox
{
// Construction
public:
	void DrawItem(LPDRAWITEMSTRUCT lpDIS) override;

public:
	COLORREF m_color = ::GetSysColor(COLOR_WINDOWTEXT);
};
