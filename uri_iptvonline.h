#pragma once
#include "uri_stream.h"

class uri_iptvonline : public uri_stream
{
public:
	uri_iptvonline() { epg_proxy = true; }
	void parse_uri(const std::wstring& url) override;
	std::wstring get_templated_stream(StreamSubType subType, const TemplateParams& params) const override;
	std::wstring get_epg_uri_json(bool first, const std::wstring& id, time_t for_time = 0) const override;
	std::wstring get_playlist_template(const PlaylistTemplateParams& params) const override;
};
