#pragma once
#include "uri_stream.h"

class uri_fox : public uri_stream
{
public:
	void parse_uri(const std::string& url) override;
	std::string get_templated(StreamSubType subType, const TemplateParams& params) const override;
	std::string get_epg1_uri(const std::string& id) const override;
	std::string get_playlist_template(bool first = true) const override;
};
