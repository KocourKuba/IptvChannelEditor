#include "StdAfx.h"
#include "uri_edem.h"
#include "utils.h"

void uri_edem::parse_uri(const std::string& url)
{
	// http://localhost/iptv/00000000000000/204/index.m3u8

	static std::regex re_url(R"(http?:\/\/([0-9a-z\.]+)\/iptv\/([0-9A-Za-z]+)\/(\d+)\/index.m3u8)");
	std::smatch m;
	if (std::regex_match(url, m, re_url))
	{
		set_template(true);
		domain = m[1].str();
		uid = m[2].str();
		id = m[3].str();
	}
	else
	{
		// http://rtmp.api.rt.com/hls/rtdru.m3u8
		set_template(false);
		uri_base::set_uri(url);
	}
}
