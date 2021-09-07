#include "StdAfx.h"
#include "uri_antifriz.h"
#include "utils.h"

void uri_antifriz::parse_uri(const std::string& url)
{
	// http://tchaikovsky.antifriz.tv:1600/s/ibfpt72t/demo-4k/video.m3u8
	// http://tchaikovsky.antifriz.tv:80/demo-4k/mpegts?token=ibfpt72t

	static std::regex re_url_hls(R"(^https?:\/\/(.+)\/s\/(.+)\/(.+)\/video\.m3u8$)");
	static std::regex re_url_mpeg(R"(^https?:\/\/(.+)\/(.+)\/mpegts\?token=(.+)$)");
	std::smatch m;
	if (std::regex_match(url, m, re_url_hls))
	{
		set_template(true);
		domain = m[1].str();
		token = m[2].str();
		id = m[3].str();
		return;
	}

	if (std::regex_match(url, m, re_url_mpeg))
	{
		set_template(true);
		domain = m[1].str();
		id = m[2].str();
		token = m[3].str();
		return;
	}

	uri_stream::parse_uri(url);
}
