
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

class CEdemChannelEditorApp : public CWinAppEx
{
public:
	CEdemChannelEditorApp();

// Overrides
public:
	BOOL InitInstance() override;

	std::wstring GetAppPath(LPCTSTR szSubFolder = nullptr);

	DECLARE_MESSAGE_MAP()
};

extern CEdemChannelEditorApp theApp;
