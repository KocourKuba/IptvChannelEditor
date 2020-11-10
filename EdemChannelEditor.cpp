
// EdemChannelEditor.cpp : Defines the class behaviors for the application.
//

#include "StdAfx.h"
#include "framework.h"
#include <Shlwapi.h>

#include "EdemChannelEditor.h"
#include "EdemChannelEditorDlg.h"
#include "utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CEdemChannelEditorApp

BEGIN_MESSAGE_MAP(CEdemChannelEditorApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CEdemChannelEditorApp construction

CEdemChannelEditorApp::CEdemChannelEditorApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CEdemChannelEditorApp object

CEdemChannelEditorApp theApp;


// CEdemChannelEditorApp initialization

BOOL CEdemChannelEditorApp::InitInstance()
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

	CWinApp::InitInstance();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager* pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization

	m_pszAppName = _tcsdup(_T("Dune Edem TV Channel Editor"));
	SetRegistryKey(_T("Dune Edem TV Channel Editor"));

	CEdemChannelEditorDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	// Delete the shell manager created above.
	delete pShellManager;

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

CString CEdemChannelEditorApp::GetAppPath(LPCTSTR szSubFolder /*= nullptr*/)
{
	CString fileName;

	if (GetModuleFileName(theApp.m_hInstance, fileName.GetBuffer(_MAX_PATH), _MAX_PATH) != 0)
	{
		fileName.ReleaseBuffer();
		int pos = fileName.ReverseFind('\\');
		if(pos != -1)
			fileName.Truncate(pos + 1);
	}

	return fileName + szSubFolder;
}

BOOL CEdemChannelEditorApp::LoadImage(const CString& fullPath, CImage& image)
{
	// png size
#if 0
	int pixelX = 245;
	int pixelY = 140;
	static const int multiplier = 1000;
	CRect rc(0, 0, multiplier, multiplier);
	MapDialogRect(wnd.GetParent()->GetSafeHwnd(), rc);
	int dlX = pixelX * multiplier / rc.Width();
	int dlY = pixelY * multiplier / rc.Width();
	float ratio = (float)dlX / (float)dlY;
	int newdlY = 80.F / ratio;
#endif // 0

	HRESULT hr = E_FAIL;
	if (utils::CrackUrl(fullPath.GetString()))
	{
		std::vector<BYTE> data;
		if (utils::DownloadFile(fullPath.GetString(), data))
		{
			// Still not clear if this is making a copy internally
			CComPtr<IStream> stream(SHCreateMemStream((BYTE*)data.data(), data.size()));
			hr = image.Load(stream);
		}
	}
	else
	{
		hr = image.Load(fullPath);
	}

	return SUCCEEDED(hr);
}

void CEdemChannelEditorApp::SetImage(const CImage& image, CStatic& wnd)
{
	HBITMAP hImg = nullptr;
	if (image)
	{
		CRect rc;
		wnd.GetClientRect(rc);

		CImage resized;
		resized.Create(rc.Width(), rc.Height(), 32);
		HDC dcImage = resized.GetDC();
		SetStretchBltMode(dcImage, COLORONCOLOR);
		image.StretchBlt(dcImage, rc, SRCCOPY);
		// The next two lines test the image on a picture control.
		image.StretchBlt(wnd.GetDC()->m_hDC, rc, SRCCOPY);

		resized.ReleaseDC();
		hImg = (HBITMAP)resized.Detach();
	}

	HBITMAP hOld = wnd.SetBitmap(hImg);
	if (hOld)
		::DeleteObject(hOld);
}
