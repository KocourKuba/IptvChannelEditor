#pragma once
#include "uri_stream.h"

class epg_technic : public uri_stream
{
public:
	epg_technic(std::array<std::wstring, 2> sources);

	std::array<std::map<std::wstring, std::wstring>, 2> get_tvg_id_mapper() override;
	std::wstring get_epg_uri_json(int epg_idx, const std::wstring& id, time_t for_time = 0) const override;

protected:
	nlohmann::json get_epg_root(int epg_idx, const nlohmann::json& epg_data) const override;
	std::string get_epg_name(int epg_idx, const nlohmann::json& val) const override;
	std::string get_epg_desc(int epg_idx, const nlohmann::json& val) const override;
	time_t get_epg_time_start(int epg_idx, const nlohmann::json& val) const override;
	time_t get_epg_time_end(int epg_idx, const nlohmann::json& val) const override;

protected:
	std::array<std::wstring, 2> m_sources;
	std::array<bool, 2> m_use_mapper = { false, false };
};
