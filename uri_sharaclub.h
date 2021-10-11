#pragma once
#include "uri_stream.h"

class uri_sharaclub : public uri_stream
{
public:
	void parse_uri(const std::wstring& url) override;
	std::wstring get_templated(StreamSubType subType, const TemplateParams& params) const override;
	std::wstring get_epg1_uri(const std::wstring& id) const override;
	std::wstring get_epg2_uri(const std::wstring& id) const override;
	std::wstring get_epg1_uri_json(const std::wstring& id) const override;
	std::wstring get_epg2_uri_json(const std::wstring& id) const override;
	std::wstring get_access_url(const std::wstring& login, const std::wstring& password) const override;
	std::wstring get_playlist_template(bool first = true) const override;
	std::string get_epg_root() const override { return ""; }

	bool isHasAccessInfo() const override { return true; }
};
