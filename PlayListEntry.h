#pragma once
#include "BaseInfo.h"
#include "m3u_entry.h"

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
	void search_id(const std::map<m3u_entry::info_tags, std::string>& tags);
	void search_group(const std::map<m3u_entry::info_tags, std::string>& tags);
	void search_archive(const std::map<m3u_entry::info_tags, std::string>& tags);
	void search_epg(const std::map<m3u_entry::info_tags, std::string>& tags);
	void search_logo(const std::map<m3u_entry::info_tags, std::string>& tags);
	void check_adult(const std::wstring& category);

protected:
	int channel_len = 0;
	std::wstring category;
};
