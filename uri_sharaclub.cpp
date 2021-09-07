#include "StdAfx.h"
#include "uri_sharaclub.h"

void uri_sharaclub::parse_uri(const std::string& url)
{
	// http://em.gazoni1.com:80/live/s.277258.1d25esee4e77f0419432d2ed8eb0ee525/pervyHD/video.m3u8

	static std::regex re_url(R"(^https?:\/\/(.+)\/live\/(.+)\/(.+)\/.+\.m3u8$)");
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
