#pragma once
#include "uri_stream.h"

class uri_oneott : public uri_stream
{
public:
	void parse_uri(const std::wstring& url) override;
	std::wstring get_templated(StreamSubType subType, TemplateParams& params) const override;
	std::wstring get_epg1_uri(const std::wstring& id) const override;
	std::wstring get_epg2_uri(const std::wstring& id) const override;
	std::wstring get_epg1_uri_json(const std::wstring& id) const override;
	std::wstring get_epg2_uri_json(const std::wstring& id) const override;
	std::wstring get_access_url(const std::wstring& login, const std::wstring& password) const override;
	std::wstring get_playlist_template(bool first = true) const override;
	std::string get_epg_root(bool first = true) const override { return first ? "" : "epg_data"; }
	std::string get_epg_name(bool first = true) const override { return first ? "epg" : "title"; }
	std::string get_epg_desc(bool first = true) const override { return first ? "desc" : "descr"; }
	std::string get_epg_time_start(bool first = true) const override { return first ? "start" : "time"; }
	std::string get_epg_time_end(bool first = true) const override { return first ? "stop" : "time"; }

	bool parse_access_info(const std::vector<BYTE>& json_data, std::map<std::string, std::wstring>& params) const override;
	bool has_epg2() const override { return true; };
};
