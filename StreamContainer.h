#pragma once
#include "uri.h"

class StreamContainer
{
public:
	StreamContainer() = default;
	~StreamContainer() = default;

	const uri_stream& get_stream_uri() const { return stream_uri; }
	uri_stream& get_stream_uri() { return stream_uri; }
	void set_stream_uri(const uri_stream& val) { stream_uri = val; }
	void set_stream_uri(const std::string& val) { stream_uri.set_uri(val); }

private:
	uri_stream stream_uri;
};