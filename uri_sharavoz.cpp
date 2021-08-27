#include "StdAfx.h"
#include "uri_sharavoz.h"
#include "utils.h"

void uri_sharavoz::parse_uri(const std::string& url)
{
	// http://domain.com/204/index.m3u8?token=adsdaSDFJKHKJd
	// http://domain.com/204/mpegts?token=adsdaSDFJKHKJd

	static std::regex re_url(R"(https?:\/\/([\w\.]+)\/(\d+)\/(?:mpegts|index\.m3u8)\?token=([\w]+))");
	std::smatch m;
	if (std::regex_match(url, m, re_url))
	{
		set_template(true);
		domain = m[1].str();
		token = m[3].str();
		id = m[2].str();
		return;
	}

	uri_stream::parse_uri(url);
}
