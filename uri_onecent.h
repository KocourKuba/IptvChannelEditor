#pragma once
#include "epg_technic.h"

class uri_onecent : public epg_technic
{
public:
	uri_onecent();
	void parse_uri(const std::wstring& url) override;
	std::wstring get_templated_stream(StreamSubType subType, const TemplateParams& params) const override;
	std::wstring get_playlist_template(const PlaylistTemplateParams& params) const override;
};
