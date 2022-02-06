#pragma once
#include "uri_stream.h"

class uri_fox : public uri_stream
{
public:
	void parse_uri(const std::wstring& url) override;
	std::wstring get_templated(StreamSubType subType, const TemplateParams& params) const override;
	std::wstring get_epg1_uri(const std::wstring& id) const override;
	std::wstring get_epg1_uri_json(const std::wstring& id) const override;
	std::wstring get_playlist_template(bool first = true) const override;

	std::vector<std::tuple<StreamSubType, std::wstring>>& getSupportedStreamType() const override
	{
		static std::vector<std::tuple<StreamSubType, std::wstring>> streams = { {StreamSubType::enHLS, L"HLS"} };
		return streams;
	};
};
