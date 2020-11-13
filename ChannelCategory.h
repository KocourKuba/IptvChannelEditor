#pragma once
#include "rapidxml.hpp"
#include "uri.h"
#include "ColoringProperty.h"
#include "IconContainer.h"

// <tv_category>
//   <id>1</id>
//   <caption>Общие</caption>
//   <icon_url>plugin_file://icons/1.png</icon_url>
// </tv_category>

class ChannelCategory
	: public IconContainer
	, public ColoringProperty
{
public:
	static constexpr auto TV_CATEGORY = "tv_category";
	static constexpr auto ID = "id";
	static constexpr auto CAPTION = "caption";

public:
	ChannelCategory();
	ChannelCategory(rapidxml::xml_node<>* node);

public:
	void ParseNode(rapidxml::xml_node<>* node);
	rapidxml::xml_node<>* GetNode(rapidxml::memory_pool<>& alloc) const;

	int get_id() const { return id; }
	void set_id(int val) { id = val; }

	void swap_id(ChannelCategory& src) { std::swap(id, src.id); }

	const std::wstring& get_caption() const { return caption; }
	void set_caption(const std::wstring& val) { caption = val; }

private:
	int id = 0;
	std::wstring caption;
};

