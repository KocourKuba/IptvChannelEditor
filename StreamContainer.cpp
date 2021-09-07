#include "StdAfx.h"
#include "StreamContainer.h"
#include "uri_channels.h"
#include "uri_edem.h"
#include "uri_antifriz.h"
#include "uri_sharavoz.h"
#include "uri_sharaclub.h"
#include "uri_glanz.h"

StreamContainer::StreamContainer(StreamType type) : stream_type(type)
{
	stream_uri = get_instance(type);
}

LPCTSTR StreamContainer::get_name(StreamType type)
{
	switch (type)
	{
		case StreamType::enBase: // ChannelsCategory
			return _T("Base");
		case StreamType::enChannels: // Channels list
			return _T("Channels");
		case StreamType::enEdem: // Edem playlist
			return _T("Edem");
		case StreamType::enSharavoz: // Sharavoz playlist
			return _T("Sharavoz");
		case StreamType::enSharaclub: // Sharaclub playlist
			return _T("Sharaclub");
		case StreamType::enGlanz: // Glanz playlist
			return _T("Glanz");
		case StreamType::enAntifriz: // Antifriz playlist
			return _T("Antifriz");
		default:
			ASSERT(false);
			return _T("");
	}
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
		newStream->copy(stream_uri.get());
		stream_uri = std::move(newStream);
		stream_type = type;
	}
}
