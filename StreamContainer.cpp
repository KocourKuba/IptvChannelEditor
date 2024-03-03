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
#include "StreamContainer.h"
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

static std::vector <std::pair<PluginType, std::string>> type_to_name = {
	{ PluginType::enCustom,     "custom"     },
	{ PluginType::en101film,    "101film"    },
	{ PluginType::enAntifriz,   "antifriz"   },
	{ PluginType::enBcuMedia,   "bcumedia"   },
	{ PluginType::enCbilling,   "cbilling"   },
	{ PluginType::enCRDTV,      "crdtv"      },
	{ PluginType::enEdem,       "edem"       },
	{ PluginType::enFilmax,     "filmax"     },
	{ PluginType::enFox,        "fox"        },
	{ PluginType::enGlanz,      "glanz"      },
	{ PluginType::enIptvOnline, "iptvonline" },
	{ PluginType::enIpstream,   "ipstream"   },
	{ PluginType::enItv,        "itv"        },
	{ PluginType::enKineskop,   "kineskop"   },
	{ PluginType::enLightIptv,  "lightiptv"  },
	{ PluginType::enMymagic,    "mymagic"    },
	{ PluginType::enOneCent,    "onecent"    },
	{ PluginType::enOneOtt,     "oneott"     },
	{ PluginType::enOneUsd,     "oneusd"     },
	{ PluginType::enOnlineOtt,  "onlineott"  },
	{ PluginType::enOttclub,    "ottclub"    },
	{ PluginType::enOttIptv,    "ottiptv"    },
	{ PluginType::enPing,       "ping"       },
	{ PluginType::enRusskoeTV,  "russkoetv"  },
	{ PluginType::enRuTV,       "rutv"       },
	{ PluginType::enSatq,       "satq"       },
	{ PluginType::enSharaTV,    "sharatv"    },
	{ PluginType::enSharaclub,  "sharaclub"  },
	{ PluginType::enSharavoz,   "sharavoz"   },
	{ PluginType::enShuraTV,    "shuratv"    },
	{ PluginType::enSmile,      "smile"      },
	{ PluginType::enTVClub,     "tvclub"     },
	{ PluginType::enTvizi,      "tvizi"      },
	{ PluginType::enTvTeam,     "tvteam"     },
	{ PluginType::enVidok,      "vidok"      },
	{ PluginType::enVipLime,    "viplime"    },
	{ PluginType::enYossoTV,    "yosso"      },
};

std::shared_ptr<base_plugin> StreamContainer::create_plugin(PluginType type)
{
	std::shared_ptr<base_plugin> plugin;

	const auto& it = std::find_if(type_to_name.begin(), type_to_name.end(), [&type](const auto& pair) { return pair.first == type; });

	if (it != type_to_name.end())
	{
		const auto& name = it->second;
		switch (type)
		{
			case PluginType::enAntifriz:
				plugin = std::make_shared<plugin_antifriz>(name);
				break;

			case PluginType::enCbilling:
				plugin = std::make_shared<plugin_cbilling>(name);
				break;

			case PluginType::enItv:
				plugin = std::make_shared<plugin_itv>(name);
				break;

			case PluginType::enOneOtt:
				plugin = std::make_shared<plugin_oneott>(name);
				break;

			case PluginType::enSharaclub:
				plugin = std::make_shared<plugin_sharaclub>(name);
				break;

			case PluginType::enTVClub:
				plugin = std::make_shared<plugin_tvclub>(name);
				break;

			case PluginType::enVidok:
				plugin = std::make_shared<plugin_vidok>(name);
				break;

			default:
				plugin = std::make_shared<base_plugin>(name);
				break;
		}

		if (plugin)
		{
			plugin->set_config(type, m_config_storage[name]);
		}
	}

	return plugin;
}

bool StreamContainer::load_configs()
{
	bool res = false;
	std::stringstream data;
	const auto& url = fmt::format(L"http://iptv.esalecrm.net/editor/configs?ver={:d}.{:d}.{:d}", MAJOR, MINOR, BUILD);
	utils::CUrlDownload dl;
	dl.SetUserAgent(fmt::format(L"IPTV Channel Editor/{:d}.{:d}.{:d}", MAJOR, MINOR, BUILD));
	dl.SetCacheTtl(3600);

	if (!dl.DownloadFile(url, data))
	{
		std::ifstream in_file(GetAppPath() + L"defaults.json");
		if (in_file.good())
		{
			data << in_file.rdbuf();
			in_file.close();
		}
	}

	JSON_ALL_TRY
	{
		nlohmann::json config = nlohmann::json::parse(data.str());
		for (const auto& item : config["plugins"].items())
		{
			plugin_config cfg;
			plugin_config::from_json(item, cfg);
			m_config_storage.emplace(item.key(), cfg);
		}
		res = true;
	}
	JSON_ALL_CATCH;

	return res;
}
