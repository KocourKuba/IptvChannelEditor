/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2023): sharky72 (https://github.com/KocourKuba)

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
#include "MenuEdit.h"
#include "TooltipPropertyPage.h"
#include "plugin_config.h"

class CPluginConfigPageTV : public CTooltipPropertyPage
{
	DECLARE_DYNAMIC(CPluginConfigPageTV)

public:
	CPluginConfigPageTV();   // standard constructor
	virtual ~CPluginConfigPageTV() = default;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PLUGIN_CONFIG_TV };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	BOOL OnInitDialog() override;
	BOOL OnSetActive() override;

	void AssignMacros();
	void FillControls() override;

	void FillPlaylistSettings(size_t index);

	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedButtonEditTemplates();
	afx_msg void OnCbnSelchangeComboPlaylistTemplate();
	afx_msg void OnBnClickedButtonPlaylistShow();
	afx_msg void OnBnClickedButtonStreamRegexTest();
	afx_msg void OnBnClickedCheckMapTagToId();
	afx_msg void OnBnClickedCheckPerChannelToken();
	afx_msg void OnBnClickedCheckEpgIdFromId();

	afx_msg void OnCbnSelchangeComboStreamType();
	afx_msg void OnCbnSelchangeComboTags();
	afx_msg void OnCbnSelchangeComboCatchupType();

	afx_msg void OnEnChangeEditParsePattern();
	afx_msg void OnEnChangeEditPlaylistTemplate();
	afx_msg void OnEnChangeEditDuration();
	afx_msg void OnEnChangeEditStreamTemplate();
	afx_msg void OnEnChangeEditStreamArcTemplate();
	afx_msg void OnEnChangeEditCustomStreamArcTemplate();
	afx_msg void OnEnChangeEditDuneParams();

private:
	void UpdateControls();
	void FillControlsStream();
	StreamParameters& GetSupportedStream();

protected:

	CMenuEdit m_wndPlaylistTemplate;
	CMenuEdit m_wndParseStream;
	CEdit m_wndDuration;
	CMenuEdit m_wndStreamTemplate;
	CMenuEdit m_wndStreamArchiveTemplate;
	CMenuEdit m_wndCustomStreamArchiveTemplate;
	CMenuEdit m_wndDuneParams;

	CComboBox m_wndPlaylistTemplates;
	CComboBox m_wndStreamType;
	CComboBox m_wndCatchupType;
	CComboBox m_wndTags;

	CButton m_wndBtnEditTemplates;
	CButton m_wndChkPerChannelToken;
	CButton m_wndChkEpgIdFromID;
	CButton m_wndBtnPlaylistTest;
	CButton m_wndBtnStreamParseTest;
	CButton m_wndCheckMapTags;

	CString m_PlaylistTemplate;
	CString m_ParseStream;
	CString m_DuneParams;
	CString m_StreamTemplate;
	CString m_StreamArchiveTemplate;
	CString m_CustomStreamArchiveTemplate;

	int m_Duration = 0;
};
