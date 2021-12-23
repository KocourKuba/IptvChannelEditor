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
	std::wstring get_templated(StreamSubType subType, TemplateParams& params) const override;
	std::wstring get_epg1_uri(const std::wstring& id) const override;
	std::wstring get_epg1_uri_json(const std::wstring& id) const override;
	std::wstring get_playlist_template(bool first = true) const override;
};
