#pragma once
#include "uri_stream.h"

class uri_vidok : public uri_stream
{
public:
	void parse_uri(const std::wstring& url) override;
	std::wstring get_templated(StreamSubType subType, const TemplateParams& params) const override;
	std::wstring get_epg1_uri(const std::wstring& id) const override;
	std::wstring get_epg1_uri_json(const std::wstring& id) const override;
	std::wstring get_playlist_template(bool first = true) const override;
	std::string get_epg_root(bool first = true) const override { return "epg"; }
	std::string get_epg_name(bool first = true) const override { return "title"; }
	std::string get_epg_desc(bool first = true) const override { return "description"; }
	std::string get_epg_time_start(bool first = true) const override { return "start"; }
	std::string get_epg_time_end(bool first = true) const override { return "end"; }

	std::wstring get_access_url(const std::wstring& login, const std::wstring& password) const override;
	bool parse_access_info(const std::vector<BYTE>& json_data, std::list<AccountParams>& params) const override;

	std::vector<std::tuple<StreamSubType, std::wstring>>& getSupportedStreamType() const override
	{
		static std::vector<std::tuple<StreamSubType, std::wstring>> streams = { {StreamSubType::enHLS, L"HLS"} };
		return streams;
	};
};
