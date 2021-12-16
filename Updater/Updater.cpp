// Updater.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include <tchar.h>
#include <iostream>
#include <wtypes.h>

#include "CommandLine.hpp"

#include "UtilsLib\utils.h"
#include "UtilsLib\inet_utils.h"
#include "UtilsLib\rapidxml_value.hpp"
#include "UtilsLib\Crc32.h"

#include "SevenZip\7zip\SevenZipWrapper.h"

static LPCTSTR g_sz_Run_GUID = _T("Global\\IPTVChannelEditor.{E4DC62B5-45AD-47AA-A016-512BA5995352}");

#ifdef _DEBUG
std::wstring DEV_PATH = LR"(..\)";
std::wstring PACK_DLL_PATH = LR"(dll\)";
#else
std::wstring DEV_PATH;
std::wstring PACK_DLL_PATH;
#endif // _DEBUG

constexpr auto PACK_DLL = L"7za.dll";

constexpr auto err_download_info = 100;
constexpr auto err_download_pkg  = 102;
constexpr auto err_download_pl   = 103;
constexpr auto err_load_info     = 104;
constexpr auto err_save_info     = 105;
constexpr auto err_save_pkg      = 106;
constexpr auto err_save_pl       = 107;
constexpr auto err_open_pkg      = 108;
constexpr auto err_open_pl       = 109;
constexpr auto err_parse         = 200;

struct update_node
{
	std::wstring name;
	uint32_t crc;
	bool opt;
};

struct UpdateInfo
{
	std::wstring info_file; // name of the update.xml or other file
	std::vector<BYTE> update_info; // content of update_file
	std::wstring update_path; // folder where is downloaded files stored
	std::wstring version; // version of the update
	std::vector<update_node> update_files; // all files to be replaced
	bool install_option_files = false;
};

std::wstring GetAppPath(const wchar_t* szSubFolder /*= nullptr*/)
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
	fileName += szSubFolder;

	return std::filesystem::absolute(fileName);
}


int parse_info(UpdateInfo& info)
{
	// Parse the buffer using the xml file parsing library into doc
	rapidxml::xml_document<> doc;

	try
	{
		doc.parse<0>((char*)info.update_info.data());
	}
	catch (rapidxml::parse_error&)
	{
		return err_parse; // Incorrect update info!
	}

	auto info_node = doc.first_node("update_info");
	if (!info_node)
	{
		return err_parse; // Incorrect update info!
	}

	info.version = rapidxml::get_value_wstring(info_node->first_attribute("version"));
	std::wcout << L"Version: " << info.version << std::endl;

	auto pkg_node = doc.first_node("package");
	if (!pkg_node)
	{
		return err_parse; // Incorrect update info!
	}

	// Iterate <tv_category> nodes
	for (auto file_node = pkg_node->first_node("file"); file_node != nullptr; file_node = file_node->next_sibling())
	{
		update_node node;
		node.name = rapidxml::get_value_wstring(file_node->first_attribute("name"));
		node.crc = rapidxml::get_value_int(file_node->first_attribute("hash"));
		node.opt = rapidxml::get_value_string(file_node->first_attribute("opt")) == "true";
		std::wcout << L"Name: " << node.name << std::endl;
		std::wcout << L"hash: " << node.crc << std::endl;
		std::wcout << L"opt:  " << node.opt << std::endl;
		info.update_files.emplace_back(std::move(node));
	}

	//info.package_name= fmt::format(L"dune_channel_editor_{:s}.7z", info.version);
	//info.playlists_name = fmt::format(L"playlists_{:s}.7z", info.version);

	return 0;
}

int check_for_update(UpdateInfo& info)
{
	if (!utils::DownloadFile(L"http://igores.ru/sharky72/update.xml", info.update_info))
	{
		return err_download_info; // Unable to download update info!
	}

	info.update_info.emplace_back('\0');

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
			std::wcout << L"checking file: " << item.name;
			const auto& loaded_file = fmt::format(L"{:s}{:s}", info.update_path, item.name);
			if (std::filesystem::exists(loaded_file))
			{
				std::ifstream stream(loaded_file, std::istream::binary);
				std::vector<unsigned char> file_data;
				file_data.assign(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
				uint32_t crc = crc32_bitwise(file_data.data(), file_data.size());
				std::wcout << " Ok" << std::endl;
				if (item.crc == crc) continue;
			}

			std::vector<unsigned char> file_data;
			const auto& url = fmt::format(L"http://igores.ru/sharky72/{:s}/{:s}", info.version, item.name);
			std::wcout << L" downloading: " << url << std::endl;
			if (!utils::DownloadFile(url, file_data))
			{
				ret = err_download_pkg; // Unable to download update package!
				break;
			}

			if (!utils::WriteDataToFile(loaded_file, file_data))
			{
				ret = err_save_pkg; // Unable to save update package!
				break;
			}
		}
	} while (false);

	return ret;
}

