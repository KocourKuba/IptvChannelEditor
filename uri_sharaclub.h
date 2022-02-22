#pragma once
#include "uri_stream.h"

class uri_sharaclub : public uri_stream
{
public:
	uri_sharaclub() { epg2 = true; }

	void parse_uri(const std::wstring& url) override;
	std::wstring get_templated_stream(StreamSubType subType, const TemplateParams& params) const override;
	std::wstring get_epg_uri_json(bool first, const std::wstring& id) const override;
	std::wstring get_playlist_template(const PlaylistTemplateParams& params) const override;
	bool parse_access_info(const PlaylistTemplateParams& params, std::list<AccountInfo>& info_list) const override;

protected:
	const nlohmann::json& get_epg_root(bool first, const nlohmann::json& epg_data) const override { return epg_data; }
};
