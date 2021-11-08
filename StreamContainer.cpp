#include "StdAfx.h"
#include "StreamContainer.h"
#include "uri_channels.h"
#include "uri_edem.h"
#include "uri_antifriz.h"
#include "uri_sharavoz.h"
#include "uri_sharaclub.h"
#include "uri_glanz.h"
#include "uri_fox.h"
#include "uri_onecent.h"
#include "uri_oneusd.h"
#include "uri_itv.h"
#include "uri_viplime.h"

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
		case StreamType::enEdem: // Edem playlist
			return  std::make_unique<uri_edem>();
		case StreamType::enSharavoz: // Sharavoz playlist
			return  std::make_unique<uri_sharavoz>();
		case StreamType::enSharaclub: // Sharaclub playlist
			return  std::make_unique<uri_sharaclub>();
		case StreamType::enGlanz: // Glanz playlist
			return  std::make_unique<uri_glanz>();
		case StreamType::enAntifriz: // Antifriz playlist
			return  std::make_unique<uri_antifriz>();
		case StreamType::enFox: // Fox playlist
			return  std::make_unique<uri_fox>();
		case StreamType::enOneCent: // 1CENT playlist
			return  std::make_unique<uri_onecent>();
		case StreamType::enOneUsd: // 1USD playlist
			return  std::make_unique<uri_oneusd>();
		case StreamType::enItv: // ITV playlist
			return  std::make_unique<uri_itv>();
		case StreamType::enVipLime: // VipLime playlist
			return  std::make_unique<uri_viplime>();
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
