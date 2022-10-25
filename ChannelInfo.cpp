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

#include "pch.h"
#include "ChannelInfo.h"
#include "ChannelCategory.h"
#include "StreamContainer.h"

#include "UtilsLib\rapidxml_value.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

ChannelInfo::ChannelInfo(std::shared_ptr<base_plugin>& plugin, const std::wstring& root_path)
	: uri_stream(InfoType::enChannel, plugin, root_path)
{
	set_icon_uri(utils::ICON_TEMPLATE);
}

ChannelInfo::ChannelInfo(const ChannelInfo& src) : uri_stream(src)
{
	*this = src;
}

void ChannelInfo::ParseNode(rapidxml::xml_node<>* node)
{
	if (!node)
		return;

	set_is_template(true);
	set_id(rapidxml::get_value_wstring(node->first_node(utils::CHANNEL_ID)));
	set_int_id(rapidxml::get_value_wstring(node->first_node(utils::INT_ID)));

	set_title(rapidxml::get_value_wstring(node->first_node(utils::CAPTION)));
	set_epg_id(0, rapidxml::get_value_wstring(node->first_node(utils::EPG1_ID)));
	set_epg_id(1, rapidxml::get_value_wstring(node->first_node(utils::EPG2_ID)));
	set_icon_uri(rapidxml::get_value_wstring(node->first_node(utils::ICON_URL)));
	set_disabled(utils::string_tolower(rapidxml::get_value_string(node->first_node(utils::DISABLED))) == "true");
	set_favorite(utils::string_tolower(rapidxml::get_value_string(node->first_node(utils::FAVORITE))) == "true");
	set_time_shift_hours(rapidxml::get_value_int(node->first_node(utils::TIME_SHIFT_HOURS)));

	auto cnode = node->first_node(utils::TV_CATEGORIES);
	if (cnode)
	{
		auto catNode = cnode->first_node();
		while (catNode)
		{
			auto cat_id = rapidxml::get_value_int(catNode);
			categories.emplace(cat_id);
			catNode = catNode->next_sibling();
		}
	}
	else
	{
		categories.emplace(rapidxml::get_value_int(node->first_node(utils::TV_CATEGORY_ID)));
	}

	const auto& stream_url = rapidxml::get_value_wstring(node->first_node(utils::STREAMING_URL));
	if (!stream_url.empty())
	{
		parent_plugin->parse_stream_uri(stream_url, this);
	}
	get_hash();

	set_custom_archive_url(rapidxml::get_value_wstring(node->first_node(utils::CATCHUP_URL_TEMPLATE)));
	set_archive_days(rapidxml::get_value_int(node->first_node(utils::ARCHIVE)));
	set_adult(rapidxml::get_value_int(node->first_node(utils::PROTECTED)));
}

rapidxml::xml_node<>* ChannelInfo::GetNode(rapidxml::memory_pool<>& alloc) const
{
	// <tv_channel>
	auto channel_node = rapidxml::alloc_node(alloc, utils::TV_CHANNEL);

	// <caption>Первый канал</caption>
	channel_node->append_node(rapidxml::alloc_node(alloc, utils::CAPTION, utils::utf16_to_utf8(get_title()).c_str()));

	// <channel_id>1</channel_id> or <channel_id>tv3</channel_id>
	channel_node->append_node(rapidxml::alloc_node(alloc, utils::CHANNEL_ID, utils::utf16_to_utf8(get_id()).c_str()));

	// used in glanz
	// <int_id>1</int_id>
	if (!get_int_id().empty())
	{
		channel_node->append_node(rapidxml::alloc_node(alloc, utils::INT_ID, utils::utf16_to_utf8(get_int_id()).c_str()));
	}

	// <epg_id>8</epg_id>
	if (!get_epg_id(0).empty() && get_epg_id(0) != L"0")
	{
		channel_node->append_node(rapidxml::alloc_node(alloc, utils::EPG1_ID, utils::utf16_to_utf8(get_epg_id(0)).c_str()));
	}

	// <tvg_id>1</tvg_id>
	if (!get_epg_id(1).empty() && get_epg_id(1) != L"0")
	{
		channel_node->append_node(rapidxml::alloc_node(alloc, utils::EPG2_ID, utils::utf16_to_utf8(get_epg_id(1)).c_str()));
	}

	// <icon_url>plugin_file://icons/channels/pervyi.png</icon_url>
	// <icon_url>http://epg.it999.ru/img/146.png</icon_url>
	if (!get_icon_uri().get_uri().empty())
	{
		channel_node->append_node(rapidxml::alloc_node(alloc, utils::ICON_URL, utils::utf16_to_utf8(get_icon_uri().get_uri()).c_str()));
	}

	if (get_time_shift_hours() != 0)
	{
		channel_node->append_node(rapidxml::alloc_node(alloc, utils::TIME_SHIFT_HOURS, std::to_string(get_time_shift_hours()).c_str()));
	}

	// <tv_category_id>1</tv_category_id>
	// new version must not have more than on entry
	channel_node->append_node(rapidxml::alloc_node(alloc, utils::TV_CATEGORY_ID, std::to_string(*categories.begin()).c_str()));

	// Only if channel not templated. Otherwise template handled by plugin
	// <streaming_url>http://ts://{DOMAIN}/iptv/{TOKEN}/127/index.m3u8</streaming_url>
	if (!get_is_template() && !get_uri().empty())
	{
		channel_node->append_node(rapidxml::alloc_node(alloc, utils::STREAMING_URL, utils::utf16_to_utf8(get_uri()).c_str()));
		if (!get_custom_archive_url().empty())
		{
			channel_node->append_node(rapidxml::alloc_node(alloc, utils::CATCHUP_URL_TEMPLATE, utils::utf16_to_utf8(get_custom_archive_url()).c_str()));
		}
	}

	// <archive>1</archive>
	if (is_archive())
	{
		channel_node->append_node(rapidxml::alloc_node(alloc, utils::ARCHIVE, std::to_string(get_archive_days()).c_str()));
	}

	// <protected>1</protected>
	if (get_adult())
	{
		channel_node->append_node(rapidxml::alloc_node(alloc, utils::PROTECTED, std::to_string(get_adult()).c_str()));
	}

	if (is_disabled())
	{
		channel_node->append_node(rapidxml::alloc_node(alloc, utils::DISABLED, "true"));
	}

	return channel_node;
}

bool ChannelInfo::is_icon_local() const
{
	return (get_icon_uri().get_schema() == uri_base::PLUGIN_SCHEME);
}
