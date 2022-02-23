#pragma once
#include "uri_stream.h"

class uri_tvclub : public uri_stream
{
public:
	void parse_uri(const std::wstring& url) override;
	std::wstring get_templated_stream(StreamSubType subType, const TemplateParams& params) const override;
	std::wstring get_epg_uri_json(bool first, const std::wstring& id, time_t for_time = 0) const override;
	std::wstring get_playlist_template(const PlaylistTemplateParams& params) const override;
	std::wstring get_api_token(const std::wstring& login, const std::wstring& password) const override;
	bool parse_access_info(const PlaylistTemplateParams& params, std::list<AccountInfo>& info_list) const override;

	std::vector<std::tuple<StreamSubType, std::wstring>>& get_supported_stream_type() const override
	{
		static std::vector<std::tuple<StreamSubType, std::wstring>> streams = { {StreamSubType::enMPEGTS, L"MPEG-TS"} };
		return streams;
	};

protected:
	const nlohmann::json& get_epg_root(bool first, const nlohmann::json& epg_data) const override { return epg_data["epg"]["channels"][0]["epg"]; }
	std::string get_epg_name(bool first, const nlohmann::json& val) const override { return get_json_value("text", val); }
	std::string get_epg_desc(bool first, const nlohmann::json& val) const override { return get_json_value("description", val); }
	time_t get_epg_time_start(bool first, const nlohmann::json& val) const override { return get_json_int_value("start", val); }
	time_t get_epg_time_end(bool first, const nlohmann::json& val) const override { return get_json_int_value("end", val); }

private:
	std::wstring& append_archive(std::wstring& url) const override;
};