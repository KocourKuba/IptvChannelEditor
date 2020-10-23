#pragma once
#include <string>
#include "rapidxml.hpp"

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
	static constexpr auto PLUGIN_SCHEME = "plugin_file://";

public:
	ChannelCategory() = default;
	ChannelCategory(const ChannelCategory& src)
	{
		if (&src != this)
		{
			*this = src;
		}
	}
	ChannelCategory(rapidxml::xml_node<>* node)
	{
		ParseNode(node);
	}

public:
	static std::string IconPathToPluginPath(const std::string& iconPath)
	{
		return PLUGIN_SCHEME + iconPath;
	}

	void ParseNode(rapidxml::xml_node<>* node);
	rapidxml::xml_node<>* GetNode(rapidxml::memory_pool<>& alloc);
	std::string GetIconPath();

	int get_id() const { return id; }
	void set_id(int val) { id = val; }

	std::wstring get_caption() const { return caption; }
	void set_caption(std::wstring val) { caption = val; }

	std::string get_icon_url() const { return icon_url; }
	void set_icon_url(std::string val) { icon_url = val; }

protected:
	int id = 0;
	std::wstring caption;
	std::string icon_url;
};

