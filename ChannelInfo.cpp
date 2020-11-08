#include "StdAfx.h"
#include <regex>
#include "ChannelInfo.h"
#include "utils.h"

static constexpr auto ICON_URL = "icon_url";
static constexpr auto EPG_ID = "epg_id";
static constexpr auto TVG_ID = "tvg_id";
static constexpr auto CAPTION = "caption";
static constexpr auto PLUGIN_PATH = R"(plugin_file://)";
static constexpr auto NUM_PAST_EPG_DAYS = "num_past_epg_days";
static constexpr auto NUM_FUTURE_EPG_DAYS = "num_future_epg_days";
static constexpr auto TV_CATEGORIES = "tv_categories";
static constexpr auto TV_CATEGORY_ID = "tv_category_id";
static constexpr auto STREAMING_URL = "streaming_url";
static constexpr auto ARCHIVE = "archive";
static constexpr auto PROTECTED = "protected";
static constexpr auto DISABLED = "disabled";


ChannelInfo::ChannelInfo()
{
	set_icon_uri(utils::ICON_TEMPLATE);
	set_stream_uri(utils::URI_TEMPLATE);
}

ChannelInfo::ChannelInfo(rapidxml::xml_node<>* node)
{
	ParseNode(node);
}

void ChannelInfo::ParseNode(rapidxml::xml_node<>* node)
{
	if (!node)
		return;

	set_title(utils::get_value_wstring(node->first_node(CAPTION)));
	set_tvg_id(utils::get_value_int(node->first_node(TVG_ID)));
	set_epg_id(utils::get_value_int(node->first_node(EPG_ID)));
	set_icon_uri(utils::get_value_string(node->first_node(ICON_URL)));
	set_prev_epg_days(utils::get_value_int(node->first_node(NUM_PAST_EPG_DAYS)));
	set_next_epg_days(utils::get_value_int(node->first_node(NUM_FUTURE_EPG_DAYS)));
	set_disabled(utils::get_value_string(node->first_node(DISABLED)) == "true");

	auto cnode = node->first_node(TV_CATEGORIES);
	if (cnode)
	{
		std::set<int> values;
		auto category = cnode->first_node();
		while (category)
		{
			auto res = values.emplace(utils::get_value_int(category));
			ASSERT(res.second);
			category = category->next_sibling();
		}
		set_categores(values);
	}

	set_stream_uri((utils::get_value_string(node->first_node(STREAMING_URL))));
	set_has_archive(utils::get_value_int(node->first_node(ARCHIVE)));
	set_adult(utils::get_value_int(node->first_node(PROTECTED)));
}

rapidxml::xml_node<>* ChannelInfo::GetNode(rapidxml::memory_pool<>& alloc) const
{
	// <tv_channel>
	auto channel_node = utils::alloc_node(alloc, TV_CHANNEL);

	// <caption>Первый канал</caption>
	channel_node->append_node(utils::alloc_node(alloc, CAPTION, utils::utf16_to_utf8(get_title()).c_str()));

	// <tvg_id>1</tvg_id>
	channel_node->append_node(utils::alloc_node(alloc, TVG_ID, utils::int_to_char(get_tvg_id()).c_str()));

	// <epg_id>8</epg_id>
	channel_node->append_node(utils::alloc_node(alloc, EPG_ID, utils::int_to_char(get_epg_id()).c_str()));

	// <icon_url>plugin_file://icons/channels/pervyi.png</icon_url>
	// <icon_url>http://epg.it999.ru/img/146.png</icon_url>
	channel_node->append_node(utils::alloc_node(alloc, ICON_URL, get_icon_uri().get_uri().c_str()));

	// <num_past_epg_days>4</num_past_epg_days>
	channel_node->append_node(utils::alloc_node(alloc, NUM_PAST_EPG_DAYS, utils::int_to_char(get_prev_epg_days()).c_str()));

	// <num_future_epg_days>2</num_future_epg_days>
	channel_node->append_node(utils::alloc_node(alloc, NUM_FUTURE_EPG_DAYS, utils::int_to_char(get_next_epg_days()).c_str()));

	// <tv_categories>
	//    <tv_category_id>1</tv_category_id>
	// </tv_categories>
	if (!categories.empty())
	{
		auto node = utils::alloc_node(alloc, TV_CATEGORIES);
		for (const auto& category : categories)
		{
			node->append_node(utils::alloc_node(alloc, TV_CATEGORY_ID, utils::int_to_char(category).c_str()));
		}
		channel_node->append_node(node);
	}

	// <streaming_url>http://ts://{SUBDOMAIN}/iptv/{UID}/127/index.m3u8</streaming_url>
	channel_node->append_node(utils::alloc_node(alloc, STREAMING_URL, get_stream_uri().get_id_translated_url().c_str()));

	// <archive>1</archive>
	if (get_has_archive())
	{
		channel_node->append_node(utils::alloc_node(alloc, ARCHIVE, utils::int_to_char(get_has_archive()).c_str()));
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

	return channel_node;
}

bool ChannelInfo::is_icon_local() const
{
	return (icon_uri.get_schema() == "plugin_file://");
}

std::wstring ChannelInfo::GetIconRelativePath(LPCTSTR szRoot /*= nullptr*/) const
{
	return icon_uri.get_icon_relative_path(szRoot);
}
