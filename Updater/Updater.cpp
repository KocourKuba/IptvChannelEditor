/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2024): sharky72 (https://github.com/KocourKuba)

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
#include <tchar.h>
#include <iostream>
#pragma warning(push)
#pragma warning(disable:4091)
#include <dbghelp.h>
#pragma warning(pop)

#include "CommandLine.hpp"

#include "UtilsLib\utils.h"
#include "UtilsLib\inet_utils.h"
#include "UtilsLib\Crc32.h"
#include "UtilsLib\FileVersionInfo.h"

#include "BugTrap\Source\Client\BugTrap.h"

static LPCTSTR g_sz_Run_GUID = _T("Global\\IPTVChannelEditor.{E4DC62B5-45AD-47AA-A016-512BA5995352}");

#ifdef _DEBUG
std::wstring DEV_PATH = LR"(..\..\)";
#ifdef _WIN64
std::wstring PACK_DLL_PATH = LR"(dll64\)";
#else
std::wstring PACK_DLL_PATH = LR"(dll\)";
#endif // _WIN64
#else
std::wstring DEV_PATH;
std::wstring PACK_DLL_PATH;
#endif // _DEBUG

constexpr auto PACK_DLL = L"7z.dll";

constexpr auto err_no_updates    = -1;
constexpr auto no_error          = 0;
constexpr auto err_download_info = 100;
constexpr auto err_download_pkg  = 102;
constexpr auto err_download_pl   = 103;
constexpr auto err_load_info     = 104;
constexpr auto err_save_info     = 105;
constexpr auto err_save_pkg      = 106;
constexpr auto err_save_pl       = 107;
constexpr auto err_open_pkg      = 108;
constexpr auto err_open_pl       = 109;
constexpr auto err_create_dir    = 110;
constexpr auto err_parse         = 200;

struct update_node
{
	std::wstring name;
	uint32_t crc{};
	bool opt = false;
};

struct UpdateInfo
{
	std::wstring info_file; // name of the update.xml or other file
	std::stringstream update_info; // content of update_file
	std::wstring update_path; // folder where is downloaded files stored
	std::wstring version; // version of the update
	std::wstring server; // update server
	std::vector<update_node> update_files; // all files to be replaced
	int parent_handle = 0;
	bool install_option_files = false;
};

std::wstring GetAppPath(const wchar_t* szSubFolder = nullptr)
{
	std::wstring fileName;
	fileName.resize((_MAX_PATH));

	DWORD size = GetModuleFileNameW(nullptr, fileName.data(), _MAX_PATH);
	if (size != 0)
	{
		fileName.resize(size);
		size_t pos = fileName.rfind('\\');
		if (pos != std::wstring::npos)
			fileName.resize(pos + 1);
	}

	//fileName += DEV_PATH;
	if (szSubFolder)
		fileName += szSubFolder;

	return std::filesystem::absolute(fileName);
}

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

	BT_SetAppName(fmt::format(L"Updater{:d}_{:d}_{:d}", MAJOR, MINOR, BUILD).c_str());
	BT_SetFlags(BTF_DETAILEDMODE | BTF_LISTPROCESSES | BTF_ATTACHREPORT | BTF_SHOWADVANCEDUI | BTF_INTERCEPTSUEF);
	BT_SetActivityType(BTA_SHOWUI);
	BT_SetDumpType(dwDumpType);
	BT_SetReportFilePath(GetAppPath().c_str());
	// required for VS 2005 & 2008
	BT_InstallSehFilter();
}

inline void LogProtocol(const std::string& str)
{
	SYSTEMTIME sTime;
	GetLocalTime(&sTime);

	const auto& csTimeStamp = fmt::format("[{:04d}:{:02d}:{:02d}][{:02d}:{:02d}:{:02d}.{:03d}]",
										  sTime.wYear, sTime.wMonth, sTime.wDay,
										  sTime.wHour, sTime.wMinute, sTime.wSecond, sTime.wMilliseconds);

	std::stringstream out;
	std::stringstream ss(str);
	std::string line;

	while (std::getline(ss, line))
	{
		while (line.back() == '\r')
			line.pop_back();

		out << csTimeStamp << ' ' << line;
	}

	std::cout << out.str() << std::endl;

	std::ofstream file(GetAppPath() + L"updater.log", std::ofstream::binary | std::ofstream::app);
	file << out.str() << std::endl;
}

