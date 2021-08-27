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
	const std::wstring& get_category() const { return category; }
	const std::string& get_domain() const { return domain; }
	const std::string& get_access_key() const { return access_key; }

protected:
	int channel_len = 0;
	std::wstring category;
	std::string domain;
	std::string access_key;
};
