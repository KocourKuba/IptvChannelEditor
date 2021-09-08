#pragma once
#include "uri_stream.h"

/// <summary>
/// Container for stream interface
/// </summary>
class StreamContainer
{
public:
	StreamContainer() = delete;
	StreamContainer(StreamType type);
	~StreamContainer() = default;

	static LPCTSTR get_name(StreamType type);

	std::unique_ptr<uri_stream> get_instance(StreamType type);

	void set_type(StreamType type);

	std::unique_ptr<uri_stream> stream_uri;
	StreamType stream_type;
};
