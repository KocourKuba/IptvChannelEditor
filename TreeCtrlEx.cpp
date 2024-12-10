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
#include "TreeCtrlEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr auto TCEX_EDITLABEL = 1;  // Edit label timer event;
#define COLORREF_NULL ((COLORREF)-1)

// CTreeCtrlEx

IMPLEMENT_DYNAMIC(CTreeCtrlEx, CTreeCtrl)

BEGIN_MESSAGE_MAP(CTreeCtrlEx, CTreeCtrl)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	ON_WM_PAINT()
	ON_NOTIFY_REFLECT_EX(TVN_ITEMEXPANDING, &CTreeCtrlEx::OnItemexpanding)
	ON_NOTIFY_REFLECT_EX(NM_SETFOCUS, &CTreeCtrlEx::OnSetfocus)
	ON_NOTIFY_REFLECT_EX(NM_KILLFOCUS, &CTreeCtrlEx::OnKillfocus)
	ON_NOTIFY_REFLECT_EX(TVN_SELCHANGED, &CTreeCtrlEx::OnSelchanged)
	ON_NOTIFY_REFLECT_EX(TVN_DELETEITEM, &CTreeCtrlEx::OnDeleteItem)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_TIMER()
	ON_WM_NCHITTEST()

END_MESSAGE_MAP()

BOOL CTreeCtrlEx::Create(DWORD dwStyle, DWORD dwExStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	return CTreeCtrl::CreateEx(dwExStyle, dwStyle, rect, pParentWnd, nID);
}

BOOL CTreeCtrlEx::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	return CTreeCtrl::Create(dwStyle, rect, pParentWnd, nID);
}

// The tree control dosn't support multiple selection. However we can simulate
// it by taking control of the left mouse click and arrow key press before the
// control gets them, and setting/clearing the TVIS_SELECTED style on the items
void CTreeCtrlEx::OnLButtonDown(UINT nFlags, CPoint point)
{
	UINT nHitFlags = 0;
	HTREEITEM hClickedItem = HitTest(point, &nHitFlags);

	// Must invoke label editing explicitly. The base class OnLButtonDown would normally
	// do this, but we can't call it here because of the multiple selection...
	if (!(nFlags & (MK_CONTROL | MK_SHIFT)) && (GetStyle() & TVS_EDITLABELS) && (nHitFlags & TVHT_ONITEMLABEL))
	{
		if (hClickedItem == GetSelectedItem())
		{
			// Clear multple selection before label editing
			ClearSelection();
			SelectItem(hClickedItem);

			// Invoke label editing
			m_bEditLabelPending = TRUE;
			m_idTimer = SetTimer(TCEX_EDITLABEL, GetDoubleClickTime(), nullptr);

			return;
		}
	}

	m_bEditLabelPending = FALSE;

	if (nHitFlags & TVHT_ONITEM)
	{
		SetFocus();

		m_hClickedItem = hClickedItem;

		// Is the clicked item already selected ?
		BOOL bIsClickedItemSelected = GetItemState(hClickedItem, TVIS_SELECTED) & TVIS_SELECTED;

		if (bIsClickedItemSelected)
		{
			// Maybe user wants to drag/drop multiple items!
			// So, wait until OnLButtonUp() to do the selection stuff.
			m_bSelectPending = TRUE;
		}
		else
		{
			SelectMultiple(hClickedItem, nFlags, point);
			m_bSelectPending = FALSE;
		}

		m_ptClick = point;
	}
	else
	{
		if (nHitFlags & TVHT_NOWHERE || nHitFlags & TVHT_ONITEMRIGHT)
		{
			ClearSelection();
		}

		CTreeCtrl::OnLButtonDown(nFlags, point);
	}
}

void CTreeCtrlEx::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bSelectPending)
	{
		// A select has been waiting to be performed here
		SelectMultiple(m_hClickedItem, nFlags, point);
		m_bSelectPending = FALSE;
	}

	m_hClickedItem = nullptr;

	CTreeCtrl::OnLButtonUp(nFlags, point);
}

void CTreeCtrlEx::OnRButtonDown(UINT /*nFlags*/, CPoint point)
{
	if (GetStyle() & TVS_SINGLEEXPAND)
	{
		Default();
		return;
	}

	// hittest to get the tree item under the cursor
	// and select it.
	UINT uFlags = 0;
	HTREEITEM hItem = HitTest(point, &uFlags);
	if (hItem != nullptr && (uFlags & TVHT_ONITEM) != 0)
	{
		// if the item is not selected, clear previous
		// selections and select the item under cursor.
		if (!IsSelected(hItem))
		{
			ClearSelection();
			SelectItem(hItem);
		}
	}
	else
	{
		// clear previous selections.
		ClearSelection();
	}

	// call Default() so correct notification messages are
	// sent such as TVN_BEGINRDRAG.
	Default();
}

