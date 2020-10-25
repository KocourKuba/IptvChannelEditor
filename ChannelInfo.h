#pragma once
#include <string>
#include <set>
#include "rapidxml.hpp"

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
{
public:
	static constexpr auto TV_CHANNEL = "tv_channel";
	static constexpr auto CAPTION = "caption";
	static constexpr auto TVG_ID = "tvg_id";
	static constexpr auto EPG_ID = "epg_id";
	static constexpr auto ICON_URL = "icon_url";
	static constexpr auto NUM_PAST_EPG_DAYS = "num_past_epg_days";
	static constexpr auto NUM_FUTURE_EPG_DAYS = "num_future_epg_days";
	static constexpr auto TV_CATEGORIES = "tv_categories";
	static constexpr auto TV_CATEGORY_ID = "tv_category_id";
	static constexpr auto STREAMING_URL = "streaming_url";
	static constexpr auto ARCHIVE = "archive";
	static constexpr auto PROTECTED = "protected";
	static constexpr auto PLUGIN_PATH = R"(plugin_file://)";

public:
	ChannelInfo();

	ChannelInfo(rapidxml::xml_node<>* node)
	{
		ParseNode(node);
	}

	void ParseNode(rapidxml::xml_node<>* node);


	rapidxml::xml_node<>* GetNode(rapidxml::memory_pool<>& doc);

	std::string CombineEdemStreamingUrl(const std::string& sub_domain, const std::string& ott_key);
	std::string SetChannelIdForStreamingUrl(int id);
	std::string GetIconRelativePath();
	void SetIconPluginPath(const std::string& relative_path);

	static int GetEdemStreamID(const std::string& edem_url);
	static std::string TranslateStreamingUrl(const std::string& url);
	static std::string ConvertPlainUrlToStreamingUrl(const std::string& url);

// Getters/Setters
	int get_edem_channel_id() const { return edem_channel_id; }
	void set_edem_channel_id(int val) { edem_channel_id = val; }

	const std::wstring& get_name() const { return name; }
	void set_name(const std::wstring& val) { name = val; }

	const std::string& get_tvguide_id() const { return tvguide_id; }
	void set_tvguide_id(const std::string& val) { tvguide_id = val; }

	const std::string& get_epg_id() const { return epg_id; }
	void set_epg_id(const std::string& val) { epg_id = val; }

	int get_adult() const { return adult; }
	void set_adult(int val) { adult = val; }

	const std::string& get_icon_url() const { return icon_url; }
	void set_icon_url(const std::string& val) { icon_url = val; }

	int get_prev_epg_days() const { return prev_epg_days; }
	void set_prev_epg_days(int val) { prev_epg_days = val; }

	int get_next_epg_days() const { return next_epg_days; }
	void set_next_epg_days(int val) { next_epg_days = val; }

	const std::set<int>& get_categores() const { return categories; }
	std::set<int>& get_categores() { return categories; }
	void set_categores(const std::set<int>& val) { categories = val; }

	const std::string& get_streaming_url() const { return streaming_url; }
	void set_streaming_url(const std::string& val) { streaming_url = val; set_edem_channel_id(ChannelInfo::GetEdemStreamID(val)); }

	int get_has_archive() const { return has_archive; }
	void set_has_archive(int val) { has_archive = val; }

protected:
	std::wstring name;
	std::string tvguide_id;
	std::string epg_id; // for compatibility not used in dune edem.tv plugin
	std::string icon_url;
	std::string streaming_url;
	std::set<int> categories;
	int edem_channel_id = 0;
	int prev_epg_days = 4;
	int next_epg_days = 2;
	int adult = 0;
	int has_archive = 0;
};

