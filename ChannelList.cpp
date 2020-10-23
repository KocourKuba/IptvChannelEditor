#include "StdAfx.h"
#include <fstream>
#include "ChannelList.h"
#include "rapidxml.hpp"
#include "rapidxml_print.hpp"
#include <algorithm>

int ChannelList::GetFreeID()
{
	std::set<int> busy;
	for (const auto& category : categories)
	{
		busy.emplace(category.second->get_id());
	}

	if (busy.empty())
		return 1;

	int free_id = *busy.begin();
	for (const auto& id : busy)
	{
		if (id != free_id) break;

		free_id = id;
		free_id++;
	}

	return free_id;
}

bool ChannelList::LoadFromFile(const std::wstring& path)
{
	categories.clear();
	channels.clear();

	std::ifstream is(path, std::istream::binary);
	if (!is.good())
		return false;

	// Read the xml file into a vector
	std::vector<char> buffer((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
	buffer.emplace_back('\0');

	// Parse the buffer using the xml file parsing library into doc
	rapidxml::xml_document<> doc;

	try
	{
		doc.parse<0>(buffer.data());
	}
	catch (rapidxml::parse_error& ex)
	{
		ex;
		return false;
	}

	auto i_node = doc.first_node(TV_INFO);

	auto cat_node = i_node->first_node(TV_CATEGORIES)->first_node(ChannelCategory::TV_CATEGORY);
	// Iterate <tv_category> nodes
	while (cat_node)
	{
		auto category = std::make_unique<ChannelCategory>(cat_node);
		categories.emplace(category->get_id(), std::move(category));
		cat_node = cat_node->next_sibling();
	}

	auto ch_node = i_node->first_node(TV_CHANNELS)->first_node(ChannelInfo::TV_CHANNEL);
	// Iterate <tv_channel> nodes
	while (ch_node)
	{
		channels.emplace_back(std::move(std::make_unique<ChannelInfo>(ch_node)));
		ch_node = ch_node->next_sibling();
	}

	return true;
}

bool ChannelList::SaveToFile(const std::wstring& path)
{
	// create document;
	rapidxml::xml_document<> doc;
	auto decl = doc.allocate_node(rapidxml::node_declaration);

	// adding attributes at the top of our xml
	decl->append_attribute(doc.allocate_attribute("version", "1.0"));
	decl->append_attribute(doc.allocate_attribute("encoding", "UTF-8"));
	doc.append_node(decl);

	// create <tv_info> root node
	auto root = doc.allocate_node(rapidxml::node_element, TV_INFO);

	// create <tv_categories> node
	auto cat_node = doc.allocate_node(rapidxml::node_element, TV_CATEGORIES);

	// append <tv_category> to <tv_categories> node
	for (auto& category : categories)
	{
		cat_node->append_node(category.second->GetNode(doc));
	}
	// append <tv_categories> to <tv_info> node
	root->append_node(cat_node);

	// create <tv_channels> node
	auto ch_node = doc.allocate_node(rapidxml::node_element, TV_CHANNELS);
	// append <tv_channel> to <tv_channels> node
	for (auto& channel : channels)
	{
		ch_node->append_node(channel->GetNode(doc));
	}
	// append <tv_channel> to <tv_info> node
	root->append_node(ch_node);

	doc.append_node(root);

	// write document
	std::ofstream os(path, std::istream::binary);
	os << doc;

	return true;
}
