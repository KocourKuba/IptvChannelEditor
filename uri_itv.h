#pragma once
#include "uri_stream.h"

class uri_itv : public uri_stream
{
public:
	void parse_uri(const std::wstring& url) override;
	std::wstring get_templated(StreamSubType subType, const TemplateParams& params) const override;
	std::wstring get_access_url(const std::wstring& login, const std::wstring& password) const override;
	std::wstring get_epg1_uri(const std::wstring& id) const override;
	std::wstring get_epg1_uri_json(const std::wstring& id) const override;
	std::wstring get_playlist_template(bool first = true) const override;
	std::string get_epg_root() const override { return "res"; }
	std::string get_epg_name() const override { return "title"; }
	std::string get_epg_desc() const override { return "desc"; }
	std::string get_epg_time_start() const override { return "startTime"; }
	std::string get_epg_time_end() const override { return "stopTime"; }
	bool isHasAccessInfo() const override { return true; }
};
