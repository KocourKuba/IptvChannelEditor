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
#pragma warning(push)
#pragma warning(disable:4091)
#include <dbghelp.h>
#pragma warning(pop)

#include "IPTVChannelEditor.h"
#include "IPTVChannelEditorDlg.h"
#include "StreamContainer.h"
#include "IconCache.h"
#include "AccountSettings.h"
#include "Constants.h"

#include "UtilsLib\FileVersionInfo.h"
#include "UtilsLib\inet_utils.h"
#include "UtilsLib\rapidxml_print.hpp"
#include "UtilsLib\rapidxml_value.hpp"
#include "UtilsLib\md5.h"

#include "7zip\SevenZipWrapper.h"

#include "BugTrap\Source\Client\BugTrap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace SevenZip;

constexpr auto PACK_PATH = L"{:s}_plugin\\";
constexpr auto PACK_DLL = L"7za.dll";

static LPCTSTR g_sz_Run_GUID = _T("Global\\IPTVChannelEditor.{E4DC62B5-45AD-47AA-A016-512BA5995352}");
static HANDLE g_hAppRunningMutex = nullptr;

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

	BT_SetAppName(fmt::format(L"IPTVChannelEditor_v{:d}_{:d}_{:d}", MAJOR, MINOR, BUILD).c_str());
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
			AccountSettings::DEV_PATH = LR"(..\)";
			AccountSettings::PACK_DLL_PATH = LR"(dll\)";
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
		AccountSettings::DEV_PATH = LR"(..\)";
		AccountSettings::PACK_DLL_PATH = LR"(dll\)";
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

#ifndef _DEBUG
	SetupExceptionHandler();
	BT_SetTerminate(); // set_terminate() must be called from every thread
