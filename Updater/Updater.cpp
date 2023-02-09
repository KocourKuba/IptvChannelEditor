/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2023): sharky72 (https://github.com/KocourKuba)

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
#include <wtypes.h>
#pragma warning(push)
#pragma warning(disable:4091)
#include <dbghelp.h>
#pragma warning(pop)

#include "CommandLine.hpp"

#include "UtilsLib\utils.h"
#include "UtilsLib\inet_utils.h"
#include "UtilsLib\rapidxml_value.hpp"
#include "UtilsLib\Crc32.h"
#include "UtilsLib\FileVersionInfo.h"

#include "SevenZip\7zip\SevenZipWrapper.h"

#include "BugTrap\Source\Client\BugTrap.h"

static LPCTSTR g_sz_Run_GUID = _T("Global\\IPTVChannelEditor.{E4DC62B5-45AD-47AA-A016-512BA5995352}");

#ifdef _DEBUG
static LPCWSTR g_szPath = L"http://iptv.esalecrm.net/update";
#else
static LPCWSTR g_szPath = L"http://igores.ru/sharky72";
#endif // _DEBUG

#ifdef _DEBUG
std::wstring DEV_PATH = L"..\\";
std::wstring PACK_DLL_PATH = L"dll\\";
#else
std::wstring DEV_PATH;
std::wstring PACK_DLL_PATH;
#endif // _DEBUG

constexpr auto PACK_DLL = L"7za.dll";

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

inline void LogProtocol(std::wstring& str)
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


int parse_info(UpdateInfo& info)
{
	// Parse the buffer using the xml file parsing library into doc
	auto doc = std::make_unique<rapidxml::xml_document<>>();

	auto& xml = info.update_info.str();
	try
	{
		doc->parse<0>(xml.data());
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

int check_for_update(UpdateInfo& info)
{
	LogProtocol("Try to download update info...");
	if (!utils::DownloadFile(fmt::format(L"{:s}/update.xml", g_szPath), info.update_info))
	{
		return err_download_info; // Unable to download update info!
	}

	return parse_info(info);
}

int download_update(UpdateInfo& info)
{
	int ret = 0;
	do
	{
		ret = check_for_update(info);
		if (ret != 0) break;

		for (const auto& item : info.update_files)
		{
			const auto& loaded_file = fmt::format(L"{:s}{:s}", info.update_path, item.name);
			if (std::filesystem::exists(loaded_file))
			{
				std::ifstream stream(loaded_file, std::istream::binary);
				std::vector<unsigned char> file_data;
				file_data.assign(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
				uint32_t crc = crc32_bitwise(file_data.data(), file_data.size());
				if (item.crc == crc)
				{
					LogProtocol(fmt::format(L"file: {:s} already downloaded, skip download", item.name));
					continue;
				}
			}

			std::stringstream file_data;
			const auto& url = fmt::format(L"{:s}/{:s}/{:s}", g_szPath, info.version, item.name);
			LogProtocol(fmt::format(L"download: {:s}", url));
			if (!utils::DownloadFile(url, file_data))
			{
				ret = err_download_pkg; // Unable to download update package!
				break;
			}

			std::ofstream os(loaded_file, std::ofstream::binary);
			os << file_data.rdbuf();
			if (os.bad())
			{
				ret = err_save_pkg; // Unable to save update package!
				break;
			}
			LogProtocol(fmt::format(L"saved to: {:s}", loaded_file));
		}
	} while (false);

	return ret;
}

int update_app(UpdateInfo& info)
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

	if (ret != 0)
	{
		if (ret != -1)
		{
			LogProtocol("Unable to download update!");
		}

		return ret;
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
		std::filesystem::path src(source_file);
		if (src.extension() == ".7z")
		{
			SevenZip::SevenZipWrapper archiver(pack_dll);
			if (!archiver.OpenArchive(source_file))
			{
				LogProtocol("Error open archive. Aborting.");
				return err_open_pkg;
			}

			folder = true;
			LogProtocol(fmt::format(L"unpacking: {:s} to {:s}", src.wstring(), info.update_path));
			if (!archiver.GetExtractor().ExtractArchive(info.update_path))
			{
				LogProtocol("Error unpacking archive. Aborting.");
				return err_open_pkg;
			}

			source_file = info.update_path + src.stem().wstring();
			target_file = target_path + src.stem().wstring();
			bak_file = target_file + L".bak";
		}

		std::error_code err;
		if (std::filesystem::exists(target_file))
		{
			// we can check hash only for file
			if (std::filesystem::is_regular_file(target_file))
			{
				std::ifstream stream(target_file, std::istream::binary);
				std::vector<unsigned char> file_data;
				file_data.assign(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
				uint32_t crc = crc32_bitwise(file_data.data(), file_data.size());
				stream.close();

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

		std::filesystem::copy_options opt = std::filesystem::copy_options::none;
		std::wstring type = L"copy file ";
		if (std::filesystem::is_directory(source_file))
		{
			opt = std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing;
			type = L"copy unpacked archive ";
		}

		LogProtocol(fmt::format(L"{:s}{:s} to {:s}", type, item.name, target_file));
		std::filesystem::copy(source_file, target_file, opt, err);
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

	// First configure all possible command line options.
	CommandLine args("IPTV Channel Editor Updater 1.2\n(c) Pavel Abakanov (aka sharky72 at forum.hdtv.ru)\n");

	args.addArgument({ "check" }, &check, "Check for update");
	args.addArgument({ "download" }, &download, "Download update");
	args.addArgument({ "update" }, &update, "Perform update");
	args.addArgument({ "-o", "--optional" }, &playlists, "Download or Update optional packages (playlists)");
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
	info.info_file = L"update.xml";
	info.update_path = GetAppPath(L"Updates\\");

	if (!std::filesystem::create_directories(info.update_path, err) && err.value())
	{
		return err_create_dir; // Unable to create update directory!
	}

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
		return update_app(info);
	}

	args.printHelp();

	return no_error;
}
