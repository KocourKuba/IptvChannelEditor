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

	static std::unique_ptr<uri_stream> get_instance(StreamType type);

	void set_type(StreamType type);

	const StreamContainer& operator=(const StreamContainer& src)
	{
		if (this != &src)
		{
			*stream_uri = *src.stream_uri;
			stream_type = src.stream_type;
		}

		return *this;
	}

	std::unique_ptr<uri_stream> stream_uri;
	StreamType stream_type;
};
