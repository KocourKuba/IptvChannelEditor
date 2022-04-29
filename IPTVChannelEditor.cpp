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

#include "pch.h"
#include "IPTVChannelEditor.h"
#include "IPTVChannelEditorDlg.h"
#include "IconCache.h"

#include "UtilsLib\FileVersionInfo.h"
#include "UtilsLib\inet_utils.h"

#include "7zip\SevenZipWrapper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace SevenZip;

constexpr auto PACK_PATH = L"{:s}_plugin\\";
constexpr auto PACK_DLL = L"7za.dll";

static LPCTSTR g_sz_Run_GUID = _T("Global\\IPTVChannelEditor.{E4DC62B5-45AD-47AA-A016-512BA5995352}");
static HANDLE g_hAppRunningMutex = nullptr;

void CCommandLineInfoEx::ParseParam(LPCTSTR szParam, BOOL bFlag, BOOL bLast)
{
	if (bFlag)
	{
		if (_tcsicmp(szParam, _T("Dev")) == 0)
		{
			PluginsConfig::DEV_PATH = LR"(..\)";
			PluginsConfig::PACK_DLL_PATH = LR"(dll\)";
			m_bDev = true;
		}
		else if (_tcsicmp(szParam, _T("MakeAll")) == 0)
		{
			m_bMakeAll = true;
		}
		else if (_tcsicmp(szParam, _T("MakePortable")) == 0)
		{
			m_bPortable = true;
		}
		else if (_tcsicmp(szParam, _T("RestorePortable")) == 0)
		{
			m_bRestoreReg = true;
		}
		else if (_tcsicmp(szParam, _T("NoEmbed")) == 0)
		{
			m_bNoEmbed = true;
		}
	}

#ifdef _DEBUG
	m_bDev = true;
#endif // _DEBUG

	if (m_bDev) //-V547
	{
		PluginsConfig::DEV_PATH = LR"(..\)";
		PluginsConfig::PACK_DLL_PATH = LR"(dll\)";
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
	// TODO: call AfxInitRichEdit2() to initialize richedit2 library."
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls = { 0 };
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	AfxInitRichEdit2();

	CWinAppEx::InitInstance();

	HANDLE hAppRunningMutex = OpenMutex(READ_CONTROL, FALSE, g_sz_Run_GUID);
	if (hAppRunningMutex)
	{
		AfxMessageBox(IDS_STRING_ALREADY_RUNNING, MB_OK | MB_ICONEXCLAMATION);
		CloseHandle(hAppRunningMutex);
		ExitProcess(0);
	}

	g_hAppRunningMutex = CreateMutex(nullptr, FALSE, g_sz_Run_GUID);

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

	GetConfig().LoadSettings();

	if (GetConfig().get_string(true, REG_OUTPUT_PATH).empty())
		GetConfig().set_string(true, REG_OUTPUT_PATH, L".\\");

	if (GetConfig().get_string(true, REG_LISTS_PATH).empty())
		GetConfig().set_string(true, REG_LISTS_PATH, L".\\playlists\\");

	CCommandLineInfoEx cmdInfo;
	ParseCommandLine(cmdInfo);
	if (cmdInfo.m_bPortable)
	{
		GetConfig().SaveSettingsToJson();
		return FALSE;
	}

	if (cmdInfo.m_bRestoreReg)
	{
		if (GetConfig().IsPortable())
			GetConfig().SaveSettingsToRegistry();
		else
			AfxMessageBox(IDS_STRING_ERR_NOT_PORTABLE, MB_ICONERROR | MB_OK);
	}

	if (cmdInfo.m_bMakeAll)
	{
		std::wstring output_path;
		if (cmdInfo.m_strFileName.IsEmpty())
			output_path = GetConfig().get_string(true, REG_OUTPUT_PATH);
		else
			output_path = cmdInfo.m_strFileName.GetString();

		output_path = std::filesystem::absolute(output_path);
		if (output_path.back() != '\\')
			output_path += '\\';

		const auto& lists_path = GetConfig().get_string(true, REG_LISTS_PATH);
		for (const auto& item : GetConfig().get_plugins_info())
		{
			if (!PackPlugin(item.type, output_path, lists_path, false, cmdInfo.m_bNoEmbed))
			{
				CString str;
				str.Format(IDS_STRING_ERR_FAILED_PACK_PLUGIN, item.title.c_str());
				if (IDNO == AfxMessageBox(str, MB_YESNO)) break;
			}
		}

		return FALSE;
	}

	FillLangMap();
	int nLangCurrent = GetConfig().get_int(true, REG_LANGUAGE);

	AfxSetResourceHandle(AfxGetInstanceHandle());
	if (auto pair = m_LangMap.find(nLangCurrent); pair != m_LangMap.cend())
	{
		if (nLangCurrent != 1033)
		{
			HMODULE hLib = LoadLibrary(pair->second.csPath);
			if (hLib != nullptr)
				AfxSetResourceHandle(hLib);
		}
	}

	// cleanup old files
	std::error_code err;
	std::filesystem::directory_iterator dir_iter(GetAppPath(), err);
	for (auto const& dir_entry : dir_iter)
	{
		const auto& path = dir_entry.path();
		if (path.extension() != _T(".bak")) continue;

		if (std::filesystem::is_regular_file(path))
			std::filesystem::remove(path, err);
		else if (std::filesystem::is_directory(path))
			std::filesystem::remove_all(path, err);
	}

	int freq = GetConfig().get_int(true, REG_UPDATE_FREQ, 3);
	if (freq && GetConfig().get_int64(true, REG_NEXT_UPDATE) < time(nullptr))
	{
		std::wstring cmd = L"check";
		if (RequestToUpdateServer(cmd) == 0)
		{
			if (IDYES == AfxMessageBox(IDS_STRING_UPDATE_AVAILABLE, MB_YESNO))
			{
				cmd = L"update";
				if (GetConfig().get_int(true, REG_UPDATE_PL))
					cmd += L" --optional";

				int err = RequestToUpdateServer(cmd);
				if (err == 0)
				{
					time_t next_check = time(nullptr) + (time_t)freq * 24 * 3600;
					GetConfig().set_int64(true, REG_NEXT_UPDATE, next_check);
					AfxMessageBox(IDS_STRING_UPDATE_DONE, MB_OK | MB_ICONINFORMATION);
					return FALSE;
				}

				CString csErr;
				csErr.Format(IDS_STRING_ERR_UPDATE, err);
				AfxMessageBox(csErr, MB_OK | MB_ICONERROR);
			}
		}

		time_t next_check = time(nullptr) + (time_t)freq * 24 * 3600;
		GetConfig().set_int64(true, REG_NEXT_UPDATE, next_check);
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

	if (g_hAppRunningMutex)
		::CloseHandle(g_hAppRunningMutex);

	return FALSE;
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

//////////////////////////////////////////////////////////////////////////

std::wstring GetAppPath(LPCWSTR szSubFolder /*= nullptr*/)
{
	CStringW fileName;

	if (GetModuleFileNameW(AfxGetInstanceHandle(), fileName.GetBuffer(_MAX_PATH), _MAX_PATH) != 0)
	{
		fileName.ReleaseBuffer();
		int pos = fileName.ReverseFind('\\');
		if (pos != -1)
			fileName.Truncate(pos + 1);
	}

	//fileName += PluginsConfig::DEV_PATH.c_str();
	if (szSubFolder)
		fileName += szSubFolder;

	return std::filesystem::absolute(fileName.GetString());
}

bool PackPlugin(const StreamType plugin_type,
				const std::wstring& output_path,
				const std::wstring& lists_path,
				bool showMessage,
				bool noEmbed /*= false*/)
{
	const auto& old_plugin_type = GetConfig().get_plugin_type();
	GetConfig().set_plugin_type(plugin_type);

	const auto& plugin_info = GetConfig().get_plugin_info();
	const auto& short_name_w = utils::utf8_to_utf16(plugin_info.short_name);
	const auto& packFolder = std::filesystem::temp_directory_path().wstring() + fmt::format(PACK_PATH, short_name_w);

	std::error_code err;
	// remove previous packed folder if exist
	std::filesystem::remove_all(packFolder, err);

	// copy entire plugin folder
	const auto& plugin_root = GetAppPath(utils::PLUGIN_ROOT);
	std::filesystem::copy(plugin_root, packFolder, std::filesystem::copy_options::recursive, err);

	// remove config's folder
	std::filesystem::remove_all(packFolder + L"configs", err);

	// remove cbilling_vod base class
	if (plugin_type != StreamType::enAntifriz && plugin_type != StreamType::enCbilling)
	{
		std::filesystem::remove(packFolder + L"cbilling_vod_impl.php", err);
	}

	// remove images for other plugins
	std::vector<std::string> to_remove;
	for (const auto& info : GetConfig().get_plugins_info())
	{
		if (info.short_name != plugin_info.short_name)
		{
			to_remove.emplace_back(fmt::format("bg_{:s}.jpg", info.short_name));
			to_remove.emplace_back(fmt::format("logo_{:s}.jpg", info.short_name));
		}
	}

	for (const auto& dir_entry : std::filesystem::directory_iterator{ packFolder + LR"(icons\)" })
	{
		if (std::find(to_remove.begin(), to_remove.end(), dir_entry.path().filename().string()) != to_remove.end())
		{
			std::filesystem::remove(dir_entry, err);
		}
	}

	// create plugin manifest
	std::string data;
	std::ifstream istream(plugin_root + L"dune_plugin.xml");
	data.assign(std::istreambuf_iterator<char>(istream), std::istreambuf_iterator<char>());

	utils::string_replace_inplace(data, "{plugin_name}", plugin_info.int_name.c_str());
	utils::string_replace_inplace(data, "{plugin_title}", utils::utf16_to_utf8(plugin_info.title).c_str());
	utils::string_replace_inplace(data, "{plugin_short_name}", plugin_info.short_name.c_str());
	utils::string_replace_inplace(data, "{plugin_version}", plugin_info.version.c_str());
	utils::string_replace_inplace(data, "{plugin_release_date}", RELEASEDATE);

	std::ofstream ostream(packFolder + L"dune_plugin.xml", std::ios::out | std::ios::binary);
	ostream << data;
	ostream.close();

	std::filesystem::copy_file(fmt::format(LR"({:s}configs\{:s}_config.php)", plugin_root, short_name_w),
							   fmt::format(L"{:s}{:s}_config.php", packFolder, short_name_w),
							   std::filesystem::copy_options::overwrite_existing, err);

	// copy channel lists
	const auto& playlistPath = fmt::format(L"{:s}{:s}\\", lists_path, short_name_w);
	std::filesystem::directory_iterator dir_iter(playlistPath, err);
	for (auto const& dir_entry : dir_iter)
	{
		const auto& path = dir_entry.path();
		if (path.extension() == L".xml")
		{
			std::filesystem::copy_file(path, packFolder + path.filename().c_str(), std::filesystem::copy_options::overwrite_existing, err);
			ASSERT(!err.value());
		}
	}

	// write setup file
	unsigned char smarker[3] = { 0xEF, 0xBB, 0xBF }; // UTF8 BOM
	std::ofstream os(packFolder + _T("plugin_type.php"), std::ios::out | std::ios::binary);
	os.write((const char*)smarker, sizeof(smarker));
	os << fmt::format("<?php\nrequire_once '{:s}_config.php';\n\nconst PLUGIN_TYPE = '{:s}PluginConfig';\n",
					  GetPluginName<char>(plugin_type),
					  GetPluginName<char>(plugin_type, true));
	os.close();

	if (!noEmbed && GetConfig().get_int(false, REG_EMBED_INFO))
	{
		try
		{
			nlohmann::json node;
			switch (plugin_type)
			{
				case StreamType::enEdem: // ott_key
					node["domain"] = utils::utf16_to_utf8(GetConfig().get_string(false, REG_DOMAIN));
					node["ott_key"] = utils::utf16_to_utf8(GetConfig().get_string(false, REG_TOKEN));
					node["vportal"] = utils::utf16_to_utf8(GetConfig().get_string(false, REG_VPORTAL));
					break;
				case StreamType::enFox: // login/pass
				case StreamType::enGlanz:
				case StreamType::enSharaclub:
				case StreamType::enSharaTV:
				case StreamType::enOneOtt:
				case StreamType::enVidok:
				case StreamType::enTVClub:
					node["login"] = utils::utf16_to_utf8(GetConfig().get_string(false, REG_LOGIN));
					node["password"] = utils::utf16_to_utf8(GetConfig().get_string(false, REG_PASSWORD));
					break;
				case StreamType::enAntifriz: // pin
				case StreamType::enItv:
				case StreamType::enOneCent:
				case StreamType::enOneUsd:
				case StreamType::enSharavoz:
				case StreamType::enVipLime:
				case StreamType::enTvTeam:
				case StreamType::enLightIptv:
				case StreamType::enCbilling:
				case StreamType::enOttclub:
				case StreamType::enIptvOnline:
				case StreamType::enShuraTV:
					node["password"] = utils::utf16_to_utf8(GetConfig().get_string(false, REG_PASSWORD));
					break;
				default:
					break;
			}

			if (!node.empty())
			{
				const auto& js = node.dump();
				utils::CBase64Coder enc;
				enc.Encode(js.c_str(), js.size(), ATL_BASE64_FLAG_NOCRLF | ATL_BASE64_FLAG_NOPAD);

				std::ofstream out_file(packFolder + L"account.dat");
				const auto& str = utils::generateRandomId(5) + enc.GetResultString();
				out_file.write(str.c_str(), str.size());
				out_file.close();
			}
		}
		catch (const nlohmann::json::out_of_range& ex)
		{
			// out of range errors may happen if provided sizes are excessive
			TRACE("out of range error: %s\n", ex.what());
		}
		catch (const nlohmann::detail::type_error& ex)
		{
			// type error
			TRACE("type error: %s\n", ex.what());
		}
		catch (...)
		{
			TRACE("unknown exception\n");
		}
	}

	// revert back to previous state
	GetConfig().set_plugin_type(old_plugin_type);

	// pack folder
	const auto& pack_dll = GetAppPath((PluginsConfig::DEV_PATH + PluginsConfig::PACK_DLL_PATH).c_str()) + PACK_DLL;
	SevenZipWrapper archiver(pack_dll);
	archiver.GetCompressor().SetCompressionFormat(CompressionFormat::Zip);
	bool res = archiver.GetCompressor().AddFiles(packFolder, _T("*.*"), true);
	if (!res)
	{
		AfxMessageBox(IDS_STRING_ERR_FILE_MISSING, MB_OK | MB_ICONSTOP);
		return false;
	}

	const auto& pluginFile = output_path + fmt::format(utils::DUNE_PLUGIN_NAME, short_name_w);

	res = archiver.CreateArchive(pluginFile);
	// remove temporary folder
	std::filesystem::remove_all(packFolder, err);
	if (!res)
	{
		if (showMessage)
		{
			std::filesystem::remove(pluginFile, err);
			CString msg;
			msg.Format(IDS_STRING_ERR_FAILED_PACK, pluginFile.c_str());
			AfxMessageBox(msg, MB_OK | MB_ICONSTOP);
		}
		return false;
	}

	if (showMessage)
	{
		CString msg;
		msg.Format(IDS_STRING_INFO_CREATE_SUCCESS, output_path.c_str());
		AfxMessageBox(msg, MB_OK);
	}

	return true;
}

void RestoreWindowPos(HWND hWnd, LPCTSTR name)
{
	const auto& bin = GetConfig().get_binary(true, name);
	if (bin.empty())
		return;

	// Success
	WINDOWPLACEMENT wp = { 0 };
	UINT nSize = 0;
	::memcpy((void*)&wp, bin.data(), sizeof(wp));

	// Get a handle to the monitor
	HMONITOR hMonitor = ::MonitorFromPoint(CPoint(wp.rcNormalPosition.left, wp.rcNormalPosition.top), MONITOR_DEFAULTTONEAREST);

	// Get the monitor info
	MONITORINFO monInfo{};
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

void SaveWindowPos(HWND hWnd, LPCTSTR name)
{
	// Get the window position
	WINDOWPLACEMENT wp{};
	wp.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(hWnd, &wp);
	// Save the info
	GetConfig().set_binary(true, name, (LPBYTE)&wp, sizeof(wp));
}

BOOL LoadImage(const std::wstring& fullPath, CImage& image)
{
	HRESULT hr = E_FAIL;
	std::wstring host;
	std::wstring path;
	WORD port = 0;
	if (utils::CrackUrl(fullPath, host, path, port))
	{
		std::vector<BYTE> data;
		if (utils::DownloadFile(fullPath, data))
		{
			// Still not clear if this is making a copy internally
			CComPtr<IStream> stream(SHCreateMemStream((BYTE*)data.data(), data.size()));
			hr = image.Load(stream);
		}
		else
		{
			hr = image.Load(fullPath.c_str());
		}
	}
	else
	{
		hr = image.Load(fullPath.c_str());
	}

	return SUCCEEDED(hr);
}

void SetImage(const CImage& image, CStatic& wnd)
{
	HBITMAP hImg = nullptr;
	if (image)
	{
		CRect rcDst;
		wnd.GetClientRect(rcDst);

		int srcWidth = image.GetWidth();
		int srcHeight = image.GetHeight();

		float Factor_X = (float)((float)rcDst.Width() / (float)srcWidth);
		float Factor_Y = (float)((float)rcDst.Height() / (float)srcHeight);

		int x = 0;
		int y = 0;
		if (Factor_X > Factor_Y)
		{
			x = (int)((rcDst.right - (int)((float)srcWidth * Factor_Y)) / 2);
			Factor_X = Factor_Y;
		}
		else
		{
			y = (int)((rcDst.bottom - (int)((float)srcHeight * Factor_X)) / 2);
			Factor_Y = Factor_X;
		}

		int Width = (int)(Factor_X * srcWidth);
		int Height = (int)(Factor_Y * srcHeight);

		CImage resized;
		resized.Create(rcDst.Width(), rcDst.Height(), 32);
		HDC dcImage = resized.GetDC();
		SetStretchBltMode(dcImage, COLORONCOLOR);
		image.StretchBlt(dcImage, x, y, Width, Height, 0, 0, srcWidth, srcHeight);
		image.StretchBlt(wnd.GetDC()->m_hDC, x, y, Width, Height, 0, 0, srcWidth, srcHeight);
		resized.ReleaseDC();
		hImg = (HBITMAP)resized.Detach();
	}

	HBITMAP hOld = wnd.SetBitmap(hImg);
	if (hOld)
		::DeleteObject(hOld);
}

int RequestToUpdateServer(const std::wstring& command)
{
	const auto& cur_ver = utils::utf8_to_utf16(STRPRODUCTVER, sizeof(STRPRODUCTVER));
	const auto& avail_ver = GetConfig().get_string(true, REG_AVAIL_UPDATE);

	do
	{
		STARTUPINFO si = { 0 };
		si.cb = sizeof(STARTUPINFO);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
		si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
		si.hStdInput = nullptr;
		si.hStdOutput = nullptr;

		PROCESS_INFORMATION pi = { nullptr };

		const auto& app = fmt::format(L"{:s}Updater.exe", GetAppPath());
		std::wstring csCommand = fmt::format(L"\"{:s}\" {:s}", app, command);	// argv[0] имя исполняемого файла
		BOOL bRunProcess = CreateProcessW(app.c_str(),		// 	__in_opt     LPCTSTR lpApplicationName
										 csCommand.data(),	// 	__inout_opt  LPTSTR lpCommandLine
										 nullptr,			// 	__in_opt     LPSECURITY_ATTRIBUTES lpProcessAttributes
										 nullptr,			// 	__in_opt     LPSECURITY_ATTRIBUTES lpThreadAttributes
										 TRUE,				// 	__in         BOOL bInheritHandles
										 CREATE_SUSPENDED,	// 	__in         DWORD dwCreationFlags
										 nullptr,			// 	__in_opt     LPVOID lpEnvironment
										 nullptr,			// 	__in_opt     LPCTSTR lpCurrentDirectory
										 &si,				// 	__in         LPSTARTUPINFO lpStartupInfo
										 &pi);				// 	__out        LPPROCESS_INFORMATION lpProcessInformation

		if (!bRunProcess)
		{
			TRACE("Error start process %d\n", GetLastError());
			return -1;
		}

		ResumeThread(pi.hThread);

		int nErrorCount = 0;
		DWORD dwExitCode = STILL_ACTIVE;
		uint64_t dwStart = utils::ChronoGetTickCount();
		BOOL bTimeout = FALSE;
		for (;;)
		{
			if (dwExitCode != STILL_ACTIVE)
			{
				break;
			}

			if (utils::CheckForTimeOut(dwStart, 60 * 1000))
			{
				bTimeout = TRUE;
				::TerminateProcess(pi.hProcess, 0);
				break;
			}

			if (!::GetExitCodeProcess(pi.hProcess, &dwExitCode))
			{
				nErrorCount++;
				if (nErrorCount > 10) break;
				continue;
			}
		}

		return dwExitCode;
	} while (false);

	return -1;
}
