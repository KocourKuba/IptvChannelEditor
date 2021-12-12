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

#include "SevenZip\7zip\SevenZipWrapper.h"

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

struct UpdateInfo
{
	std::wstring version;
	std::wstring update_path;
	std::wstring info_file;
	std::wstring package_name;
	std::wstring playlists_name;
	bool force = false;
	bool playlists = false;
};

std::wstring GetAppPath(const wchar_t* szSubFolder /*= nullptr*/)
{
	std::wstring fileName;
	fileName.reserve((_MAX_PATH));

	if (GetModuleFileNameW(nullptr, fileName.data(), _MAX_PATH) != 0)
	{
		size_t pos = fileName.rfind('\\');
		if (pos != std::wstring::npos)
			fileName.resize(pos + 1);
	}

	fileName += DEV_PATH;
	fileName += szSubFolder;

	return std::filesystem::absolute(fileName);
}


int parse_info(UpdateInfo& info)
{
	std::ifstream stream(info.update_path + info.info_file);
	if (!stream.good())
	{
		return err_load_info; // Unable to load update info!
	}

	std::vector<BYTE> update_info;
	update_info.assign((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
	update_info.emplace_back('\0');

	// Parse the buffer using the xml file parsing library into doc
	rapidxml::xml_document<> doc;

	try
	{
		doc.parse<0>((char*)update_info.data());
	}
	catch (rapidxml::parse_error&)
	{
		return err_parse; // Incorrect update info!
	}

	auto i_node = doc.first_node("update_info");
	if (!i_node)
	{
		return err_parse; // Incorrect update info!
	}

	info.version = rapidxml::get_value_wstring(i_node->first_node("version"));
	info.package_name= fmt::format(L"dune_channel_editor_{:s}.7z", info.version);
	info.playlists_name = fmt::format(L"dune_channel_editor_{:s}.7z", info.version);

	return 0;
}

int check_for_update(UpdateInfo& info)
{
	int ret = 0;

	const auto& updateFile = info.update_path + info.info_file;
	if (info.force || !std::filesystem::exists(updateFile))
	{
		std::vector<BYTE> update_info;
		if (!utils::DownloadFile(L"http://igores.ru/sharky72/update.xml", update_info))
		{
			return err_download_info; // Unable to download update info!
		}

		if (!utils::WriteDataToFile(updateFile, update_info))
		{
			return err_save_info; // Unable to save update info!
		}
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

		if (!std::filesystem::exists(info.update_path + info.package_name))
		{
			std::vector<BYTE> package_data;
			if (!utils::DownloadFile(L"http://igores.ru/sharky72/" + info.package_name, package_data))
			{
				ret = err_download_pkg; // Unable to download update package!
				break;
			}

			if (!utils::WriteDataToFile(info.update_path + info.package_name, package_data))
			{
				ret = err_save_pkg; // Unable to save update package!
				break;
			}
		}

		if (info.playlists && !std::filesystem::exists(info.update_path + info.playlists_name))
		{
			std::vector<BYTE> playlists_data;
			if (!utils::DownloadFile(L"http://igores.ru/sharky72/" + info.playlists_name, playlists_data))
			{
				ret = err_download_pl; // Unable to download channels list!
				break;
			}

			if (!playlists_data.empty() && !utils::WriteDataToFile(info.update_path + info.playlists_name, playlists_data))
			{
				ret = err_save_pl; // Unable to save update channels list!
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

	const auto& pack_dll = GetAppPath(PACK_DLL_PATH.c_str()) + PACK_DLL;
	SevenZip::SevenZipWrapper archiver(pack_dll);
	if (!archiver.OpenArchive(info.update_path + info.package_name))
		return err_open_pkg;

	const auto& packed_files = archiver.GetExtractor().GetItemsNames();
	const auto& unpack_pkg_folder = fmt::format(L"{:s}dune_package_{:s}", info.update_path, info.version);
	if (!archiver.GetExtractor().ExtractArchive(unpack_pkg_folder))
		return err_open_pkg;

	const auto& root = GetAppPath(L"\\");
	for (const auto& file : packed_files)
	{
		// rename only root items (files and folders)
		if (file.find('\\') != std::wstring::npos) continue;

		const auto& old_file = root + file;
		const auto& bak_file = old_file + L".bak";
		std::error_code err;
		std::filesystem::rename(old_file, bak_file, err);
		if (!err.value())
		{
			const auto& new_file = unpack_pkg_folder + L"\\" + file;
			std::filesystem::copy(new_file, old_file, std::filesystem::copy_options::recursive, err);
		}
	}

	if (info.playlists)
	{
		const auto& unpack_playlists_folder = fmt::format(L"{:s}playlists_{:s}", info.update_path, info.version);
		if (!archiver.OpenArchive(info.update_path + info.playlists_name))
			return err_open_pl;

		if (!archiver.GetExtractor().ExtractArchive(unpack_playlists_folder))
			return err_open_pl;

		std::error_code err;
		std::filesystem::rename(root + L"playlists", root + L"playlists.bak", err);
		std::filesystem::copy(unpack_playlists_folder, root, std::filesystem::copy_options::recursive, err);
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

	args.addArgument({ "-c", "--check" }, &check, "Check for update");
	args.addArgument({ "-d", "--download" }, &download, "Download update");
	args.addArgument({ "-p", "--playlists" }, &playlists, "Download playlists");
	args.addArgument({ "-u", "--update" }, &update, "Perform update");
	args.addArgument({ "-h", "--help" }, &printHelp, "Show parameters info");

	std::error_code err;
	const auto& updateFolder = GetAppPath(L"Updates\\");
	if (!std::filesystem::create_directories(updateFolder, err) && err.value())
	{
		return -2; // Unable to create update directory!
	}

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
	info.playlists = playlists;

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
