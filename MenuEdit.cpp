#include "pch.h"
#include "MenuEdit.h"
#include "IPTVChannelEditor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMenuEdit

#define WM_TEMPLATE_ITEM_ID (WM_USER + 0x1000)
#define WM_MENU_ITEM_MAX	(0x20)

BEGIN_MESSAGE_MAP(CMenuEdit, CEdit)
	ON_WM_CONTEXTMENU()
	ON_COMMAND_RANGE(WM_TEMPLATE_ITEM_ID, WM_TEMPLATE_ITEM_ID + WM_MENU_ITEM_MAX, &CMenuEdit::OnProcessMenuItemCommand)
END_MESSAGE_MAP()

void CMenuEdit::OnContextMenu(CWnd* pWnd, CPoint point)
{
	SetFocus();
	CMenu menu;
	menu.LoadMenu(IDR_MENU_TEMPLATE);

	CMenu* pMenu = menu.GetSubMenu(0);
	if (!pMenu)
		return;

	if (!m_items.empty())
	{
		CMenu subMenu;
		subMenu.CreatePopupMenu();
		int i = 0;
		for(const auto& item : m_items)
		{
			subMenu.AppendMenu(MF_STRING, WM_TEMPLATE_ITEM_ID + i++, item.c_str());
		}
		pMenu->InsertMenu(0, MF_BYPOSITION | MF_POPUP, (UINT_PTR)subMenu.Detach(), load_string_resource(IDS_STRING_INSERT_VARS).c_str());
		pMenu->InsertMenu(1, MF_BYPOSITION | MF_SEPARATOR);
	}

	bool bReadOnly = IsReadOnly();
	bool bTextSelected = IsTextSelected();

	DWORD flag = bReadOnly || !CanUndo() ? MF_GRAYED : 0;
	pMenu->EnableMenuItem(EM_UNDO, MF_BYCOMMAND | flag);

	DWORD sel = GetSel();
	flag = bTextSelected ? 0 : MF_GRAYED;
	pMenu->EnableMenuItem(WM_COPY, MF_BYCOMMAND | flag);

	flag = bTextSelected && !bReadOnly ? 0 : MF_GRAYED;
	pMenu->EnableMenuItem(WM_CUT, MF_BYCOMMAND | flag);
	pMenu->EnableMenuItem(WM_CLEAR, MF_BYCOMMAND | flag);

	pMenu->EnableMenuItem(WM_PASTE, IsClipboardFormatAvailable(CF_TEXT) && !bReadOnly);

	int len = GetWindowTextLength();
	flag = (!len || (LOWORD(sel) == 0 && HIWORD(sel) == len)) ? MF_GRAYED : 0;
	pMenu->EnableMenuItem(ID_SELECT_ALL, MF_BYCOMMAND | flag);

	if (point.x == -1 || point.y == -1)
	{
		CRect rc;
		GetClientRect(&rc);
		point = rc.CenterPoint();
		ClientToScreen(&point);
	}

	int nCmd = pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD | TPM_RIGHTBUTTON, point.x, point.y, this);
	if (nCmd >= 0)
	{
		switch (nCmd)
		{
			case EM_UNDO:
			case WM_CUT:
			case WM_COPY:
			case WM_CLEAR:
			case WM_PASTE:
				SendMessage(nCmd);
				break;
			case ID_SELECT_ALL:
				SendMessage(EM_SETSEL, 0, -1);
				break;
			default:
				SendMessage(WM_COMMAND, nCmd);
		}
	}
}

void CMenuEdit::OnProcessMenuItemCommand(UINT nID)
{
	size_t nItemID = nID - WM_TEMPLATE_ITEM_ID;
	if (nItemID >= m_items.size()) return;

	ReplaceSel(m_items[nItemID].c_str(), TRUE);
}

bool CMenuEdit::IsTextSelected()
{
	DWORD sel = GetSel();
	return LOWORD(sel) != HIWORD(sel);
}

bool CMenuEdit::IsReadOnly()
{
	return GetStyle() & ES_READONLY;
}