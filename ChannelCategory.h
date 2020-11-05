#pragma once
#include <string>
#include "rapidxml.hpp"
#include "uri.h"

// <tv_category>
//   <id>1</id>
//   <caption>Общие</caption>
//   <icon_url>plugin_file://icons/1.png</icon_url>
// </tv_category>

class ChannelCategory
{
public:
	static constexpr auto TV_CATEGORY = "tv_category";
	static constexpr auto ID = "id";
	static constexpr auto CAPTION = "caption";
	static constexpr auto ICON_URL = "icon_url";

public:
	ChannelCategory();
	ChannelCategory(const ChannelCategory& src)
	{
		if (&src != this)
		{
			*this = src;
		}
	}
	ChannelCategory(rapidxml::xml_node<>* node);

public:
	void ParseNode(rapidxml::xml_node<>* node);
	rapidxml::xml_node<>* GetNode(rapidxml::memory_pool<>& alloc);
	std::string GetIconPath();

	int get_id() const { return id; }
	void set_id(int val) { id = val; }

	const std::wstring& get_caption() const { return caption; }
	void set_caption(const std::wstring& val) { caption = val; }

	const uri& get_icon_uri() const { return icon_uri; }
	void set_icon_uri(const uri& val) { icon_uri = val; }

protected:
	int id = 0;
	std::wstring caption;
	uri icon_uri;
};