void CTreeCtrlEx::OnMouseMove(UINT nFlags, CPoint point)
{
	// If there is a select pending, check if cursor has moved so much away from the
	// down-click point that we should cancel the pending select and initiate
	// a drag/drop operation instead!

	if (m_hClickedItem)
	{
		CSize sizeMoved = m_ptClick - point;

		if (abs(sizeMoved.cx) > GetSystemMetrics(SM_CXDRAG) || abs(sizeMoved.cy) > GetSystemMetrics(SM_CYDRAG))
		{
			m_bSelectPending = FALSE;

			// Notify parent that he may begin drag operation
			// Since we have taken over OnLButtonDown(), the default handler doesn't
			// do the normal work when clicking an item, so we must provide our own
			// TVN_BEGINDRAG notification for the parent!

			CWnd* pWnd = GetParent();
			if (pWnd && !(GetStyle() & TVS_DISABLEDRAGDROP))
			{
				NMTREEVIEW tv = { nullptr };

				tv.hdr.hwndFrom = GetSafeHwnd();
				tv.hdr.idFrom = GetWindowLong(GetSafeHwnd(), GWL_ID);
				tv.hdr.code = TVN_BEGINDRAG;

				tv.itemNew.hItem = m_hClickedItem;
				tv.itemNew.state = GetItemState(m_hClickedItem, 0xffffffff);
				tv.itemNew.lParam = GetItemData(m_hClickedItem);

				tv.ptDrag.x = point.x;
				tv.ptDrag.y = point.y;

				pWnd->SendMessage(WM_NOTIFY, tv.hdr.idFrom, (LPARAM)&tv);
			}

			m_hClickedItem = nullptr;
		}
	}

	CTreeCtrl::OnMouseMove(nFlags, point);
}

void CTreeCtrlEx::SelectMultiple(HTREEITEM hClickedItem, UINT nFlags, CPoint point)
{
	// Start preparing an NM_TREEVIEW struct to send a notification after selection is done
	NMTREEVIEW tv = { nullptr };

	CWnd* pWnd = GetParent();

	HTREEITEM hOldItem = GetSelectedItem();
	if (hOldItem)
	{
		tv.itemOld.hItem = hOldItem;
		tv.itemOld.state = GetItemState(hOldItem, 0xffffffff);
		tv.itemOld.lParam = GetItemData(hOldItem);
		tv.itemOld.mask = TVIF_HANDLE | TVIF_STATE | TVIF_PARAM;
	}

	// Flag signaling that selection process is NOT complete.
	// (Will prohibit TVN_SELCHANGED from being sent to parent)
	m_bSelectionComplete = FALSE;

	// Action depends on whether the user holds down the Shift or Ctrl key
	if (nFlags & MK_SHIFT)
	{
		// Select from first selected item to the clicked item
		if (!m_hFirstSelectedItem)
			m_hFirstSelectedItem = GetSelectedItem();

		SelectItems(m_hFirstSelectedItem, hClickedItem);
	}
	else if (nFlags & MK_CONTROL)
	{
		// Find which item is currently selected
		HTREEITEM hSelectedItem = GetSelectedItem();

		// Is the clicked item already selected ?
		BOOL bIsClickedItemSelected = GetItemState(hClickedItem, TVIS_SELECTED) & TVIS_SELECTED;
		BOOL bIsSelectedItemSelected = FALSE;
		if (hSelectedItem)
			bIsSelectedItemSelected = GetItemState(hSelectedItem, TVIS_SELECTED) & TVIS_SELECTED;

		// Must synthesize a TVN_SELCHANGING notification
		if (pWnd)
		{
			tv.hdr.hwndFrom = GetSafeHwnd();
			tv.hdr.idFrom = GetWindowLong(GetSafeHwnd(), GWL_ID);
			tv.hdr.code = TVN_SELCHANGING;

			tv.itemNew.hItem = hClickedItem;
			tv.itemNew.state = GetItemState(hClickedItem, 0xffffffff);
			tv.itemNew.lParam = GetItemData(hClickedItem);

			tv.itemOld.hItem = nullptr;
			tv.itemOld.mask = 0;

			tv.action = TVC_BYMOUSE;

			tv.ptDrag.x = point.x;
			tv.ptDrag.y = point.y;

			pWnd->SendMessage(WM_NOTIFY, tv.hdr.idFrom, (LPARAM)&tv);
		}

		// If the previously selected item was selected, re-select it
		if (bIsSelectedItemSelected)
			SetItemState(hSelectedItem, TVIS_SELECTED, TVIS_SELECTED);

		// We want the newly selected item to toggle its selected state,
		// so unselect now if it was already selected before
		if (bIsClickedItemSelected)
		{
			SetItemState(hClickedItem, 0, TVIS_SELECTED);
		}
		else
		{
			SetItemState(hClickedItem, TVIS_SELECTED, TVIS_SELECTED);
		}

		// If the previously selected item was selected, re-select it
		if (bIsSelectedItemSelected && hSelectedItem != hClickedItem)
			SetItemState(hSelectedItem, TVIS_SELECTED, TVIS_SELECTED);

		// Store as first selected item (if not already stored)
		if (m_hFirstSelectedItem == nullptr)
			m_hFirstSelectedItem = hClickedItem;
	}
	else
	{
		// Clear selection of all "multiple selected" items first
		ClearSelection();

		// Then select the clicked item
		SelectItem(hClickedItem);
		SetItemState(hClickedItem, TVIS_SELECTED, TVIS_SELECTED);

		// Store as first selected item
		m_hFirstSelectedItem = hClickedItem;
	}

	// Selection process is now complete. Since we have 'eaten' the TVN_SELCHANGED
	// notification provided by Windows' treectrl, we must now produce one ourselves,
	// so that our parent gets to know about the change of selection.
	m_bSelectionComplete = TRUE;

	if (pWnd)
	{
		tv.hdr.hwndFrom = GetSafeHwnd();
		tv.hdr.idFrom = GetWindowLong(GetSafeHwnd(), GWL_ID);
		tv.hdr.code = TVN_SELCHANGED;

		tv.itemNew.hItem = m_hClickedItem;
		tv.itemNew.state = GetItemState(m_hClickedItem, 0xffffffff);
		tv.itemNew.lParam = GetItemData(m_hClickedItem);
		tv.itemNew.mask = TVIF_HANDLE | TVIF_STATE | TVIF_PARAM;

		tv.action = TVC_UNKNOWN;

		pWnd->SendMessage(WM_NOTIFY, tv.hdr.idFrom, (LPARAM)&tv);
	}
}