inline void LogProtocol(const std::wstring& str)
{
	SYSTEMTIME sTime;
	GetLocalTime(&sTime);

	const auto& csTimeStamp = fmt::format("[{:04d}:{:02d}:{:02d}][{:02d}:{:02d}:{:02d}.{:03d}]",
										  sTime.wYear, sTime.wMonth, sTime.wDay,
										  sTime.wHour, sTime.wMinute, sTime.wSecond, sTime.wMilliseconds);

	std::stringstream out;
	std::stringstream ss(utils::utf16_to_utf8(str));
	std::string line;

	while (std::getline(ss, line))
	{
		while (line.back() == '\r')
			line.pop_back();

		out << csTimeStamp << ' ' << line;
	}

	std::cout << out.str() << std::endl;

	std::ofstream file(GetAppPath() + L"updater.log", std::ofstream::binary | std::ofstream::app);
	file << out.str() << std::endl;
}


static int parse_info(UpdateInfo& info)
{
	// Parse the buffer using the xml file parsing library into doc
	auto doc = std::make_unique<rapidxml::xml_document<>>();

	auto& xml = info.update_info.str();
	try
	{
		doc->parse<rapidxml::parse_default>(xml.data());
	}
	catch (rapidxml::parse_error&)
	{
		return err_parse; // Incorrect update info!
	}

	auto info_node = doc->first_node("update_info");
	if (!info_node)
	{
		LogProtocol("Incorrect update info: update_info node missing");
		return err_parse; // Incorrect update info!
	}

	info.version = rapidxml::get_value_wstring(info_node->first_attribute("version"));
	LogProtocol(fmt::format(L"Version on server: {:s}", info.version));

	auto pkg_node = doc->first_node("package");
	if (!pkg_node)
	{
		LogProtocol("Incorrect update info: package node missing");
		return err_parse; // Incorrect update info!
	}

	// Iterate <tv_category> nodes
	for (auto file_node = pkg_node->first_node("file"); file_node != nullptr; file_node = file_node->next_sibling())
	{
		update_node node;
		node.name = rapidxml::get_value_wstring(file_node->first_attribute("name"));
		node.crc = rapidxml::get_value_int(file_node->first_attribute("hash"));
		node.opt = rapidxml::get_value_string(file_node->first_attribute("opt")) == "true";
		LogProtocol(fmt::format(L"Name: {:s} ({:d}) opt={:d}", node.name, node.crc, node.opt));
		info.update_files.emplace_back(std::move(node));
	}

	CFileVersionInfo cVer;
	const auto& editor = GetAppPath() + L"IPTVChannelEditor.exe";
	cVer.Open(editor.c_str());
	const auto& cur_ver = fmt::format(L"{:d}.{:d}.{:d}", cVer.GetFileVersionMajor(), cVer.GetFileVersionMinor(), cVer.GetFileVersionBuild());
	if (cur_ver >= info.version)
	{
		LogProtocol(fmt::format("No updates. Current version is up to date or newer. {:d}", err_no_updates));
		return err_no_updates;
	}

	return no_error;
}

static int check_for_update(UpdateInfo& info)
{
	LogProtocol("Try to download update info...");
	utils::CUrlDownload dl;
	dl.SetUrl(fmt::format(L"{:s}/{:s}", info.server, utils::UPDATE_NAME));
	if (!dl.DownloadFile(info.update_info))
	{
		LogProtocol(fmt::format(L"{:s}", dl.GetLastErrorMessage()));
		return err_download_info; // Unable to download update info!
	}

	return parse_info(info);
}

