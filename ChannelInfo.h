#pragma once
#include "rapidxml.hpp"
#include "BaseInfo.h"

// <tv_channel>
//     <caption>Первый канал</caption>
//     <tvg_id>1</tvg_id>
//     <epg_id>8</epg_id>
//     <icon_url>plugin_file://icons/channels/pervyi.png</icon_url>
//     <num_past_epg_days>4</num_past_epg_days>
//     <num_future_epg_days>2</num_future_epg_days>
//     <tv_categories>
//         <tv_category_id>1</tv_category_id>
//     </tv_categories>
//     <streaming_url>http://ts://{SUBDOMAIN}/iptv/{TOKEN}/127/index.m3u8</streaming_url>
//     <archive>1</archive>
//     <protected>1</protected>
// /tv_channel>

class ChannelCategory;

class ChannelInfo : public BaseInfo
{
public:
	static constexpr auto TV_CHANNEL = "tv_channel";
	static constexpr auto CHANNEL_ID = "channel_id";
	static constexpr auto EPG1_ID = "epg_id";
	static constexpr auto EPG2_ID = "tvg_id";
	static constexpr auto CAPTION = "caption";
	static constexpr auto PLUGIN_PATH = R"(plugin_file://)";
	static constexpr auto NUM_PAST_EPG_DAYS = "num_past_epg_days";
	static constexpr auto NUM_FUTURE_EPG_DAYS = "num_future_epg_days";
	static constexpr auto TIME_SHIFT_HOURS = "timeshift_hours";
	static constexpr auto TV_CATEGORIES = "tv_categories";
	static constexpr auto TV_CATEGORY_ID = "tv_category_id";
	static constexpr auto STREAMING_URL = "streaming_url";
	static constexpr auto ARCHIVE = "archive";
	static constexpr auto PROTECTED = "protected";
	static constexpr auto DISABLED = "disabled";
	static constexpr auto FAVORITE = "favorite";

public:
	ChannelInfo() = delete;
	ChannelInfo(StreamType streamType, const std::wstring& root_path);
	ChannelInfo(rapidxml::xml_node<>* node, StreamType streamType, const std::wstring& root_path);

public:
	void ParseNode(rapidxml::xml_node<>* node);

	rapidxml::xml_node<>* GetNode(rapidxml::memory_pool<>& doc) const;

// Getters/Setters

	bool is_icon_local() const;

	int get_time_shift_hours() const { return time_shift_hours; }
	void set_time_shift_hours(int val) { time_shift_hours = val; }

	std::set<int>& get_category_ids() { return categories; }

	bool is_favorite() const { return favorite; }
	void set_favorite(bool val) { favorite = val; }

	bool is_disabled() const { return disabled; }
	void set_disabled(bool val) { disabled = val; }

	bool is_changed() const { return changed; }

private:
	int time_shift_hours = 0;
	bool disabled = false;
	bool favorite = false;
	std::set<int> categories;
	bool changed = false;
};
