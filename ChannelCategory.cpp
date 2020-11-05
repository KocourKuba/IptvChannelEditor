#include "StdAfx.h"
#include "ChannelCategory.h"
#include "utils.h"
#include "uri.h"

ChannelCategory::ChannelCategory()
{
	icon_uri.set_uri(utils::ICON_TEMPLATE);
}

std::string ChannelCategory::GetIconPath()
{
	return icon_uri.is_local() ? icon_uri.get_path() : icon_uri.get_uri();
}

ChannelCategory::ChannelCategory(rapidxml::xml_node<>* node)
{
	ParseNode(node);
}

void ChannelCategory::ParseNode(rapidxml::xml_node<>* node)
{
	if (!node)
		return;

	// <id>1</id>
	id = utils::get_value_int(node->first_node(ID));
	// <caption>Общие</caption>
	caption = utils::get_value_wstring(node->first_node(CAPTION));
	// <icon_url>plugin_file://icons/1.png</icon_url>
	icon_uri.set_uri(utils::get_value_string(node->first_node(ICON_URL)));
}

rapidxml::xml_node<>* ChannelCategory::GetNode(rapidxml::memory_pool<>& alloc)
{
	// <tv_channel>
	auto category_node = utils::alloc_node(alloc, TV_CATEGORY);

	// <id>1</id>
	category_node->append_node(utils::alloc_node(alloc, ID, utils::int_to_char(id).c_str()));

	// <caption>Общие</caption>
	category_node->append_node(utils::alloc_node(alloc, CAPTION, utils::utf16_to_utf8(caption).c_str()));

	// <icon_url>plugin_file://icons/1.png</icon_url>
	category_node->append_node(utils::alloc_node(alloc, ICON_URL, icon_uri.get_uri().c_str()));

	return category_node;
}
