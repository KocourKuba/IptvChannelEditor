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
#include "uri_channels.h"
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

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

StreamContainer::StreamContainer(StreamType type) : stream_type(type)
{
	stream_uri = get_instance(type);
}

std::unique_ptr<uri_stream> StreamContainer::get_instance(StreamType type)
{
	switch (type)
	{
		case StreamType::enBase: // ChannelsCategory
			return std::make_unique<uri_stream>();

		case StreamType::enChannels: // Channels list
			return std::make_unique<uri_channels>();

		case StreamType::enAntifriz:
			return std::make_unique<uri_antifriz>();

		case StreamType::enEdem:
			return std::make_unique<uri_edem>();

		case StreamType::enFox:
			return std::make_unique<uri_fox>();

		case StreamType::enGlanz:
			return std::make_unique<uri_glanz>();

		case StreamType::enItv:
			return std::make_unique<uri_itv>();

		case StreamType::enOneCent:
			return std::make_unique<uri_onecent>();

		case StreamType::enOneUsd:
			return std::make_unique<uri_oneusd>();

		case StreamType::enSharaclub:
			return std::make_unique<uri_sharaclub>();

		case StreamType::enSharavoz:
			return std::make_unique<uri_sharavoz>();

		case StreamType::enVipLime:
			return std::make_unique<uri_viplime>();

		case StreamType::enSharaTV:
			return std::make_unique<uri_sharatv>();

		case StreamType::enTvTeam:
			return std::make_unique<uri_tvteam>();

		case StreamType::enOneOtt:
			return std::make_unique<uri_oneott>();

		case StreamType::enLightIptv:
			return std::make_unique<uri_lightiptv>();

		case StreamType::enCbilling:
			return std::make_unique<uri_cbilling>();

		case StreamType::enOttclub:
			return std::make_unique<uri_ottclub>();

		case StreamType::enIptvOnline:
			return std::make_unique<uri_iptvonline>();

		case StreamType::enVidok:
			return std::make_unique<uri_vidok>();

		case StreamType::enShuraTV:
			return std::make_unique<uri_shuratv>();

		case StreamType::enTVClub:
			return std::make_unique<uri_tvclub>();

		case StreamType::enFilmax:
			return std::make_unique<uri_filmax>();

		case StreamType::enKineskop:
			return std::make_unique<uri_kineskop>();

		case StreamType::enMymagic:
			return std::make_unique<uri_mymagic>();

		case StreamType::enRusskoeTV:
			return std::make_unique<uri_russkoetv>();

		default:
			ASSERT(false);
			return nullptr;
	}
}

void StreamContainer::set_type(StreamType type)
{
	if (stream_type != type)
	{
		auto newStream = get_instance(type);
		newStream->copy(stream_uri);
		stream_uri = std::move(newStream);
		stream_type = type;
	}
}
