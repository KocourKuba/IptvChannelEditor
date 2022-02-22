#pragma once
#include "uri_stream.h"

class uri_shuratv : public uri_stream
{
public:
	uri_shuratv() { use_duration1 = true; }
	void parse_uri(const std::wstring& url) override;
	std::wstring get_templated_stream(StreamSubType subType, const TemplateParams& params) const override;
	std::wstring get_epg_uri_json(bool first, const std::wstring& id) const override;
	std::wstring get_playlist_template(const PlaylistTemplateParams& params) const override;

protected:
	const nlohmann::json& get_epg_root(bool first, const nlohmann::json& epg_data) const override { return first ? epg_data : uri_stream::get_epg_root(true, epg_data); }
	std::string get_epg_name(bool first, const nlohmann::json& val) const override { return first ? get_json_value("name", val) : uri_stream::get_epg_name(true, val); }
	std::string get_epg_desc(bool first, const nlohmann::json& val) const override { return first ? get_json_value("text", val) : uri_stream::get_epg_desc(true, val); }
	time_t get_epg_time_start(bool first, const nlohmann::json& val) const override { return first ? get_json_int_value("start_time", val) : uri_stream::get_epg_time_start(true, val); }
	time_t get_epg_time_end(bool first, const nlohmann::json& val) const override { return first ? get_json_int_value("duration", val) : uri_stream::get_epg_time_end(true, val); }

private:
	std::wstring& append_archive(std::wstring& url) const override;
};