void CTreeCtrlEx::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CWnd* pWnd = GetParent();

	if (nChar == VK_NEXT || nChar == VK_PRIOR)
	{
		if (!(GetKeyState(VK_SHIFT) & 0x8000))
		{
			// User pressed Pg key without holding 'Shift':
			// Clear multiple selection (if multiple) and let base class do
			// normal selection work!
			if (GetSelectedCount() > 1)
				ClearSelection(TRUE);

			CTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
			m_hFirstSelectedItem = GetSelectedItem();
			return;
		}

		// Flag signaling that selection process is NOT complete.
		// (Will prohibit TVN_SELCHANGED from being sent to parent)
		m_bSelectionComplete = FALSE;

		// Let base class select the item
		CTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
		HTREEITEM hSelectedItem = GetSelectedItem();

		// Then select items in between
		SelectItems(m_hFirstSelectedItem, hSelectedItem);

		// Selection process is now complete. Since we have 'eaten' the TVN_SELCHANGED
		// notification provided by Windows' treectrl, we must now produce one ourselves,
		// so that our parent gets to know about the change of selection.
		m_bSelectionComplete = TRUE;

		if (pWnd)
		{
			NMTREEVIEW tv = { nullptr };

			tv.hdr.hwndFrom = GetSafeHwnd();
			tv.hdr.idFrom = GetWindowLong(GetSafeHwnd(), GWL_ID);
			tv.hdr.code = TVN_SELCHANGED;

			tv.itemNew.hItem = hSelectedItem;
			tv.itemNew.state = GetItemState(hSelectedItem, 0xffffffff);
			tv.itemNew.lParam = GetItemData(hSelectedItem);
			tv.itemNew.mask = TVIF_HANDLE | TVIF_STATE | TVIF_PARAM;

			tv.action = TVC_UNKNOWN;

			pWnd->SendMessage(WM_NOTIFY, tv.hdr.idFrom, (LPARAM)&tv);
		}
	}
	else if (nChar == VK_UP || nChar == VK_DOWN)
	{
		// Find which item is currently selected
		HTREEITEM hSelectedItem = GetSelectedItem();

		HTREEITEM hNextItem;
		if (nChar == VK_UP)
			hNextItem = GetPrevVisibleItem(hSelectedItem);
		else
			hNextItem = GetNextVisibleItem(hSelectedItem);

		if (!(GetKeyState(VK_SHIFT) & 0x8000))
		{
			// User pressed arrow key without holding 'Shift':
			// Clear multiple selection (if multiple) and let base class do
			// normal selection work!
			if (GetSelectedCount() > 1)
				ClearSelection(TRUE);

			if (hNextItem)
				CTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);

			m_hFirstSelectedItem = GetSelectedItem();
			return;
		}

		if (hNextItem)
		{
			// Flag signaling that selection process is NOT complete.
			// (Will prohibit TVN_SELCHANGED from being sent to parent)
			m_bSelectionComplete = FALSE;

			// If the next item is already selected, we assume user is
			// "moving back" in the selection, and thus we should clear
			// selection on the previous one
			BOOL bSelect = !(GetItemState(hNextItem, TVIS_SELECTED) & TVIS_SELECTED);

			// Select the next item (this will also deselect the previous one!)
			SelectItem(hNextItem);

			// Now, re-select the previously selected item
			if (bSelect || (!(GetItemState(hSelectedItem, TVIS_SELECTED) & TVIS_SELECTED)))
				SelectItems(m_hFirstSelectedItem, hNextItem);

			// Selection process is now complete. Since we have 'eaten' the TVN_SELCHANGED
			// notification provided by Windows' treectrl, we must now produce one ourselves,
			// so that our parent gets to know about the change of selection.
			m_bSelectionComplete = TRUE;

			if (pWnd)
			{
				NM_TREEVIEW tv{};

				tv.hdr.hwndFrom = GetSafeHwnd();
				tv.hdr.idFrom = GetWindowLong(GetSafeHwnd(), GWL_ID);
				tv.hdr.code = TVN_SELCHANGED;

				tv.itemNew.hItem = hNextItem;
				tv.itemNew.state = GetItemState(hNextItem, 0xffffffff);
				tv.itemNew.lParam = GetItemData(hNextItem);
				tv.itemNew.mask = TVIF_HANDLE | TVIF_STATE | TVIF_PARAM;

				tv.action = TVC_UNKNOWN;

				pWnd->SendMessage(WM_NOTIFY, tv.hdr.idFrom, (LPARAM)&tv);
			}
		}

		// Since the base class' OnKeyDown() isn't called in this case,
		// we must provide our own TVN_KEYDOWN notification to the parent

		CWnd* pWnd = GetParent();
		if (pWnd)
		{
			NMTVKEYDOWN tvk{};

			tvk.hdr.hwndFrom = GetSafeHwnd();
			tvk.hdr.idFrom = GetWindowLong(GetSafeHwnd(), GWL_ID);
			tvk.hdr.code = TVN_KEYDOWN;

			tvk.wVKey = nChar;
			tvk.flags = 0;

			pWnd->SendMessage(WM_NOTIFY, tvk.hdr.idFrom, (LPARAM)&tvk);
		}
	}
	else
	{
		// Behave normally
		CTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}

