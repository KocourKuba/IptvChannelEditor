#pragma once
#include "uri_stream.h"

enum class StreamType
{
	enNoStream = 0,
	enChannels,
	enEdem,
	enSharovoz,
	enGlanz,
	enSharaclub,
};

/// <summary>
/// Container for stream interface
/// </summary>
class StreamContainer
{
public:
	StreamContainer() = delete;
	StreamContainer(StreamType type);
	~StreamContainer() = default;

	uri_stream* get_stream_uri() { return stream_uri.get(); }
	void set_stream_uri(const uri_stream* val) { ASSERT(val); *stream_uri = *val; }
	void set_stream_uri(const std::string& val) { stream_uri->set_uri(val); }
	StreamType get_stream_type() const { return stream_type; }

protected:
	std::unique_ptr<uri_stream> stream_uri;
	StreamType stream_type = StreamType::enEdem;
};
