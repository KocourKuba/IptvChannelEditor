
// EdemChannelEditor.cpp : Defines the class behaviors for the application.
//

#include "StdAfx.h"
#include "framework.h"
#include <Shlwapi.h>
#include <filesystem>

#include "IPTVChannelEditor.h"
#include "IPTVChannelEditorDlg.h"
#include "utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CEdemChannelEditorApp

BEGIN_MESSAGE_MAP(CIPTVChannelEditorApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CEdemChannelEditorApp construction

CIPTVChannelEditorApp::CIPTVChannelEditorApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CEdemChannelEditorApp object

CIPTVChannelEditorApp theApp;


// CEdemChannelEditorApp initialization

BOOL CIPTVChannelEditorApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	//CShellManager* pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization

	m_pszAppName = _tcsdup(_T("Dune IPTV Channel Editor"));
	SetRegistryKey(_T("Editor"));

	InitContextMenuManager();

	CIPTVChannelEditorDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	// Delete the shell manager created above.
	//delete pShellManager;

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

std::wstring CIPTVChannelEditorApp::GetAppPath(LPCTSTR szSubFolder /*= nullptr*/)
{
	CString fileName;

	if (GetModuleFileName(m_hInstance, fileName.GetBuffer(_MAX_PATH), _MAX_PATH) != 0)
	{
		fileName.ReleaseBuffer();
		int pos = fileName.ReverseFind('\\');
		if(pos != -1)
			fileName.Truncate(pos + 1);
	}

	fileName += szSubFolder;

	return std::filesystem::absolute(fileName.GetString());
}
