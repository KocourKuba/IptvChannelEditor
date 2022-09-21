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
#include "BaseInfo.h"

#include "UtilsLib\rapidxml.hpp"

// <tv_channel>
//     <caption>Первый канал</caption>
//     <tvg_id>1</tvg_id>
//     <epg_id>8</epg_id>
//     <icon_url>plugin_file://icons/channels/pervyi.png</icon_url>
//     <num_past_epg_days>4</num_past_epg_days>
//     <num_future_epg_days>2</num_future_epg_days>
//     <tv_categories>
//         <tv_category_id>1</tv_category_id>
//     </tv_categories>
//     <streaming_url>http://ts://{DOMAIN}/iptv/{TOKEN}/127/index.m3u8</streaming_url>
//     <archive>1</archive>
//     <protected>1</protected>
// /tv_channel>

class ChannelCategory;

class ChannelInfo : public BaseInfo
{
public:
	ChannelInfo() = delete;
	ChannelInfo(const ChannelInfo& src);
	ChannelInfo(PluginType streamType, const std::wstring& root_path);
	ChannelInfo(rapidxml::xml_node<>* node, PluginType streamType, const std::wstring& root_path);

public:
	void ParseNode(rapidxml::xml_node<>* node);

	rapidxml::xml_node<>* GetNode(rapidxml::memory_pool<>& doc) const;

	// Getters/Setters

	bool is_icon_local() const;

	// categories contains this channel. Compatibility with old channels list
	std::set<int>& get_category_ids() { return categories; }

	bool is_favorite() const { return favorite; }
	void set_favorite(bool val) { favorite = val; }

	bool is_disabled() const { return disabled; }
	void set_disabled(bool val) { disabled = val; }

	const ChannelInfo& operator= (const ChannelInfo& src)
	{
		if (this != &src)
		{
			BaseInfo::operator=(src);
			disabled = src.disabled;
			favorite = src.favorite;
			categories = src.categories;
		}

		return *this;
	}

private:
	bool disabled = false;
	bool favorite = false;
	std::set<int> categories;
};
