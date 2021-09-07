#include "StdAfx.h"
#include "uri_antifriz.h"
#include "utils.h"

static constexpr auto PLAYLIST_TEMPLATE_ANTIFRIZ = "https://antifriz.tv/playlist/{:s}.m3u8";
static constexpr auto URI_TEMPLATE_ANTIFRIZ_HLS = "http://{SUBDOMAIN}/s/{TOKEN}/{ID}/video.m3u8";
static constexpr auto URI_TEMPLATE_ANTIFRIZ_MPEG = "http://{SUBDOMAIN}/{ID}/mpegts?token={TOKEN}";
static constexpr auto EPG1_TEMPLATE_ANTIFRIZ = "http://epg.ott-play.com/antifriz/epg/%s.json";
static constexpr auto EPG2_TEMPLATE_ANTIFRIZ = "http://epg.ott-play.com/antifriz/epg/%s.json";

void uri_antifriz::parse_uri(const std::string& url)
{
	// http://tchaikovsky.antifriz.tv:1600/s/ibzsdpt2t/demo-4k/video.m3u8
	// http://tchaikovsky.antifriz.tv:80/demo-4k/mpegts?token=ibzsdpt2t

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

std::string uri_antifriz::get_templated(StreamSubType subType, int shift_back) const
{
	std::string uri_template;
	switch (subType)
	{
		case StreamSubType::enHLS:
			uri_template = URI_TEMPLATE_ANTIFRIZ_HLS;
			break;
		case StreamSubType::enMPEGTS:
			uri_template = URI_TEMPLATE_ANTIFRIZ_MPEG;
			break;
	}

	if (shift_back)
	{
		uri_template += fmt::format("&utc={:d}&lutc={:d}", shift_back, _time32(nullptr));
	}

	return uri_template;
}

std::string uri_antifriz::get_epg1_uri(const std::string& id) const
{
	return fmt::format(EPG1_TEMPLATE_ANTIFRIZ, id);
}

std::string uri_antifriz::get_epg2_uri(const std::string& id) const
{
	return fmt::format(EPG2_TEMPLATE_ANTIFRIZ, id);
}

std::string uri_antifriz::get_playlist_url(const std::string& /*login*/, const std::string& password) const
{
	return fmt::format(PLAYLIST_TEMPLATE_ANTIFRIZ, password.c_str());
}
