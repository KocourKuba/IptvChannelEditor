#include "StdAfx.h"
#include "ChannelCategory.h"
#include "utils.h"
#include "uri.h"

std::string ChannelCategory::GetIconPath()
{
	static const auto scheme_size = strlen(uri::PLUGIN_SCHEME);
	std::string iconPath = get_icon_url();
	if (iconPath.substr(0, scheme_size) == uri::PLUGIN_SCHEME)
		iconPath.erase(0, scheme_size);

	return iconPath;
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
	set_id(utils::get_value_int(node->first_node(ID)));
	// <caption>Общие</caption>
	set_caption(utils::get_value_wstring(node->first_node(CAPTION)));
	// <icon_url>plugin_file://icons/1.png</icon_url>
	set_icon_url(utils::get_value_string(node->first_node(ICON_URL)));
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
	category_node->append_node(utils::alloc_node(alloc, ICON_URL, icon_url.c_str()));

	return category_node;
}