static int download_update(UpdateInfo& info)
{
	int ret = no_error;
	do
	{
		ret = check_for_update(info);
		if (ret != err_no_updates && ret != no_error) break;

		const auto& update_file = fmt::format(L"{:s}{:s}", info.update_path, info.info_file);
		std::ofstream os(update_file, std::ofstream::binary);
		info.update_info.seekg(0);
		os << info.update_info.rdbuf();
		if (os.fail())
		{
			ret = err_save_pkg; // Unable to save update package!
			LogProtocol(fmt::format(L"error save: {:s} Error code: {:d}", update_file, GetLastError()));
			break;
		}
		LogProtocol(fmt::format(L"saved to: {:s}", update_file));
		os.close();

		utils::CUrlDownload dl;
		for (const auto& item : info.update_files)
		{
			const auto& loaded_file = fmt::format(L"{:s}{:s}", info.update_path, item.name);
			LogProtocol(fmt::format(L"Check: {:s}", loaded_file));
			if (std::filesystem::exists(loaded_file) && item.crc == file_crc32(loaded_file))
			{
				LogProtocol(fmt::format(L"file: {:s} already downloaded, skip download", item.name));
				continue;
			}

			std::stringstream file_data;
			dl.SetUrl(fmt::format(L"{:s}/{:s}/{:s}", info.server, info.version, item.name));
			LogProtocol(dl.GetUrl());
			if (!dl.DownloadFile(file_data))
			{
				ret = err_download_pkg; // Unable to download update package!
				LogProtocol(fmt::format(L"{:s}", dl.GetLastErrorMessage()));
				break;
			}

			std::ofstream os_file(loaded_file, std::ofstream::binary);
			file_data.seekg(0);
			os_file << file_data.rdbuf();
			if (os_file.fail())
			{
				ret = err_save_pkg; // Unable to save update package!
				LogProtocol(fmt::format(L"error save: {:s} Error code: {:d}", update_file, GetLastError()));
				break;
			}
			LogProtocol(fmt::format(L"saved to: {:s}", loaded_file));
		}
	} while (false);

	return ret;
}

static int update_app(UpdateInfo& info, bool force)
{
	LogProtocol("Checking for an application present in memory.");
	int i = 0;
	HANDLE hAppRunningMutex = OpenMutex(READ_CONTROL, FALSE, g_sz_Run_GUID);
	while(hAppRunningMutex != nullptr)
	{
		LogProtocol("Try to close app...");
		CloseHandle(hAppRunningMutex);
		hAppRunningMutex = nullptr;

		if (i > 20)
		{
			LogProtocol("Unable to close IPTV Channel Editor. Aborting update.");
			return err_no_updates;
		}

		HWND hwnd = ::FindWindow(nullptr, L"IPTV Channel Editor");
		if (hwnd)
			::PostMessage(hwnd, WM_CLOSE, 0, 0);

		LogProtocol("Waiting for closing IPTV Channel Editor.");
		hAppRunningMutex = OpenMutex(READ_CONTROL, FALSE, g_sz_Run_GUID);
		Sleep(500);
		i++;
	}

	int ret = download_update(info);

	if (ret != no_error)
	{
		if (ret != err_no_updates)
		{
			LogProtocol("Unable to download update!");
			return ret;
		}
		if (!force)
		{
			return ret;
		}
	}

	LogProtocol("Unpack and replace files...");
	const auto& pack_dll = GetAppPath((DEV_PATH + PACK_DLL_PATH).c_str()) + PACK_DLL;

	const auto& target_path = GetAppPath();
	for (const auto& item : info.update_files)
	{
		if (item.opt && !info.install_option_files)
		{
			LogProtocol(fmt::format(L"skipping optional package: {:s}", item.name));
			continue;
		}

		auto source_file = fmt::format(L"{:s}{:s}", info.update_path, item.name);
		auto target_file = target_path + item.name;
		auto bak_file = target_file + L".bak";

		bool success = true;
		bool folder = false;
		std::error_code err;
		if (std::filesystem::exists(target_file))
		{
			// we can check hash only for file
			if (std::filesystem::is_regular_file(target_file))
			{
				uint32_t crc = file_crc32(target_file);
				// if file already the same skip it
				if (crc == item.crc)
				{
					LogProtocol(fmt::format(L"{:s} is up to date", item.name));
					continue;
				}

				if (std::filesystem::exists(bak_file))
				{
					LogProtocol(fmt::format(L"remove old backup file: {:s}", bak_file));
					if (!std::filesystem::remove(bak_file, err) && err.value() != 0)
					{
						LogProtocol(fmt::format(L"Error with delete old backup file {:s} Error code: {:d}", bak_file, err.value()));
						err.clear();
					}
				}

				LogProtocol(fmt::format(L"rename: {:s} to {:s}", target_file, bak_file));
				std::filesystem::rename(target_file, bak_file, err);
				if (err.value() != 0)
				{
					LogProtocol(fmt::format(L"Unable to rename {:s} Error code: {:d}", target_file, err.value()));
					LogProtocol(err.message());
					success = false;
					continue;
				}
			}
		}

		LogProtocol(fmt::format(L"copy file: {:s} to {:s}", item.name, target_file));
		std::filesystem::copy(source_file, target_file, std::filesystem::copy_options::overwrite_existing, err);
		if (err.value() == 0)
		{
			// try to remove backup file (it can't be removed in some case)
			std::filesystem::remove_all(bak_file, err);
		}
	}

	LogProtocol("Replace files done.");
	return no_error;
}

