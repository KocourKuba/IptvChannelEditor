#include "StdAfx.h"
#include "ChannelInfo.h"
#include "utils.h"

ChannelInfo::ChannelInfo(const std::map<int, std::unique_ptr<ChannelCategory>>& all_categories)
	: m_all_categories(all_categories)
{
	set_icon_uri(utils::ICON_TEMPLATE);
	get_stream_uri().set_uri(utils::URI_TEMPLATE);
}

ChannelInfo::ChannelInfo(rapidxml::xml_node<>* node, const std::map<int, std::unique_ptr<ChannelCategory>>& all_categories)
	: m_all_categories(all_categories)
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
	prev_epg_days = utils::get_value_int(node->first_node(NUM_PAST_EPG_DAYS));
	next_epg_days = utils::get_value_int(node->first_node(NUM_FUTURE_EPG_DAYS));
	set_icon_uri(utils::get_value_string(node->first_node(ICON_URL)));
	set_disabled(utils::string_tolower(utils::get_value_string(node->first_node(DISABLED))) == "true");

	auto cnode = node->first_node(TV_CATEGORIES);
	if (cnode)
	{
		auto catNode = cnode->first_node();
		while (catNode)
		{
			auto cat_id = utils::get_value_int(catNode);
			auto pair = m_all_categories.find(cat_id);
			ASSERT(pair != m_all_categories.end());
			auto res = categories.emplace(cat_id, pair->second.get());
			ASSERT(res.second);
			catNode = catNode->next_sibling();
		}
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
			node->append_node(utils::alloc_node(alloc, TV_CATEGORY_ID, utils::int_to_char(category.second->get_id()).c_str()));
		}
		channel_node->append_node(node);
	}

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

	return channel_node;
}

bool ChannelInfo::is_icon_local() const
{
	return (get_icon_uri().get_schema() == "plugin_file://");
}

void ChannelInfo::set_categores(const std::set<int>& val)
{
	categories.clear();
	for (const auto& item : val)
	{
		if (auto pair = m_all_categories.find(item); pair != m_all_categories.end())
			categories.emplace(item, pair->second.get());
	}
}

const std::set<int> ChannelInfo::get_category_ids() const
{
	std::set<int> ids;
	for (const auto& pair : categories)
		ids.emplace(pair.first);

	return ids;
}

void ChannelInfo::set_category(int val)
{
	if (auto pair = m_all_categories.find(val); pair != m_all_categories.end())
		categories.emplace(val, pair->second.get());
}

ChannelCategory* ChannelInfo::find_category(int id)
{
	auto pair = categories.find(id);
	return pair != categories.end() ? pair->second : nullptr;
}

void ChannelInfo::rebiuld_categories()
{
	std::map<int, ChannelCategory*> new_categories;
	for (const auto& pair : categories)
		new_categories.emplace(pair.second->get_id(), pair.second);

	std::swap(categories, new_categories);
}

std::wstring ChannelInfo::GetIconRelativePath(LPCTSTR szRoot /*= nullptr*/) const
{
	return get_icon_uri().get_icon_relative_path(szRoot);
}
