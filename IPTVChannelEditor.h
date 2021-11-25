
// EdemChannelEditor.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
#error "include 'StdAfx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "Config.h"

// CEdemChannelEditorApp:
// See EdemChannelEditor.cpp for the implementation of this class
//

class CIPTVChannelEditorApp : public CWinAppEx
{
	typedef struct
	{
		CString csLang;
		CString csPath;
		CString csSuffix;
	} LangStruct;

public:
	CIPTVChannelEditorApp();

	// Overrides
public:
	BOOL InitInstance() override;

	void FillLangMap();

	DECLARE_MESSAGE_MAP()

public:
	std::map<WORD, LangStruct> m_LangMap;
};

class CCommandLineInfoEx : public CCommandLineInfo
{
public:
	CCommandLineInfoEx() = default;
	void ParseParam(LPCTSTR szParam, BOOL bFlag, BOOL bLast) override;

public:
	BOOL m_bDev = FALSE;
	BOOL m_bMakeAll = FALSE;
};

std::wstring GetAppPath(LPCWSTR szSubFolder = nullptr);

bool PackPlugin(const StreamType plugin_type,
				const std::wstring& output_path,
				const std::wstring& lists_path,
				bool showMessage);

void SaveWindowPos(HWND hWnd, LPCTSTR name);
void RestoreWindowPos(HWND hWnd, LPCTSTR name);

extern CIPTVChannelEditorApp theApp;
