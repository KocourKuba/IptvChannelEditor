#include "StdAfx.h"
#include "StreamContainer.h"
#include "uri_channels.h"
#include "uri_edem.h"
#include "uri_sharavoz.h"
#include "uri_sharaclub.h"
#include "uri_glanz.h"

StreamContainer::StreamContainer(StreamType type) : stream_type(type)
{
	switch (stream_type)
	{
		case StreamType::enBase: // ChannelsCategory
			stream_uri = std::make_unique<uri_stream>();
			break;
		case StreamType::enChannels: // Channels list
			stream_uri = std::make_unique<uri_channels>();
			break;
		case StreamType::enEdem: // Edem playlist
			stream_uri = std::make_unique<uri_edem>();
			break;
		case StreamType::enSharavoz: // Sharavoz playlist
			stream_uri = std::make_unique<uri_sharavoz>();
			break;
		case StreamType::enSharaclub: // Sharaclub playlist
			stream_uri = std::make_unique<uri_sharaclub>();
			break;
		case StreamType::enGlanz: // Glanz playlist
			stream_uri = std::make_unique<uri_glanz>();
			break;
		default:
			ASSERT(false);
			break;
	}
}
