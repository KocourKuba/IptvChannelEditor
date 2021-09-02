#pragma once
#include "BaseInfo.h"

class PlaylistEntry : public BaseInfo
{
public:
	PlaylistEntry() = delete;
	PlaylistEntry(StreamType streamType, const std::wstring& root_path)
		: BaseInfo(InfoType::enPlEntry, streamType, root_path) {}

	bool Parse(const std::string& str);

	int get_channel_length() const { return channel_len; }
	const auto& get_category() const { return category; }
	const auto& get_uri_stream() { return stream_uri; }

protected:
	int channel_len = 0;
	std::wstring category;
};
