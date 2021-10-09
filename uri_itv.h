#pragma once
#include "uri_stream.h"

class uri_itv : public uri_stream
{
public:
	void parse_uri(const std::string& url) override;
	std::string get_templated(StreamSubType subType, const TemplateParams& params) const override;
	std::string get_epg1_uri(const std::string& id) const override;
	std::string get_epg1_uri_json(const std::string& id) const override;
	std::string get_playlist_template(bool first = true) const override;
	std::string get_epg_root() const override { return "res"; }
	std::string get_epg_name() const override { return "title"; }
	std::string get_epg_desc() const override { return "desc"; }
	std::string get_epg_time_start() const override { return "startTime"; }
	std::string get_epg_time_end() const override { return "stopTime"; }
};
