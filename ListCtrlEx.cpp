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
#include "ListCtrlEx.h"
#include "AccountSettings.h"
#include "IPTVChannelEditor.h"

IMPLEMENT_DYNAMIC(CListCtrlEx, CListCtrl)

BEGIN_MESSAGE_MAP(CListCtrlEx, CListCtrl)
	ON_WM_DESTROY()                                                                            \
END_MESSAGE_MAP()

bool CListCtrlEx::BuildColumns(int nCols, int* nWidth, int* nColString)
{
	for (int i = 0; i < nCols; ++i)
	{
		if (InsertColumn(i, load_string_resource(nColString[i]).c_str(), LVCFMT_LEFT, nWidth[i], i) == -1)
		{
			return false;
		}
	}

	return true;
}

bool CListCtrlEx::BuildColumns(int nCols, int* nWidth, std::wstring* strColString)
{
	for (int i = 0; i < nCols; ++i)
	{
		if (InsertColumn(i, strColString[i].c_str(), LVCFMT_LEFT, nWidth[i], i) == -1)
		{
			return false;
		}
	}

	return true;
}

void CListCtrlEx::AutoSaveColumns(LPCTSTR lpszSection, LPCTSTR lpszDefault /*= nullptr*/)
{
	m_bAutoSave = true;

	m_strSection = lpszSection;

	if (lpszDefault != nullptr)
	{
		m_strDefault = lpszDefault;
	}

	LoadColumnWidths();
}

void CListCtrlEx::SaveColumnWidths()
{
	// if we are not in report mode return.
	if ((GetWindowLong(m_hWnd, GWL_STYLE) & LVS_TYPEMASK) != LVS_REPORT)
	{
		return;
	}

	// get a pointer to the header control.
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	if (pHeaderCtrl == nullptr)
	{
		return;
	}

	std::wstring strValue;
	for (int i = 0; i < pHeaderCtrl->GetItemCount(); ++i)
	{
		strValue += fmt::format(L"{:d},", GetColumnWidth(Header_OrderToIndex(pHeaderCtrl->m_hWnd, i)));;
	}

	GetConfig().set_string(true, m_strSection, strValue);
	GetConfig().SaveSettings();
}

void CListCtrlEx::LoadColumnWidths()
{
	// get the handle for the list control.
	HWND hWndList = GetSafeHwnd();
	if (!::IsWindow(hWndList))
	{
		return;
	}

	// get the style for list control.
	DWORD dwStyle = ::GetWindowLong(hWndList, GWL_STYLE);

	// if we are not in report mode return.
	if ((dwStyle & LVS_TYPEMASK) != LVS_REPORT)
	{
		return;
	}

	// get a pointer to the header control.
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	if (!::IsWindow(pHeaderCtrl->GetSafeHwnd()))
	{
		return;
	}

	// load the stored width for each column.
	for (int nCol = 0; nCol < pHeaderCtrl->GetItemCount(); ++nCol)
	{
		int nOrderCol = Header_OrderToIndex(pHeaderCtrl->m_hWnd, nCol);
		SetStoredWidth(nOrderCol);
	}
}

void CListCtrlEx::SetStoredWidth(int nCol)
{
	SetColumnWidth(nCol, GetStoredWidth(nCol));
}

int CListCtrlEx::GetStoredWidth(int nCol)
{
	// get the value from the registry.
	const auto& strValue = GetConfig().get_string(true, m_strSection, m_strDefault.c_str());

	if (!strValue.empty())
	{
		// extract the sub string to get the column width.
		CString strWidth;
		AfxExtractSubString(strWidth, strValue.c_str(), nCol, _T(','));

		if (!strWidth.IsEmpty())
		{
			// return the width from the registry.
			return _ttoi(strWidth);
		}
	}

	// value was not found, return default.
	return GetColumnWidth(nCol);
}

void CListCtrlEx::OnDestroy()
{
	if (m_bAutoSave)
	{
		SaveColumnWidths();
	}
}