int update_app(UpdateInfo& info)
{
	int ret = download_update(info);
	if (ret != 0)
		return ret;

	int i = 0;
	HANDLE hAppRunningMutex = OpenMutex(READ_CONTROL, FALSE, g_sz_Run_GUID);
	if (hAppRunningMutex)
	{
		CloseHandle(hAppRunningMutex);

		HWND hwnd = ::FindWindow(nullptr, L"IPTV Channel Editor");
		if (hwnd)
		{
			::PostMessage(hwnd, WM_CLOSE, 0, 0);
			Sleep(500);

			for(;;)
			{
				HANDLE hAppRunningMutex = OpenMutex(READ_CONTROL, FALSE, g_sz_Run_GUID);
				if (!hAppRunningMutex)
				{
					i = 0;
					break;
				}

				CloseHandle(hAppRunningMutex);
				hAppRunningMutex = nullptr;
				Sleep(500);
				i++;
			}
		}
	}

	if (i != 0)
		return -1;

	const auto& dll_path = std::wstring(DEV_PATH) + std::wstring(PACK_DLL_PATH);
	const auto& pack_dll = GetAppPath(dll_path.c_str()) + PACK_DLL;

	const auto& target_path = GetAppPath(L"\\");
	for (const auto& item : info.update_files)
	{
		if (item.opt) continue;

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
				return err_open_pkg;

			folder = true;
			std::wcout << L"unpacking " << src.filename() <<  std::endl;
			if (!archiver.GetExtractor().ExtractArchive(info.update_path))
				return err_open_pkg;

			source_file = info.update_path, src.stem().c_str();
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

				// if file already the same skip it
				if (crc == item.crc)
				{
					std::wcout << item.name << L" not required to updated"<< std::endl;
					continue;
				}
			}

			std::wcout << L"rename " << target_file << L" to " << bak_file << std::endl;
			if (std::filesystem::exists(bak_file))
				std::filesystem::remove_all(bak_file, err);

			std::filesystem::rename(target_file, bak_file, err);
			if (err.value() != 0)
			{
				success = false;
				continue;
			}
		}

		std::filesystem::copy_options opt = std::filesystem::copy_options::none;
		std::wstring type = L"copy file ";
		if (std::filesystem::is_directory(source_file))
		{
			opt = std::filesystem::copy_options::recursive;
			type = L"copy unpacked archive ";
		}

		std::wcout << type << item.name << L" to " << target_file << std::endl;
		std::filesystem::copy(source_file, target_file, opt, err);
		if (err.value() == 0)
		{
			// try to remove backup file (it can't be removed in some case)
			std::filesystem::remove_all(bak_file, err);
		}
	}

	return 0;
}

int main(int argc, char* argv[])
{
	// This variables can be set via the command line.
	bool check = false;
	bool download = false;
	bool update = false;
	bool playlists = false;
	bool printHelp = false;

	// First configure all possible command line options.
	CommandLine args("IPTV Channel Editor Updater 1.0\n");

	args.addArgument({ "check" }, &check, "Check for update");
	args.addArgument({ "download" }, &download, "Download update");
	args.addArgument({ "update" }, &update, "Perform update");
	args.addArgument({ "-p", "--playlists" }, &playlists, "Download or Update playlists");
	args.addArgument({ "-h", "--help" }, &printHelp, "Show parameters info");

	// Then do the actual parsing.
	try
	{
		args.parse(argc, argv);
	}
	catch (std::runtime_error const& e)
	{
		std::cout << e.what() << std::endl;
		return -1;
	}

	UpdateInfo info;
	info.info_file = L"update.xml";
	info.update_path = GetAppPath(L"Updates\\");
	info.install_option_files = playlists;

	std::error_code err;
	if (!std::filesystem::create_directories(info.update_path, err) && err.value())
	{
		return -2; // Unable to create update directory!
	}

	if (check)
	{
		std::cout << "checking.." << std::endl;
		return check_for_update(info);
	}

	if (download)
	{
		std::cout << "downloading.." << std::endl;
		return download_update(info);
	}

	if (update)
	{
		std::cout << "updating.." << std::endl;
		return update_app(info);
	}

	args.printHelp();

	return 0;
}
