/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2025): sharky72 (https://github.com/KocourKuba)

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
#pragma warning(push)
#pragma warning(disable:4091)
#include <dbghelp.h>
#pragma warning(pop)

#include "IPTVChannelEditor.h"
#include "IPTVChannelEditorDlg.h"
#include "PluginFactory.h"
#include "AccountSettings.h"
#include "Constants.h"

#include "UtilsLib\FileVersionInfo.h"
#include "UtilsLib\inet_utils.h"
#include "UtilsLib\md5.h"
#include "UtilsLib\crc32.h"

#include "BugTrap\Source\Client\BugTrap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace SevenZip;

constexpr auto PACK_PATH = L"{:s}_plugin\\";

#ifdef _DEBUG
std::wstring CIPTVChannelEditorApp::DEV_PATH = LR"(..\..\)";
#ifdef _WIN64
std::wstring CIPTVChannelEditorApp::PACK_DLL_PATH = LR"(dll64\)";
#else
std::wstring CIPTVChannelEditorApp::PACK_DLL_PATH = LR"(dll\)";
#endif //  _WIN64
#else
std::wstring CIPTVChannelEditorApp::DEV_PATH;
std::wstring CIPTVChannelEditorApp::PACK_DLL_PATH;
#endif // _DEBUG

static LPCTSTR g_sz_Run_GUID = _T("Global\\IPTVChannelEditor.{E4DC62B5-45AD-47AA-A016-512BA5995352}");
static HANDLE g_hAppRunningMutex = nullptr;
std::string g_szServerPath = "http://iptv.esalecrm.net";

static void SetupExceptionHandler()
{
	// Setup exception handler
	// http://www.debuginfo.com/articles/effminidumps.html

	// MaxiDump
	// DWORD dwDumpType = (MiniDumpWithFullMemory | MiniDumpWithFullMemoryInfo | MiniDumpWithHandleData | MiniDumpWithThreadInfo | MiniDumpWithUnloadedModules)
	// MidiDump
	DWORD dwDumpType = (MiniDumpWithPrivateReadWriteMemory
						| MiniDumpWithDataSegs
						| MiniDumpWithHandleData
						| MiniDumpWithFullMemoryInfo
						| MiniDumpWithThreadInfo
						| MiniDumpWithUnloadedModules);

	BT_SetAppName(std::format(L"IPTVChannelEditor_v{:d}_{:d}_{:d}", MAJOR, MINOR, BUILD).c_str());
	BT_SetFlags(BTF_DETAILEDMODE | BTF_LISTPROCESSES | BTF_ATTACHREPORT | BTF_SHOWADVANCEDUI | BTF_INTERCEPTSUEF);
	BT_SetActivityType(BTA_SHOWUI);
	BT_SetDumpType(dwDumpType);
	BT_SetReportFilePath(GetAppPath().c_str());
	const auto& export_path = std::filesystem::temp_directory_path().wstring() + L"\\export.reg";
	BT_ExportRegistryKey(export_path.c_str(), L"HKCU\\SOFTWARE\\Dune IPTV Channel Editor");
	BT_AddLogFile(export_path.c_str());
	BT_AddLogFile(GetAppPath(L"settings.cfg").c_str());
	// required for VS 2005 & 2008
	BT_InstallSehFilter();
}

void CCommandLineInfoEx::ParseParam(LPCTSTR szParam, BOOL bFlag, BOOL bLast)
{
	if (bFlag)
	{
		if (_tcsicmp(szParam, _T("Dev")) == 0)
		{
			m_bDev = true;
		}
		else if (_tcsicmp(szParam, _T("Make")) == 0)
		{
			m_bMake = true;
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
		else if (_tcsicmp(szParam, _T("NoCustom")) == 0)
		{
			m_bNoCustom = true;
		}
		else if (_tcsicmp(szParam, _T("CleanupRegistry")) == 0)
		{
			m_bCleanupReg = true;
		}
	}

#ifdef _DEBUG
	m_bDev = true;
#endif // _DEBUG

	if (m_bDev) //-V547
	{
		CIPTVChannelEditorApp::DEV_PATH = LR"(..\..\)";
#ifdef _WIN64
		CIPTVChannelEditorApp::PACK_DLL_PATH = LR"(dll64\)";
#else
		CIPTVChannelEditorApp::PACK_DLL_PATH = LR"(dll\)";
#endif //  _WIN64
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

#ifndef _DEBUG
	SetupExceptionHandler();
	BT_SetTerminate(); // set_terminate() must be called from every thread
#endif // _DEBUG

	CCommandLineInfoEx cmdInfo;
	ParseCommandLine(cmdInfo);
	m_bDev = cmdInfo.m_bDev;

	utils::Logger::getInstance().setLogName(GetAppPath() + L"IPTVChannelEditor.log");

	if (!GetPluginFactory().load_configs(m_bDev))
	{
		AfxMessageBox(IDS_STRING_ERR_CANT_LOAD_CONFIG, MB_OK | MB_ICONEXCLAMATION);
		ExitProcess(0);
	}

	GetConfig().LoadSettings();

	FillLangMap();
	int nLangCurrent = GetConfig().get_int(true, REG_LANGUAGE);

	AfxSetResourceHandle(AfxGetInstanceHandle());
	if (auto pair = m_LangMap.find(nLangCurrent); pair != m_LangMap.cend())
	{
		auto hLib = std::get<HMODULE>(pair->second);
		if (nLangCurrent != 1033 && hLib != nullptr)
		{
			AfxSetResourceHandle(hLib);
		}
	}

	if (!GetConfig().IsPortable())
	{
		HANDLE hAppRunningMutex = OpenMutex(READ_CONTROL, FALSE, g_sz_Run_GUID);
		if (hAppRunningMutex && !GetConfig().IsPortable())
		{
			AfxMessageBox(IDS_STRING_ALREADY_RUNNING, MB_OK | MB_ICONEXCLAMATION);
			CloseHandle(hAppRunningMutex);
			ExitProcess(0);
		}
		g_hAppRunningMutex = CreateMutex(nullptr, FALSE, g_sz_Run_GUID);
	}

	const auto& pack_dll = GetAppPath((CIPTVChannelEditorApp::PACK_DLL_PATH).c_str()) + PACK_DLL;
	if (!std::filesystem::exists(pack_dll))
	{
		AfxMessageBox(IDS_STRING_ERR_DLL_MISSING, MB_OK | MB_ICONSTOP);
		return false;
	}

	m_archiver.SetLibPath(pack_dll);

	// set default value
	if (GetConfig().get_chrono(true, REG_MAX_CACHE_TTL).count() < 1)
	{
		GetConfig().set_chrono(true, REG_MAX_CACHE_TTL, 24h);
	}

	// check and create required directories
	CheckAndCreateDirs(REG_OUTPUT_PATH, GetAppPath());
	CheckAndCreateDirs(REG_WEB_UPDATE_PATH, GetAppPath(L"WebUpdate\\"));
	CheckAndCreateDirs(REG_SAVE_SETTINGS_PATH, GetAppPath(L"Settings\\"));

	auto& extractor = m_archiver.GetExtractor();

	// prepare and unpack default picons
	const std::filesystem::path image_cache = CheckAndCreateDirs(REG_SAVE_IMAGE_PATH, GetAppPath(L"ImageCache\\"));
	if (   !std::filesystem::exists(image_cache / utils::CHANNELS_LOGO_PATH)
		|| !std::filesystem::exists(image_cache / utils::CATEGORIES_LOGO_PATH))
	{
		const auto& package = GetAppPath(utils::PICONS_PACKAGE);
		extractor.SetArchivePath(package);
		if (!extractor.ExtractArchive(image_cache.wstring()))
		{
			AfxMessageBox(load_string_resource_fmt(IDS_STRING_ERR_FAILED_UNPACK_PACKAGE, package, image_cache.wstring()).c_str(), MB_OK | MB_ICONSTOP);
			return false;
		}
	}

	// check and unpack default channels list
	const std::filesystem::path channels_dir = CheckAndCreateDirs(REG_LISTS_PATH, GetAppPath(utils::CHANNELS_LIST_PATH));
	if (std::filesystem::is_empty(channels_dir))
	{
		const auto& channels_pkg = GetAppPath(utils::CHANNELS_LIST_PACKAGE);
		if (std::filesystem::exists(channels_pkg))
		{
			extractor.SetArchivePath(channels_pkg);
			if (!extractor.ExtractArchive(channels_dir))
			{
				AfxMessageBox(load_string_resource_fmt(IDS_STRING_ERR_FAILED_UNPACK_PACKAGE, channels_pkg, channels_dir.wstring()).c_str(), MB_OK | MB_ICONSTOP);
			}
		}
	}

	//ConvertAccounts();

	if (cmdInfo.m_bCleanupReg)
	{
		RegDeleteTree(HKEY_CURRENT_USER, _T("SOFTWARE\\Dune IPTV Channel Editor"));

		if (GetConfig().IsPortable())
		{
			GetConfig().SaveSettingsToJson();
		}
		else
		{
			GetConfig().SaveSettingsToRegistry();
		}

		return FALSE;
	}

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

	if (cmdInfo.m_bMake)
	{
		std::wstring output_path;
		if (cmdInfo.m_strFileName.IsEmpty())
			output_path = GetConfig().get_string(true, REG_OUTPUT_PATH);
		else
			output_path = cmdInfo.m_strFileName.GetString();

		const auto& plugin_type = GetConfig().get_plugin_type();
		if (!PackPlugin(plugin_type, true, false, output_path, cmdInfo.m_bNoEmbed, cmdInfo.m_bNoCustom))
		{
			auto plugin = GetPluginFactory().create_plugin(plugin_type);
			if (plugin)
			{
				const auto& msg = load_string_resource_fmt(IDS_STRING_ERR_FAILED_PACK_PLUGIN, plugin->get_title());
				AfxMessageBox(msg.c_str(), MB_YESNO);
			}
		}

		return FALSE;
	}

	if (cmdInfo.m_bMakeAll)
	{
		std::wstring output_path;
		if (cmdInfo.m_strFileName.IsEmpty())
		{
			output_path = GetConfig().get_string(true, REG_OUTPUT_PATH);
		}
		else
		{
			output_path = cmdInfo.m_strFileName.GetString();
		}

		for (const auto& [key, value] : GetPluginFactory().get_all_configs())
		{
			if (value.get_custom()) continue;

			if (!PackPlugin(key, false, false, output_path, cmdInfo.m_bNoEmbed, cmdInfo.m_bNoCustom))
			{
				auto plugin = GetPluginFactory().create_plugin(key);
				if (plugin)
				{
					const auto& msg = load_string_resource_fmt(IDS_STRING_ERR_FAILED_PACK_PLUGIN, plugin->get_title());
					if (IDNO == AfxMessageBox(msg.c_str(), MB_YESNO)) break;
				}
			}
		}

		return FALSE;
	}

	int res = CheckPluginConsistency(m_bDev);
	if (res == 0)
	{
		return FALSE;
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
	if (res == 2 || (freq && GetConfig().get_int64(true, REG_NEXT_UPDATE) < time(nullptr)))
	{
		std::wstring cmd = L"check";
		if (RequestToUpdateServer(cmd) == 0)
		{
			if (IDYES == AfxMessageBox(IDS_STRING_UPDATE_AVAILABLE, MB_YESNO))
			{
				// clear cache before update.
				std::filesystem::path cache_file = std::filesystem::temp_directory_path().append(L"iptv_cache");
				std::error_code err;
				std::filesystem::remove_all(cache_file, err);

				cmd = L"update";
				if (GetConfig().get_int(true, REG_UPDATE_PL))
					cmd += L" --optional";

				const time_t next_check = time(nullptr) + (time_t)freq * 24 * 3600;
				GetConfig().set_int64(true, REG_NEXT_UPDATE, next_check);
				RequestToUpdateServer(cmd, false);
				return FALSE;
			}
		}

		const time_t next_check = time(nullptr) + (time_t)freq * 24 * 3600;
		GetConfig().set_int64(true, REG_NEXT_UPDATE, next_check);
	}

	auto pDlg = std::make_unique<CIPTVChannelEditorDlg>();
	m_pMainWnd = pDlg.get();
	const INT_PTR nResponse = pDlg->DoModal();
	if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	for (auto& lang : m_LangMap)
	{
		::FreeLibrary(std::get<HMODULE>(lang.second));
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

std::wstring CIPTVChannelEditorApp::CheckAndCreateDirs(const std::wstring& settings, const std::wstring& default_path)
{
	std::error_code err;
	auto dir = GetConfig().get_string(true, settings);
	if (dir.empty())
	{
		dir = default_path;
		GetConfig().set_string(true, settings, dir);
	}
	if (std::filesystem::create_directories(dir, err) == false && err.value() != 0)
	{
		LOG_PROTOCOL(L"Error create directory: " + dir);
	}

	return dir;
}

int CIPTVChannelEditorApp::CheckPluginConsistency(bool isDev)
{
	const auto& plugin_root = GetAppPath(utils::PLUGIN_ROOT, true);
	auto dir_status = std::filesystem::symlink_status(plugin_root).type();
	if (isDev || dir_status == std::filesystem::file_type::junction || dir_status == std::filesystem::file_type::symlink)
	{
		return 1;
	}

	if (std::filesystem::exists(plugin_root))
	{
		std::error_code err;
		std::filesystem::rename(plugin_root, plugin_root + L".del", err);
		if (err.value() != 0) {
			LOG_PROTOCOL(std::format(L"Unable to rename {:s} Error code: {:d}", plugin_root, err.value()));
			LOG_PROTOCOL(err.message());
		}
	}

	const auto& update_pkg = GetAppPath(utils::UPDATES_FOLDER) + utils::UPDATE_NAME;
	if (!std::filesystem::exists(update_pkg))
	{
		std::wstring cmd = L"download --force";
		if (RequestToUpdateServer(cmd, false) != 0)
		{
			AfxMessageBox(load_string_resource_fmt(IDS_STRING_ERR_FAILED_DOWNLOAD_PACKAGE, update_pkg).c_str(), MB_OK | MB_ICONSTOP);
			return 0;
		}
		Sleep(500);
	}

	// Parse the buffer using the xml file parsing library into doc

	if (!std::filesystem::exists(update_pkg))
	{
		AfxMessageBox(load_string_resource_fmt(IDS_STRING_ERR_FILE_MISSING, update_pkg).c_str(), MB_OK | MB_ICONSTOP);
		return 0;
	}

	std::ifstream file(update_pkg);
	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();
	std::string xml_data = buffer.str();

	auto doc = std::make_unique<rapidxml::xml_document<>>();
	try
	{
		doc->parse<rapidxml::parse_default>(xml_data.data());
	}
	catch (rapidxml::parse_error& ex)
	{
		std::wstring msg = load_string_resource(IDS_STRING_ERR_FILES_WRONG);
		msg += utils::utf8_to_utf16(std::format("\nIncorrect update info: parse error: {:s}", ex.what()));
		AfxMessageBox(msg.c_str(), MB_OK | MB_ICONSTOP);
		return 0;
	}

	auto info_node = doc->first_node("update_info");
	if (!info_node)
	{
		std::wstring msg = load_string_resource(IDS_STRING_ERR_FILES_WRONG);
		msg += L"\nIncorrect update info: update_info node missing";
		AfxMessageBox(msg.c_str(), MB_OK | MB_ICONSTOP);
		return 0;
	}

	const auto& version = rapidxml::get_value_string(info_node->first_attribute("version"));
	auto pkg_node = doc->first_node("package");
	if (!pkg_node)
	{
		std::wstring msg = load_string_resource(IDS_STRING_ERR_FILES_WRONG);
		msg += L"\nIncorrect update info: package node missing";
		AfxMessageBox(msg.c_str(), MB_OK | MB_ICONSTOP);
		return 0;
	}

	if (version != STRPRODUCTVER)
	{
		std::wstring msg = load_string_resource(IDS_STRING_ERR_FILES_WRONG);
		msg += utils::utf8_to_utf16(std::format("\ncurrent version {:s}\nexpected {:s}", version, STRPRODUCTVER));
		AfxMessageBox(msg.c_str(), MB_OK | MB_ICONSTOP);
		return 2;
	}

	// Iterate <tv_category> nodes
	for (auto file_node = pkg_node->first_node("file"); file_node != nullptr; file_node = file_node->next_sibling())
	{
		const auto& name = rapidxml::get_value_wstring(file_node->first_attribute("name"));
		if (name != utils::PLUGIN_PACKAGE && name != utils::PICONS_PACKAGE) continue;

		int crc = rapidxml::get_value_int(file_node->first_attribute("hash"));
		const auto& target = GetAppPath(name.c_str());
		if (!std::filesystem::exists(target) || file_crc32(target) != crc)
		{
			std::error_code err;
			std::filesystem::copy(GetAppPath(utils::UPDATES_FOLDER) + name, target, std::filesystem::copy_options::overwrite_existing, err);
			if (err.value() != 0)
			{
				const auto& msg = load_string_resource_fmt(IDS_STRING_ERR_COPY, err.value(), name, target);
				AfxMessageBox(msg.c_str(), MB_OK | MB_ICONSTOP);
				return 0;
			}
		}
	}

	return 1;
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

	m_LangMap.emplace(nExeTrans, std::make_tuple<HMODULE, std::wstring>(nullptr, L"English"));

	std::filesystem::path cFile(fileName.GetString());
	cFile.replace_filename(cFile.stem().native() + _T("???.dll"));

	CString csLang;
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

		csLang.LoadString(hRes, IDS_LANGUAGE);
		std::wstring lang(csLang.GetString());
		m_LangMap.emplace(nLibTrans, std::make_tuple(hRes, lang));
	}
}

bool CIPTVChannelEditorApp::PackPlugin(const std::string& plugin_type,
									   bool showMessage,
									   bool make_web_update /*= false*/,
									   std::wstring output_path /*= L""*/,
									   bool noEmbed /*= false*/,
									   bool noCustom /*= false*/)
{
	const std::wstring& pack_dll = GetAppPath((CIPTVChannelEditorApp::PACK_DLL_PATH).c_str()) + PACK_DLL;
	if (!std::filesystem::exists(pack_dll))
	{
		if (showMessage)
		{
			AfxMessageBox(IDS_STRING_ERR_DLL_MISSING, MB_OK | MB_ICONSTOP);
		}
		else
		{
			LOG_PROTOCOL(load_string_resource(IDS_STRING_ERR_DLL_MISSING));
		}

		return false;
	}

	if (output_path.empty())
	{
		output_path = GetConfig().get_string(true, REG_OUTPUT_PATH);
	}

	output_path = std::filesystem::absolute(output_path);
	if (output_path.back() != '\\')
	{
		output_path += '\\';
	}

	std::error_code err;
	std::filesystem::create_directory(output_path, err);

	const auto& old_plugin_type = GetConfig().get_plugin_type();
	GetConfig().set_plugin_type(plugin_type);

	const auto& selected = GetConfig().get_int(false, REG_ACTIVE_ACCOUNT);
	if (selected == -1)
	{
		// revert back to previous state
		GetConfig().set_plugin_type(old_plugin_type);
		LOG_PROTOCOL("No active account selected");
		return false;
	}

	auto plugin = GetPluginFactory().create_plugin(plugin_type);

	const auto& all_credentials = GetConfig().LoadCredentials();

	if (selected >= (int)all_credentials.size())
	{
		// revert back to previous state
		GetConfig().set_plugin_type(old_plugin_type);
		LOG_PROTOCOL("Incorrect credentials selected");
		return false;
	}

	const auto& cred = all_credentials[selected];

	// load plugin settings
	plugin->load_plugin_parameters(utils::utf8_to_utf16(cred.config), plugin->get_internal_name());
	plugin->configure_plugin();

	COleDateTime cur_dt = COleDateTime::GetCurrentTime();
	const auto& date_string = std::format("{:d}{:02d}{:02d}{:02d}", cur_dt.GetYear(), cur_dt.GetMonth(), cur_dt.GetDay(), cur_dt.GetHour());
	std::string version_index;
	if (cred.custom_increment && !cred.version_id.empty())
	{
		version_index = cred.version_id;
	}
	else
	{
		version_index = date_string;
	}

	if (make_web_update && !cred.use_dropbox && (cred.update_url.empty() || cred.update_package_url.empty()))
	{
		// revert back to previous state
		GetConfig().set_plugin_type(old_plugin_type);
		if (showMessage)
		{
			AfxMessageBox(IDS_STRING_ERR_SETTINGS_MISSING, MB_OK | MB_ICONSTOP);
		}
		else
		{
			LOG_PROTOCOL(load_string_resource(IDS_STRING_ERR_SETTINGS_MISSING));
		}
		return false;
	}

	// collect plugin channels list;
	const std::filesystem::path channelsListPath = GetConfig().get_string(true, REG_LISTS_PATH) + plugin->get_internal_name();
	std::map<std::string, std::string> channels_list;
	std::filesystem::directory_iterator dir_iter(channelsListPath, err);
	for (auto const& dir_entry : dir_iter)
	{
		const auto& fname = dir_entry.path().filename();
		const auto& channels = fname.string();
		if (fname.extension() == L".xml"
			&& (noCustom || std::find(cred.ch_list.begin(), cred.ch_list.end(), channels) != cred.ch_list.end()))
		{
			std::string link;
			if (const auto& pair = cred.m_direct_links.find(channels); pair != cred.m_direct_links.end())
			{
				link = pair->second;
			}
			channels_list.emplace(channels, link);
		}
	}

	if (channels_list.empty())
	{
		if (showMessage)
		{
			AfxMessageBox(IDS_STRING_ERR_NO_CHANNELS_SEL, MB_OK | MB_ICONSTOP);
		}
		else
		{
			LOG_PROTOCOL(load_string_resource(IDS_STRING_ERR_NO_CHANNELS_SEL));
		}
		return false;
	}

	constexpr auto default_copy = std::filesystem::copy_options::none;
	constexpr auto recursive_copy = std::filesystem::copy_options::recursive;
	constexpr auto overwrite = std::filesystem::copy_options::overwrite_existing;
	constexpr auto skip = std::filesystem::copy_options::skip_existing;

	// extract plugin package
	const auto& plugin_dir = utils::PLUGIN_ROOT;
	const auto& plugin_root = std::filesystem::temp_directory_path() / utils::PLUGIN_ROOT;
	auto dir_status = std::filesystem::symlink_status(plugin_dir).type();
#ifdef _DEBUG
	if (true)
#else
	if (dir_status == std::filesystem::file_type::junction || dir_status == std::filesystem::file_type::symlink)
#endif // _DEBUG
	{
		std::filesystem::copy(plugin_dir, plugin_root, recursive_copy | overwrite, err);
		if (err.value() != 0)
		{
			const auto& msg = load_string_resource_fmt(IDS_STRING_ERR_COPY, err.value(), plugin_dir, plugin_root.wstring());
			if (showMessage)
			{
				AfxMessageBox(msg.c_str(), MB_OK | MB_ICONSTOP);
			}
			else
			{
				LOG_PROTOCOL(msg);
			}
			return false;
		}
	}
	else
	{
		auto& extractor = m_archiver.GetExtractor();
		const auto& plugin_package = GetAppPath(utils::PLUGIN_PACKAGE);
		extractor.SetArchivePath(plugin_package);
		extractor.DetectCompressionFormat();
		if (!extractor.ExtractArchive(plugin_root))
		{
			const auto& msg = load_string_resource_fmt(IDS_STRING_ERR_FAILED_UNPACK_PACKAGE, plugin_package, plugin_root.wstring());
			if (showMessage)
			{
				AfxMessageBox(msg.c_str(), MB_OK | MB_ICONSTOP);
			}
			else
			{
				LOG_PROTOCOL(msg);
			}
			return false;
		}
	}

	const auto& packFolder = std::filesystem::temp_directory_path() / std::format(PACK_PATH, plugin->get_internal_name());
	std::filesystem::remove_all(packFolder, err);

	std::filesystem::copy(plugin_root, packFolder, default_copy, err);
	std::filesystem::copy(plugin_root / L"lib", packFolder / L"lib", recursive_copy, err);
	std::filesystem::copy(plugin_root / L"img", packFolder / L"img", recursive_copy, err);
	std::filesystem::copy(plugin_root / L"translations", packFolder / L"translations", recursive_copy, err);

	const std::filesystem::path image_cache_path = GetConfig().get_string(true, REG_SAVE_IMAGE_PATH);
	for (const auto& ch_list : channels_list)
	{
		const auto& channel_path = channelsListPath / ch_list.first;
		std::ifstream stream(channel_path.wstring());
		if (!stream.good()) continue;

		std::string line;
		while (std::getline(stream, line))
		{
			const auto& posStart = line.find("<icon_url>");
			if (posStart == std::string::npos) continue;
			const auto& posEnd = line.find("</icon_url>", posStart + 10);
			if (posEnd == std::string::npos) continue;

			if (line.substr(posStart + 10, 14) != utils::PLUGIN_SCHEME_A) continue;

			const auto& url = line.substr(posStart + 24, posEnd - posStart - 24);

			std::filesystem::create_directories((packFolder / url).parent_path().native(), err);
			std::filesystem::copy(image_cache_path / url, packFolder / url, skip, err);
			if (err.value() != 0)
			{
				LOG_PROTOCOL(std::format(L"error copy {:s}", (image_cache_path / url).native()));
			}
		}
	}

	const std::filesystem::path www_path = LR"(www\cgi-bin\)";
	const std::filesystem::path bin_path = LR"(bin\)";
	const std::filesystem::path arm_path = LR"(.platform.87xx\bin\)";

	static std::map<std::filesystem::path, std::vector<std::wstring>> mandatory =
	{
		{
			bin_path,
			{
				L"cgi_config.php",
				L"cgi_wrapper.sh",
				L"curl.864x",
				L"curl.865x",
				L"curl.867x",
				L"curl.87xx",
				L"index_epg.php",
				L"php-cgi",
				L"php.ini",
				L"media_check.sh",
				L"update_suppliers.sh"
			}
		},
		{
			www_path,
			{
				L"updater.sh",
				L"channels",
				L"epg"
			}
		},
		{
			arm_path,
			{
				L"php-cgi"
			}
		}
	};

	// copy mandatory files
	for (const auto& [key, value] : mandatory)
	{
		std::filesystem::create_directories(packFolder / key, err);

		for (const auto& path : value)
		{
			const auto& file = key / path;
			const auto& source = plugin_root / file;
			const auto& target = packFolder / file;
			std::filesystem::copy_file(source, target, default_copy, err);
			if (err.value() != 0)
			{
				const auto& msg = load_string_resource_fmt(IDS_STRING_ERR_COPY, err.value(), source.wstring(), target.wstring());
				if (showMessage)
				{
					AfxMessageBox(msg.c_str(), MB_OK | MB_ICONSTOP);
				}
				else
				{
					LOG_PROTOCOL(msg);
				}
				return false;
			}
		}
	}

	// copy user files
	for (const auto& item : plugin->get_files_list())
	{
		const auto& file = item.get_name();
		const auto& source = plugin_root / file;
		const auto& target = packFolder / file;
		std::filesystem::copy_file(source, target, default_copy, err);
		if (err.value() != 0)
		{
			const auto& msg = load_string_resource_fmt(IDS_STRING_ERR_COPY, err.value(), source.wstring(), target.wstring());
			if (showMessage)
			{
				AfxMessageBox(msg.c_str(), MB_OK | MB_ICONSTOP);
			}
			else
			{
				LOG_PROTOCOL(msg);
			}
			return false;
		}
	}


	// copy logo and background
	std::filesystem::path plugin_logo;
	std::filesystem::path plugin_bgnd;
	if (!noCustom)
	{
		if (cred.custom_logo && !cred.logo.empty())
		{
			plugin_logo = utils::utf8_to_utf16(cred.logo);
		}

		if (cred.custom_background && !cred.background.empty())
		{
			plugin_bgnd = utils::utf8_to_utf16(cred.background);
		}
	}

	if (plugin_logo.empty())
		plugin_logo = std::format(LR"({:s}plugins_image\logo_{:s}.png)", plugin_root.wstring(), plugin->get_internal_name());
	else if (!plugin_logo.has_parent_path())
		plugin_logo = std::format(LR"({:s}plugins_image\{:s})", plugin_root.wstring(), plugin_logo.wstring());

	if (plugin_bgnd.empty())
		plugin_bgnd = std::format(LR"({:s}plugins_image\bg_{:s}.png)", plugin_root.wstring(), plugin->get_internal_name());
	else if (!plugin_bgnd.has_parent_path())
		plugin_bgnd = std::format(LR"({:s}plugins_image\{:s})", plugin_root.wstring(), plugin_bgnd.wstring());

	const auto& packFolderIcons = packFolder / LR"(img\)";
	std::filesystem::copy(plugin_logo, packFolderIcons, err);
	std::filesystem::copy(plugin_bgnd, packFolderIcons, err);

	if (!std::filesystem::exists(packFolderIcons / plugin_logo.filename()))
	{
		plugin_logo = "default_logo.png";
	}

	if (!std::filesystem::exists(packFolderIcons / plugin_bgnd.filename()))
	{
		plugin_bgnd = "default_bg.png";
	}

	const auto& logo_subst = utils::utf16_to_utf8(std::format(L"{:s}img/{:s}", utils::PLUGIN_SCHEME, plugin_logo.filename().wstring()));
	const auto& bg_subst = utils::utf16_to_utf8(std::format(L"{:s}img/{:s}", utils::PLUGIN_SCHEME, plugin_bgnd.filename().wstring()));

	// save config
	utils::CBase64Coder enc;
	enc.Encode(g_szServerPath.c_str(), 0, ATL_BASE64_FLAG_NOCRLF | ATL_BASE64_FLAG_NOPAD);

	int idx = GetConfig().get_int(false, REG_PLAYLIST_TYPE);
	plugin->set_playlist_idx(idx);
	plugin->set_dev_path(enc.GetResultAsString());
	plugin->save_plugin_parameters(std::format(L"{:s}config.json", packFolder.wstring()), plugin->get_internal_name(), true);

	// create plugin manifest
	std::string config_data;
	std::ifstream istream(plugin_root / utils::PLUGIN_MANIFEST);
	config_data.assign(std::istreambuf_iterator<char>(istream), std::istreambuf_iterator<char>());

	std::string plugin_caption;
	if (!cred.custom_caption || cred.caption.empty())
		plugin_caption = utils::utf16_to_utf8(plugin->get_title());
	else
		plugin_caption = cred.caption;

	// preprocess common values
	utils::string_replace_inplace(config_data, "{name}", plugin->get_name().c_str());
	utils::string_replace_inplace(config_data, "{plugin_caption}", plugin_caption.c_str());
	utils::string_replace_inplace(config_data, "{plugin_logo}", logo_subst.c_str());
	utils::string_replace_inplace(config_data, "{plugin_bg}", bg_subst.c_str());

	// copy plugin config class if they exist
	const auto& src_config_file = utils::utf8_to_utf16(plugin->get_class_name() + ".php");
	const auto& src_path = std::format(LR"({:s}configs\{:s})", plugin_root.wstring(), src_config_file);
	if (std::filesystem::exists(src_path))
	{
		std::filesystem::copy_file(src_path, packFolder / src_config_file, std::filesystem::copy_options::overwrite_existing, err);
	}

	const auto& package_info_name = plugin->compile_name_template((cred.custom_update_name && !cred.get_update_name().empty()
																   ? cred.get_update_name() : utils::DUNE_UPDATE_INFO_NAME), cred);

	const auto& version_string = std::format("{:s}.{:s}", STRPRODUCTVER, date_string);
	std::string update_url(cred.update_url);
	if (!cred.update_url.empty())
	{
		if (cred.update_url.find("?rlkey") == std::string::npos)
		{
			update_url = std::format("{:s}{:s}.xml", cred.update_url, utils::utf16_to_utf8(package_info_name));
		}
	}

	// rewrite xml nodes
	try
	{
		std::ofstream ostream(packFolder / utils::PLUGIN_MANIFEST, std::ofstream::binary);

		auto doc = std::make_unique<rapidxml::xml_document<>>();
		doc->parse<rapidxml::parse_no_data_nodes>(config_data.data());

		// change values

		// <dune_plugin>
		//    <type_name>{type_name}</type_name>
		//    <name>{plugin_name}</name>
		//    <caption>{plugin_title}</caption>
		//    <class_name>{plugin_class_name}</class_name>
		//    <icon_url>{plugin_logo}</icon_url>
		//    <background>{plugin_background}</background>
		//    <version_index>{plugin_version_index}</version_index>
		//    <version>{plugin_version}</version>
		//    <release_date>{plugin_release_date}</release_date>
		//    <channels_url_path>{plugin_channels_url_path}</channels_url_path>
		//    <list_version_support>6</list_version_support>

		const auto& channels_list_version = std::to_string(CHANNELS_LIST_VERSION);
		auto d_node = doc->first_node("dune_plugin");

		d_node->first_node("type_name")->value(plugin->get_internal_name_a().c_str());
		d_node->first_node("class_name")->value(plugin->get_class_name().c_str());
		d_node->first_node("version_index")->value(version_index.c_str());
		d_node->first_node("version")->value(version_string.c_str());
		d_node->first_node("release_date")->value(RELEASEDATE);
		d_node->first_node("list_version_support")->value(channels_list_version.c_str());
		if (!noCustom)
		{
			if (!cred.ch_web_path.empty())
			{
				d_node->first_node("channels_url_path")->value(cred.ch_web_path.c_str());
			}

			if (!cred.m_direct_links.empty())
			{
				auto node = d_node->first_node("channels_direct_links");
				for (const auto& [key, value] : channels_list)
				{
					if (value.empty()) continue;

					auto links = doc->allocate_node(rapidxml::node_type::node_element, "links_info");
					links->append_node(rapidxml::alloc_node(*doc, "list", key.c_str()));
					links->append_node(rapidxml::alloc_node(*doc, "link", value.c_str()));
					node->append_node(links);
				}
			}
		}

		auto actions_node = d_node->first_node("global_actions");
		for (const auto& entry : plugin->get_manifest_list())
		{
			if (entry.name.empty()) continue;

			auto action_entry = doc->allocate_node(rapidxml::node_type::node_element, entry.id.c_str());
			if (entry.name[0] == '#')
			{
				const auto& param = entry.name.substr(1);
				action_entry->append_node(rapidxml::alloc_node(*doc, "type", "plugin_handle_user_input"));
				auto node_data = doc->allocate_node(rapidxml::node_type::node_element, "params");
				node_data->append_node(rapidxml::alloc_node(*doc, "handler_id", "entry_handler"));
				node_data->append_node(rapidxml::alloc_node(*doc, "control_id", "plugin_entry"));
				node_data->append_node(rapidxml::alloc_node(*doc, "action_id", param.c_str()));
				action_entry->append_node(node_data);
			}
			else
			{
				action_entry->append_node(rapidxml::alloc_node(*doc, "type", "plugin_system"));

				auto node_data = doc->allocate_node(rapidxml::node_type::node_element, "data");
				node_data->append_node(rapidxml::alloc_node(*doc, "run_string", entry.name.c_str()));
				action_entry->append_node(node_data);
			}

			actions_node->append_node(action_entry);
		}

		// <check_update>
		//	 <schema>2</schema>
		//	 <url>http://mysite.net/update/update_edem_mod.xml</url>
		//	 <timeout>0</timeout>
		// </check_update>

		if (!noCustom && !cred.update_url.empty())
		{
			auto cu_node = d_node->first_node("check_update");
			cu_node->first_node("url")->value(update_url.c_str());
		}

		ostream << *doc;
		ostream.close();
	}
	catch (const rapidxml::parse_error& ex)
	{
		if (showMessage)
		{
			CString error(ex.what());
			AfxMessageBox(error, MB_OK | MB_ICONERROR);
		}
		else
		{
			LOG_PROTOCOL(ex.what());
		}
		return false;
	}
	catch (const std::exception& ex)
	{
		if (showMessage)
		{
			CString error(ex.what());
			AfxMessageBox(error, MB_OK | MB_ICONERROR);
		}
		else
		{
			LOG_PROTOCOL(ex.what());
		}
		return false;
	}

	std::ofstream os_supplier(packFolder / bin_path / L"update_suppliers.sh", std::ofstream::binary | std::ofstream::app);
	os_supplier << R"(cat << EOF > "$filepath")" << std::endl << "{" << std::endl;
	os_supplier << R"(  "plugin" : "$plugin_name",)" << std::endl;
	os_supplier << R"(  "caption" : ")" << plugin_caption.c_str() << R"(",)" << std::endl;
	os_supplier << R"(  "tv_app" : "{\"type\":\"plugin\",\"plugin_name\":\"$plugin_name\")";
	if (!update_url.empty())
	{
		os_supplier << R"(,\"update_url\":\")" << update_url.c_str() << R"(\")";
	}
	os_supplier << R"(}")" << std::endl << "}" << std::endl;
	os_supplier << "EOF" << std::endl << std::endl;
	os_supplier.close();

	// copy channel lists
	for (const auto& item : channels_list)
	{
		const auto& channels = utils::utf8_to_utf16(item.first);
		std::filesystem::copy_file(channelsListPath / channels, packFolder / channels, std::filesystem::copy_options::overwrite_existing, err);
	}

	// copy embedded info
	if (!noEmbed && cred.embed && !make_web_update)
	{
		JSON_ALL_TRY
		{
			nlohmann::json node;
			switch (plugin->get_access_type())
			{
				using enum AccountAccessType;
				case enPin:
					node["password"] = cred.password;
					break;
				case enLoginPass:
					node["login"] = cred.login;
					node["password"] = cred.password;
					break;
				case enOtt:
					node["subdomain"] = cred.subdomain;
					node["ott_key"] = cred.ott_key;
					node["vportal"] = cred.portal;
					break;
				default:
					break;
			}

			if (!plugin->get_devices_list().empty())
				node["device_id"] = plugin->get_devices_list().at(cred.device_id).id;
			if (!plugin->get_servers_list().empty())
				node["server_id"] = plugin->get_servers_list().at(cred.server_id).id;
			if (!plugin->get_qualities_list().empty())
				node["quality_id"] = plugin->get_qualities_list().at(cred.quality_id).id;
			if (!plugin->get_profiles_list().empty())
				node["profile_id"] = plugin->get_profiles_list().at(cred.profile_id).id;
			if (!plugin->get_domains_list().empty())
				node["domain_id"] = plugin->get_domains_list().at(cred.domain_id).id;

			if (!node.empty())
			{
				const auto& js = node.dump();
				utils::CBase64Coder enc;
				enc.Encode(js.c_str(), (int)js.size(), ATL_BASE64_FLAG_NOCRLF | ATL_BASE64_FLAG_NOPAD);

				std::ofstream out_file(packFolder / L"account.dat", std::ofstream::binary);
				const auto& str = utils::generateRandomId(5) + enc.GetResultAsString();
				out_file.write(str.c_str(), str.size());
				out_file.close();
			}
		}
		JSON_ALL_CATCH
	}

	// revert back to previous state
	GetConfig().set_plugin_type(old_plugin_type);

	// pack folder
	auto plugin_installed_size = calc_folder_size(packFolder);

	SevenZipWrapper archiver(pack_dll);
	auto& compressor = archiver.GetCompressor();
	bool res = compressor.AddFiles(packFolder, _T("*.*"), true);
	if (!res)
	{
		if (showMessage)
		{
			AfxMessageBox(IDS_STRING_ERR_FILES_MISSING, MB_OK | MB_ICONSTOP);
		}
		else
		{
			LOG_PROTOCOL(load_string_resource(IDS_STRING_ERR_FILES_MISSING));
		}

		return false;
	}

	std::wstring packed_file;
	if (make_web_update)
	{
		const auto& update_path = GetConfig().get_string(true, REG_WEB_UPDATE_PATH);
		std::filesystem::create_directory(update_path, err);

		auto packed_tar = update_path + package_info_name + L".tar";
		auto packed_gz = update_path + package_info_name + L".tar.gz";
		std::filesystem::remove(packed_tar, err);
		std::filesystem::remove(packed_gz, err);

		packed_file = packed_tar;
		compressor.SetCompressionFormat(CompressionFormat::_Format::Tar);
		res = archiver.CreateArchive(packed_file);
		if (res)
		{
			compressor.ClearList();
			res = compressor.AddFile(packed_file);
			if (res)
			{
				packed_file.swap(packed_gz);
				compressor.SetCompressionFormat(CompressionFormat::_Format::GZip);
				res = archiver.CreateArchive(packed_file);
				if (res)
				{
					std::filesystem::remove(packed_tar, err);
				}
			}
		}
	}
	else
	{
		packed_file = std::format(L"{:s}dune_plugin_{:s}.zip",
								  output_path,
								  plugin->compile_name_template((cred.custom_plugin_name && !cred.get_plugin_name().empty()
																 ? cred.get_plugin_name() : utils::DUNE_PLUGIN_FILE_NAME), cred));

		compressor.SetCompressionFormat(CompressionFormat::_Format::Zip);
		res = archiver.CreateArchive(packed_file);
	}

	// remove temporary folder
	std::filesystem::remove_all(packFolder, err);
	if (!res)
	{
		std::filesystem::remove(packed_file, err);
		const auto& msg = load_string_resource_fmt(IDS_STRING_ERR_FAILED_PACK, packed_file);
		if (showMessage)
		{
			AfxMessageBox(msg.c_str(), MB_OK | MB_ICONSTOP);
		}
		else
		{
			LOG_PROTOCOL(msg);
		}

		return false;
	}

	if (make_web_update)
	{
		// <dune_plugin_update_info>
		// 	 <schema>2</schema>
		// 	 <name>plugin_name</name>
		// 	 <caption>plugin_caption</caption>
		// 	 <plugin_version_descriptor>
		// 	    <version_index>5</version_index>
		// 	    <version>130528_1700</version>
		// 	    <beta>no</beta>
		// 	    <critical>no</critical>
		// 	    <url>http://some-server/some-path/edem_update.tar.gz</url>
		//      <md5>a209a234fc9c518d3a165750a1f0dde4</md5>
		// 	    <size>2244608</size>
		// 	    <caption>plugin_caption</caption>
		// 	 </plugin_version_descriptor>
		// </dune_plugin_update_info>

		// create update info file
		try
		{
			// create document;
			auto doc = std::make_unique<rapidxml::xml_document<>>();

			// adding attributes at the top of our xml
			auto decl = doc->allocate_node(rapidxml::node_type::node_declaration);
			decl->append_attribute(doc->allocate_attribute("version", "1.0"));
			decl->append_attribute(doc->allocate_attribute("encoding", "UTF-8"));
			doc->append_node(decl);

			auto update_info = doc->allocate_node(rapidxml::node_type::node_element, "dune_plugin_update_info");

			update_info->append_node(rapidxml::alloc_node(*doc, "schema", "2"));
			update_info->append_node(rapidxml::alloc_node(*doc, "name", plugin->get_name().c_str()));
			update_info->append_node(rapidxml::alloc_node(*doc, "caption", plugin_caption.c_str()));

			auto version_info = doc->allocate_node(rapidxml::node_type::node_element, "plugin_version_descriptor");
			version_info->append_node(rapidxml::alloc_node(*doc, "version_index", version_index.c_str()));
			version_info->append_node(rapidxml::alloc_node(*doc, "version", version_string.c_str()));
			version_info->append_node(rapidxml::alloc_node(*doc, "beta", "no"));
			version_info->append_node(rapidxml::alloc_node(*doc, "critical", "no"));

			if (cred.update_package_url.find(".dropbox") != std::string::npos)
			{
				version_info->append_node(rapidxml::alloc_node(*doc, "url", cred.update_package_url.c_str()));
			}
			else
			{
				version_info->append_node(rapidxml::alloc_node(*doc, "url", std::format("{:s}{:s}.tar.gz",
																						cred.update_package_url,
																						utils::utf16_to_utf8(package_info_name)).c_str()));
			}
			version_info->append_node(rapidxml::alloc_node(*doc, "md5", utils::md5_hash_file(packed_file).c_str()));
			version_info->append_node(rapidxml::alloc_node(*doc, "size", std::to_string(plugin_installed_size).c_str()));
			version_info->append_node(rapidxml::alloc_node(*doc, "caption", plugin_caption.c_str()));

			update_info->append_node(version_info);
			doc->append_node(update_info);

			const auto& update_path = GetConfig().get_string(true, REG_WEB_UPDATE_PATH);
			std::ofstream os(update_path + package_info_name + L".xml", std::ofstream::binary);
			os << *doc;

		}
		catch (const rapidxml::parse_error& ex)
		{
			if (showMessage)
			{
				CString error(ex.what());
				AfxMessageBox(error, MB_OK | MB_ICONERROR);
			}
			return false;
		}
		catch (const std::exception& ex)
		{
			if (showMessage)
			{
				CString error(ex.what());
				AfxMessageBox(error, MB_OK | MB_ICONERROR);
			}
			else
			{
				LOG_PROTOCOL(ex.what());
			}
			return false;
		}
	}

	// Show success message
	CString msg;
	msg.Format(make_web_update ? IDS_STRING_INFO_W_CREATE_SUCCESS : IDS_STRING_INFO_CREATE_SUCCESS, packed_file.c_str());
	if (showMessage)
	{
		AfxMessageBox(msg, MB_OK);
	}
	else
	{
		LOG_PROTOCOL(msg.GetString());
	}

	return true;
}

void SetButtonImage(UINT imgId, CButton& btn)
{
	CImage img;
	if (LoadPngImage(imgId, img))
	{
		HBITMAP hOld = btn.SetBitmap(img.Detach());
		if (hOld != nullptr)
			::DeleteObject(hOld);
	}
}

void SetButtonImage(UINT imgId, CButton* pBtn)
{
	if (!pBtn)
		return;

	CImage img;
	if (LoadPngImage(imgId, img))
	{
		HBITMAP hOld = pBtn->SetBitmap(img.Detach());
		if (hOld != nullptr)
			::DeleteObject(hOld);
	}
}

//////////////////////////////////////////////////////////////////////////

std::wstring GetAppPath(LPCWSTR szSubFolder /*= nullptr*/, bool no_end_slash /*= false*/)
{
	CStringW fileName;

	if (GetModuleFileNameW(AfxGetInstanceHandle(), fileName.GetBuffer(_MAX_PATH), _MAX_PATH) != 0)
	{
		fileName.ReleaseBuffer();
		int pos = fileName.ReverseFind('\\');
		if (pos != -1)
			fileName.Truncate(pos + 1);
	}

	fileName += CIPTVChannelEditorApp::DEV_PATH.c_str();

	if (szSubFolder)
		fileName += szSubFolder;

	if (no_end_slash)
	{
		fileName.TrimRight('\\');
	}

	return std::filesystem::absolute(fileName.GetString());
}

std::string get_array_value(const std::vector<std::wstring>& creds, size_t& last)
{
	std::wstring str = creds.size() > last ? creds[last] : L"";
	last++;
	return utils::utf16_to_utf8(utils::string_trim(str, utils::EMSLiterals<wchar_t>::quote));
}

void ConvertAccounts()
{
	const auto& old_plugin_type = GetConfig().get_plugin_type();
	for (const auto& pair : GetPluginFactory().get_all_configs())
	{
		GetConfig().set_plugin_type(pair.first);

		bool need_convert = false;
		auto acc_data = GetConfig().get_string(false, REG_ACCOUNT_DATA);
		if (acc_data.empty())
		{
			need_convert = true;
			acc_data = GetConfig().get_string(false, REG_CREDENTIALS);
		}

		if (need_convert)
		{
			auto plugin = GetPluginFactory().create_plugin(pair.first);
			if (plugin)
			{
				const auto& access_type = plugin->get_access_type();
				const auto& login = utils::utf16_to_utf8(GetConfig().get_string(false, REG_LOGIN));
				const auto& password = utils::utf16_to_utf8(GetConfig().get_string(false, REG_PASSWORD));
				const auto& token = utils::utf16_to_utf8(GetConfig().get_string(false, REG_TOKEN));
				const auto& domain = utils::utf16_to_utf8(GetConfig().get_string(false, REG_DOMAIN));
				const auto& portal = utils::utf16_to_utf8(GetConfig().get_string(false, REG_VPORTAL));
				const auto& device_id = GetConfig().get_int(false, REG_DEVICE_ID);
				const auto& profile_id = GetConfig().get_int(false, REG_PROFILE_ID);
				const auto& quality_id = GetConfig().get_int(false, REG_QUALITY_ID);
				const auto& embed = GetConfig().get_int(false, REG_EMBED_INFO);

				std::vector<Credentials> all_credentials;
				int idx = 0;
				int selected = 0;
				const auto& all_accounts = utils::string_split(acc_data, L';');
				for (const auto& account : all_accounts)
				{
					const auto& creds = utils::string_split(account, L',');
					if (creds.empty() || creds.front().empty()) continue;

					size_t last = 0;
					Credentials cred;
					switch (access_type)
					{
						using enum AccountAccessType;
						case enPin:
							cred.password = get_array_value(creds, last);
							if (cred.password == password)
							{
								selected = idx;
							}
							break;

						case enLoginPass:
							cred.login = get_array_value(creds, last);
							cred.password = get_array_value(creds, last);
							if (cred.login == login)
							{
								selected = idx;
							}
							break;

						case enOtt:
							cred.token = get_array_value(creds, last);
							cred.portal = get_array_value(creds, last);
							if (cred.token == token)
							{
								selected = idx;
							}
							break;

						default:break;
					}

					cred.comment = get_array_value(creds, last);
					cred.server_id = device_id;
					cred.profile_id = profile_id;
					cred.quality_id = quality_id;
					cred.embed = embed;

					all_credentials.emplace_back(cred);
				}

				if (all_credentials.empty())
				{
					Credentials cred;
					switch (access_type)
					{
						using enum AccountAccessType;
						case enPin:
							cred.password = password;
							break;

						case enLoginPass:
							cred.login = login;
							cred.password = password;
							break;

						case enOtt:
							cred.token = token;
							cred.portal = portal;
							break;

						default: break;
					}

					all_credentials.emplace_back(cred);
				}

				nlohmann::json j_serialize = all_credentials;
				GetConfig().set_string(false, REG_ACCOUNT_DATA, utils::utf8_to_utf16(nlohmann::to_string(j_serialize)));
				GetConfig().set_int(false, REG_ACTIVE_ACCOUNT, selected);
			}
		}

		GetConfig().delete_setting(false, REG_LOGIN);
		GetConfig().delete_setting(false, REG_PASSWORD);
		GetConfig().delete_setting(false, REG_TOKEN);
		GetConfig().delete_setting(false, REG_DOMAIN);
		GetConfig().delete_setting(false, REG_VPORTAL);
		GetConfig().delete_setting(false, REG_DEVICE_ID);
		GetConfig().delete_setting(false, REG_PROFILE_ID);
		GetConfig().delete_setting(false, REG_EMBED_INFO);
		GetConfig().delete_setting(false, REG_CREDENTIALS);
	}

	GetConfig().set_plugin_type(old_plugin_type);
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
	GetConfig().set_binary(true, name, std::span((unsigned char*)&wp, sizeof(wp)));
}

bool LoadPngImage(UINT id, CImage& img)
{
	HGLOBAL hgblResourceData = nullptr;
	bool res = false;
	do
	{
		HRSRC hrsrc = FindResource(nullptr, MAKEINTRESOURCE(id), _T("PNG"));
		if (hrsrc == nullptr) break;

		DWORD dwResourceSize = SizeofResource(nullptr, hrsrc);
		HGLOBAL hglbImage = LoadResource(nullptr, hrsrc);
		if (hglbImage == nullptr) break;

		LPVOID pvSourceResourceData = LockResource(hglbImage);
		if (pvSourceResourceData == nullptr) break;

		hgblResourceData = GlobalAlloc(GMEM_MOVEABLE, dwResourceSize);
		if (hgblResourceData == nullptr) break;

		LPVOID pvResourceData = GlobalLock(hgblResourceData);

		if (pvResourceData == nullptr) break;

		CopyMemory(pvResourceData, pvSourceResourceData, dwResourceSize);
		GlobalUnlock(hgblResourceData);
		IStream* pStream = nullptr;
		if (FAILED(CreateStreamOnHGlobal(hgblResourceData, TRUE, &pStream)) || !pStream) break;

		img.Load(pStream);
		img.SetHasAlphaChannel(true);
		pStream->Release();
		res = true;
	} while (false);

	if (hgblResourceData == nullptr)
		::GlobalFree(hgblResourceData);

	return res;
}

BOOL LoadImageFromUrl(const std::wstring& fullPath, CImage& image)
{
	HRESULT hr = E_FAIL;
	if (utils::CrackUrl(fullPath))
	{
		utils::http_request req{ fullPath, 1h };
		if (utils::AsyncDownloadFile(req).get())
		{
			// Still not clear if this is making a copy internally
			auto view = req.body.rdbuf()->_Get_buffer_view();
			CComPtr<IStream> stream(SHCreateMemStream((BYTE*)view._Ptr, (unsigned int)view._Size));
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

void SetImageControl(const CImage& image, CStatic& wnd)
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
		resized.Create(rcDst.Width(), rcDst.Height(), 32, CImage::createAlphaChannel);
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

int RequestToUpdateServer(const std::wstring& command, bool isThread /*= true*/)
{
	const auto& cur_ver = utils::utf8_to_utf16(STRPRODUCTVER);
	const auto& avail_ver = GetConfig().get_string(true, REG_AVAIL_UPDATE);

	do
	{
#ifdef _DEBUG
		const auto& app = std::format(L"{:s}Debug Unicode\\Updater.exe", GetAppPath());
#else
		const auto& app = std::format(L"{:s}Updater.exe", GetAppPath());
#endif // _DEBUG

		std::wstring newCmd(command);
		//const auto& url = GetConfig().get_int(true, REG_UPDATE_SERVER, 0) == 0 ? utils::UPDATE_SERVER1 : utils::UPDATE_SERVER1;
		newCmd += std::format(L" --server={:s}", utils::UPDATE_SERVER1);

		if (!isThread)
		{
			::ShellExecute(nullptr, _T("open"), app.c_str(), newCmd.c_str(), nullptr, SW_SHOWMINIMIZED);
			return 0;
		}

		STARTUPINFO si = { 0 };
		si.cb = sizeof(STARTUPINFO);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
		si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
		si.hStdInput = nullptr;
		si.hStdOutput = nullptr;

		PROCESS_INFORMATION pi = { nullptr };

		std::wstring cmd = std::format(L"\"{:s}\" {:s}", app, newCmd);	// argv[0] имя исполняемого файла
		BOOL bRunProcess = CreateProcessW(app.c_str(),		// 	__in_opt     LPCTSTR lpApplicationName
										  cmd.data(),	    // 	__inout_opt  LPTSTR lpCommandLine
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
		const auto dwStart = utils::ChronoGetTickCount();
		for (;;)
		{
			if (dwExitCode != STILL_ACTIVE)
			{
				break;
			}

			if (utils::CheckForTimeOut(dwStart, 60s))
			{
				::TerminateProcess(pi.hProcess, 0);
				dwExitCode = STATUS_TIMEOUT;
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

std::wstring load_string_resource(unsigned int id)
{
	wchar_t* p = nullptr;
	int len = ::LoadStringW(AfxGetResourceHandle(), id, reinterpret_cast<LPWSTR>(&p), 0);
	if (len > 0)
	{
		return std::wstring{ p, static_cast<size_t>(len) };
	}
	// Return empty string; optionally replace with throwing an exception.
	return {};
}

std::wstring load_string_resource(unsigned int cp, unsigned int id)
{
	HMODULE hRes = nullptr;
	if (auto pair = theApp.m_LangMap.find(cp); pair != theApp.m_LangMap.end())
	{
		hRes = std::get<HMODULE>(pair->second);
	}

	wchar_t* p = nullptr;
	int len = ::LoadStringW(hRes, id, reinterpret_cast<LPWSTR>(&p), 0);
	if (len > 0)
	{
		return std::wstring{ p, static_cast<size_t>(len) };
	}

	// Return empty string; optionally replace with throwing an exception.
	return {};
}

std::string load_string_resource_a(unsigned int id)
{
	return utils::utf16_to_utf8(load_string_resource(id));
}

std::string load_string_resource_a(unsigned int cp, unsigned int id)
{
	return utils::utf16_to_utf8(load_string_resource(cp, id));
}

uintmax_t calc_folder_size(const std::wstring& path)
{
	uintmax_t total_size = 0;
	for (const auto& dir_entry : std::filesystem::recursive_directory_iterator(path))
	{
		if (!dir_entry.is_directory())
			total_size += dir_entry.file_size();
	}
	return total_size;
}

std::wstring GetPluginTypeNameW(const std::string& plugin_type, bool bCamel /*= false*/)
{
	std::wstring plugin_name;
	auto plugin = GetPluginFactory().create_plugin(plugin_type);
	if (plugin != nullptr)
	{
		// convert to wstring or string
		plugin_name = plugin->get_internal_name();
		if (bCamel)
		{
			plugin_name[0] = std::toupper(plugin_name[0]);
		}
	}

	return plugin_name;
}

std::string GetPluginTypeNameA(const std::string& plugin_type, bool bCamel /*= false*/)
{
	std::string plugin_name;
	auto plugin = GetPluginFactory().create_plugin(plugin_type);
	if (plugin != nullptr)
	{
		// convert to wstring or string
		plugin_name = utils::utf16_to_utf8(plugin->get_internal_name());
		if (bCamel)
		{
			plugin_name[0] = std::toupper(plugin_name[0]);
		}
	}

	return plugin_name;
}
