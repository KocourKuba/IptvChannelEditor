#include "StdAfx.h"
#include <regex>
#include "ChannelInfo.h"
#include "utils.h"

static constexpr auto url_template = "http://ts://{SUBDOMAIN}/iptv/{UID}/{ID}/index.m3u8";
static constexpr auto icon_template = "file://icons/channel_unset.png";

ChannelInfo::ChannelInfo()
	: icon_url(icon_template)
{
	streaming_url = url_template;
}

void ChannelInfo::ParseNode(rapidxml::xml_node<>* node)
{
	if (!node)
		return;

	set_name(utils::get_value_wstring(node->first_node(CAPTION)));
	set_tvguide_id(utils::get_value_string(node->first_node(TVG_ID)));
	set_epg_id(utils::get_value_string(node->first_node(EPG_ID)));
	set_icon_url(utils::get_value_string(node->first_node(ICON_URL)));
	set_prev_epg_days(utils::get_value_int(node->first_node(NUM_PAST_EPG_DAYS)));
	set_next_epg_days(utils::get_value_int(node->first_node(NUM_FUTURE_EPG_DAYS)));

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

	set_streaming_url(utils::get_value_string(node->first_node(STREAMING_URL)));
	set_has_archive(utils::get_value_int(node->first_node(ARCHIVE)));
	set_adult(utils::get_value_int(node->first_node(PROTECTED)));
	set_edem_channel_id(ChannelInfo::GetEdemStreamID(get_streaming_url()));
}

int ChannelInfo::GetEdemStreamID(const std::string& edem_url)
{
	// http://ts://{SUBDOMAIN}/iptv/{UID}/205/index.m3u8
	int id = 0;
	std::regex re(R"(http[s]{0,1}:\/\/ts:\/\/\{SUBDOMAIN\}\/iptv\/\{UID\}\/(\d+)\/index.m3u8)");
	std::smatch m;
	if (std::regex_match(edem_url, m, re))
	{
		id = utils::char_to_int(m[1].str().c_str());
	}

	return id;
}

rapidxml::xml_node<>* ChannelInfo::GetNode(rapidxml::memory_pool<>& alloc)
{
	// <tv_channel>
	auto channel_node = utils::alloc_node(alloc, TV_CHANNEL);

	// <caption>Первый канал</caption>
	channel_node->append_node(utils::alloc_node(alloc, CAPTION, utils::utf16_to_utf8(name).c_str()));

	// <tvg_id>1</tvg_id>
	channel_node->append_node(utils::alloc_node(alloc, TVG_ID, tvguide_id.c_str()));

	// <epg_id>8</epg_id>
	channel_node->append_node(utils::alloc_node(alloc, EPG_ID, epg_id.c_str()));

	// <icon_url>plugin_file://icons/channels/pervyi.png</icon_url>
	channel_node->append_node(utils::alloc_node(alloc, ICON_URL, icon_url.c_str()));

	// <num_past_epg_days>4</num_past_epg_days>
	channel_node->append_node(utils::alloc_node(alloc, NUM_PAST_EPG_DAYS, utils::int_to_char(prev_epg_days).c_str()));

	// <num_future_epg_days>2</num_future_epg_days>
	channel_node->append_node(utils::alloc_node(alloc, NUM_FUTURE_EPG_DAYS, utils::int_to_char(next_epg_days).c_str()));

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
	channel_node->append_node(utils::alloc_node(alloc, STREAMING_URL, streaming_url.c_str()));

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

	return channel_node;
}

std::string ChannelInfo::CombineEdemStreamingUrl(const std::string& sub_domain, const std::string& ott_key)
{
	// http://ts://{SUBDOMAIN}/iptv/{UID}/205/index.m3u8

	std::regex re_domain(R"(\{SUBDOMAIN\})");
	std::regex re_uid(R"(\{UID\})");
	std::regex re_ts(R"(ts:\/\/)");

	std::string url = get_streaming_url();
	url = std::regex_replace(url, re_domain, sub_domain);
	url = std::regex_replace(url, re_uid, ott_key);
	url = std::regex_replace(url, re_ts, "");

	return url;
}

std::string ChannelInfo::TranslateStreamingUrl(const std::string& url)
{
	// http://ts://rtmp.api.rt.com/hls/rtdru.m3u8

	std::regex re_ts(R"(ts:\/\/)");

	return std::regex_replace(url, re_ts, "");
}

std::string ChannelInfo::ConvertPlainUrlToStreamingUrl(const std::string& url)
{
	// convert http://6asfsdb6bc.akadatel.com/iptv/FERWEE2VNSK/2402/index.m3u8
	// to http://ts://{SUBDOMAIN}/iptv/{UID}/205/index.m3u8

	std::regex re(R"((http[s]{0,1}:\/\/)([0-9a-z\.]+)(\/iptv\/)([0-9A-Z]+)(\/\d+\/index.m3u8))");

	return std::regex_replace(url, re, "$1ts://{SUBDOMAIN}$3{UID}$5");
}

std::string ChannelInfo::SetChannelIdForStreamingUrl(int id)
{
	if (id == 0)
		return get_streaming_url();

	const auto& channel = utils::int_to_char(id);
	std::regex re_ch(R"(\{ID\})");
	std::string url = std::regex_replace(url_template, re_ch, channel);

	return url;
}

std::string ChannelInfo::GetIconRelativePath()
{
	std::regex re_pf(R"(plugin_file:\/\/)");
	return std::regex_replace(get_icon_url(), re_pf, "");
}

void ChannelInfo::SetIconPluginPath(const std::string& relative_path)
{
	std::string normilized(PLUGIN_PATH + relative_path);
	std::replace(normilized.begin(), normilized.end(), '\\', '/');
	set_icon_url(normilized);
}
