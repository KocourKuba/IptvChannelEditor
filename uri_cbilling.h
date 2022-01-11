#pragma once
#include "uri_stream.h"

class uri_cbilling : public uri_stream
{
public:
	void parse_uri(const std::wstring& url) override;
	std::wstring get_templated(StreamSubType subType, TemplateParams& params) const override;
	std::wstring get_access_url(const std::wstring& login, const std::wstring& password) const override;

	std::wstring get_epg1_uri(const std::wstring& id) const override;
	std::wstring get_epg1_uri_json(const std::wstring& id) const override;
	std::wstring get_epg2_uri(const std::wstring& id) const override;
	std::wstring get_epg2_uri_json(const std::wstring& id) const override;
	std::wstring get_playlist_template(bool first = true) const override;

	std::wstring get_access_info_header() const override;
	bool parse_access_info(const std::vector<BYTE>& json_data, std::list<AccountParams>& params) const override;
	bool has_epg2() const override { return true; };

	std::vector<std::tuple<StreamSubType, std::wstring>>& getSupportedStreamType() const override
	{
		static std::vector<std::tuple<StreamSubType, std::wstring>> streams = { {StreamSubType::enHLS, L"HLS"}, {StreamSubType::enHLS2, L"HLS2"}, {StreamSubType::enMPEGTS, L"MPEG-TS"} };
		return streams;
	};
};
