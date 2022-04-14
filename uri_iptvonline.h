#pragma once
#include "epg_technic.h"

class uri_iptvonline : public epg_technic
{
public:
	uri_iptvonline() : epg_technic({ L"iptvxone", L"iptvx" }) {}

	void parse_uri(const std::wstring& url) override;
	std::wstring get_templated_stream(StreamSubType subType, const TemplateParams& params) const override;
	std::wstring get_playlist_template(const PlaylistTemplateParams& params) const override;
};
