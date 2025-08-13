/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2025): sharky72 (https://github.com/KocourKuba)

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
#include "ChannelCategory.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

ChannelCategory::ChannelCategory(const std::wstring& root_path)
	: uri_stream(InfoType::enCategory)
	, IconContainer(root_path)
{
	set_icon_uri(utils::ICON_TEMPLATE);
}

void ChannelCategory::ParseNode(rapidxml::xml_node<>* node)
{
	if (!node)
		return;

	// <id>1</id>
	set_key(rapidxml::get_value_int(node->first_node(utils::ID)));
	// <caption>Общие</caption>
	set_title(rapidxml::get_value_wstring(node->first_node(utils::CAPTION)));
	// <icon_url>plugin_file://icons/1.png</icon_url>
	set_icon_uri(rapidxml::get_value_wstring(node->first_node(utils::ICON_URL)));
	// <disabled>true</disabled>
	auto disabled = rapidxml::get_value_string(node->first_node(utils::DISABLED));
	set_disabled(utils::string_tolower(disabled) == "true");
}

rapidxml::xml_node<>* ChannelCategory::GetNode(rapidxml::memory_pool<>& alloc) const
{
	// <tv_channel>
	auto category_node = rapidxml::alloc_node(alloc, utils::TV_CATEGORY);

	// <id>1</id>
	category_node->append_node(rapidxml::alloc_node(alloc, utils::ID, std::to_string(get_key()).c_str()));

	// <caption>Общие</caption>
	category_node->append_node(rapidxml::alloc_node(alloc, utils::CAPTION, utils::utf16_to_utf8(get_title()).c_str()));

	// <icon_url>plugin_file://icons/1.png</icon_url>
	category_node->append_node(rapidxml::alloc_node(alloc, utils::ICON_URL, utils::utf16_to_utf8(get_icon_uri().get_uri()).c_str()));

	// <special_group>true</special_group>
	if (is_favorite())
	{
		category_node->append_node(rapidxml::alloc_node(alloc, utils::SPECIAL_GROUP, utils::FAVORITES));
	}
	else if (is_vod())
	{
		category_node->append_node(rapidxml::alloc_node(alloc, utils::SPECIAL_GROUP, utils::VOD_GROUP));
	}
	else if (is_all_channels())
	{
		category_node->append_node(rapidxml::alloc_node(alloc, utils::SPECIAL_GROUP, utils::ALL_GROUP));
	}
	else if (is_history())
	{
		category_node->append_node(rapidxml::alloc_node(alloc, utils::SPECIAL_GROUP, utils::HISTORY_GROUP));
	}

	// <disabled>true</disabled>
	if (is_disabled())
	{
		category_node->append_node(rapidxml::alloc_node(alloc, utils::DISABLED, "true"));
	}

	return category_node;
}

void ChannelCategory::move_channels(const std::shared_ptr<ChannelInfo>& range_start, const std::shared_ptr<ChannelInfo>& range_end, bool down)
{
	if (!range_start || !range_end)
		return;

	if (down)
	{
		auto it_bgn = std::find(channels.begin(), channels.end(), range_start);
		auto it_end = std::find(channels.begin(), channels.end(), range_end);
		std::rotate(it_bgn, it_end + 1, it_end + 2);
	}
	else
	{
		auto it_bgn = std::find(channels.rbegin(), channels.rend(), range_start);
		auto it_end = std::find(channels.rbegin(), channels.rend(), range_end);
		std::rotate(it_end, it_bgn + 1, it_bgn + 2);
	}
}

bool ChannelCategory::add_channel(const std::shared_ptr<ChannelInfo>& channel)
{
	if (channels_map.find(channel->get_id()) == channels_map.end())
	{
		channels_map.emplace(channel->get_id(), channel);
		channels.emplace_back(channel);
		return true;
	}

	return false;
}

bool ChannelCategory::remove_channel(const std::wstring& ch_id)
{
	if (auto pair = channels_map.find(ch_id); pair != channels_map.end())
	{
		channels.erase(std::find(channels.begin(), channels.end(), pair->second));
		channels_map.erase(pair);
		return true;
	}

	return false;
}

void ChannelCategory::sort_channels()
{
	if (!channels.empty())
	{
		std::sort(channels.begin(), channels.end(), [](const auto& left, const auto& right)
				  {
					  return left->get_title() < right->get_title();
				  });
	}
}

std::shared_ptr<ChannelInfo> ChannelCategory::find_channel(const std::wstring& ch_id)
{
	auto pair = channels_map.find(ch_id);
	return pair != channels_map.end() ? pair->second : nullptr;
}
