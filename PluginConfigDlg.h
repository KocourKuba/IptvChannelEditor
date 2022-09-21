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

#pragma once
#include "uri_stream.h"

class CPluginConfigDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPluginConfigDlg)

public:
	CPluginConfigDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CPluginConfigDlg() = default;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PLUGIN_CONFIG };
#endif

protected:
	BOOL OnInitDialog() override;
	BOOL DestroyWindow() override;
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedButtonEditConfig();
	afx_msg void OnBnClickedButtonLoadConfig();
	afx_msg void OnBnClickedButtonSaveConfig();
	afx_msg void OnBnClickedButtonEpgTest();

	afx_msg void OnCbnSelchangeComboStreamSubType();
	afx_msg void OnCbnSelchangeComboEpgType();
	afx_msg void OnCbnSelchangeComboPluginType();

	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);

private:
	void EnableControls(BOOL enable);
	void FillControlsCommon();
	void FillControlsStream();
	void FillControlsEpg();

public:
	PluginType m_plugin_type = PluginType::enCustom;

protected:
	CStatic m_wndDurationCaption;

	CEdit m_wndName;
	CEdit m_wndTitle;
	CEdit m_wndShortName;
	CEdit m_wndProviderUrl;
	CEdit m_wndPlaylistTemplate;
	CEdit m_wndParseTemplate;
	CEdit m_wndSubst;
	CEdit m_wndDuration;
	CEdit m_wndStreamTemplate;
	CEdit m_wndStreamArchiveTemplate;
	CEdit m_wndEpgUrl;
	CEdit m_wndEpgRoot;
	CEdit m_wndEpgName;
	CEdit m_wndEpgDesc;
	CEdit m_wndEpgStart;
	CEdit m_wndEpgEnd;
	CEdit m_wndDateFormat;
	CEdit m_wndEpgTimeFormat;
	CEdit m_wndEpgTimezone;
	CEdit m_wndSetID;

	CComboBox m_wndAccessType;
	CComboBox m_wndStreamSubType;
	CComboBox m_wndCatchupType;
	CComboBox m_wndEpgType;
	CComboBox m_wndPluginType;

	CButton m_wndLoadConf;
	CButton m_wndTest;

	CString m_Name;
	CString m_Title;
	CString m_ShortName;
	CString m_ProviderUrl;
	CString m_PlaylistTemplate;
	CString m_ParseTemplate;
	CString m_Subst;
	CString m_StreamTemplate;
	CString m_StreamArchiveTemplate;
	CString m_EpgUrl;
	CString m_EpgRoot;
	CString m_EpgName;
	CString m_EpgDesc;
	CString m_EpgStart;
	CString m_EpgEnd;
	CString m_EpgDateFormat;
	CString m_EpgTimeFormat;
	CString m_SetID;
	CString m_Token;

	COleDateTime m_Date;

	int m_Duration = 0;
	int m_EpgTimezone = 0;

private:
	std::unique_ptr<uri_stream> m_plugin;
	BOOL allowEdit = FALSE;
};
