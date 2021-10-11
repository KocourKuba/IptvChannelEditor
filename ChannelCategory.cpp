#include "StdAfx.h"
#include "ChannelCategory.h"
#include "utils.h"

ChannelCategory::ChannelCategory(StreamType streamType, const std::wstring& root_path)
	: BaseInfo(InfoType::enCategory, streamType, root_path)
{
	set_icon_uri(utils::ICON_TEMPLATE);
}

ChannelCategory::ChannelCategory(rapidxml::xml_node<>* node, StreamType streamType, const std::wstring& root_path)
	: BaseInfo(InfoType::enCategory, streamType, root_path)
{
	ParseNode(node);
}

void ChannelCategory::ParseNode(rapidxml::xml_node<>* node)
{
	if (!node)
		return;

	// <id>1</id>
	set_key(utils::get_value_int(node->first_node(ID)));
	// <caption>Общие</caption>
	set_title(utils::get_value_wstring(node->first_node(CAPTION)));
	// <icon_url>plugin_file://icons/1.png</icon_url>
	set_icon_uri(utils::get_value_wstring(node->first_node(ICON_URL)));
}

rapidxml::xml_node<>* ChannelCategory::GetNode(rapidxml::memory_pool<>& alloc) const
{
	// <tv_channel>
	auto category_node = utils::alloc_node(alloc, TV_CATEGORY);

	// <id>1</id>
	category_node->append_node(utils::alloc_node(alloc, ID, utils::int_to_char(get_key()).c_str()));

	// <caption>Общие</caption>
	category_node->append_node(utils::alloc_node(alloc, CAPTION, utils::utf16_to_utf8(get_title()).c_str()));

	// <icon_url>plugin_file://icons/1.png</icon_url>
	category_node->append_node(utils::alloc_node(alloc, ICON_URL, utils::utf16_to_utf8(get_icon_uri().get_uri()).c_str()));

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

void ChannelCategory::add_channel(const std::shared_ptr<ChannelInfo>& channel)
{
	if (channels_map.find(channel->stream_uri->get_id()) == channels_map.end())
	{
		channels_map.emplace(channel->stream_uri->get_id(), channel);
		channels.emplace_back(channel);
	}
}

void ChannelCategory::remove_channel(const std::wstring& ch_id)
{
	if (auto pair = channels_map.find(ch_id); pair != channels_map.end())
	{
		channels.erase(std::find(channels.begin(), channels.end(), pair->second));
		channels_map.erase(pair);
	}
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