int main(int argc, char* argv[])
{
#ifndef _DEBUG
	SetupExceptionHandler();
	BT_SetTerminate(); // set_terminate() must be called from every thread
#endif // _DEBUG

	// This variables can be set via the command line.
	bool check = false;
	bool download = false;
	bool update = false;
	bool playlists = false;
	bool printHelp = false;
	std::string server;
	bool debug = false;
	bool force = false;

	// First configure all possible command line options.
	CommandLine args("IPTV Channel Editor Updater 1.4\n(c) Pavel Abakanov (aka sharky72 at forum.hdtv.ru)\n");

	args.addArgument({ "check" }, &check, "Check for update");
	args.addArgument({ "download" }, &download, "Download update");
	args.addArgument({ "update" }, &update, "Perform update");
	args.addArgument({ "-s", "--server" }, &server, "-s url or --server=url Select server url that contains update");
	args.addArgument({ "-o", "--optional" }, &playlists, "Download or Update optional packages (Channels Lists)");
	args.addArgument({ "-f", "--force" }, &force, "Force update");
	args.addArgument({ "-d", "--debug" }, &debug, "");
	args.addArgument({ "-h", "--help" }, &printHelp, "Show parameters info");

	CFileVersionInfo cVer;
	const auto& editor = GetAppPath() + L"IPTVChannelEditor.exe";
	cVer.Open(editor.c_str());
	const auto& cur_ver = fmt::format(L"{:d}.{:d}.{:d}", cVer.GetFileVersionMajor(), cVer.GetFileVersionMinor(), cVer.GetFileVersionBuild());

	LogProtocol(fmt::format("Updater version: {:d}.{:d}.{:d}", MAJOR, MINOR, BUILD));
	LogProtocol(fmt::format(L"Current app version: {:s}", cur_ver));

	std::error_code err;
	// std::filesystem::remove(GetAppPath() + L"updater.log", err);
	// Then do the actual parsing.
	try
	{
		args.parse(argc, argv);
	}
	catch (std::runtime_error const& e)
	{
		LogProtocol(fmt::format("xml parse error: {:s}", e.what()));
		return err_parse;
	}

	UpdateInfo info;
	info.update_path = GetAppPath(utils::UPDATES_FOLDER);
	info.info_file = utils::UPDATE_NAME;
	info.server = utils::UPDATE_SERVER1;

	LogProtocol(fmt::format(L"Updates folder: {:s}", info.update_path));

	if (!std::filesystem::create_directories(info.update_path, err) && err.value())
	{
		return err_create_dir; // Unable to create update directory!
	}

	LogProtocol(fmt::format("server param: {:s}", server));
	if (!server.empty())
	{
		info.server = utils::utf8_to_utf16(server);
	}

	if (debug) //-V547
	{
		info.server = utils::UPDATE_SERVER2;
	}

	LogProtocol(fmt::format(L"Update server url: {:s}", info.server));

	if (check) //-V547
	{
		LogProtocol("Checking for update.");
		return check_for_update(info);
	}

	if (download) //-V547
	{
		LogProtocol("Downloading update package.");
		return download_update(info);
	}

	if (update) //-V547
	{
		LogProtocol("Performing update application.");
		return update_app(info, force);
	}

	args.printHelp();

	return no_error;
}
