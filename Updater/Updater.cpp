// Updater.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include <tchar.h>
#include <iostream>
#include <wtypes.h>

#include "CommandLine.hpp"
#include "utils.h"

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


int ParseInfo(std::wstring& version)
{
	const auto& updateFile = GetAppPath(L"Updates\\") + L"update.xml";
	std::ifstream stream(updateFile);
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

	version = utils::get_value_wstring(i_node->first_node("version"));
	return 0;
}

int check_for_update()
{
	int ret = 0;

	std::vector<BYTE> update_info;
	if (!utils::DownloadFile(L"http://igores.ru/sharky72/update.xml", update_info))
	{
		return err_download_info; // Unable to download update info!
	}

	const auto& updateFile = GetAppPath(L"Updates\\") + L"update.xml";
	if (!utils::WriteDataToFile(updateFile, update_info))
	{
		return err_save_info; // Unable to save update info!
	}

	std::wstring version;
	return ParseInfo(version);
}

int download_update(bool playlists)
{
	int ret = 0;
	do
	{
		const auto& updateFolder = GetAppPath(L"Updates\\");
		const auto& updateFile = updateFolder + L"update.xml";
		if (!std::filesystem::exists(updateFile))
		{
			ret = check_for_update();
			if (ret != 0) break;
		}

		std::wstring version;
		ret = ParseInfo(version);
		if (ret != 0) break;

		const auto& package_name = fmt::format(L"dune_channel_editor_{:s}.7z", version);
		std::vector<BYTE> update_package;
		if (!utils::DownloadFile(L"http://igores.ru/sharky72/" + package_name, update_package))
		{
			ret = err_download_pkg; // Unable to download update package!
			break;
		}

		if (!utils::WriteDataToFile(updateFolder + package_name, update_package))
		{
			ret = err_save_pkg; // Unable to save update package!
			break;
		}

		const auto& channels_name = fmt::format(L"playlists_{:s}.7z", version);
		std::vector<BYTE> update_channels;
		if (playlists)
		{
			if (!utils::DownloadFile(L"http://igores.ru/sharky72/" + channels_name, update_channels))
			{
				ret = err_download_pl; // Unable to download channels list!
				break;
			}

			if (!update_channels.empty() && !utils::WriteDataToFile(updateFolder + channels_name, update_channels))
			{
				ret = err_save_pl; // Unable to save update package!
				break;
			}
		}

	} while (false);

	return ret;
}

int update_app()
{
	std::wstring version;
	int ret = ParseInfo(version);
	if (ret != 0) return ret;

	if (version.empty()) return err_load_info;

	const auto& updates_path = GetAppPath(L"Updates\\");
	const auto& package_name = fmt::format(L"{:s}dune_channel_editor_{:s}.7z", updates_path, version);
	const auto& channels_name = fmt::format(L"{:s}playlists_{:s}.7z", updates_path, version);

	const auto& pack_dll = GetAppPath(PACK_DLL_PATH.c_str()) + PACK_DLL;
	SevenZip::SevenZipWrapper archiver(pack_dll);
	if (!archiver.OpenArchive(package_name))
		return err_open_pkg;

	const auto& packed_files = archiver.GetExtractor().GetItemsNames();
	if (!archiver.GetExtractor().ExtractArchive(updates_path + L"package"))
		return err_open_pkg;

	const auto& root = GetAppPath(L"\\");
	for (const auto& file : packed_files)
	{
		if (file.find('\\') != std::wstring::npos) continue;

		const auto& old_file = root + file;
		const auto& bak_file = old_file + L".bak";
		std::error_code err;
		std::filesystem::rename(old_file, bak_file, err);
		if (!err.value())
		{
			const auto& new_file = fmt::format(L"{:s}package\\{:s}", updates_path, file);
			std::filesystem::copy(new_file, old_file, std::filesystem::copy_options::recursive, err);
		}
	}

	if (!archiver.OpenArchive(channels_name))
		return err_open_pl;

	if (!archiver.GetExtractor().ExtractArchive(updates_path))
		return err_open_pl;

	std::error_code err;
	std::filesystem::rename(root + L"playlists", root + L"playlists.bak", err);
	std::filesystem::copy(updates_path + L"playlists", root, std::filesystem::copy_options::recursive, err);

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

	if (check)
	{
		std::cout << "checking.." << std::endl;
		return check_for_update();
	}

	if (download)
	{
		std::cout << "downloading.." << std::endl;
		return download_update(playlists);
	}

	if (update)
	{
		std::cout << "updating.." << std::endl;
		return update_app();
	}

	args.printHelp();

	return 0;
}
