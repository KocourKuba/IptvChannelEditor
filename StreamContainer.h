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
	void set_stream_uri(const std::wstring& val) { stream_uri.set_uri(val); }

	const std::string& get_audio() const { return audio; }
	void set_audio(const std::string& val) { audio = val; }

	const std::string& get_video() const { return video; }
	void set_video(const std::string& val) { video = val; }

private:
	std::string audio;
	std::string video;
	uri_stream stream_uri;
};