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
#include "StreamContainer.h"
#include "plugin_antifriz.h"
#include "plugin_edem.h"
#include "plugin_fox.h"
#include "plugin_glanz.h"
#include "plugin_itv.h"
#include "plugin_onecent.h"
#include "plugin_oneott.h"
#include "plugin_oneusd.h"
#include "plugin_sharaclub.h"
#include "plugin_sharatv.h"
#include "plugin_sharavoz.h"
#include "plugin_tvteam.h"
#include "plugin_viplime.h"
#include "plugin_lightiptv.h"
#include "plugin_cbilling.h"
#include "plugin_ottclub.h"
#include "plugin_iptvonline.h"
#include "plugin_vidok.h"
#include "plugin_shuratv.h"
#include "plugin_tvclub.h"
#include "plugin_filmax.h"
#include "plugin_kineskop.h"
#include "plugin_mymagic.h"
#include "plugin_russkoetv.h"
#include "plugin_smile.h"
#include "plugin_ping.h"
#include "plugin_yosso.h"
#include "plugin_ottiptv.h"
#include "plugin_bcumedia.h"
#include "plugin_101film.h"
#include "plugin_ipstream.h"
#include "plugin_onlineott.h"
#include "plugin_tvizi.h"
#include "plugin_satq.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

std::shared_ptr<base_plugin> StreamContainer::get_instance(PluginType type)
{
	std::shared_ptr<base_plugin> plugin;
	switch (type)
	{
		case PluginType::enCustom:
			plugin = std::make_shared<base_plugin>();
			break;

		case PluginType::enAntifriz:
			plugin = std::make_shared<plugin_antifriz>();
			break;

		case PluginType::enBcuMedia:
			plugin = std::make_shared<plugin_bcumedia>();
			break;

		case PluginType::enEdem:
			plugin = std::make_shared<plugin_edem>();
			break;

		case PluginType::enFox:
			plugin = std::make_shared<plugin_fox>();
			break;

		case PluginType::enGlanz:
			plugin = std::make_shared<plugin_glanz>();
			break;

		case PluginType::enItv:
			plugin = std::make_shared<plugin_itv>();
			break;

		case PluginType::enOneCent:
			plugin = std::make_shared<plugin_onecent>();
			break;

		case PluginType::enOneUsd:
			plugin = std::make_shared<plugin_oneusd>();
			break;

		case PluginType::enSharaclub:
			plugin = std::make_shared<plugin_sharaclub>();
			break;

		case PluginType::enSharavoz:
			plugin = std::make_shared<plugin_sharavoz>();
			break;

		case PluginType::enVipLime:
			plugin = std::make_shared<plugin_viplime>();
			break;

		case PluginType::enSharaTV:
			plugin = std::make_shared<plugin_sharatv>();
			break;

		case PluginType::enTvTeam:
			plugin = std::make_shared<plugin_tvteam>();
			break;

		case PluginType::enOneOtt:
			plugin = std::make_shared<plugin_oneott>();
			break;

		case PluginType::enLightIptv:
			plugin = std::make_shared<plugin_lightiptv>();
			break;

		case PluginType::enCbilling:
			plugin = std::make_shared<plugin_cbilling>();
			break;

		case PluginType::enOttclub:
			plugin = std::make_shared<plugin_ottclub>();
			break;

		case PluginType::enIptvOnline:
			plugin = std::make_shared<plugin_iptvonline>();
			break;

		case PluginType::enVidok:
			plugin = std::make_shared<plugin_vidok>();
			break;

		case PluginType::enShuraTV:
			plugin = std::make_shared<plugin_shuratv>();
			break;

		case PluginType::enTVClub:
			plugin = std::make_shared<plugin_tvclub>();
			break;

		case PluginType::enFilmax:
			plugin = std::make_shared<plugin_filmax>();
			break;

		case PluginType::enKineskop:
			plugin = std::make_shared<plugin_kineskop>();
			break;

		case PluginType::enMymagic:
			plugin = std::make_shared<plugin_mymagic>();
			break;

		case PluginType::enRusskoeTV:
			plugin = std::make_shared<plugin_russkoetv>();
			break;

		case PluginType::enSmile:
			plugin = std::make_shared<plugin_smile>();
			break;

		case PluginType::enPing:
			plugin = std::make_shared<plugin_ping>();
			break;

		case PluginType::enYossoTV:
			plugin = std::make_shared<plugin_yosso>();
			break;

		case PluginType::enOttIptv:
			plugin = std::make_shared<plugin_ottiptv>();
			break;

		case PluginType::en101film:
			plugin = std::make_shared<plugin_101film>();
			break;

		case PluginType::enIpstream:
			plugin = std::make_shared<plugin_ipstream>();
			break;

		case PluginType::enOnlineOtt:
			plugin = std::make_shared<plugin_onlineott>();
			break;

		case PluginType::enTvizi:
			plugin = std::make_shared<plugin_tvizi>();
			break;

		case PluginType::enSatq:
			plugin = std::make_shared<plugin_satq>();
			break;

		default:
			break;
	}

	if (plugin)
	{
		plugin->set_plugin_defaults(type);
	}

	return plugin;
}
