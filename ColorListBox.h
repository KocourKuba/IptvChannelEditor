#pragma once

class CColorListBox : public CListBox
{
// Construction
public:
	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) override;
};
