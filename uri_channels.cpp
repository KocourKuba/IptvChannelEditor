#include "StdAfx.h"
#include "uri_channels.h"
#include "utils.h"

void uri_channels::parse_uri(const std::string& url)
{
	// http://ts://{SUBDOMAIN}/iptv/{UID}/205/index.m3u8

	static std::regex re(R"(http?:\/\/ts:\/\/\{SUBDOMAIN\}\/iptv\/\{UID\}\/(\d+)\/index.m3u8)");
	std::smatch m;
	if (std::regex_match(url, m, re))
	{
		set_template(true);
		id = m[1].str();
	}
	else
	{
		// http://ts://rtmp.api.rt.com/hls/rtdru.m3u8
		set_template(false);
		uri_base::set_uri(url);
	}
}
