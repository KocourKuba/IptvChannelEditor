#include "StdAfx.h"
#include "uri_antifriz.h"
#include "utils.h"

static constexpr auto PLAYLIST_TEMPLATE = "https://antifriz.tv/playlist/{:s}.m3u8";
static constexpr auto URI_TEMPLATE_HLS = "http://{SUBDOMAIN}/s/{TOKEN}/{ID}/video.m3u8";
static constexpr auto URI_TEMPLATE_MPEG = "http://{SUBDOMAIN}/{ID}/mpegts?token={TOKEN}";
static constexpr auto URI_TEMPLATE_ARCH_HLS = "http://{SUBDOMAIN}/{ID}/archive-{START}-10800.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_ARCH_MPEG = "http://{SUBDOMAIN}/{ID}/archive-{START}-10800.ts?token={TOKEN}";
static constexpr auto EPG1_TEMPLATE = "http://epg.ott-play.com/antifriz/epg/{:s}.json";

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

std::string uri_antifriz::get_templated(StreamSubType subType, const TemplateParams& params) const
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

std::string uri_antifriz::get_epg1_uri(const std::string& id) const
{
	return fmt::format(EPG1_TEMPLATE, id);
}

std::string uri_antifriz::get_playlist_template(bool first /*= true*/) const
{
	return PLAYLIST_TEMPLATE;
}
