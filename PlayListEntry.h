/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2024): sharky72 (https://github.com/KocourKuba)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#pragma once
#include "uri_stream.h"
#include "m3u_entry.h"
#include "ChannelCategory.h"
#include "PluginEnums.h"

using m3u_tags = std::map<m3u_entry::info_tags, std::string>;

class PlaylistEntry;
class base_plugin;

class Playlist
{
public:
	std::string catchup_source;
	int catchup_days;
	CatchupType catchup = CatchupType::cu_not_set;
	bool per_channel_catchup = false;
	std::string logo_root;
	m3u_entry m3u_header;
	std::map<int, std::shared_ptr<ChannelCategory>> categories;
	std::vector<std::shared_ptr<PlaylistEntry>> m_entries;
};

class PlaylistEntry : public uri_stream, public IconContainer
{
public:
	PlaylistEntry() = delete;
	PlaylistEntry(std::unique_ptr<Playlist>& m3u_playlist, std::wstring root_path = L"")
		: uri_stream(InfoType::enPlEntry)
		, IconContainer(root_path)
		, playlist(m3u_playlist)
	{}

	bool Parse(const std::string& str);
	int get_channel_length() const { return channel_len; }
	void set_category(const std::wstring& val) { category = utils::utf16_to_utf8(val); }
	const auto& get_category() const { return category; }
	auto get_category_w() const { return utils::utf8_to_utf16(category); }
	void search_id(const std::wstring& search_tag);

	m3u_entry& get_m3u_entry() { return m3uEntry; }
	const m3u_entry& get_m3u_entry() const { return m3uEntry; }

	PlaylistEntry& operator=(PlaylistEntry& src) = delete;
	PlaylistEntry& operator=(PlaylistEntry&& src) = delete;

protected:
	void search_group(const m3u_tags& tags);
	int search_archive(const m3u_tags& tags);
	void search_epg(const m3u_tags& tags);
	std::string search_logo(const m3u_tags& tags);
	std::string search_catchup_source(const m3u_tags& tags);
	CatchupType search_catchup(const m3u_tags& tags);
	void check_adult(const m3u_tags& tags, const std::string& category);

protected:
	int channel_len = 0;
	std::string category;
	m3u_entry m3uEntry;
	std::unique_ptr<Playlist>& playlist;
};
