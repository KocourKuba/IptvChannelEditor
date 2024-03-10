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
#include "PluginFactory.h"
#include "plugin_antifriz.h"
#include "plugin_itv.h"
#include "plugin_oneott.h"
#include "plugin_sharaclub.h"
#include "plugin_cbilling.h"
#include "plugin_vidok.h"
#include "plugin_tvclub.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
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
	{ PluginType::enOttIptv,    "ottiptv"    },
	{ PluginType::en101film,    "101film"    },
	{ PluginType::enIpstream,   "ipstream"   },
	{ PluginType::enOnlineOtt,  "onlineott"  },
	{ PluginType::enTvizi,      "tvizi"      },
	{ PluginType::enSatq,       "satq"       },
	{ PluginType::enRuTV,       "rutv"       },
	{ PluginType::enCRDTV,      "crdtv"      },
	{ PluginType::enCustom,     "custom"     },
};

static std::vector <std::pair<EpgPresets, std::string>> s_presets = {
	{ EpgPresets::enDRM,         "drm"           },
	{ EpgPresets::enIptvxOne,    "iptvx.one"     },
	{ EpgPresets::enCbilling,    "cbilling"      },
	{ EpgPresets::enItvLive,     "itvlive"       },
	{ EpgPresets::enPropgNet,    "propg.net"     },
	{ EpgPresets::enTVClub,      "tvclub"        },
	{ EpgPresets::enVidok,       "vidok"         },
	{ EpgPresets::enMyEPGServer, "my.epg.server" },
	{ EpgPresets::enOttClub,     "ottclub"       },
	{ EpgPresets::enTVTeam,      "tvteam"        },
	{ EpgPresets::enSharaClub,   "sharaclub"     },
	{ EpgPresets::enSharavoz,    "sharavoz"      },
	{ EpgPresets::enCustom,      "Custom"        },
	{ EpgPresets::enLast,        "Last"          },
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

			case PluginType::enItv:
				plugin = std::make_shared<plugin_itv>();
				break;

			case PluginType::enOneOtt:
				plugin = std::make_shared<plugin_oneott>();
				break;

			case PluginType::enSharaclub:
				plugin = std::make_shared<plugin_sharaclub>();
				break;

			case PluginType::enTVClub:
				plugin = std::make_shared<plugin_tvclub>();
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

EpgParameters PluginFactory::get_epg_preset(EpgPresets idx) const
{
	if (idx < EpgPresets::enCustom)
		return known_presets[(size_t)idx];

	return {};
}

bool PluginFactory::load_configs()
{
	bool res = false;
	std::stringstream data;

#ifndef _DEBUG
	const auto& url = fmt::format(L"http://iptv.esalecrm.net/editor/configs?ver={:d}.{:d}.{:d}", MAJOR, MINOR, BUILD);
	utils::CUrlDownload dl;
	dl.SetUserAgent(fmt::format(L"IPTV Channel Editor/{:d}.{:d}.{:d}", MAJOR, MINOR, BUILD));
	dl.SetCacheTtl(3600);

	if (!dl.DownloadFile(url, data))
	{
#endif // _DEBUG
		std::ifstream in_file(GetAppPath() + L"defaults_8.0.json");
		if (in_file.good())
		{
			data << in_file.rdbuf();
			in_file.close();
		}
#ifndef _DEBUG
	}
#endif // _DEBUG

	JSON_ALL_TRY
	{
		nlohmann::json config = nlohmann::json::parse(data.str());
		for (const auto& item : config["epg_presets"].items())
		{
			const auto& it = std::find_if(s_presets.begin(), s_presets.end(), [&item](const auto& pair) { return pair.second == item.key(); });
			if (it != s_presets.end())
			{
				EpgParameters preset;
				EpgParameters::from_json_wrapper(item.value(), preset);
				known_presets[(size_t)it->first] = preset;
			}
		}

		for (const auto& item : config["plugins"].items())
		{
			plugin_config cfg;
			plugin_config::from_json_wrapper(item.value(), cfg);
			m_config_storage.emplace(item.key(), cfg);
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
	return idx < s_all_plugins.size() ? s_all_plugins[idx].first : PluginType::enEdem;
}