// Get number of selected items
UINT CTreeCtrlEx::GetSelectedCount() const
{
	// Only visible items should be selected!
	UINT uCount = 0;
	for (HTREEITEM hItem = GetRootItem(); hItem != nullptr; hItem = GetNextVisibleItem(hItem))
	{
		if (GetItemState(hItem, TVIS_SELECTED) & TVIS_SELECTED)
			uCount++;
	}

	return uCount;
}

HTREEITEM CTreeCtrlEx::GetPrevItem(HTREEITEM hItem) const
{
	HTREEITEM hti = nullptr;

	hti = GetPrevSiblingItem(hItem);
	if (hti == nullptr)
	{
		hti = GetParentItem(hItem);
	}
	else
	{
		hti = GetLastItem(hti);
	}

	return hti;
}

HTREEITEM CTreeCtrlEx::GetNextItem(HTREEITEM hItem) const
{
	HTREEITEM hti = nullptr;

	if (ItemHasChildren(hItem))
	{
		hti = GetChildItem(hItem);
	}

	if (hti == nullptr)
	{
		while ((hti = GetNextSiblingItem(hItem)) == nullptr)
		{
			if ((hItem = GetParentItem(hItem)) == nullptr)
				return nullptr;
		}
	}

	return hti;
}

HTREEITEM CTreeCtrlEx::GetNextItem(HTREEITEM hItem, UINT nCode) const
{
	ASSERT(::IsWindow(m_hWnd));
	return hItem ? GetNextItem(hItem, nCode) : nullptr;
}

