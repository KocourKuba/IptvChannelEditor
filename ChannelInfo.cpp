#include "StdAfx.h"
#include "ChannelInfo.h"
#include "utils.h"
#include "ChannelCategory.h"

ChannelInfo::ChannelInfo(StreamType streamType, const std::wstring& root_path)
	: BaseInfo(InfoType::enChannel, streamType, root_path)
{
	set_icon_uri(utils::ICON_TEMPLATE);
}

ChannelInfo::ChannelInfo(rapidxml::xml_node<>* node, StreamType streamType, const std::wstring& root_path)
	: BaseInfo(InfoType::enChannel, streamType, root_path)
{
	ParseNode(node);
}

void ChannelInfo::ParseNode(rapidxml::xml_node<>* node)
{
	if (!node)
		return;

	set_title(utils::get_value_wstring(node->first_node(CAPTION)));
	stream_uri->set_id(utils::get_value_string(node->first_node(CHANNEL_ID)));
	set_epg1_id(utils::get_value_string(node->first_node(EPG_ID)));
	set_epg2_id(utils::get_value_string(node->first_node(TVG_ID)));
	set_icon_uri(utils::get_value_string(node->first_node(ICON_URL)));
	set_disabled(utils::string_tolower(utils::get_value_string(node->first_node(DISABLED))) == "true");
	set_favorite(utils::string_tolower(utils::get_value_string(node->first_node(FAVORITE))) == "true");
	time_shift_hours = utils::get_value_int(node->first_node(TIME_SHIFT_HOURS));

	auto cnode = node->first_node(TV_CATEGORIES);
	if (cnode)
	{
		changed = true;
		auto catNode = cnode->first_node();
		while (catNode)
		{
			auto cat_id = utils::get_value_int(catNode);
			categories.emplace(cat_id);
			catNode = catNode->next_sibling();
		}
	}
	else
	{
		categories.emplace(utils::get_value_int(node->first_node(TV_CATEGORY_ID)));
	}

	if (stream_uri->get_id().empty())
		stream_uri->parse_uri(utils::get_value_string(node->first_node(STREAMING_URL)));

	set_archive_days(utils::get_value_int(node->first_node(ARCHIVE)));
	set_adult(utils::get_value_int(node->first_node(PROTECTED)));
}

rapidxml::xml_node<>* ChannelInfo::GetNode(rapidxml::memory_pool<>& alloc) const
{
	// <tv_channel>
	auto channel_node = utils::alloc_node(alloc, TV_CHANNEL);

	// <caption>Первый канал</caption>
	channel_node->append_node(utils::alloc_node(alloc, CAPTION, utils::utf16_to_utf8(get_title()).c_str()));

	// <channel_id>1</channel_id> or <channel_id>tv3</channel_id>
	channel_node->append_node(utils::alloc_node(alloc, CHANNEL_ID, get_id().c_str()));

	// <epg_id>8</epg_id>
	channel_node->append_node(utils::alloc_node(alloc, EPG_ID, get_epg1_id().c_str()));

	// <tvg_id>1</tvg_id>
	channel_node->append_node(utils::alloc_node(alloc, TVG_ID, get_epg2_id().c_str()));

	// <icon_url>plugin_file://icons/channels/pervyi.png</icon_url>
	// <icon_url>http://epg.it999.ru/img/146.png</icon_url>
	channel_node->append_node(utils::alloc_node(alloc, ICON_URL, get_icon_uri().get_uri().c_str()));

	if (time_shift_hours != 0)
		channel_node->append_node(utils::alloc_node(alloc, TIME_SHIFT_HOURS, utils::int_to_char(time_shift_hours).c_str()));

	// <tv_category_id>1</tv_category_id>
	// new version must not have more than on entry
	channel_node->append_node(utils::alloc_node(alloc, TV_CATEGORY_ID, utils::int_to_char(*categories.begin()).c_str()));

	// Only if channel not templated. Otherwise template handled by plugin
	// <streaming_url>http://ts://{SUBDOMAIN}/iptv/{TOKEN}/127/index.m3u8</streaming_url>
	if (!stream_uri->is_template())
	{
		channel_node->append_node(utils::alloc_node(alloc, STREAMING_URL, stream_uri->get_uri().c_str()));
	}

	// <archive>1</archive>
	if (is_archive())
	{
		channel_node->append_node(utils::alloc_node(alloc, ARCHIVE, utils::int_to_char(get_archive_days()).c_str()));
	}

	// <protected>1</protected>
	if (get_adult())
	{
		channel_node->append_node(utils::alloc_node(alloc, PROTECTED, utils::int_to_char(get_adult()).c_str()));
	}

	if (is_disabled())
	{
		channel_node->append_node(utils::alloc_node(alloc, DISABLED, "true"));
	}

	if (is_favorite())
	{
		channel_node->append_node(utils::alloc_node(alloc, FAVORITE, "true"));
	}

	return channel_node;
}

bool ChannelInfo::is_icon_local() const
{
	return (get_icon_uri().get_schema() == "plugin_file://");
}
