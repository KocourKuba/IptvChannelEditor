#include "StdAfx.h"
#include "uri_sharavoz.h"
#include "utils.h"

static constexpr auto PLAYLIST_TEMPLATE = "http://sharavoz.tk/iptv/p/{:s}/Sharavoz.Tv.navigator-ott.m3u";
static constexpr auto URI_TEMPLATE_HLS = "http://{SUBDOMAIN}/{ID}/index.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_MPEG = "http://{SUBDOMAIN}/{ID}/mpegts?token={TOKEN}";
static constexpr auto EPG1_TEMPLATE = "http://api.program.spr24.net/api/program?epg={:s}&date={:4d}-{:02d}-{:02d}";
static constexpr auto EPG2_TEMPLATE = "http://epg.arlekino.tv/api/program?epg={:s}&date={:4d}-{:02d}-{:02d}";

void uri_sharavoz::parse_uri(const std::string& url)
{
	// http://domain.com/204/index.m3u8?token=adsdaSDFJKHKJd
	// http://domain.com/204/mpegts?token=adsdaSDFJKHKJd

	static std::regex re_url(R"(^https?:\/\/(.+)\/(\d+)\/(?:mpegts|index\.m3u8)\?token=(.+)$)");
	std::smatch m;
	if (std::regex_match(url, m, re_url))
	{
		set_template(true);
		domain = m[1].str();
		id = m[2].str();
		token = m[3].str();
		return;
	}

	uri_stream::parse_uri(url);
}

std::string uri_sharavoz::get_templated(StreamSubType subType, const TemplateParams& params) const
{
	std::string url;

	if (!is_template())
	{
		url = get_uri();
	}
	else
	{
		switch (subType)
		{
			case StreamSubType::enHLS:
				url = URI_TEMPLATE_HLS;
				break;
			case StreamSubType::enMPEGTS:
				url = URI_TEMPLATE_MPEG;
				break;
		}

		// http://{SUBDOMAIN}/{ID}/index.m3u8?token={TOKEN}
		// http://{SUBDOMAIN}/{ID}/mpegts?token={TOKEN}
		utils::string_replace_inplace(url, "{SUBDOMAIN}", params.domain);
		utils::string_replace_inplace(url, "{ID}", get_id());
		utils::string_replace_inplace(url, "{TOKEN}", params.token);
	}

	if (params.shift_back)
	{
		url += fmt::format("&utc={:d}&lutc={:d}", params.shift_back, _time32(nullptr));
	}

	return url;
}

std::string uri_sharavoz::get_epg1_uri(const std::string& id) const
{
	COleDateTime dt = COleDateTime::GetCurrentTime();
	return fmt::format(EPG1_TEMPLATE, id, dt.GetYear(), dt.GetMonth(), dt.GetDay());
}

std::string uri_sharavoz::get_epg2_uri(const std::string& id) const
{
	COleDateTime dt = COleDateTime::GetCurrentTime();
	return fmt::format(EPG2_TEMPLATE, id, dt.GetYear(), dt.GetMonth(), dt.GetDay());
}

std::string uri_sharavoz::get_playlist_template(bool first /*= true*/) const
{
	return PLAYLIST_TEMPLATE;
}