HTREEITEM CTreeCtrlEx::GetLastItem(HTREEITEM hItem) const
{
	// Temporary used variable
	HTREEITEM htiNext;

	// Get the last item at the top level
	if (hItem == nullptr)
	{
		hItem = GetRootItem();

		htiNext = GetNextSiblingItem(hItem);
		while (htiNext != nullptr)
		{
			hItem = htiNext;
			htiNext = GetNextSiblingItem(htiNext);
		}
	}

	while (ItemHasChildren(hItem) != NULL)
	{
		// Find the last child of hItem
		htiNext = GetChildItem(hItem);
		while (htiNext != nullptr)
		{
			hItem = htiNext;
			htiNext = GetNextSiblingItem(htiNext);
		}
	}

	return hItem;
}

// Helpers to list out selected items. (Use similar to GetFirstVisibleItem(),
// GetNextVisibleItem() and GetPrevVisibleItem()!)
HTREEITEM CTreeCtrlEx::GetFirstSelectedItem() const
{
	for (HTREEITEM hItem = GetRootItem(); hItem != nullptr; hItem = GetNextVisibleItem(hItem))
	{
		if (GetItemState(hItem, TVIS_SELECTED) & TVIS_SELECTED)
			return hItem;
	}

	return nullptr;
}

HTREEITEM CTreeCtrlEx::GetLastSelectedItem() const
{
	HTREEITEM hLast = nullptr;
	for (HTREEITEM hItem = GetRootItem(); hItem != nullptr; hItem = GetNextVisibleItem(hItem))
	{
		if (GetItemState(hItem, TVIS_SELECTED) & TVIS_SELECTED)
			hLast = hItem;
	}

	return hLast;
}

HTREEITEM CTreeCtrlEx::GetNextSelectedItem(HTREEITEM hItem) const
{
	for (hItem = GetNextVisibleItem(hItem); hItem != nullptr; hItem = GetNextVisibleItem(hItem))
	{
		if (GetItemState(hItem, TVIS_SELECTED) & TVIS_SELECTED)
			return hItem;
	}

	return nullptr;
}

HTREEITEM CTreeCtrlEx::GetPrevSelectedItem(HTREEITEM hItem) const
{
	for (hItem = GetPrevVisibleItem(hItem); hItem != nullptr; hItem = GetPrevVisibleItem(hItem))
	{
		if (GetItemState(hItem, TVIS_SELECTED) & TVIS_SELECTED)
			return hItem;
	}

	return nullptr;
}

std::vector<HTREEITEM> CTreeCtrlEx::GetSelectedItems() const
{
	std::vector<HTREEITEM> selected;
	for (auto hItem = GetFirstSelectedItem(); hItem != nullptr; hItem = GetNextSelectedItem(hItem))
		selected.emplace_back(hItem);

	return std::move(selected);
}

// Select/unselect item without unselecting other items
BOOL CTreeCtrlEx::SelectItemEx(HTREEITEM hItem, BOOL bSelect/*=TRUE*/)
{
	HTREEITEM hSelItem = GetSelectedItem();

	if (hItem == hSelItem)
	{
		if (!bSelect)
		{
			SelectItem(nullptr);
			return TRUE;
		}

		return FALSE;
	}

	SelectItem(hItem);
	m_hFirstSelectedItem = hItem;

	// Reselect previous "real" selected item which was unselected byt SelectItem()
	if (hSelItem)
		SetItemState(hSelItem, TVIS_SELECTED, TVIS_SELECTED);

	return TRUE;
}

// Select visible items between specified 'from' and 'to' item (including these!)
// If the 'to' item is above the 'from' item, it traverses the tree in reverse
// direction. Selection on other items is cleared!
BOOL CTreeCtrlEx::SelectItems(HTREEITEM hFromItem, HTREEITEM hToItem)
{
	// Determine direction of selection
	// (see what item comes first in the tree)
	HTREEITEM hItem = GetRootItem();

	while (hItem && hItem != hFromItem && hItem != hToItem)
		hItem = GetNextVisibleItem(hItem);

	if (!hItem)
		return FALSE;   // Items not visible in tree

	BOOL bReverse = (hItem == hToItem);

	// "Really" select the 'to' item (which will deselect
	// the previously selected item)

	SelectItem(hToItem);

	// Go through all visible items again and select/unselect

	hItem = GetRootItem();
	BOOL bSelect = FALSE;

	while (hItem)
	{
		if (hItem == (bReverse ? hToItem : hFromItem))
			bSelect = TRUE;

		if (bSelect)
		{
			if (!(GetItemState(hItem, TVIS_SELECTED) & TVIS_SELECTED))
				SetItemState(hItem, TVIS_SELECTED, TVIS_SELECTED);
		}
		else
		{
			if (GetItemState(hItem, TVIS_SELECTED) & TVIS_SELECTED)
				SetItemState(hItem, 0, TVIS_SELECTED);
		}

		if (hItem == (bReverse ? hFromItem : hToItem))
			bSelect = FALSE;

		hItem = GetNextVisibleItem(hItem);
	}

	return TRUE;
}

