#pragma once
#include "uri_stream.h"

class epg_technic : public uri_stream
{
public:
	std::map<std::wstring, std::wstring> get_tvg_id_mapper() const override;
	std::wstring get_epg_uri_json(bool first, const std::wstring& id, time_t for_time = 0) const override;

protected:
	nlohmann::json get_epg_root(bool first, const nlohmann::json& epg_data) const override;
	std::string get_epg_name(bool first, const nlohmann::json& val) const override;
	std::string get_epg_desc(bool first, const nlohmann::json& val) const override;
	time_t get_epg_time_start(bool first, const nlohmann::json& val) const override;
	time_t get_epg_time_end(bool first, const nlohmann::json& val) const override;

protected:
	std::wstring m_source;
};
