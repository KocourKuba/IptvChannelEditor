#include "StdAfx.h"
#include "uri_fox.h"
#include "utils.h"

static constexpr auto PLAYLIST_TEMPLATE = "https://antifriz.tv/playlist/{:s}.m3u8";
static constexpr auto URI_TEMPLATE_HLS = "http://{SUBDOMAIN}/{TOKEN}/video.m3u8";
static constexpr auto URI_TEMPLATE_MPEG = "http://{SUBDOMAIN}/{TOKEN}";
static constexpr auto URI_TEMPLATE_ARCH_HLS = "http://{SUBDOMAIN}/{ID}/archive-{START}-10800.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_ARCH_MPEG = "http://{SUBDOMAIN}/{ID}/archive-{START}-10800.ts?token={TOKEN}";
static constexpr auto EPG1_TEMPLATE = "http://epg.ott-play.com/php/show_prog.php?f=antifriz/epg/{:s}.json";

void uri_fox::parse_uri(const std::string& url)
{
	// http://ost.fox-tv.fun/vLm0zdTg_dG9rZW49W3N0Yl90b2tlbl0iLCJhIjoiaHR0cDovL3N0cjIuZm94LXR2LmZ1bjo5OTg1L1BlcnZpeWthbmFsL3ZpZGVvLXRpbWVzaGlmdF9hYnMtW3RpbWVfc3RhcnRdLm0zdTg_dG9rZW49W3N0Yl90b2tlbl1bY3Vycl90aW1lXSIsImwiOiI2NTgxMWQwZCIsInAiOiI2NTgxMWQwZDNjMTRjMTFlIiwiYyI6IjEiLCJ0IjoiN2FiMDJjOTk4MmY4NjI4NGU1ODhkYTliZjc0YmU4YTgiLCJkIjoiMjk2NjciLCJyIjoiMTI5NjY4In0eyJ1IjoiaHR0cDovL3N0cjIuZm94LXR2LmZ1bjo5OTg2L1BlcnZpeWthbmFsL3ZpZGV/video.m3u8
	// http://ost.fox-tv.fun/0cz90b2tlbj1bc3RiX3Rva2VuXSIsImEiOiJodHRwOi8vc3RyMi5mb3gtdHYuZnVuOjk5ODUvUGVydml5a2FuYWwvdGltZXNoaWZ0X2Ficy1bdGltZV9zdGFydF0udHM_dG9rZW49W3N0Yl90b2tlbl0iLCJsIjoiNjU4MTFkMGQiLCJwIjoiNjU4MTFkMGQzYzE0YzExZSIsImMiOiIxIiwidCI6IjdhYjAyYzk5ODJmODYyODRlNTg4ZGE5YmY3NGJlOGE4IiwiZCI6IjI5NjY3IiwiciI6IjEyOTY2OCJ9eyJ1IjoiaHR0cDovL3N0cjIuZm94LXR2LmZ1bjo5OTg2L1BlcnZpeWthbmFsL21wZWd

	static std::regex  re_url_hls(R"(^https?:\/\/(.+)\/(.+)\/video\.m3u8$)");
	static std::regex re_url_mpeg(R"(^https?:\/\/(.+)\/(.+)$)");
	std::smatch m;
	if (std::regex_match(url, m, re_url_hls))
	{
		set_template(true);
		domain = m[1].str();
		token = m[2].str();
		return;
	}

	if (std::regex_match(url, m, re_url_mpeg))
	{
		set_template(true);
		domain = m[1].str();
		token = m[2].str();
		return;
	}

	uri_stream::parse_uri(url);
}

std::string uri_fox::get_templated(StreamSubType subType, const TemplateParams& params) const
{
	std::string url;

	if (!is_template())
	{
		url = get_uri();
		if (params.shift_back)
		{
			url += fmt::format("&utc={:d}&lutc={:d}", params.shift_back, _time32(nullptr));
		}
	}
	else
	{
		std::string no_port(params.domain);
		if (auto pos = no_port.find(':'); pos != std::string::npos)
		{
			no_port = no_port.substr(0, pos);
		}

		std::string subdomain;
		switch (subType)
		{
			case StreamSubType::enHLS:
				url = params.shift_back ? URI_TEMPLATE_ARCH_HLS : URI_TEMPLATE_HLS;
				subdomain = params.shift_back ? no_port : params.domain;
				break;
			case StreamSubType::enMPEGTS:
				url = params.shift_back ? URI_TEMPLATE_ARCH_MPEG : URI_TEMPLATE_MPEG;
				subdomain = no_port;
				break;
		}

		// http://{SUBDOMAIN}/s/{TOKEN}/{ID}/video.m3u8
		// http://{SUBDOMAIN}/{ID}/mpegts?token={TOKEN}
		// http://{SUBDOMAIN}/{ID}/archive-{START}-10800.m3u8?token={TOKEN}
		// http://{SUBDOMAIN}/{ID}/archive-{START}-10800.ts?token={TOKEN}
		utils::string_replace_inplace(url, "{SUBDOMAIN}", subdomain);
		utils::string_replace_inplace(url, "{TOKEN}", params.token);
		utils::string_replace_inplace(url, "{ID}", get_id());
		utils::string_replace_inplace(url, "{START}", utils::int_to_char(params.shift_back));
	}

	return url;
}

std::string uri_fox::get_epg1_uri(const std::string& id) const
{
	return fmt::format(EPG1_TEMPLATE, id);
}

std::string uri_fox::get_playlist_template(bool first /*= true*/) const
{
	return PLAYLIST_TEMPLATE;
}
