#pragma once
#include "rapidxml.hpp"
#include "uri.h"
#include "ColoringProperty.h"
#include "IconContainer.h"
#include "InfoContainer.h"
#include "ChannelCategory.h"

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

class ChannelInfo
	: public IconContainer
	, public InfoContainer
	, public ColoringProperty
{
public:
	static constexpr auto TV_CHANNEL = "tv_channel";
	static constexpr auto EPG_ID = "epg_id";
	static constexpr auto TVG_ID = "tvg_id";
	static constexpr auto CAPTION = "caption";
	static constexpr auto PLUGIN_PATH = R"(plugin_file://)";
	static constexpr auto NUM_PAST_EPG_DAYS = "num_past_epg_days";
	static constexpr auto NUM_FUTURE_EPG_DAYS = "num_future_epg_days";
	static constexpr auto TV_CATEGORIES = "tv_categories";
	static constexpr auto TV_CATEGORY_ID = "tv_category_id";
	static constexpr auto STREAMING_URL = "streaming_url";
	static constexpr auto ARCHIVE = "archive";
	static constexpr auto PROTECTED = "protected";
	static constexpr auto DISABLED = "disabled";

public:
	ChannelInfo(const std::map<int, std::unique_ptr<ChannelCategory>>& all_categories);
	ChannelInfo(rapidxml::xml_node<>* node, const std::map<int, std::unique_ptr<ChannelCategory>>& all_categories);

public:
	void ParseNode(rapidxml::xml_node<>* node);

	rapidxml::xml_node<>* GetNode(rapidxml::memory_pool<>& doc) const;

	std::wstring GetIconRelativePath(LPCTSTR szRoot = nullptr) const;

	bool is_custom() const { return stream_uri.is_template(); }

// Getters/Setters

	int get_channel_id() const { return stream_uri.get_Id(); }
	void set_channel_id(int val) { stream_uri.set_Id(val); }

	const std::wstring& get_title() const { return title; }
	void set_title(const std::wstring& val) { title = val; }

	int get_tvg_id() const { return tvg_id; }
	void set_tvg_id(int val) { tvg_id = val; }

	int get_epg_id() const { return epg_id; }
	void set_epg_id(int val) { epg_id = val; }

	int get_adult() const { return adult; }
	void set_adult(int val) { adult = val; }

	bool is_icon_local() const;

	int get_prev_epg_days() const { return prev_epg_days; }
	void set_prev_epg_days(int val) { prev_epg_days = val; }

	int get_next_epg_days() const { return next_epg_days; }
	void set_next_epg_days(int val) { next_epg_days = val; }

	const std::map<int, ChannelCategory*>& get_categores() const { return categories; }
	void set_categores(const std::set<int>& val);
	const std::set<int> get_category_ids() const;

	void set_category(int val);
	void erase_category(int id) { categories.erase(id); }
	ChannelCategory* find_category(int id);
	void rebiuld_categories();

	const uri_stream& get_stream_uri() const { return stream_uri; }
	void set_stream_uri(const uri_stream& val) { stream_uri = val; }
	void set_stream_uri(const std::string& val) { stream_uri.set_uri(val); }

	int get_has_archive() const { return has_archive; }
	void set_has_archive(int val) { has_archive = val; }

private:
	std::wstring title;
	int tvg_id = 0; // TVGuide id http://www.teleguide.info/kanal%d.html
	int epg_id = 0; // ott-play epg http://epg.ott-play.com/edem/epg/%d.json
	uri_stream stream_uri;
	std::map<int, ChannelCategory*> categories;
	const std::map<int, std::unique_ptr<ChannelCategory>>& m_all_categories;
	int prev_epg_days = 4;
	int next_epg_days = 2;
	int adult = 0;
	int has_archive = 0;
};
