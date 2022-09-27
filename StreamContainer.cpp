/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2022): sharky72 (https://github.com/KocourKuba)

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
#include "Config.h"
#include "uri_antifriz.h"
#include "uri_edem.h"
#include "uri_fox.h"
#include "uri_glanz.h"
#include "uri_itv.h"
#include "uri_onecent.h"
#include "uri_oneott.h"
#include "uri_oneusd.h"
#include "uri_sharaclub.h"
#include "uri_sharatv.h"
#include "uri_sharavoz.h"
#include "uri_tvteam.h"
#include "uri_viplime.h"
#include "uri_lightiptv.h"
#include "uri_cbilling.h"
#include "uri_ottclub.h"
#include "uri_iptvonline.h"
#include "uri_vidok.h"
#include "uri_shuratv.h"
#include "uri_tvclub.h"
#include "uri_filmax.h"
#include "uri_kineskop.h"
#include "uri_mymagic.h"
#include "uri_russkoetv.h"
#include "uri_smile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

StreamContainer::StreamContainer(PluginType type) : stream_type(type)
{
	stream_uri = get_instance(type);
}

std::unique_ptr<uri_stream> StreamContainer::get_instance(PluginType type)
{
	std::unique_ptr<uri_stream> plugin;
	switch (type)
	{
		case PluginType::enBase: // ChannelsCategory
			return std::make_unique<uri_stream>();

		case PluginType::enAntifriz:
			plugin = std::make_unique<uri_antifriz>();
			break;

		case PluginType::enEdem:
			plugin = std::make_unique<uri_edem>();
			break;

		case PluginType::enFox:
			plugin = std::make_unique<uri_fox>();
			break;

		case PluginType::enGlanz:
			plugin = std::make_unique<uri_glanz>();
			break;

		case PluginType::enItv:
			plugin = std::make_unique<uri_itv>();
			break;

		case PluginType::enOneCent:
			plugin = std::make_unique<uri_onecent>();
			break;

		case PluginType::enOneUsd:
			plugin = std::make_unique<uri_oneusd>();
			break;

		case PluginType::enSharaclub:
			plugin = std::make_unique<uri_sharaclub>();
			break;

		case PluginType::enSharavoz:
			plugin = std::make_unique<uri_sharavoz>();
			break;

		case PluginType::enVipLime:
			plugin = std::make_unique<uri_viplime>();
			break;

		case PluginType::enSharaTV:
			plugin = std::make_unique<uri_sharatv>();
			break;

		case PluginType::enTvTeam:
			plugin = std::make_unique<uri_tvteam>();
			break;

		case PluginType::enOneOtt:
			plugin = std::make_unique<uri_oneott>();
			break;

		case PluginType::enLightIptv:
			plugin = std::make_unique<uri_lightiptv>();
			break;

		case PluginType::enCbilling:
			plugin = std::make_unique<uri_cbilling>();
			break;

		case PluginType::enOttclub:
			plugin = std::make_unique<uri_ottclub>();
			break;

		case PluginType::enIptvOnline:
			plugin = std::make_unique<uri_iptvonline>();
			break;

		case PluginType::enVidok:
			plugin = std::make_unique<uri_vidok>();
			break;

		case PluginType::enShuraTV:
			plugin = std::make_unique<uri_shuratv>();
			break;

		case PluginType::enTVClub:
			plugin = std::make_unique<uri_tvclub>();
			break;

		case PluginType::enFilmax:
			plugin = std::make_unique<uri_filmax>();
			break;

		case PluginType::enKineskop:
			plugin = std::make_unique<uri_kineskop>();
			break;

		case PluginType::enMymagic:
			plugin = std::make_unique<uri_mymagic>();
			break;

		case PluginType::enRusskoeTV:
			plugin = std::make_unique<uri_russkoetv>();
			break;

		case PluginType::enSmile:
			plugin = std::make_unique<uri_smile>();
			break;

		default:
			break;
	}

	if (plugin)
	{
		//plugin->load_plugin_parameters();
		plugin->load_default();
	}

	return plugin;
}

void StreamContainer::set_type(PluginType type)
{
	if (stream_type != type)
	{
		auto newStream = get_instance(type);
		newStream->copy(stream_uri);
		stream_uri = std::move(newStream);
		stream_type = type;
	}
}
