#pragma once
#include "uri_stream.h"

class uri_shuratv : public uri_stream
{
public:
	void parse_uri(const std::wstring& url) override;
	std::wstring get_templated(StreamSubType subType, const TemplateParams& params) const override;
	std::wstring get_epg1_uri(const std::wstring& id) const override;
	std::wstring get_epg1_uri_json(const std::wstring& id) const override;
	std::wstring get_playlist_template(bool first = true) const override;
	std::string get_epg_root(bool first = true) const override { return first ? "" : "epg_data"; }
	std::string get_epg_name(bool first = true) const override { return first ? "name" : "title"; }
	std::string get_epg_desc(bool first = true) const override { return first ? "text" : "descr"; }
	std::string get_epg_time_start(bool first = true) const override { return first ? "start_time" : "time"; }
	std::string get_epg_time_end(bool first = true) const override { return first ? "duration" : "time"; }
	bool get_use_duration(bool first = true) const override { return true; }

protected:
	std::wstring& AppendArchive(std::wstring& url) const override;
};
