#pragma once
#include "uri_stream.h"

class uri_glanz : public uri_stream
{
	static constexpr auto REPL_LOGIN = L"{LOGIN}";
	static constexpr auto REPL_PASSWORD = L"{PASSWORD}";
	static constexpr auto REPL_INT_ID = L"{INT_ID}";
	static constexpr auto REPL_HOST = L"{HOST}";

public:
	void parse_uri(const std::wstring& url) override;
	std::wstring get_templated_stream(StreamSubType subType, const TemplateParams& params) const override;
	std::wstring get_epg_uri_json(bool first, const std::wstring& id) const override;
	std::wstring get_playlist_template(const PlaylistTemplateParams& params) const override;
};