// Clear selected state on all visible items
void CTreeCtrlEx::ClearSelection(BOOL bMultiOnly /*=FALSE*/)
{
	HTREEITEM hFirst = GetSelectedItem();
	for (HTREEITEM hItem = GetRootItem(); hItem != nullptr; hItem = GetNextVisibleItem(hItem))
	{
		if (hFirst != hItem && GetItemState(hItem, TVIS_SELECTED) & TVIS_SELECTED)
			SetItemState(hItem, 0, TVIS_SELECTED);
	}
}

// If a node is collapsed, we should clear selections of its child items
BOOL CTreeCtrlEx::OnItemexpanding(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	if (pNMTreeView->action == TVE_COLLAPSE || pNMTreeView->action == TVE_COLLAPSERESET)
	{
		HTREEITEM hItem = GetChildItem(pNMTreeView->itemNew.hItem);

		while (hItem)
		{
			if (GetItemState(hItem, TVIS_SELECTED) & TVIS_SELECTED)
				SetItemState(hItem, 0, TVIS_SELECTED);

			// Get the next node: First see if current node has a child
			HTREEITEM hNextItem = GetChildItem(hItem);
			if (!hNextItem)
			{
				// No child: Get next sibling item
				if (!(hNextItem = GetNextSiblingItem(hItem)))
				{
					HTREEITEM hParentItem = hItem;
					while (!hNextItem)
					{
						// No more children: Get parent
						if (!(hParentItem = GetParentItem(hParentItem)))
							break;

						// Quit when parent is the collapsed node
						// (Don't do anything to siblings of this)
						if (hParentItem == pNMTreeView->itemNew.hItem)
							break;

						// Get next sibling to parent
						hNextItem = GetNextSiblingItem(hParentItem);
					}

					// Quit when parent is the collapsed node
					if (hParentItem == pNMTreeView->itemNew.hItem)
						break;
				}
			}

			hItem = hNextItem;
		}
	}

	*pResult = 0;
	return FALSE;   // Allow parent to handle this notification as well
}

// Intercept TVN_SELCHANGED and pass it only to the parent window of the
// selection process is finished
BOOL CTreeCtrlEx::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	// Return TRUE if selection is not complete. This will prevent the
	// notification from being sent to parent.
	return m_bSelectionComplete == 0;
}

// Ensure the multiple selected items are drawn correctly when loosing/getting the focus
BOOL CTreeCtrlEx::OnSetfocus(NMHDR* pNMHDR, LRESULT* pResult)
{
	//'emulated' selected items will remain greyed
	// if application gets covered
	HTREEITEM hItem = GetFirstSelectedItem();
	while (hItem)
	{
		RECT rect;
		GetItemRect(hItem, &rect, TRUE);
		InvalidateRect(&rect);
		hItem = GetNextSelectedItem(hItem);
	}

	Default();
	*pResult = 0;
	return FALSE;
}

BOOL CTreeCtrlEx::OnKillfocus(NMHDR* pNMHDR, LRESULT* pResult)
{
	Default();
	//'emulated' selected items may not get
	// refreshed to grey
	HTREEITEM hItem = GetFirstSelectedItem();
	while (hItem)
	{
		RECT rect;
		GetItemRect(hItem, &rect, TRUE);
		InvalidateRect(&rect);
		hItem = GetNextSelectedItem(hItem);
	}

	*pResult = 0;
	return FALSE;
}

void CTreeCtrlEx::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// We stop label editing.
	m_bEditLabelPending = FALSE;
	CTreeCtrl::OnLButtonDblClk(nFlags, point);
}

void CTreeCtrlEx::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TCEX_EDITLABEL)
	{
		// Stop the timer.
		KillTimer(m_idTimer);

		// Invoke label editing.
		if (m_bEditLabelPending)
			EditLabel(GetSelectedItem());

		m_bEditLabelPending = FALSE;
		return;
	}

	CTreeCtrl::OnTimer(nIDEvent);
}

