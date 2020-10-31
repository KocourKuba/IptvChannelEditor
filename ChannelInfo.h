#pragma once
#include <string>
#include <set>
#include "rapidxml.hpp"
#include "uri.h"
#include "ColoringProperty.h"

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
//     <streaming_url>http://ts://{SUBDOMAIN}/iptv/{UID}/127/index.m3u8</streaming_url>
//     <archive>1</archive>
//     <protected>1</protected>
// /tv_channel>

class ChannelInfo : public ColoringProperty
{
public:
	static constexpr auto TV_CHANNEL = "tv_channel";

public:
	ChannelInfo();

	ChannelInfo(rapidxml::xml_node<>* node);

	void ParseNode(rapidxml::xml_node<>* node);

	rapidxml::xml_node<>* GetNode(rapidxml::memory_pool<>& doc);

	std::wstring GetIconRelativePath(LPCTSTR szRoot = nullptr);

	bool is_custom() { return streaming_uri.is_template(); }

// Getters/Setters

	int get_channel_id() const { return streaming_uri.get_Id(); }
	void set_channel_id(int val) { streaming_uri.set_Id(val); }

	const std::wstring& get_name() const { return name; }
	void set_name(const std::wstring& val) { name = val; }

	int get_tvguide_id() const { return tvguide_id; }
	void set_tvguide_id(int val) { tvguide_id = val; }

	int get_epg_id() const { return epg_id; }
	void set_epg_id(int val) { epg_id = val; }

	int get_adult() const { return adult; }
	void set_adult(int val) { adult = val; }

	bool is_icon_local() const;
	void set_icon_url(const std::string& val) { icon_uri.set_uri(val); }

	const uri& get_icon_uri() const { return icon_uri; }
	uri& get_icon_uri() { return icon_uri; }
	void set_icon_uri(const uri& val) { icon_uri = val; }

	int get_prev_epg_days() const { return prev_epg_days; }
	void set_prev_epg_days(int val) { prev_epg_days = val; }

	int get_next_epg_days() const { return next_epg_days; }
	void set_next_epg_days(int val) { next_epg_days = val; }

	const std::set<int>& get_categores() const { return categories; }
	std::set<int>& get_categores() { return categories; }
	void set_categores(const std::set<int>& val) { categories = val; }

	const uri_stream& get_streaming_uri() const { return streaming_uri; }
	uri_stream& get_streaming_uri() { return streaming_uri; }
	void set_stream_uri(const uri_stream& val) { streaming_uri = val; }

	int get_has_archive() const { return has_archive; }
	void set_has_archive(int val) { has_archive = val; }

protected:
	std::wstring name;
	int tvguide_id;
	int epg_id; // for compatibility not used in dune edem.tv plugin
	uri icon_uri;
	uri_stream streaming_uri;
	std::set<int> categories;
	int edem_channel_id = 0;
	int prev_epg_days = 4;
	int next_epg_days = 2;
	int adult = 0;
	int has_archive = 0;
};

