#include "pch.h"
#include "ChannelInfo.h"
#include "ChannelCategory.h"

#include "UtilsLib\rapidxml_value.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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

ChannelInfo::ChannelInfo(const ChannelInfo& src) : BaseInfo(src)
{
	*this = src;
}

void ChannelInfo::ParseNode(rapidxml::xml_node<>* node)
{
	if (!node)
		return;

	set_title(rapidxml::get_value_wstring(node->first_node(utils::CAPTION)));
	stream_uri->set_template(true);
	stream_uri->set_id(rapidxml::get_value_wstring(node->first_node(utils::CHANNEL_ID)));
	stream_uri->set_int_id(rapidxml::get_value_wstring(node->first_node(utils::INT_ID)));
	set_epg1_id(rapidxml::get_value_wstring(node->first_node(utils::EPG1_ID)));
	set_epg2_id(rapidxml::get_value_wstring(node->first_node(utils::EPG2_ID)));
	set_icon_uri(rapidxml::get_value_wstring(node->first_node(ICON_URL)));
	set_disabled(utils::string_tolower(rapidxml::get_value_string(node->first_node(utils::DISABLED))) == "true");
	set_favorite(utils::string_tolower(rapidxml::get_value_string(node->first_node(utils::FAVORITE))) == "true");
	time_shift_hours = rapidxml::get_value_int(node->first_node(utils::TIME_SHIFT_HOURS));

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

	if (stream_uri->get_id().empty())
	{
		stream_uri->set_template(false);
		stream_uri->parse_uri(rapidxml::get_value_wstring(node->first_node(utils::STREAMING_URL)));
		stream_uri->get_hash();
	}

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
	if (stream_uri->is_template())
		channel_node->append_node(rapidxml::alloc_node(alloc, utils::CHANNEL_ID, utils::utf16_to_utf8(stream_uri->get_id()).c_str()));

	// used in glanz
	// <int_id>1</int_id>
	if (!stream_uri->get_int_id().empty())
	{
		channel_node->append_node(rapidxml::alloc_node(alloc, utils::INT_ID, utils::utf16_to_utf8(stream_uri->get_int_id()).c_str()));
	}

	// <epg_id>8</epg_id>
	if (!get_epg1_id().empty() && get_epg1_id() != L"0")
	{
		channel_node->append_node(rapidxml::alloc_node(alloc, utils::EPG1_ID, utils::utf16_to_utf8(get_epg1_id()).c_str()));
	}

	// <tvg_id>1</tvg_id>
	if (!get_epg2_id().empty() && get_epg2_id() != L"0")
	{
		channel_node->append_node(rapidxml::alloc_node(alloc, utils::EPG2_ID, utils::utf16_to_utf8(get_epg2_id()).c_str()));
	}

	// <icon_url>plugin_file://icons/channels/pervyi.png</icon_url>
	// <icon_url>http://epg.it999.ru/img/146.png</icon_url>
	if (!get_icon_uri().get_uri().empty())
	{
		channel_node->append_node(rapidxml::alloc_node(alloc, ICON_URL, utils::utf16_to_utf8(get_icon_uri().get_uri()).c_str()));
	}

	if (time_shift_hours != 0)
	{
		channel_node->append_node(rapidxml::alloc_node(alloc, utils::TIME_SHIFT_HOURS, std::to_string(time_shift_hours).c_str()));
	}

	// <tv_category_id>1</tv_category_id>
	// new version must not have more than on entry
	channel_node->append_node(rapidxml::alloc_node(alloc, utils::TV_CATEGORY_ID, std::to_string(*categories.begin()).c_str()));

	// Only if channel not templated. Otherwise template handled by plugin
	// <streaming_url>http://ts://{SUBDOMAIN}/iptv/{TOKEN}/127/index.m3u8</streaming_url>
	if (!stream_uri->is_template() && !stream_uri->get_uri().empty())
	{
		channel_node->append_node(rapidxml::alloc_node(alloc, utils::STREAMING_URL, utils::utf16_to_utf8(stream_uri->get_uri()).c_str()));
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

	if (is_favorite())
	{
		channel_node->append_node(rapidxml::alloc_node(alloc, utils::FAVORITE, "true"));
	}

	return channel_node;
}

bool ChannelInfo::is_icon_local() const
{
	return (get_icon_uri().get_schema() == uri_base::PLUGIN_SCHEME);
}
