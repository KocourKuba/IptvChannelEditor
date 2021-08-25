#include "StdAfx.h"
#include "uri_sharavoz.h"
#include "utils.h"

void uri_sharavoz::parse_uri(const std::string& url)
{
	// http://domain.com/204/index.m3u8?token=adsdaSDFJKHKJd
	// http://domain.com/204/mpegts?token=adsdaSDFJKHKJd

	static std::regex re_url(R"(http?:\/\/([0-9a-z\.]+)\/(\d+)\/(?:mpegts|index.m3u8)\?token=([0-9A-Za-z]+))");
	std::smatch m;
	if (std::regex_match(url, m, re_url))
	{
		set_template(true);
		domain = m[1].str();
		uid = m[3].str();
		id = m[2].str();
	}
	else
	{
		// http://rtmp.api.rt.com/hls/rtdru.m3u8
		set_template(false);
		uri_base::set_uri(url);
	}
}
