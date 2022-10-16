/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2022): sharky72 (https://github.com/KocourKuba)

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

class PlaylistEntry : public uri_stream
{
public:
	PlaylistEntry() = delete;
	PlaylistEntry(std::shared_ptr<base_plugin>& plugin, std::wstring root_path = L"")
		: uri_stream(InfoType::enPlEntry, plugin, root_path)
	{}

	bool Parse(const std::wstring& str, const m3u_entry& entry);

	int get_channel_length() const { return channel_len; }
	const auto& get_category() const { return category; }
	void set_logo_root(const std::wstring& val) { logo_root = val; }
	std::wstring get_logo_root() { return logo_root; }

protected:
	void search_id(const std::map<m3u_entry::info_tags, std::wstring>& tags);
	void search_group(const std::map<m3u_entry::info_tags, std::wstring>& tags);
	void search_archive(const std::map<m3u_entry::info_tags, std::wstring>& tags);
	void search_epg(const std::map<m3u_entry::info_tags, std::wstring>& tags);
	void search_logo(const std::map<m3u_entry::info_tags, std::wstring>& tags);
	void search_catchup(const std::map<m3u_entry::info_tags, std::wstring>& tags);
	void check_adult(const std::wstring& category);

protected:
	int channel_len = 0;
	std::wstring category;
	std::wstring logo_root;
};

class Playlist
{
public:
	std::map<int, std::shared_ptr<ChannelCategory>> categories;
	std::vector<std::shared_ptr<PlaylistEntry>> m_entries;
};