BOOL CTreeCtrlEx::OnDeleteItem(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	// Remove the tree item from the map.
	m_mapColorFont.RemoveKey(pNMTreeView->itemOld.hItem);
	*pResult = 0;

	return FALSE;
}

BOOL CTreeCtrlEx::CreateDragImageEx(CPoint ptDragPoint)
{
	if (GetSelectedCount() <= 0)
		return FALSE; // no row selected

	CRect rectSingle;
	CRect rectComplete(0, 0, 0, 0);

	HTREEITEM hItem;

	// Putting together the bounding rectangle
	for (hItem = GetFirstSelectedItem(); hItem; hItem = GetNextSelectedItem(hItem))
	{
		CImageList* pSingleImageList = CreateDragImage(hItem);

		// The image bounding rectangle is zero based - but has the correct size
		// GetItemRect correct in offset, but does not contain icon size
		IMAGEINFO ImageInfo;
		pSingleImageList->GetImageInfo(0, &ImageInfo);
		GetItemRect(hItem, rectSingle, TRUE);
		rectSingle.left = rectSingle.right - ImageInfo.rcImage.right;

		pSingleImageList->DeleteImageList();
		delete pSingleImageList;

		rectComplete.UnionRect(rectComplete, rectSingle);
	}

	CClientDC dcClient(this);
	CDC dcMem;
	CBitmap Bitmap;

	if (!dcMem.CreateCompatibleDC(&dcClient))
		return FALSE;

	if (!Bitmap.CreateCompatibleBitmap(&dcClient, rectComplete.Width(), rectComplete.Height()))
		return FALSE;

	CBitmap* pOldMemDCBitmap = dcMem.SelectObject(&Bitmap);

	COLORREF cMaskColor = RGB(0, 255, 0);
	dcMem.FillSolidRect(0, 0, rectComplete.Width(), rectComplete.Height(), cMaskColor);

	// Paint each DragImage in the DC
	for (hItem = GetFirstSelectedItem(); hItem; hItem = GetNextSelectedItem(hItem))
	{
		CImageList* pSingleImageList = CreateDragImage(hItem);

		if (pSingleImageList)
		{

			// The image bounding rectangle is zero based - but has the correct size
			// GetItemRect correct in offset, but does not contain icon size
			IMAGEINFO ImageInfo;
			pSingleImageList->GetImageInfo(0, &ImageInfo);
			GetItemRect(hItem, rectSingle, TRUE);
			rectSingle.left = rectSingle.right - ImageInfo.rcImage.right;

			pSingleImageList->Draw(&dcMem,
								   0,
								   CPoint(rectSingle.left - rectComplete.left, rectSingle.top - rectComplete.top),
								   ILD_TRANSPARENT);

			pSingleImageList->DeleteImageList();
			delete pSingleImageList;
		}
	}

	dcMem.SelectObject(pOldMemDCBitmap);


	if (m_CurrentDragImage.GetSafeHandle() != nullptr)
	{
		m_CurrentDragImage.DeleteImageList();
	}

	m_CurrentDragImage.Create(rectComplete.Width(), rectComplete.Height(), ILC_COLOR | ILC_MASK, 0, 1);

	// Green is used as mask color
	m_CurrentDragImage.Add(&Bitmap, cMaskColor);

	Bitmap.DeleteObject();

	m_ptHotSpot.x = ptDragPoint.x - rectComplete.left;
	m_ptHotSpot.y = ptDragPoint.y - rectComplete.top;

	return TRUE;
}

void CTreeCtrlEx::DeleteDragImageEx()
{
	m_CurrentDragImage.DeleteImageList();
}

void CTreeCtrlEx::ScrollUp()
{
	HTREEITEM hItem = GetFirstVisibleItem();
	HTREEITEM hPrevSibling = GetPrevSiblingItem(hItem);
	HTREEITEM hParent = GetParentItem(hItem);

	if (hItem)
	{
		if (hPrevSibling)
		{
			SelectSetFirstVisible(hPrevSibling);
		}
		else if (hParent)
		{
			SelectSetFirstVisible(hParent);
		}
	}
}

void CTreeCtrlEx::SetItemFont(HTREEITEM hItem, LOGFONT& logfont)
{
	CLRFONT cf;
	if (!m_mapColorFont.Lookup(hItem, cf))
	{
		cf.color = COLORREF_NULL;
	}

	cf.logfont = logfont;
	m_mapColorFont[hItem] = cf;
}

