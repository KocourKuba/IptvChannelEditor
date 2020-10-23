
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

class CEdemChannelEditorApp : public CWinApp
{
public:
	CEdemChannelEditorApp();

// Overrides
public:
	BOOL InitInstance() override;

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CEdemChannelEditorApp theApp;