#endif // _DEBUG

	GetConfig().LoadSettings();

	if (GetConfig().get_string(true, REG_OUTPUT_PATH).empty())
	{
		GetConfig().set_string(true, REG_OUTPUT_PATH, GetAppPath());
	}

	if (GetConfig().get_string(true, REG_LISTS_PATH).empty())
	{
		const auto& playlist_dir = GetAppPath(L"playlists\\");
		GetConfig().set_string(true, REG_LISTS_PATH, playlist_dir);
		std::filesystem::create_directories(playlist_dir);
	}

	if (GetConfig().get_string(true, REG_WEB_UPDATE_PATH).empty())
	{
		const auto& update_dir = GetAppPath(L"WebUpdate\\");
		GetConfig().set_string(true, REG_WEB_UPDATE_PATH, update_dir);
		std::filesystem::create_directories(update_dir);
	}

	if (GetConfig().get_string(true, REG_SAVE_SETTINGS_PATH).empty())
	{
		const auto& settings_dir = GetAppPath(L"Settings\\");
		GetConfig().set_string(true, REG_SAVE_SETTINGS_PATH, settings_dir);
		std::filesystem::create_directories(settings_dir);
	}

	if (GetConfig().get_int(true, REG_MAX_CACHE_TTL) < 1)
	{
		GetConfig().set_int(true, REG_MAX_CACHE_TTL, 24);
	}

	ConvertAccounts();

	CCommandLineInfoEx cmdInfo;
	ParseCommandLine(cmdInfo);

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

		auto plugin_type = GetConfig().get_plugin_type();
		if (!PackPlugin(plugin_type, false, false, output_path, cmdInfo.m_bNoEmbed, cmdInfo.m_bNoCustom))
		{
			auto plugin = StreamContainer::get_instance(plugin_type);
			if (plugin)
			{
				CString str;
				str.Format(IDS_STRING_ERR_FAILED_PACK_PLUGIN, plugin->get_title().c_str());
				AfxMessageBox(str, MB_YESNO);
			}
		}

		return FALSE;
	}

	if (cmdInfo.m_bMakeAll)
	{
		std::wstring output_path;
		if (cmdInfo.m_strFileName.IsEmpty())
			output_path = GetConfig().get_string(true, REG_OUTPUT_PATH);
		else
			output_path = cmdInfo.m_strFileName.GetString();

		for (const auto& item : GetConfig().get_all_plugins())
		{
			if (item == PluginType::enCustom) continue;

			if (!PackPlugin(item, false, false, output_path, cmdInfo.m_bNoEmbed, cmdInfo.m_bNoCustom))
			{
				auto plugin = StreamContainer::get_instance(item);
				if (plugin)
				{
					CString str;
					str.Format(IDS_STRING_ERR_FAILED_PACK_PLUGIN, plugin->get_title().c_str());
					if (IDNO == AfxMessageBox(str, MB_YESNO)) break;
				}
			}
		}

		return FALSE;
	}

	FillLangMap();
	int nLangCurrent = GetConfig().get_int(true, REG_LANGUAGE);

	AfxSetResourceHandle(AfxGetInstanceHandle());
	if (auto pair = m_LangMap.find(nLangCurrent); pair != m_LangMap.cend())
	{
		if (nLangCurrent != 1033 && pair->second.hLib != nullptr)
		{
			AfxSetResourceHandle(pair->second.hLib);
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

				time_t next_check = time(nullptr) + (time_t)freq * 24 * 3600;
				GetConfig().set_int64(true, REG_NEXT_UPDATE, next_check);
				RequestToUpdateServer(cmd, false);
				return FALSE;
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

	for (auto& lang : m_LangMap)
	{
		::FreeLibrary(lang.second.hLib);
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
	sLang.hLib = nullptr;
	sLang.csLang = _T("English");
	m_LangMap.emplace(nExeTrans, sLang);

	std::filesystem::path cFile(fileName.GetString());
	cFile.replace_filename(cFile.stem().native() + _T("???.dll"));

	//����� ������ ������ � �������� ��� ������, ���� �������...
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

		sLang.hLib = hRes;
		sLang.csLang.LoadString(hRes, IDS_LANGUAGE);
		m_LangMap.emplace(nLibTrans, sLang);
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

	fileName += AccountSettings::DEV_PATH.c_str();

	if (szSubFolder)
		fileName += szSubFolder;

	return std::filesystem::absolute(fileName.GetString());
}

std::string get_array_value(std::vector<std::wstring>& creds, size_t& last)
{
	std::wstring str = creds.size() > last ? creds[last] : L"";
	last++;
	return utils::utf16_to_utf8(utils::string_trim(str, utils::EMSLiterals<wchar_t>::quote));
}

void ConvertAccounts()
{
	const auto& old_plugin_type = GetConfig().get_plugin_type();
	for (const auto& item : GetConfig().get_all_plugins())
	{
		GetConfig().set_plugin_type(item);

		bool need_convert = false;
		auto acc_data = GetConfig().get_string(false, REG_ACCOUNT_DATA);
		if (acc_data.empty())
		{
			need_convert = true;
			acc_data = GetConfig().get_string(false, REG_CREDENTIALS);
		}

		if (need_convert)
		{
			auto plugin = StreamContainer::get_instance(item);
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
				auto& all_accounts = utils::string_split(acc_data, L';');
				for (const auto& account : all_accounts)
				{
					auto& creds = utils::string_split(account, L',');
					if (creds.empty() || creds.front().empty()) continue;

					size_t last = 0;
					Credentials cred;
					switch (access_type)
					{
						case AccountAccessType::enPin:
							cred.password = get_array_value(creds, last);
							if (cred.password == password)
							{
								selected = idx;
							}
							break;

						case AccountAccessType::enLoginPass:
							cred.login = get_array_value(creds, last);
							cred.password = get_array_value(creds, last);
							if (cred.login == login)
							{
								selected = idx;
							}
							break;

						case AccountAccessType::enOtt:
							cred.token = get_array_value(creds, last);
							cred.subdomain = get_array_value(creds, last);
							cred.portal = get_array_value(creds, last);
							if (cred.token == token)
							{
								selected = idx;
							}
							break;

						default:break;
					}

					cred.comment = get_array_value(creds, last);
					cred.suffix = get_array_value(creds, last);
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
						case AccountAccessType::enPin:
							cred.password = password;
							break;

						case AccountAccessType::enLoginPass:
							cred.login = login;
							cred.password = password;
							break;

						case AccountAccessType::enOtt:
							cred.token = token;
							cred.subdomain = domain;
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

bool PackPlugin(const PluginType plugin_type,
				bool showMessage,
				bool make_web_update /*= false*/,
				std::wstring output_path /*= L""*/,
				bool noEmbed /*= false*/,
				bool noCustom /*= false*/)
{
	const auto& pack_dll = GetAppPath((AccountSettings::PACK_DLL_PATH).c_str()) + PACK_DLL;
	if (!std::filesystem::exists(pack_dll))
	{
		if (showMessage)
			AfxMessageBox(IDS_STRING_ERR_DLL_MISSING, MB_OK | MB_ICONSTOP);

		return false;
	}

	const auto& lists_path = GetConfig().get_string(true, REG_LISTS_PATH);

	if (output_path.empty())
		output_path = GetConfig().get_string(true, REG_OUTPUT_PATH);

	output_path = std::filesystem::absolute(output_path);
	if (output_path.back() != '\\')
		output_path += '\\';

	std::error_code err;
	std::filesystem::create_directory(output_path, err);

	const auto& update_path = GetConfig().get_string(true, REG_WEB_UPDATE_PATH);
	std::filesystem::create_directory(update_path, err);

	const auto& old_plugin_type = GetConfig().get_plugin_type();
	GetConfig().set_plugin_type(plugin_type);

	const auto& selected = GetConfig().get_int(false, REG_ACTIVE_ACCOUNT);
	if (selected == -1)
	{
		// revert back to previous state
		GetConfig().set_plugin_type(old_plugin_type);
		return false;
	}

	auto plugin = StreamContainer::get_instance(plugin_type);

	std::vector<Credentials> all_credentials;
	nlohmann::json creds;
	JSON_ALL_TRY;
	{
		creds = nlohmann::json::parse(GetConfig().get_string(false, REG_ACCOUNT_DATA));
	}
	JSON_ALL_CATCH;
	for (const auto& item : creds.items())
	{
		const auto& val = item.value();
		if (val.empty()) continue;

		Credentials cred;
		JSON_ALL_TRY;
		{
			cred = val.get<Credentials>();
		}
		JSON_ALL_CATCH;
		all_credentials.emplace_back(cred);
	}

	if (selected >= (int)all_credentials.size())
	{
		// revert back to previous state
		GetConfig().set_plugin_type(old_plugin_type);
		return false;
	}

	const auto& cred = all_credentials[selected];
	const auto& short_name_w = plugin->get_short_name_w();
	const auto& packFolder = std::filesystem::temp_directory_path().wstring() + fmt::format(PACK_PATH, short_name_w);

	// load plugin settings
	plugin->load_plugin_parameters(utils::utf8_to_utf16(cred.config));

	const auto& packed_plugin_name = fmt::format(utils::DUNE_PLUGIN_NAME, plugin->get_short_name(), (cred.suffix.empty() || noCustom) ? "mod" : cred.suffix);

	if (make_web_update && (cred.update_url.empty() || cred.update_package_url.empty()))
	{
		// revert back to previous state
		GetConfig().set_plugin_type(old_plugin_type);
		if (showMessage)
			AfxMessageBox(IDS_STRING_ERR_SETTINGS_MISSING, MB_OK | MB_ICONSTOP);
		return false;
	}

	// collect plugin channels list;
	std::vector<std::wstring> channels_list;
	const auto& playlistPath = fmt::format(L"{:s}{:s}\\", lists_path, short_name_w);
	std::filesystem::directory_iterator dir_iter(playlistPath, err);
	for (auto const& dir_entry : dir_iter)
	{
		const auto& path = dir_entry.path();
		if (path.extension() == L".xml")
		{
			if (noCustom || cred.ch_list.empty() || std::find(cred.ch_list.begin(), cred.ch_list.end(), path.filename().string()) != cred.ch_list.end())
			{
				channels_list.emplace_back(path.filename().wstring());
			}
		}
	}

	// remove previous packed folder if exist
	std::filesystem::remove_all(packFolder, err);

	// copy entire plugin folder
	const auto& plugin_root = GetAppPath(utils::PLUGIN_ROOT);

	std::filesystem::copy(plugin_root, packFolder, std::filesystem::copy_options::none, err);
	std::filesystem::copy(plugin_root + L"bin", packFolder + L"bin", std::filesystem::copy_options::recursive, err);
	std::filesystem::copy(plugin_root + L"lib", packFolder + L"lib", std::filesystem::copy_options::recursive, err);
	std::filesystem::copy(plugin_root + L"img", packFolder + L"img", std::filesystem::copy_options::recursive, err);
	std::filesystem::copy(plugin_root + L"icons", packFolder + L"icons", std::filesystem::copy_options::recursive, err);

	// remove if old logo and backgrounds still exists in icons folder
	boost::wregex regExpName { LR"(^(?:bg|logo)_.*\.(?:jpg|png)$)" };
	for (const auto& dir_entry : std::filesystem::directory_iterator{ packFolder + LR"(icons\)" })
	{
		if (!dir_entry.is_directory() && boost::regex_match(dir_entry.path().filename().wstring(), regExpName))
			std::filesystem::remove(dir_entry.path(), err);
	}

	// remove cbilling_vod base class
	if (plugin_type != PluginType::enAntifriz && plugin_type != PluginType::enCbilling)
	{
		std::filesystem::remove(packFolder + L"cbilling_vod_impl.php", err);
	}

	std::filesystem::path plugin_logo;
	std::filesystem::path plugin_bgnd;
	if (!noCustom)
	{
		if (!cred.logo.empty())
			plugin_logo = cred.logo;

		if (!cred.background.empty())
			plugin_bgnd = cred.background;
	}

	if (plugin_logo.empty())
		plugin_logo = fmt::format(LR"({:s}plugins_image\logo_{:s}.png)", plugin_root, short_name_w);
	else if (!plugin_logo.has_parent_path())
		plugin_logo = fmt::format(LR"({:s}plugins_image\{:s})", plugin_root, plugin_logo.wstring());

	if (plugin_bgnd.empty())
		plugin_bgnd = fmt::format(LR"({:s}plugins_image\bg_{:s}.png)", plugin_root, short_name_w);
	else if (!plugin_bgnd.has_parent_path())
		plugin_bgnd = fmt::format(LR"({:s}plugins_image\{:s})", plugin_root, plugin_bgnd.wstring());

	const auto& packFolderIcons = packFolder + LR"(icons\)";
	std::filesystem::copy(plugin_logo, packFolderIcons, err);
	std::filesystem::copy(plugin_bgnd, packFolderIcons, err);

	if (!std::filesystem::exists(packFolderIcons + plugin_logo.filename().wstring()))
	{
		plugin_logo = "default_logo.png";
	}

	if (!std::filesystem::exists(packFolderIcons + plugin_bgnd.filename().wstring()))
	{
		plugin_bgnd = "default_bg.png";
	}

	const auto& logo_subst = fmt::format("plugin_file://icons/{:s}", plugin_logo.filename().string());
	const auto& bg_subst = fmt::format("plugin_file://icons/{:s}", plugin_bgnd.filename().string());

	std::string version_index;
	if (cred.custom_increment && !cred.version_id.empty())
	{
		version_index = cred.version_id;
	}
	else
	{
		COleDateTime date = COleDateTime::GetCurrentTime();
		version_index = fmt::format("{:d}{:02d}{:02d}{:02d}", date.GetYear(), date.GetMonth(), date.GetDay(), date.GetHour());
	}

	std::string update_name;
	if (cred.custom_update_name && !cred.update_name.empty())
	{
		update_name = cred.update_name;
	}
	else
	{
		update_name = fmt::format(utils::DUNE_UPDATE_NAME, plugin->get_short_name(), (cred.suffix.empty()) ? "mod" : cred.suffix) + ".xml";
	}

	std::string package_plugin_name;
	if (cred.custom_package_name && !cred.package_name.empty())
	{
		package_plugin_name = cred.package_name;
	}
	else
	{
		package_plugin_name = fmt::format(utils::DUNE_UPDATE_NAME, plugin->get_short_name(), (cred.suffix.empty()) ? "mod" : cred.suffix);
	}

	// save config
	int idx = GetConfig().get_int(false, REG_PLAYLIST_TYPE);
	plugin->set_playlist_template_idx(idx);
	plugin->save_plugin_parameters(fmt::format(L"{:s}config.json", packFolder), true);

	// create plugin manifest
	std::string config_data;
	std::ifstream istream(plugin_root + L"dune_plugin.xml");
	config_data.assign(std::istreambuf_iterator<char>(istream), std::istreambuf_iterator<char>());
	const auto& plugin_caption = cred.caption.empty() ? utils::utf16_to_utf8(plugin->get_title()) : cred.caption;

	// preprocess common values
	utils::string_replace_inplace(config_data, "{name}", plugin->get_name().c_str());
	utils::string_replace_inplace(config_data, "{short_name}", plugin->get_short_name().c_str());
	utils::string_replace_inplace(config_data, "{plugin_caption}", plugin_caption.c_str());
	utils::string_replace_inplace(config_data, "{plugin_logo}", logo_subst.c_str());
	utils::string_replace_inplace(config_data, "{plugin_bg}", bg_subst.c_str());

	// copy plugin config class if they exist
	std::string class_name = fmt::format("{:s}_config", plugin->get_short_name());
	const auto& src_config = short_name_w + L"_config.php";
	const auto& src_path = fmt::format(LR"({:s}configs\{:s})", plugin_root, src_config);
	if (std::filesystem::exists(src_path))
	{
		std::filesystem::copy_file(src_path, packFolder + src_config, std::filesystem::copy_options::overwrite_existing, err);
	}
	else
	{
		class_name = "default_config";
	}

	// rewrite xml nodes
	try
	{
		std::ofstream ostream(packFolder + L"dune_plugin.xml", std::ofstream::binary);

		auto doc = std::make_unique<rapidxml::xml_document<>>();
		doc->parse<rapidxml::parse_no_data_nodes>(config_data.data());

		// change values

		// <dune_plugin>
		//    <name>{plugin_name}</name>
		//    <caption>{plugin_title}</caption>
		//    <short_name>{plugin_short_name}</short_name>
		//    <class_name>{plugin_class_name}</class_name>
		//    <icon_url>{plugin_logo}</icon_url>
		//    <background>{plugin_background}</background>
		//    <version_index>{plugin_version_index}</version_index>
		//    <version>{plugin_version}</version>
		//    <release_date>{plugin_release_date}</release_date>
		//    <channels_url_path>{plugin_channels_url_path}</channels_url_path>

		auto d_node = doc->first_node("dune_plugin");

		d_node->first_node("class_name")->value(class_name.c_str());
		d_node->first_node("version_index")->value(version_index.c_str());
		d_node->first_node("version")->value(STRPRODUCTVER);
		d_node->first_node("release_date")->value(RELEASEDATE);
		if (!noCustom)
			d_node->first_node("channels_url_path")->value(cred.ch_web_path.c_str());

		auto cu_node = d_node->first_node("check_update");
		const auto& update_url = noCustom ? "" : fmt::format("{:s}{:s}", cred.update_url, update_name);
		cu_node->first_node("url")->value(update_url.c_str());

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
		return false;
	}
	catch (const std::exception& ex)
	{
		if (showMessage)
		{
			CString error(ex.what());
			AfxMessageBox(error, MB_OK | MB_ICONERROR);
		}
		return false;
	}

	std::ofstream os_supplier(packFolder + LR"(bin\update_suppliers)", std::ofstream::binary | std::ofstream::app);
	os_supplier << R"(cat << EOF > "$filepath")" << std::endl << "{" << std::endl;
	os_supplier << R"(   "plugin" : "$plugin_name",)" << std::endl;
	os_supplier << R"(   "caption" : ")" << plugin_caption.c_str() << R"(",)" << std::endl;
	os_supplier << R"(   "tv_app" : "{\"type\":\"plugin\",\"plugin_name\":\"$plugin_name\"}")" << std::endl;
	os_supplier << "}" << std::endl << "EOF" << std::endl;
	os_supplier.close();

	// copy channel lists
	for(const auto& item : channels_list)
	{
		std::filesystem::copy_file(playlistPath + item, packFolder + item, std::filesystem::copy_options::overwrite_existing, err);
	}

	// copy embedded info
	if (!noEmbed && cred.embed && !make_web_update)
	{
		try
		{
			nlohmann::json node;
			switch (plugin->get_access_type())
			{
				case AccountAccessType::enPin:
					node["password"] = cred.password;
					break;
				case AccountAccessType::enLoginPass:
					node["login"] = cred.login;
					node["password"] = cred.password;
					break;
				case AccountAccessType::enOtt:
					node["domain"] = cred.subdomain;
					node["ott_key"] = cred.token;
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

			if (!node.empty())
			{
				const auto& js = node.dump();
				utils::CBase64Coder enc;
				enc.Encode(js.c_str(), (int)js.size(), ATL_BASE64_FLAG_NOCRLF | ATL_BASE64_FLAG_NOPAD);

				std::ofstream out_file(packFolder + L"account.dat", std::ofstream::binary);
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
	auto plugin_installed_size = calc_folder_size(packFolder);

	SevenZipWrapper archiver(pack_dll);
	auto& compressor = archiver.GetCompressor();
	bool res = compressor.AddFiles(packFolder, _T("*.*"), true);
	if (!res)
	{
		if (showMessage)
			AfxMessageBox(IDS_STRING_ERR_FILES_MISSING, MB_OK | MB_ICONSTOP);

		return false;
	}

	std::wstring packed_file;
	if (make_web_update)
	{
		const auto& package_name = update_path + utils::utf8_to_utf16(package_plugin_name);
		const auto& packed_tar = package_name + L".tar";
		const auto& packed_gz = package_name + L".tar.gz";
		std::filesystem::remove(packed_tar, err);
		std::filesystem::remove(packed_gz, err);

		packed_file = packed_tar;
		compressor.SetCompressionFormat(CompressionFormat::Tar);
		res = archiver.CreateArchive(packed_file);
		if (res)
		{
			compressor.ClearList();
			res = compressor.AddFile(packed_file);
			if (res)
			{
				packed_file = packed_gz;
				compressor.SetCompressionFormat(CompressionFormat::GZip);
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
		packed_file = output_path + utils::utf8_to_utf16(packed_plugin_name);
		compressor.SetCompressionFormat(CompressionFormat::Zip);
		res = archiver.CreateArchive(packed_file);
	}

	// remove temporary folder
	std::filesystem::remove_all(packFolder, err);
	if (!res)
	{
		if (showMessage)
		{
			std::filesystem::remove(packed_file, err);
			CString msg;
			msg.Format(IDS_STRING_ERR_FAILED_PACK, packed_file.c_str());
			AfxMessageBox(msg, MB_OK | MB_ICONSTOP);
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
			auto decl = doc->allocate_node(rapidxml::node_declaration);
			decl->append_attribute(doc->allocate_attribute("version", "1.0"));
			decl->append_attribute(doc->allocate_attribute("encoding", "UTF-8"));
			doc->append_node(decl);

			auto update_info = doc->allocate_node(rapidxml::node_element, "dune_plugin_update_info");

			update_info->append_node(rapidxml::alloc_node(*doc, "schema", "2"));
			update_info->append_node(rapidxml::alloc_node(*doc, "name", plugin->get_name().c_str()));
			update_info->append_node(rapidxml::alloc_node(*doc, "caption", plugin_caption.c_str()));

			auto version_info = doc->allocate_node(rapidxml::node_element, "plugin_version_descriptor");
			version_info->append_node(rapidxml::alloc_node(*doc, "version_index", version_index.c_str()));
			version_info->append_node(rapidxml::alloc_node(*doc, "version", STRPRODUCTVER));
			version_info->append_node(rapidxml::alloc_node(*doc, "beta", "no"));
			version_info->append_node(rapidxml::alloc_node(*doc, "critical", "no"));
			version_info->append_node(rapidxml::alloc_node(*doc, "url",
														   fmt::format("{:s}{:s}.tar.gz", cred.update_package_url, package_plugin_name).c_str()));
			version_info->append_node(rapidxml::alloc_node(*doc, "md5", utils::md5_hash_file(packed_file).c_str()));
			version_info->append_node(rapidxml::alloc_node(*doc, "size", std::to_string(plugin_installed_size).c_str()));
			version_info->append_node(rapidxml::alloc_node(*doc, "caption", plugin_caption.c_str()));

			update_info->append_node(version_info);
			doc->append_node(update_info);

			std::ofstream os(update_path + utils::utf8_to_utf16(update_name), std::ofstream::binary);
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
			return false;
		}
	}

	// Show success message
	if (showMessage)
	{
		CString msg;
		msg.Format(make_web_update ?IDS_STRING_INFO_W_CREATE_SUCCESS : IDS_STRING_INFO_CREATE_SUCCESS, packed_file.c_str());
		AfxMessageBox(msg, MB_OK);
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
	utils::CrackedUrl cracked;
	if (utils::CrackUrl(fullPath, cracked))
	{
		CWaitCursor cur;
		std::stringstream data;
		if (utils::DownloadFile(fullPath, data))
		{
			// Still not clear if this is making a copy internally
			auto view = data.rdbuf()->_Get_buffer_view();
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
	const auto& cur_ver = utils::utf8_to_utf16(STRPRODUCTVER, sizeof(STRPRODUCTVER));
	const auto& avail_ver = GetConfig().get_string(true, REG_AVAIL_UPDATE);

	do
	{
#ifdef _DEBUG
		const auto& app = fmt::format(L"{:s}Debug Unicode\\Updater.exe", GetAppPath());
#else
		const auto& app = fmt::format(L"{:s}Updater.exe", GetAppPath());
#endif // _DEBUG

		if (!isThread)
		{
			::ShellExecute(nullptr, _T("open"), app.c_str(), command.c_str(), nullptr, SW_SHOWMINIMIZED);
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

		std::wstring cmd = fmt::format(L"\"{:s}\" {:s}", app, command);	// argv[0] ��� ������������ �����
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
		uint64_t dwStart = utils::ChronoGetTickCount();
		for (;;)
		{
			if (dwExitCode != STILL_ACTIVE)
			{
				break;
			}

			if (utils::CheckForTimeOut(dwStart, 60 * 1000))
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
	return std::wstring{};
}

std::wstring load_string_resource(unsigned int cp, unsigned int id)
{
	HMODULE hRes = nullptr;
	if (auto pair = theApp.m_LangMap.find(cp); pair != theApp.m_LangMap.end())
	{
		hRes = pair->second.hLib;
	}

	wchar_t* p = nullptr;
	int len = ::LoadStringW(hRes, id, reinterpret_cast<LPWSTR>(&p), 0);
	if (len > 0)
	{
		return std::wstring{ p, static_cast<size_t>(len) };
	}

	// Return empty string; optionally replace with throwing an exception.
	return std::wstring{};
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

std::wstring GetPluginShortNameW(const PluginType plugin_type, bool bCamel /*= false*/)
{
	std::wstring plugin_name;
	auto plugin = StreamContainer::get_instance(plugin_type);
	if (plugin != nullptr)
	{
		const auto& short_name = plugin->get_short_name();
		// convert to wstring or string
		plugin_name.assign(short_name.begin(), short_name.end());
		if (bCamel)
		{
			plugin_name[0] = std::toupper(plugin_name[0]);
		}
	}

	return plugin_name;
}

std::string GetPluginShortNameA(const PluginType plugin_type, bool bCamel /*= false*/)
{
	std::string plugin_name;
	auto plugin = StreamContainer::get_instance(plugin_type);
	if (plugin != nullptr)
	{
		// convert to wstring or string
		plugin_name = plugin->get_short_name();
		if (bCamel)
		{
			plugin_name[0] = std::toupper(plugin_name[0]);
		}
	}

	return plugin_name;
}
