/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2022): sharky72 (https://github.com/KocourKuba)

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
#include "HistoryCombo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL(CHistoryCombo, CComboBox, 0)

BEGIN_MESSAGE_MAP(CHistoryCombo, CComboBox)
	ON_CONTROL_REFLECT(CBN_EDITUPDATE, &CHistoryCombo::OnEditUpdate)
END_MESSAGE_MAP()

void CHistoryCombo::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
		SaveHistory(ar);
	else
		LoadHistory(ar);
}

int CHistoryCombo::AddString(LPCTSTR lpszString)
{
	// don't add if exist
	int nRet = CComboBox::InsertString(0, lpszString);
	int nIndex = FindStringExact(0, lpszString);
	if (nIndex != CB_ERR && nIndex != 0)
	{
		DeleteString(nIndex);
	}

	// truncate list to m_nMaxHistoryItems
	int nNumItems = GetCount();
	for (int n = m_nMaxHistoryItems; n < nNumItems; n++)
	{
		DeleteString(m_nMaxHistoryItems);
	}

	SetCurSel(nRet);
	return nRet;
}

void CHistoryCombo::StoreValue(BOOL bIgnoreEmpty /*= TRUE*/)
{
	// add current value to the list
	CString sValue;
	GetWindowText(sValue);
	if (!bIgnoreEmpty || !sValue.IsEmpty())
	{
		AddString(sValue);
	}
}

void CHistoryCombo::SetMaxHistoryItems(int nMaxItems)
{
	m_nMaxHistoryItems = nMaxItems;

	// truncate list to m_nMaxHistoryItems
	int nNumItems = GetCount();
	for (int n = m_nMaxHistoryItems; n < nNumItems; n++)
	{
		DeleteString(m_nMaxHistoryItems);
	}
}

void CHistoryCombo::LoadHistory(CArchive& ar)
{
	// clear all items
	ResetContent();
	ASSERT(ar.IsLoading());

	WORD wItems;
	ar >> wItems;
	for (WORD i = 0; i < wItems; i++)
	{
		CString sText;
		ar >> sText;
		CComboBox::AddString(sText);
	}

	CString sText;
	ar >> sText;

	// select text
	int nIndex = FindStringExact(-1, sText);
	if (nIndex == CB_ERR && (GetStyle() & CBS_DROPDOWN))
		SetWindowText(sText);
	else
		SetCurSel(nIndex);
}

void CHistoryCombo::SaveHistory(CArchive& ar)
{
	ASSERT(ar.IsStoring());
	StoreValue();

	// write entries count
	int nItems = GetCount();
	ar << (WORD)nItems;

	for (int n = 0; n < nItems; n++)
	{
		CString sText;
		GetLBText(n, sText);
		ar << sText;
	}

	// add current text
	CString sText;
	GetWindowText(sText);
	ar << sText;
}

void CHistoryCombo::LoadHistoryFromText(LPCTSTR lpszHistory,
										LPCTSTR lpszLastSelected /* = nullptr*/,
										LPCTSTR lpszDelims /* = _T("\r\n")*/)
{
	ResetContent();

	if (lpszHistory == nullptr || *lpszHistory == '\0')
	{
		if (lpszLastSelected != nullptr && lpszLastSelected[0] != '\0')
		{
			AddString(lpszLastSelected);
		}
		return;
	}

	if (lpszDelims == nullptr || *lpszDelims == '\0')
	{
		lpszDelims = _T("\r\n");
	}

	LPTSTR lpszList = _tcsdup(lpszHistory);
	LPTSTR next_token1 = nullptr;
	LPTSTR lpszTok = _tcstok_s(lpszList, lpszDelims, &next_token1);
	while (lpszTok != nullptr)
	{
		CComboBox::AddString(lpszTok);
		lpszTok = _tcstok_s(nullptr, lpszDelims, &next_token1);
	}

	if (lpszLastSelected != nullptr)
	{
		int nIndex = FindStringExact(-1, lpszLastSelected);
		if (nIndex == CB_ERR && GetStyle() & CBS_DROPDOWN)
			SetWindowText(lpszLastSelected);
		else
			SetCurSel(nIndex);
	}

	if (lpszList != nullptr)
	{
		free(lpszList);
	}
}

CString CHistoryCombo::SaveHistoryToText(LPCTSTR lpszDelims /*= _T("\r\n")*/)
{
	StoreValue();

	if (lpszDelims == nullptr || lpszDelims[0] == '\0')
	{
		lpszDelims = _T("\r\n");
	}

	CString csHistory;
	int nCount = GetCount();
	for (int n = 0; n < nCount; n++)
	{
		CString csItem;
		GetLBText(n, csItem);
		if (!csHistory.IsEmpty())
		{
			csHistory += lpszDelims;
		}

		csHistory += csItem;
	}

	return csHistory;
}

BOOL CHistoryCombo::PreTranslateMessage(MSG* pMsg)
{
	if (m_bAllowAutoComplete)
	{
		// Need to check for backspace and delete.
		// To do not add back the text the user has tried to delete.

		if (pMsg->message == WM_KEYDOWN)
		{
			m_bDoAutoComplete = TRUE;

			int nVirtKey = (int)pMsg->wParam;
			if (nVirtKey == VK_DELETE || nVirtKey == VK_BACK)
			{
				m_bDoAutoComplete = FALSE;
			}
		}
	}

	return CComboBox::PreTranslateMessage(pMsg);
}

void CHistoryCombo::OnEditUpdate()
{
	if (!m_bAllowAutoComplete || !m_bDoAutoComplete)
	{
		// return if auto complete disabled
		return;
	}

	// Get the text in the edit box
	CString str;
	GetWindowText(str);
	int nLength = str.GetLength();

	// Currently selected range
	DWORD dwCurSel = GetEditSel();
	WORD dStart = LOWORD(dwCurSel);
	WORD dEnd = HIWORD(dwCurSel);

	if (SelectString(-1, str) == CB_ERR)
	{
		SetWindowText(str); // No text selected, so restore what was there before
		if (dwCurSel != CB_ERR)
		{
			SetEditSel(dStart, dEnd); // restore cursor position
		}
	}

	// Set the text selection as the additional text that we have added
	if (dEnd < nLength && dwCurSel != CB_ERR)
		SetEditSel(dStart, dEnd);
	else
		SetEditSel(nLength, -1);
}
