#pragma once
#include "uri_stream.h"

class uri_lightiptv : public uri_stream
{
public:
	void parse_uri(const std::wstring& url) override;
	std::wstring get_templated(StreamSubType subType, const TemplateParams& params) const override;
	std::wstring get_epg1_uri(const std::wstring& id) const override;
	std::wstring get_epg1_uri_json(const std::wstring& id) const override;
	std::wstring get_playlist_template(bool first = true) const override;

/*
	std::vector<StreamSubType>& getSupportedStreamType() const override
	{
		static std::vector<StreamSubType> streams = { StreamSubType::enHLS, StreamSubType::enHLS2, StreamSubType::enMPEGTS };
		return streams;
	};
*/
};
