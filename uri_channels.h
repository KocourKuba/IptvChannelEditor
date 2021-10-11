#pragma once
#include "uri_stream.h"

class uri_channels : public uri_stream
{
public:
	void parse_uri(const std::wstring& url) override;
};
