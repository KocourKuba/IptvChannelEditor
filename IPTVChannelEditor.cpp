
// EdemChannelEditor.cpp : Defines the class behaviors for the application.
//

#include "StdAfx.h"
#include "IPTVChannelEditor.h"
#include "IPTVChannelEditorDlg.h"
#include "FileVersionInfo.h"
#include "IconCache.h"
#include "utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr auto REG_SETTINGS = _T("Settings");

void CCommandLineInfoEx::ParseParam(LPCTSTR szParam, BOOL bFlag, BOOL bLast)
{
	if (bFlag)
	{
		if (_tcsicmp(szParam, _T("Dev")) == 0)
		{
			m_bDev = TRUE;
			return;
		}
	}

	CCommandLineInfo::ParseParam(szParam, bFlag, bLast);
}

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
// TODO: call AfxInitRichEdit2() to initialize richedit2 library.\n"	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	AfxInitRichEdit2();

	CWinAppEx::InitInstance();

	AfxEnableControlContainer();

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

	m_pszAppName = _tcsdup(_T("Editor"));
	SetRegistryKey(_T("Dune IPTV Channel Editor"));

	InitContextMenuManager();

	CCommandLineInfoEx cmdInfo;
	ParseCommandLine(cmdInfo);
	m_devMode = cmdInfo.m_bDev;

	FillLangMap();
	int nLangCurrent = GetProfileInt(REG_SETTINGS, REG_LANGUAGE, 1033);

	if (auto pair = m_LangMap.find(nLangCurrent); pair != m_LangMap.cend())
	{
		if (nLangCurrent != 1033)
		{
			HMODULE hLib = LoadLibrary(pair->second.csPath);
			if (hLib != nullptr)
				AfxSetResourceHandle(hLib);
		}
	}

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
#ifdef _DEBUG
	GetIconCache().DestroyInstance();
#endif // _DEBUG

	return FALSE;
}

std::wstring CIPTVChannelEditorApp::GetAppPath(LPCWSTR szSubFolder /*= nullptr*/)
{
	CStringW fileName;

	if (GetModuleFileNameW(m_hInstance, fileName.GetBuffer(_MAX_PATH), _MAX_PATH) != 0)
	{
		fileName.ReleaseBuffer();
		int pos = fileName.ReverseFind('\\');
		if(pos != -1)
			fileName.Truncate(pos + 1);
	}

#ifdef _DEBUG
	fileName += L"..\\";
#else
	if (m_devMode)
	{
		fileName += L"..\\";
	}
#endif // _DEBUG

	fileName += szSubFolder;

	return std::filesystem::absolute(fileName.GetString());
}


void CIPTVChannelEditorApp::RestoreWindowPos(HWND hWnd, LPCTSTR name)
{
	WINDOWPLACEMENT wp = { 0 };
	UINT nSize = 0;
	WINDOWPLACEMENT* pwp = nullptr;
	if (!theApp.GetProfileBinary(REG_SETTINGS, name, (LPBYTE*)&pwp, &nSize))
		return;

	// Success
	::memcpy((void*)&wp, pwp, sizeof(wp));
	delete[] pwp; // free the buffer

	// Get a handle to the monitor
	HMONITOR hMonitor = ::MonitorFromPoint(CPoint(wp.rcNormalPosition.left, wp.rcNormalPosition.top), MONITOR_DEFAULTTONEAREST);

	// Get the monitor info
	MONITORINFO monInfo;
	monInfo.cbSize = sizeof(MONITORINFO);
	if (::GetMonitorInfo(hMonitor, &monInfo))
	{
		// Adjust for work area
		CRect rc = wp.rcNormalPosition;
		rc.OffsetRect(monInfo.rcWork.left - monInfo.rcMonitor.left, monInfo.rcWork.top - monInfo.rcMonitor.top);

		// Ensure top left point is on screen
		CRect rc_monitor(monInfo.rcWork);
		if (rc_monitor.PtInRect(rc.TopLeft()) == FALSE)
		{
			rc.OffsetRect(rc_monitor.TopLeft());
		}
		wp.rcNormalPosition = rc;
	}

	SetWindowPlacement(hWnd, &wp);
}

void CIPTVChannelEditorApp::SaveWindowPos(HWND hWnd, LPCTSTR name)
{
	// Get the window position
	WINDOWPLACEMENT wp;
	wp.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(hWnd, &wp);
	// Save the info
	theApp.WriteProfileBinary(REG_SETTINGS, name, (LPBYTE)&wp, sizeof(wp));
}

void CIPTVChannelEditorApp::FillLangMap()
{
	m_LangMap.clear();

	CStringW fileName;
	if (!GetModuleFileNameW(m_hInstance, fileName.GetBuffer(_MAX_PATH), _MAX_PATH) != 0)
		return;

	fileName.ReleaseBuffer();

	CFileVersionInfo cVer;
	cVer.Open(fileName);
	LANGID nExeTrans = cVer.GetCurLID();
	cVer.Close();

	LangStruct sLang;
	sLang.csLang = _T("English");
	sLang.csSuffix = _T("ENG");
	m_LangMap.emplace(nExeTrans, sLang);

	std::filesystem::path cFile(fileName.GetString());
	cFile.replace_filename(cFile.stem().native() + _T("???.dll"));

	//Берем первый файлик и хреначим его данные, если удастся...
	CFileFind cFind;
	BOOL bFound = cFind.FindFile(cFile.c_str());
	while (bFound)
	{
		bFound = cFind.FindNextFile();
		const auto& file = cFind.GetFilePath();
		CFileVersionInfo cVer;
		cVer.Open(file);
		LANGID nLibTrans = MAKELANGID(cVer.GetCurLID(), SUBLANG_DEFAULT);
		cVer.Close();

		HMODULE hRes = LoadLibrary(file);
		if (!hRes) continue;

		sLang.csLang.LoadString(hRes, IDS_LANGUAGE);
		sLang.csPath = file;
		sLang.csSuffix = cFind.GetFileTitle().Right(3);
		m_LangMap.emplace(nLibTrans, sLang);

		if (hRes != AfxGetResourceHandle())
		{
			::FreeLibrary(hRes);
		}
	}
}
