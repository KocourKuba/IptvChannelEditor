#include "StdAfx.h"
#include "uri_channels.h"
#include "utils.h"

void uri_channels::parse_uri(const std::wstring& url)
{
	// http://ts://{SUBDOMAIN}/iptv/{TOKEN}/205/index.m3u8

	static std::wregex re_url(LR"(https?:\/\/ts:\/\/\{SUBDOMAIN\}\/iptv\/\{UID\}\/(\d+)\/index.m3u8)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url))
	{
		set_template(true);
		id = m[1].str();
		return;
	}

	uri_stream::parse_uri(url);
}
