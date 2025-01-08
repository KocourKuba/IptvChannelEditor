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

#pragma once

constexpr auto MAX_HISTORY_ITEMS = 50;

class CHistoryCombo : public CComboBox
{
	DECLARE_SERIAL(CHistoryCombo)

protected:
	BOOL PreTranslateMessage(MSG* pMsg) override;

	DECLARE_MESSAGE_MAP()

	afx_msg void OnEditUpdate();

public:
	void Serialize(CArchive& ar) override;

	/// <summary>
	/// Add string to the combobox if not already exist.
	/// If items more than MaxHistoryItems last items will be removed
	/// </summary>
	/// <param name="lpszString"></param>
	/// <returns></returns>
	int AddString(LPCTSTR lpszString);

	/// <summary>
	/// Add current text from edit to the combobox
	/// </summary>
	/// <param name="bIgnoreIfEmpty"></param>
	void StoreValue(BOOL bIgnoreIfEmpty = TRUE);

	/// <summary>
	/// Set maximum stored items
	/// </summary>
	/// <param name="nMaxItems"></param>
	void SetMaxHistoryItems(int nMaxItems);

	/// <summary>
	/// save the history to an archive object in binary format
	/// </summary>
	/// <param name="ar"></param>
	/// <param name="bAddCurrentItemToHistory"></param>
	void SaveHistory(CArchive& ar);

	/// <summary>
	/// load the history from an archive object in binary format
	/// </summary>
	/// <param name="ar"></param>
	void LoadHistory(CArchive& ar);

	/// <summary>
	// write the history to the CString
	/// </summary>
	/// <param name="csHistory"></param>
	/// <param name="lpszDelims">default \r\n</param>
	/// <returns>Result string</returns>
	CString SaveHistoryToText(LPCTSTR lpszDelims = _T("\r\n"));

	/// <summary>
	// load the history from the lpszHistory delimited by lpszDelims
	// set the current selection to lpszLastSelected if not nullptr
	/// </summary>
	/// <param name="csHistory"></param>
	/// <param name="bAddCurrentItemToHistory"></param>
	/// <param name="lpszDelims">by default "\r\n"</param>
	void LoadHistoryFromText(LPCTSTR lpszHistory, LPCTSTR lpszLastSelected = nullptr, LPCTSTR lpszDelims = _T("\r\n"));

	void SetAutoComplete(BOOL bAutoComplete = TRUE) { m_bAllowAutoComplete = bAutoComplete; }
	BOOL GetAutoComplete() { return m_bAllowAutoComplete; }

protected:
	BOOL m_bAllowAutoComplete = TRUE;
	BOOL m_bDoAutoComplete = TRUE;
	int m_nMaxHistoryItems = MAX_HISTORY_ITEMS;
};
