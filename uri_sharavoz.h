#pragma once
#include "uri_stream.h"

class uri_sharavoz : public uri_stream
{
public:
	void parse_uri(const std::string& url) override;
	std::string get_templated(StreamSubType subType, int shift_back) const override;
	std::string get_epg1_uri(const std::string& id) const override;
	std::string get_epg2_uri(const std::string& id) const override;
};
