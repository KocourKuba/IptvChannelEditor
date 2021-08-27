#include "StdAfx.h"
#include "uri_edem.h"

void uri_edem::parse_uri(const std::string& url)
{
	// http://localhost/iptv/00000000000000/204/index.m3u8

	static std::regex re_url(R"(https?:\/\/([\w\.]+)\/iptv\/(\w+)\/(\d+)\/.*\.m3u8)");
	std::smatch m;
	if (std::regex_match(url, m, re_url))
	{
		set_template(true);
		domain = m[1].str();
		token = m[2].str();
		id = m[3].str();
		return;
	}

	uri_stream::parse_uri(url);
}
