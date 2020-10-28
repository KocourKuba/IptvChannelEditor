
// EdemChannelEditor.cpp : Defines the class behaviors for the application.
//

#include "StdAfx.h"
#include "framework.h"
#include <winhttp.h>
#include <Shlwapi.h>

#include "EdemChannelEditor.h"
#include "EdemChannelEditorDlg.h"
#include "utils.h"

#pragma comment(lib, "Winhttp.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool CrackUrl(const std::wstring& url, std::wstring& host = std::wstring(), std::wstring& path = std::wstring())
{
	URL_COMPONENTS urlComp;
	SecureZeroMemory(&urlComp, sizeof(urlComp));
	urlComp.dwStructSize = sizeof(urlComp);

	// Set required component lengths to non-zero so that they are cracked.
	urlComp.dwHostNameLength = (DWORD)-1;
	urlComp.dwUrlPathLength = (DWORD)-1;


	if(::WinHttpCrackUrl(url.c_str(), (DWORD)url.size(), 0, &urlComp))
	{
		host.assign(urlComp.lpszHostName, urlComp.dwHostNameLength);
		path.assign(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);
		return true;
	}

	return false;
}

bool DownloadIconLogo(const std::wstring& url, std::vector<BYTE>& image)
{
	std::wstring host;
	std::wstring path;
	if (!CrackUrl(url, host, path))
		return false;

	// Use WinHttpOpen to obtain a session handle.
	HINTERNET hSession = WinHttpOpen(L"WinHTTP wget/1.0",
									 WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
									 WINHTTP_NO_PROXY_NAME,
									 WINHTTP_NO_PROXY_BYPASS, 0);

	::WinHttpSetTimeouts(hSession, 0, 10000, 10000, 10000);

	// Specify an HTTP server.
	HINTERNET hConnect = nullptr;
	if (hSession)
		hConnect = WinHttpConnect(hSession, host.c_str(), INTERNET_DEFAULT_HTTP_PORT, 0);


	// Create an HTTP request handle.
	HINTERNET hRequest = nullptr;
	if (hConnect)
		hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(),
									  nullptr,
									  WINHTTP_NO_REFERER,
									  nullptr,
									  NULL);

	// Send a request.
	BOOL bResults = FALSE;
	if (hRequest)
		bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);

	// End the request.
	if (bResults)
		bResults = WinHttpReceiveResponse(hRequest, NULL);

	DWORD dwSize = 0;
	do
	{
		if (!bResults)
		{
			TRACE("Error %d has occurred.\n", GetLastError());
			break;
		}
		// Check for available data.
		if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
		{
			TRACE("Error %u in WinHttpQueryDataAvailable.\n", GetLastError());
			break;
		}

		// Allocate space for the buffer.
		if (!dwSize) break;
		std::vector<BYTE> chunk(dwSize);

		DWORD dwDownloaded = 0;
		if (WinHttpReadData(hRequest, (LPVOID)chunk.data(), dwSize, &dwDownloaded))
		{
			chunk.resize(dwSize);
			image.insert(image.end(), chunk.begin(), chunk.end());
		}
		else
		{
			TRACE("Error %u in WinHttpReadData.\n", GetLastError());
		}
	} while (dwSize);

	// Close any open handles.
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);

	return !image.empty();
}


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
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

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

void CEdemChannelEditorApp::LoadImage(CStatic& wnd, const CString& fullPath)
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

	CImage image;
	HRESULT hr = E_FAIL;
	if (CrackUrl(fullPath.GetString()))
	{
		std::vector<BYTE> data;
		DownloadIconLogo(fullPath.GetString(), data);
		// Still not clear if this is making a copy internally
		CComPtr<IStream> stream(SHCreateMemStream((BYTE*)data.data(), data.size()));
		hr = image.Load(stream);
	}
	else
	{
		hr = image.Load(fullPath);
	}

	HBITMAP hImg = nullptr;
	if (SUCCEEDED(hr))
	{
		//CDC* screenDC = GetDC();

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
		//ReleaseDC(screenDC);

		hImg = (HBITMAP)resized.Detach();
	}

	HBITMAP hOld = wnd.SetBitmap(hImg);
	if (hOld)
		::DeleteObject(hOld);
}
