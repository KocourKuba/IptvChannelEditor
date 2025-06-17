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
#include "PluginFactory.h"
#include "plugin_antifriz.h"
#include "plugin_edem.h"
#include "plugin_glanz.h"
#include "plugin_iptvonline.h"
#include "plugin_itv.h"
#include "plugin_kineskop.h"
#include "plugin_korona.h"
#include "plugin_oneott.h"
#include "plugin_ottclub.h"
#include "plugin_piktv.h"
#include "plugin_sharaclub.h"
#include "plugin_sharavoz.h"
#include "plugin_shuriktv.h"
#include "plugin_tvclub.h"
#include "plugin_tvizi.h"
#include "plugin_tvteam.h"
#include "plugin_vidok.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// special plugin id. all these plugins has own implementation not covered by configuration
constexpr const char* const antifriz = "antifriz";
constexpr const char* const cbilling = "cbilling";
constexpr const char* const edem = "edem";
constexpr const char* const glanz = "glanz";
constexpr const char* const iptvonline = "iptvonline";
constexpr const char* const itv = "itv";
constexpr const char* const kineskop = "kineskop";
constexpr const char* const korona = "korona";
constexpr const char* const oneott = "oneott";
constexpr const char* const ottclub = "ottclub";
constexpr const char* const piktv = "piktv";
constexpr const char* const sharaclub = "sharaclub";
constexpr const char* const sharavoz = "sharavoz";
constexpr const char* const shuriktv = "shuriktv";
constexpr const char* const tvclub = "tvclub";
constexpr const char* const tvizi = "tvizi";
constexpr const char* const tvteam = "tvteam";
constexpr const char* const vidok = "vidok";
constexpr const char* const custom = "~custom";


std::shared_ptr<base_plugin> PluginFactory::create_plugin(const std::string& type)
{
	std::shared_ptr<base_plugin> plugin;

	const auto& it_s = m_config_storage.find(type);

	if (type == antifriz)
	{
		plugin = std::make_shared<plugin_antifriz>();
	}
	else if (type == cbilling)
	{
		plugin = std::make_shared<plugin_cbilling>();
	}
	else if (type == edem)
	{
		plugin = std::make_shared<plugin_edem>();
	}
	else if (type == glanz)
	{
		plugin = std::make_shared<plugin_glanz>();
	}
	else if (type == iptvonline)
	{
		plugin = std::make_shared<plugin_iptvonline>();
	}
	else if (type == itv)
	{
		plugin = std::make_shared<plugin_itv>();
	}
	else if (type == kineskop)
	{
		plugin = std::make_shared<plugin_kineskop>();
	}
	else if (type == korona)
	{
		plugin = std::make_shared<plugin_korona>();
	}
	else if (type == oneott)
	{
		plugin = std::make_shared<plugin_oneott>();
	}
	else if (type == ottclub)
	{
		plugin = std::make_shared<plugin_ottclub>();
	}
	else if (type == piktv)
	{
		plugin = std::make_shared<plugin_piktv>();
	}
	else if (type == sharaclub)
	{
		plugin = std::make_shared<plugin_sharaclub>();
	}
	else if (type == sharavoz)
	{
		plugin = std::make_shared<plugin_sharavoz>();
	}
	else if (type == shuriktv)
	{
		plugin = std::make_shared<plugin_shuriktv>();
	}
	else if (type == tvclub)
	{
		plugin = std::make_shared<plugin_tvclub>();
	}
	else if (type == tvizi)
	{
		plugin = std::make_shared<plugin_tvizi>();
	}
	else if (type == tvteam)
	{
		plugin = std::make_shared<plugin_tvteam>();
	}
	else if (type == vidok)
	{
		plugin = std::make_shared<plugin_vidok>();
	}
	else if (it_s != m_config_storage.end())
	{
		plugin = std::make_shared<base_plugin>();
	}

	if (plugin)
	{
		plugin->set_plugin_type(type);
		plugin->set_internal_name(type);
		plugin->copy_config(it_s->second);
	}

	return plugin;
}

EpgParameters PluginFactory::get_epg_preset(const std::string& name) const
{
	if (const auto& pair = m_known_presets.find(name); pair != m_known_presets.end())
	{
		return pair->second;
	}

	return {};
}

const std::map<std::string, EpgParameters>& PluginFactory::get_epg_presets() const
{
	return m_known_presets;
}

IconPackInfo PluginFactory::get_icon_pack_info(const size_t idx) const
{
	if (idx < m_image_libs.size())
	{
		return m_image_libs[idx];
	}

	return {};
}

const std::vector<IconPackInfo>& PluginFactory::get_icon_packs() const
{
	return m_image_libs;
}

bool PluginFactory::load_configs(bool dev /*= false*/)
{
	bool res = false;
	std::stringstream data;
	m_known_presets.clear();

	if (!dev)
	{
		utils::CUrlDownload dl;
		dl.SetUrl(fmt::format(L"{:s}/editor/configs?ver={:d}.{:d}.{:d}", utils::utf8_to_utf16(g_szServerPath), MAJOR, MINOR, BUILD));
		dl.SetUserAgent(fmt::format(L"IPTV Channel Editor/{:d}.{:d}.{:d}", MAJOR, MINOR, BUILD));
		dl.SetCacheTtl(0);

		if (!dl.DownloadFile(data))
		{
			data.clear();
		}
	}

	if (data.tellp() == std::streampos(0))
	{
		std::wstring path;
		path = fmt::format(L"{:s}defaults_{:d}.{:d}.json", GetAppPath(), MAJOR, MINOR);
		std::ifstream in_file(path);
		if (in_file.good())
		{
			data << in_file.rdbuf();
			in_file.close();
		}
	}

	JSON_ALL_TRY
	{
		nlohmann::json config = nlohmann::json::parse(data.str());

		for (const auto& item : config["epg_presets"].items())
		{
			EpgParameters preset;
			EpgParameters::from_json_wrapper(item.value(), preset);
			m_known_presets.emplace(item.key(), preset);
		}

		for (const auto& item : config["image_libs"].items())
		{
			IconPackInfo preset;
			IconPackInfo::from_json_wrapper(item.value(), preset);
			m_image_libs.emplace_back(preset);
		}

		for (const auto& item : config["plugins"].items())
		{
			plugin_config cfg;
			cfg.clear();
			plugin_config::from_json_wrapper(item.value(), cfg);
			if (cfg.get_enabled())
			{
				m_config_storage.emplace(item.key(), cfg);
			}
		}

		res = !m_config_storage.empty();
	}
	JSON_ALL_CATCH;

	return res;
}

const std::map<std::string, plugin_config>& PluginFactory::get_all_configs() const
{
	return m_config_storage;
}

const plugin_config PluginFactory::get_config(const std::string& type) const
{
	const auto it = m_config_storage.find(type);
	return it != m_config_storage.end() ? it->second : plugin_config();
}