#pragma once
#include "rapidxml.hpp"
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
	ChannelCategory() = delete;
	ChannelCategory(StreamType streamType, const std::wstring& root_path);
	ChannelCategory(rapidxml::xml_node<>* node, StreamType streamType, const std::wstring& root_path);

public:
	void ParseNode(rapidxml::xml_node<>* node);
	rapidxml::xml_node<>* GetNode(rapidxml::memory_pool<>& alloc) const;

	bool is_empty() const { return channels_map.empty(); }

	const std::vector<std::shared_ptr<ChannelInfo>>& get_channels() const { return channels; }

	void move_channels(const std::shared_ptr<ChannelInfo>& range_start, const std::shared_ptr<ChannelInfo>& range_end, bool down);

	bool add_channel(const std::shared_ptr<ChannelInfo>& channel);

	void remove_channel(const std::wstring& ch_id);

	void sort_channels();

	std::shared_ptr<ChannelInfo> find_channel(const std::wstring& ch_id);

private:
	std::vector<std::shared_ptr<ChannelInfo>> channels;
	std::map<std::wstring, std::shared_ptr<ChannelInfo>> channels_map;
};

