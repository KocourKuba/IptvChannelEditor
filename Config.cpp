#include "StdAfx.h"
#include "Config.h"
#include "resource.h"
#include "utils.h"

#include "SevenZip/7zip/SevenZipWrapper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
CString PluginsConfig::DEV_PATH = LR"(..\)";
CString PluginsConfig::PACK_DLL_PATH = LR"(dll\)";
#else
CString PluginsConfig::DEV_PATH;
CString PluginsConfig::PACK_DLL_PATH;
#endif // _DEBUG

// special case for run under debugger from VS
constexpr auto PACK_DLL = L"7za.dll";

using namespace SevenZip;

static std::vector<std::wstring> pics_to_remove = {
	L"bg_antifriz.jpg",   L"logo_antifriz.png",
	L"bg_edem.jpg",       L"logo_edem.png",
	L"bg_fox.jpg",        L"logo_fox.png",
	L"bg_glanz.jpg",      L"logo_glanz.png",
	L"bg_itv.jpg",        L"logo_itv.png",
	L"bg_onecent.jpg",    L"logo_onecent.png",
	L"bg_oneusd.jpg",     L"logo_oneusd.png",
	L"bg_sharaclub.jpg",  L"logo_sharaclub.png",
	L"bg_sharatv.jpg",    L"logo_sharatv.png",
	L"bg_sharavoz.jpg",   L"logo_sharavoz.png",
	L"bg_tvteam.jpg",     L"logo_tvteam.png",
	L"bg_viplime.jpg",    L"logo_viplime.png",
};

static std::vector<PluginDesc> all_plugins = {
	{ StreamType::enAntifriz,  _T("Antifriz"),        "antifriz"   },
	{ StreamType::enEdem,      _T("Edem (iLook TV)"), "edem"       },
	{ StreamType::enFox,       _T("Fox TV"),          "fox"        },
	{ StreamType::enGlanz,     _T("Glanz TV"),        "glanz"      },
	{ StreamType::enItv,       _T("ITV"),             "itv"        },
	{ StreamType::enSharaclub, _T("Sharaclub TV"),    "sharaclub"  },
	{ StreamType::enSharavoz,  _T("Sharavoz TV"),     "sharavoz"   },
	{ StreamType::enOneCent,   _T("1CENT TV"),        "onecent"    },
	{ StreamType::enOneUsd,    _T("1USD TV"),         "oneusd"     },
	{ StreamType::enVipLime,   _T("VipLime TV"),      "viplime"    },
	{ StreamType::enSharaTV,   _T("Shara TV"),        "sharatv"    },
	{ StreamType::enTvTeam,    _T("TV Team"),         "tvteam"     },
};

const std::vector<PluginDesc>& PluginsConfig::get_plugins_info()
{
	return all_plugins;
}

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

	fileName += PluginsConfig::DEV_PATH + szSubFolder;

	return std::filesystem::absolute(fileName.GetString());
}

bool PackPlugin(const StreamType plugin_type,
				const std::wstring& output_path,
				const std::wstring& lists_path,
				bool showMessage)
{
	const auto& name = PluginsConfig::GetPluginName<wchar_t>(plugin_type);
	auto& temp_pack_path = std::filesystem::temp_directory_path();
	temp_pack_path += utils::PACK_PATH;
	const auto& packFolder = fmt::format(temp_pack_path.c_str(), name);

	std::error_code err;
	// remove previous packed folder if exist
	std::filesystem::remove_all(packFolder, err);

	// copy new one
	const auto& plugin_root = GetAppPath(utils::PLUGIN_ROOT);
	std::filesystem::copy(plugin_root, packFolder, std::filesystem::copy_options::recursive, err);

	// copy plugin manifest
	const auto& manifest = fmt::format(LR"({:s}manifest\{:s}_plugin.xml)", plugin_root, name);
	const auto& config = fmt::format(LR"({:s}configs\{:s}_config.php)", plugin_root, name);
	std::filesystem::copy_file(manifest, packFolder + L"dune_plugin.xml", std::filesystem::copy_options::overwrite_existing, err);
	std::filesystem::copy_file(config, fmt::format(L"{:s}{:s}_config.php", packFolder, name), std::filesystem::copy_options::overwrite_existing, err);

	// remove over config's
	std::filesystem::remove_all(packFolder + L"manifest", err);
	std::filesystem::remove_all(packFolder + L"configs", err);

	// copy channel lists
	const auto& playlistPath = fmt::format(L"{:s}{:s}\\", lists_path, name);
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

	// remove files for other plugins
	std::vector<std::wstring> to_remove(pics_to_remove);
	to_remove.erase(std::remove(to_remove.begin(), to_remove.end(), fmt::format(L"bg_{:s}.jpg", name)), to_remove.end());
	to_remove.erase(std::remove(to_remove.begin(), to_remove.end(), fmt::format(L"logo_{:s}.png", name)), to_remove.end());

	for (const auto& dir_entry : std::filesystem::directory_iterator{ packFolder + LR"(icons\)" })
	{
		if (std::find(to_remove.begin(), to_remove.end(), dir_entry.path().filename().wstring()) != to_remove.end())
			std::filesystem::remove(dir_entry, err);
	}

	// write setup file
	unsigned char smarker[3] = { 0xEF, 0xBB, 0xBF }; // UTF8 BOM
	std::ofstream os(packFolder + _T("plugin_type.php"), std::ios::out | std::ios::binary);
	os.write((const char*)smarker, sizeof(smarker));
	os << fmt::format("<?php\nrequire_once '{:s}_config.php';\n\nconst PLUGIN_TYPE = '{:s}PluginConfig';\nconst PLUGIN_BUILD = {:d};\nconst PLUGIN_DATE = '{:s}';\n",
					  PluginsConfig::GetPluginName<char>(plugin_type),
					  PluginsConfig::GetPluginName<char>(plugin_type, true),
					  BUILD,
					  RELEASEDATE
	);
	os.close();

	// pack folder
	SevenZipWrapper archiver(GetAppPath(PluginsConfig::PACK_DLL_PATH + PACK_DLL));
	archiver.GetCompressor().SetCompressionFormat(CompressionFormat::Zip);
	bool res = archiver.GetCompressor().AddFiles(packFolder, _T("*.*"), true);
	if (!res)
	{
		AfxMessageBox(_T("Some file missing!!!"), MB_OK | MB_ICONSTOP);
		return false;
	}

	const auto& pluginFile = output_path + fmt::format(utils::DUNE_PLUGIN_NAME, name);

	res = archiver.CreateArchive(pluginFile);
	// remove temporary folder
	std::filesystem::remove_all(packFolder, err);
	if (!res)
	{
		if (showMessage)
		{
			std::filesystem::remove(pluginFile, err);
			AfxMessageBox(IDS_STRING_ERR_FAILED_PACK, MB_OK | MB_ICONSTOP);
		}
		return false;
	}

	if (showMessage)
		AfxMessageBox(IDS_STRING_INFO_CREATE_SUCCESS, MB_OK);

	return true;
}
