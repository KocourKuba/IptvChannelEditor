#include "pch.h"
#include "MySplitButton.h"

IMPLEMENT_DYNAMIC(CMySplitButton, CSplitButton)

BEGIN_MESSAGE_MAP(CMySplitButton, CSplitButton)
	ON_NOTIFY_REFLECT_EX(BCN_DROPDOWN, &CMySplitButton::OnDropDown)
END_MESSAGE_MAP()


BOOL CMySplitButton::OnDropDown(NMHDR* pNMHDR, LRESULT* pResult)
{
	return FALSE;
}
