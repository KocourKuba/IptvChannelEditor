#pragma once
#include "rapidxml.hpp"
#include "uri.h"
#include "ChannelInfo.h"

// <tv_category>
//   <id>1</id>
//   <caption>Общие</caption>
//   <icon_url>plugin_file://icons/1.png</icon_url>
// </tv_category>

class ChannelCategory : public BaseInfo
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

	int get_id() const override { return id; }
	void set_id(int val) override { id = val; }

	bool is_empty() const { return channels_map.empty(); }

	const std::vector<ChannelInfo*>& get_channels() const { return channels; }

	void move_channels(const ChannelInfo* range_start, const ChannelInfo* range_end, bool down);

	void add_channel(std::shared_ptr<ChannelInfo>& channel);

	void remove_channel(int ch_id);

	void sort_channels();

	std::shared_ptr<ChannelInfo> find_channel(int ch_id);

private:
	int id = 0;
	std::vector<ChannelInfo*> channels;
	std::map<int, std::shared_ptr<ChannelInfo>> channels_map;
};

