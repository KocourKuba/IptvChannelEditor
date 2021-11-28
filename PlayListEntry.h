#pragma once
#include "BaseInfo.h"
#include "m3u_entry.h"
#include "ChannelCategory.h"

class PlaylistEntry : public BaseInfo
{
public:
	PlaylistEntry() = delete;
	PlaylistEntry(StreamType streamType, const std::wstring& root_path)
		: BaseInfo(InfoType::enPlEntry, streamType, root_path) {}

	bool Parse(const std::string& str, const m3u_entry& entry);

	int get_channel_length() const { return channel_len; }
	const auto& get_category() const { return category; }
	const auto& get_uri_stream() { return stream_uri; }
	void set_logo_root(const std::string& val) { logo_root = val; }
	std::string get_logo_root() { return logo_root; }

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
	std::string logo_root;
};

class Playlist
{
public:
	std::map<int, std::shared_ptr<ChannelCategory>> categories;
	std::vector<std::shared_ptr<PlaylistEntry>> m_entries;
};