BOOL CTreeCtrlEx::GetItemFont(HTREEITEM hItem, LOGFONT* plogfont)
{
	CLRFONT cf;
	if (!m_mapColorFont.Lookup(hItem, cf))
	{
		return FALSE;
	}

	if (cf.logfont.lfFaceName[0] == _T('\0'))
	{
		return FALSE;
	}

	*plogfont = cf.logfont;

	return TRUE;
}

void CTreeCtrlEx::SetItemBold(HTREEITEM hItem, BOOL bBold)
{
	SetItemState(hItem, bBold ? TVIS_BOLD : 0, TVIS_BOLD);
}

BOOL CTreeCtrlEx::GetItemBold(HTREEITEM hItem)
{
	return GetItemState(hItem, TVIS_BOLD) & TVIS_BOLD;
}

void CTreeCtrlEx::SetItemColor(HTREEITEM hItem, COLORREF color)
{
	CLRFONT cf;
	m_mapColorFont.Lookup(hItem, cf);

	cf.color = color;
	m_mapColorFont[hItem] = cf;
	InvalidateRect(nullptr);
}

COLORREF CTreeCtrlEx::GetItemColor(HTREEITEM hItem)
{
	CLRFONT cf;
	if (m_mapColorFont.Lookup(hItem, cf))
		return cf.color;

	return COLORREF_NULL;
}

void CTreeCtrlEx::SetItemBackColor(HTREEITEM hItem, COLORREF color)
{
	CLRFONT cf;
	m_mapColorFont.Lookup(hItem, cf);

	cf.colorBack = color;
	m_mapColorFont[hItem] = cf;
	InvalidateRect(nullptr);
}

COLORREF CTreeCtrlEx::GetItemBackColor(HTREEITEM hItem)
{
	CLRFONT cf;
	if (m_mapColorFont.Lookup(hItem, cf))
		return cf.colorBack;

	return COLORREF_NULL;
}

void CTreeCtrlEx::OnPaint()
{
	CPaintDC dc(this);
	// Create a memory DC compatible with the paint DC
	CDC memDC;
	memDC.CreateCompatibleDC(&dc);

	CRect rcClip, rcClient;
	dc.GetClipBox(&rcClip);
	GetClientRect(&rcClient);

	// Select a compatible bitmap into the memory DC
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(&dc, rcClient.Width(), rcClient.Height());
	memDC.SelectObject(&bitmap);

	// Set clip region to be same as that in paint DC
	CRgn rgn;
	rgn.CreateRectRgnIndirect(&rcClip);
	memDC.SelectClipRgn(&rgn);
	rgn.DeleteObject();

	// First let the control do its default drawing.
	CWnd::DefWindowProc(WM_PAINT, (WPARAM)memDC.m_hDC, 0);

	HTREEITEM hItem = GetFirstVisibleItem();

	int n = GetVisibleCount() + 1;
	while (hItem && n--)
	{
		// Do not meddle with selected items or drop highlighted items
		UINT selflag = TVIS_DROPHILITED | TVIS_SELECTED;
		UINT state = GetItemState(hItem, selflag);
		BOOL root = GetParentItem(hItem) == nullptr;
		if (!(state & selflag))
		{
			COLORREF crItemBack = GetSysColor(COLOR_WINDOW);
			COLORREF crItemText = ::GetSysColor(COLOR_WINDOWTEXT);

			// No font specified, so use window font
			CFont* pFont = GetFont();
			LOGFONT logfont;
			pFont->GetLogFont(&logfont);

			CLRFONT cf;
			if (m_mapColorFont.Lookup(hItem, cf))
			{
				if (cf.color != COLORREF_NULL)
					crItemText = cf.color;

				if (cf.colorBack != COLORREF_NULL)
					crItemBack = cf.colorBack;

				if (cf.logfont.lfFaceName[0] != _T('\0'))
					logfont = cf.logfont;
			}

			if (GetItemBold(hItem))
				logfont.lfWeight = 700;

			CFont fontDC;
			fontDC.CreateFontIndirect(&logfont);
			CFont* pFontDC = memDC.SelectObject(&fontDC);

			const auto& sItem = GetItemText(hItem);

			CRect rect;
			GetItemRect(hItem, &rect, TRUE);
			memDC.SetBkColor(crItemBack);
			memDC.SetTextColor(crItemText);
			memDC.TextOut(rect.left + 2, rect.top + 1, sItem);
			memDC.SelectObject(pFontDC);
		}

		hItem = GetNextVisibleItem(hItem);
	}

	dc.BitBlt(rcClip.left, rcClip.top, rcClip.Width(), rcClip.Height(), &memDC, rcClip.left, rcClip.top, SRCCOPY);
}
