#pragma once
#include "uri_stream.h"

class uri_glanz : public uri_stream
{
public:
	void parse_uri(const std::string& url) override;
};