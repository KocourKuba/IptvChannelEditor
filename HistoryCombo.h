#pragma once

constexpr auto MAX_HISTORY_ITEMS = 50;

class CHistoryCombo : public CComboBox
{
	DECLARE_SERIAL(CHistoryCombo)

public:
	CHistoryCombo() = default;
	virtual ~CHistoryCombo() = default;

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
