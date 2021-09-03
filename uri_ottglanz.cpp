#include "StdAfx.h"
#include "uri_ottglanz.h"

void uri_ottglanz::parse_uri(const std::string& url)
{
	// http://str01.ottg.cc/9195/video.m3u8?username=sharky72&password=F8D58856LWX&token=f5afea07cef148278ae074acaf67a547&ch_id=70&req_host=pkjX3BL
	// http://str01.ottg.cc/9195/mpegts?username=sharky72&password=F8D58856LWX&token=f5afea07cef148278ae074acaf67a547&ch_id=70&req_host=pkjX3BL

	static std::regex re_url(R"(^https?:\/\/(.+)\/(\d+)\/(?:mpegts|.+\.m3u8)\?username=(.+)&password=(.+)&token=(.+)&ch_id=(\d+)&req_host=(.+)$)");
	std::smatch m;
	if (std::regex_match(url, m, re_url))
	{
		set_template(true);

		set_domain(m[1].str());
		set_id(m[2].str());
		set_login(m[3].str());
		set_password(m[4].str());
		set_token(m[5].str());
		set_int_id(m[6].str());
		set_host(m[7].str());
		return;
	}

	uri_stream::parse_uri(url);
}
