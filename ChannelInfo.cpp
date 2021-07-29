#include "StdAfx.h"
#include "ChannelInfo.h"
#include "utils.h"
#include "ChannelCategory.h"

ChannelInfo::ChannelInfo()
{
	set_icon_uri(utils::ICON_TEMPLATE);
	get_stream_uri().set_uri(utils::URI_TEMPLATE);
}

ChannelInfo::ChannelInfo(rapidxml::xml_node<>* node)
{
	ParseNode(node);
}

void ChannelInfo::ParseNode(rapidxml::xml_node<>* node)
{
	if (!node)
		return;

	title = utils::get_value_wstring(node->first_node(CAPTION));
	tvg_id = utils::get_value_int(node->first_node(TVG_ID));
	epg_id = utils::get_value_int(node->first_node(EPG_ID));
	time_shift_hours = utils::get_value_int(node->first_node(TIME_SHIFT_HOURS));
	set_icon_uri(utils::get_value_string(node->first_node(ICON_URL)));
	set_disabled(utils::string_tolower(utils::get_value_string(node->first_node(DISABLED))) == "true");
	set_favorite(utils::string_tolower(utils::get_value_string(node->first_node(FAVORITE))) == "true");

	auto cnode = node->first_node(TV_CATEGORIES);
	if (cnode)
	{
		changed = true;
		auto catNode = cnode->first_node();
		while (catNode)
		{
			auto cat_id = utils::get_value_int(catNode);
			categories.insert(cat_id);
			catNode = catNode->next_sibling();
		}
	}
	else
	{
		categories.insert(utils::get_value_int(node->first_node(TV_CATEGORY_ID)));
	}

	get_stream_uri().set_uri(utils::get_value_string(node->first_node(STREAMING_URL)));
	has_archive = utils::get_value_int(node->first_node(ARCHIVE));
	adult = utils::get_value_int(node->first_node(PROTECTED));
}

rapidxml::xml_node<>* ChannelInfo::GetNode(rapidxml::memory_pool<>& alloc) const
{
	// <tv_channel>
	auto channel_node = utils::alloc_node(alloc, TV_CHANNEL);

	// <caption>Первый канал</caption>
	channel_node->append_node(utils::alloc_node(alloc, CAPTION, utils::utf16_to_utf8(title).c_str()));

	// <tvg_id>1</tvg_id>
	channel_node->append_node(utils::alloc_node(alloc, TVG_ID, utils::int_to_char(tvg_id).c_str()));

	// <epg_id>8</epg_id>
	channel_node->append_node(utils::alloc_node(alloc, EPG_ID, utils::int_to_char(epg_id).c_str()));

	// <icon_url>plugin_file://icons/channels/pervyi.png</icon_url>
	// <icon_url>http://epg.it999.ru/img/146.png</icon_url>
	channel_node->append_node(utils::alloc_node(alloc, ICON_URL, get_icon_uri().get_uri().c_str()));

	if (time_shift_hours != 0)
		channel_node->append_node(utils::alloc_node(alloc, TIME_SHIFT_HOURS, utils::int_to_char(time_shift_hours).c_str()));

	// <tv_category_id>1</tv_category_id>
	// new version must not have more than on entry
	channel_node->append_node(utils::alloc_node(alloc, TV_CATEGORY_ID, utils::int_to_char(*categories.begin()).c_str()));

	// <streaming_url>http://ts://{SUBDOMAIN}/iptv/{UID}/127/index.m3u8</streaming_url>
	channel_node->append_node(utils::alloc_node(alloc, STREAMING_URL, get_stream_uri().get_id_translated_url().c_str()));

	// <archive>1</archive>
	if (has_archive)
	{
		channel_node->append_node(utils::alloc_node(alloc, ARCHIVE, utils::int_to_char(has_archive).c_str()));
	}

	// <protected>1</protected>
	if (adult)
	{
		channel_node->append_node(utils::alloc_node(alloc, PROTECTED, utils::int_to_char(adult).c_str()));
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

std::string ChannelInfo::GetIconRelativePath(LPCSTR szRoot /*= nullptr*/) const
{
	return get_icon_uri().get_icon_absolute_path(szRoot);
}
