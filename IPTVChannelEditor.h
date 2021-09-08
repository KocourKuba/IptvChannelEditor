
// EdemChannelEditor.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'StdAfx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CEdemChannelEditorApp:
// See EdemChannelEditor.cpp for the implementation of this class
//

class CIPTVChannelEditorApp : public CWinAppEx
{
public:
	CIPTVChannelEditorApp();

// Overrides
public:
	BOOL InitInstance() override;

	std::wstring GetAppPath(LPCWSTR szSubFolder = nullptr);
	void SaveWindowPos(HWND hWnd, LPCTSTR name);
	void RestoreWindowPos(HWND hWnd, LPCTSTR name);

	DECLARE_MESSAGE_MAP()

private:
	BOOL m_devMode = FALSE;
};

class CCommandLineInfoEx : public CCommandLineInfo
{
public:
	CCommandLineInfoEx() = default;
	void ParseParam(LPCTSTR szParam, BOOL bFlag, BOOL bLast) override;

public:
	BOOL m_bDev = FALSE;
};

extern CIPTVChannelEditorApp theApp;
