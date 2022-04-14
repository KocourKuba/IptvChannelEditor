#pragma once
#include "uri_stream.h"

class uri_shuratv : public uri_stream
{
public:
	uri_shuratv() { use_duration[0] = true; }
	void parse_uri(const std::wstring& url) override;
	std::wstring get_templated_stream(StreamSubType subType, const TemplateParams& params) const override;
	std::wstring get_epg_uri_json(int epg_idx, const std::wstring& id, time_t for_time = 0) const override;
	std::wstring get_playlist_template(const PlaylistTemplateParams& params) const override;

protected:
	nlohmann::json get_epg_root(int epg_idx, const nlohmann::json& epg_data) const override;
	std::string get_epg_name(int epg_idx, const nlohmann::json& val) const override;
	std::string get_epg_desc(int epg_idx, const nlohmann::json& val) const override;
	time_t get_epg_time_start(int epg_idx, const nlohmann::json& val) const override;
	time_t get_epg_time_end(int epg_idx, const nlohmann::json& val) const override;

private:
	std::wstring& append_archive(std::wstring& url) const override;
};
