#pragma once
#include "uri_stream.h"

class uri_vidok : public uri_stream
{
public:
	void parse_uri(const std::wstring& url) override;
	std::wstring get_templated_stream(StreamSubType subType, const TemplateParams& params) const override;
	std::wstring get_epg_uri_json(int epg_idx, const std::wstring& id, time_t for_time = 0) const override;
	std::wstring get_playlist_template(const PlaylistTemplateParams& params) const override;
	std::wstring get_api_token(const std::wstring& login, const std::wstring& password) const override;
	bool parse_access_info(const PlaylistTemplateParams& params, std::list<AccountInfo>& info_list) const override;

	std::vector<std::tuple<StreamSubType, std::wstring>>& get_supported_stream_type() const override
	{
		static std::vector<std::tuple<StreamSubType, std::wstring>> streams = { {StreamSubType::enHLS, L"HLS"} };
		return streams;
	};

protected:
	nlohmann::json get_epg_root(int epg_idx, const nlohmann::json& epg_data) const override;
	std::string get_epg_name(int epg_idx, const nlohmann::json& val) const override;
	std::string get_epg_desc(int epg_idx, const nlohmann::json& val) const override;
	time_t get_epg_time_start(int epg_idx, const nlohmann::json& val) const override;
	time_t get_epg_time_end(int epg_idx, const nlohmann::json& val) const override;
};
