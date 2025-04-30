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

static std::vector<std::pair<PluginType, std::string>> s_all_plugins = {
	{ PluginType::enAntifriz,   "antifriz"   },
	{ PluginType::enBcuMedia,   "bcumedia"   },
	{ PluginType::enCbilling,   "cbilling"   },
	{ PluginType::enEdem,       "edem"       },
	{ PluginType::enFilmax,     "filmax"     },
	{ PluginType::enFox,        "fox"        },
	{ PluginType::enGlanz,      "glanz"      },
	{ PluginType::enIptvOnline, "iptvonline" },
	{ PluginType::enItv,        "itv"        },
	{ PluginType::enKineskop,   "kineskop"   },
	{ PluginType::enLightIptv,  "lightiptv"  },
	{ PluginType::enMymagic,    "mymagic"    },
	{ PluginType::enOneCent,    "onecent"    },
	{ PluginType::enOneOtt,     "oneott"     },
	{ PluginType::enOneUsd,     "oneusd"     },
	{ PluginType::enOttclub,    "ottclub"    },
	{ PluginType::enPing,       "ping"       },
	{ PluginType::enRusskoeTV,  "russkoetv"  },
	{ PluginType::enSharaTV,    "sharatv"    },
	{ PluginType::enSharaclub,  "sharaclub"  },
	{ PluginType::enSharavoz,   "sharavoz"   },
	{ PluginType::enShuraTV,    "shuratv"    },
	{ PluginType::enSmile,      "smile"      },
	{ PluginType::enTVClub,     "tvclub"     },
	{ PluginType::enTvTeam,     "tvteam"     },
	{ PluginType::enVidok,      "vidok"      },
	{ PluginType::enVipLime,    "viplime"    },
	{ PluginType::enYossoTV,    "yosso"      },
	{ PluginType::en101film,    "101film"    },
	{ PluginType::enIpstream,   "ipstream"   },
	{ PluginType::enOnlineOtt,  "onlineott"  },
	{ PluginType::enTvizi,      "tvizi"      },
	{ PluginType::enSatq,       "satq"       },
	{ PluginType::enRuTV,       "rutv"       },
	{ PluginType::enCRDTV,      "crdtv"      },
	{ PluginType::enBitTV,      "bittv"      },
	{ PluginType::enKliMedia,   "klimedia"   },
	{ PluginType::enTopIPTV,    "topiptv"    },
	{ PluginType::enIptvBest,   "iptvbest"   },
	{ PluginType::enUspeh,      "uspeh"      },
	{ PluginType::enNasharu,    "nasharu"    },
	{ PluginType::enOttPub,     "ottpub"     },
	{ PluginType::enBlinkTV,    "blinktv"    },
	{ PluginType::enPeakTV,     "peaktv"     },
	{ PluginType::enKorona,     "korona"     },
	{ PluginType::enPikTV,      "piktv"      },
	{ PluginType::enVelestore,  "velestore"  },
	{ PluginType::enIptvPlay,   "iptvplay"   },
	{ PluginType::enHnMedia,    "hnmedia"    },
	{ PluginType::enShurikTV,   "shuriktv"   },
	{ PluginType::en2tv,        "2tv"        },
	{ PluginType::enCustom,     "custom"     },
};

std::shared_ptr<base_plugin> PluginFactory::create_plugin(PluginType type)
{
	std::shared_ptr<base_plugin> plugin;

	const auto& it = std::find_if(s_all_plugins.begin(), s_all_plugins.end(), [&type](const std::pair<PluginType, std::string>& val)
								  {
									  return val.first == type;
								  });

	if (it != s_all_plugins.end())
	{
		switch (type)
		{
			case PluginType::enAntifriz:
				plugin = std::make_shared<plugin_antifriz>();
				break;

			case PluginType::enCbilling:
				plugin = std::make_shared<plugin_cbilling>();
				break;

			case PluginType::enEdem:
				plugin = std::make_shared<plugin_edem>();
				break;

			case PluginType::enGlanz:
				plugin = std::make_shared<plugin_glanz>();
				break;

			case PluginType::enIptvOnline:
				plugin = std::make_shared<plugin_iptvonline>();
				break;

			case PluginType::enItv:
				plugin = std::make_shared<plugin_itv>();
				break;

			case PluginType::enKineskop:
				plugin = std::make_shared<plugin_kineskop>();
				break;

			case PluginType::enKorona:
				plugin = std::make_shared<plugin_korona>();
				break;

			case PluginType::enOneOtt:
				plugin = std::make_shared<plugin_oneott>();
				break;

			case PluginType::enOttclub:
				plugin = std::make_shared<plugin_ottclub>();
				break;

			case PluginType::enPikTV:
				plugin = std::make_shared<plugin_piktv>();
				break;

			case PluginType::enSharaclub:
				plugin = std::make_shared<plugin_sharaclub>();
				break;

			case PluginType::enSharavoz:
				plugin = std::make_shared<plugin_sharavoz>();
				break;

			case PluginType::enShurikTV:
				plugin = std::make_shared<plugin_shuriktv>();
				break;

			case PluginType::enTVClub:
				plugin = std::make_shared<plugin_tvclub>();
				break;

			case PluginType::enTvizi:
				plugin = std::make_shared<plugin_tvizi>();
				break;

			case PluginType::enTvTeam:
				plugin = std::make_shared<plugin_tvteam>();
				break;

			case PluginType::enVidok:
				plugin = std::make_shared<plugin_vidok>();
				break;

			default:
				plugin = std::make_shared<base_plugin>();
				break;
		}

		if (plugin)
		{
			plugin->set_plugin_type(type);
			plugin->set_internal_name(it->second);
			const auto& it_s = m_config_storage.find(it->second);
			if (it_s != m_config_storage.end())
			{
				plugin->copy_config(it_s->second);
			}
		}
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

		for (const auto& item : config["plugins"].items())
		{
			plugin_config cfg;
			cfg.clear();
			plugin_config::from_json_wrapper(item.value(), cfg);
			m_config_storage.emplace(item.key(), cfg);
		}

		for (const auto& item : config["image_libs"].items())
		{
			IconPackInfo preset;
			IconPackInfo::from_json_wrapper(item.value(), preset);
			m_image_libs.emplace_back(preset);
		}

		res = true;
	}
	JSON_ALL_CATCH;

	return res;
}

const std::vector<std::pair<PluginType, std::string>>& PluginFactory::get_all_plugins() const
{
	return s_all_plugins;
}

PluginType PluginFactory::get_plugin_type(size_t idx)
{
	return idx < s_all_plugins.size() ? s_all_plugins[idx].first : s_all_plugins[0].first;
}
