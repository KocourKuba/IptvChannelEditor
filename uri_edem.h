#pragma once
#include "uri_stream.h"

class uri_edem : public uri_stream
{
public:
	void parse_uri(const std::string& url) override;
};
