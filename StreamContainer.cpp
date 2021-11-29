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
			return  std::make_unique<uri_antifriz>();

		case StreamType::enEdem:
			return  std::make_unique<uri_edem>();

		case StreamType::enFox:
			return  std::make_unique<uri_fox>();

		case StreamType::enGlanz:
			return  std::make_unique<uri_glanz>();

		case StreamType::enItv:
			return  std::make_unique<uri_itv>();

		case StreamType::enOneCent:
			return  std::make_unique<uri_onecent>();

		case StreamType::enOneUsd:
			return  std::make_unique<uri_oneusd>();

		case StreamType::enSharaclub:
			return  std::make_unique<uri_sharaclub>();

		case StreamType::enSharaTV:
			return  std::make_unique<uri_sharatv>();

		case StreamType::enSharavoz:
			return  std::make_unique<uri_sharavoz>();

		case StreamType::enTvTeam:
			return  std::make_unique<uri_tvteam>();

		case StreamType::enVipLime:
			return  std::make_unique<uri_viplime>();

		case StreamType::enOneOtt:
			return  std::make_unique<uri_oneott>();

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
