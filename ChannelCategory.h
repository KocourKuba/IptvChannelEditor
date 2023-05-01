/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2023): sharky72 (https://github.com/KocourKuba)

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
#include "ChannelInfo.h"
#include "IconContainer.h"

#include "UtilsLib\rapidxml.hpp"

// <tv_category>
//   <id>1</id>
//   <caption>Общие</caption>
//   <icon_url>plugin_file://icons/1.png</icon_url>
// </tv_category>

class ChannelCategory : public uri_stream, public IconContainer
{
public:
	ChannelCategory() = delete;
	ChannelCategory(const std::wstring& root_path);

public:
	void ParseNode(rapidxml::xml_node<>* node);
	rapidxml::xml_node<>* GetNode(rapidxml::memory_pool<>& alloc) const;

	const int get_key() const { return key; }
	void set_key(const int val) { key = val; }

	bool is_empty() const { return channels_map.empty(); }

	bool is_favorite() const { return key == ID_FAVORITE; }

	bool is_vod() const { return key == ID_VOD; }

	bool is_all_channels() const { return key == ID_ALL_CHANNELS; }

	bool is_not_movable() const { return is_favorite() || is_all_channels() || is_vod(); }

	const std::vector<std::shared_ptr<ChannelInfo>>& get_channels() const { return channels; }

	void move_channels(const std::shared_ptr<ChannelInfo>& range_start, const std::shared_ptr<ChannelInfo>& range_end, bool down);

	bool add_channel(const std::shared_ptr<ChannelInfo>& channel);

	bool remove_channel(const std::wstring& ch_id);

	void sort_channels();

	bool is_disabled() const { return disabled; }
	void set_disabled(bool val) { disabled = val; }

	std::shared_ptr<ChannelInfo> find_channel(const std::wstring& ch_id);

private:
	std::vector<std::shared_ptr<ChannelInfo>> channels;
	std::map<std::wstring, std::shared_ptr<ChannelInfo>> channels_map;
	int key = 0;
	bool disabled = false;
	bool favorite = false;
};

