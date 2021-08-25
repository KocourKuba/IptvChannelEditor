#pragma once
#include "uri_stream.h"

class uri_nostream : public uri_stream
{
public:
	void parse_uri(const std::string&) override {}
};
