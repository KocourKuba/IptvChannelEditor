#pragma once
#include "uri_stream.h"

class uri_cbilling : public uri_stream
{
public:
	uri_cbilling() { epg2 = true; }

	void parse_uri(const std::wstring& url) override;
	std::wstring get_templated_stream(StreamSubType subType, const TemplateParams& params) const override;
	std::wstring get_epg_uri_json(bool first, const std::wstring& id) const override;
	std::wstring get_playlist_template(const PlaylistTemplateParams& params) const override;

	std::wstring get_access_info_header() const override;
	bool parse_access_info(const PlaylistTemplateParams& params, std::list<AccountInfo>& info_list) const override;

	std::vector<std::tuple<StreamSubType, std::wstring>>& get_supported_stream_type() const override
	{
		static std::vector<std::tuple<StreamSubType, std::wstring>> streams = { {StreamSubType::enHLS, L"HLS"}, {StreamSubType::enHLS2, L"HLS2"}, {StreamSubType::enMPEGTS, L"MPEG-TS"} };
		return streams;
	};
};
