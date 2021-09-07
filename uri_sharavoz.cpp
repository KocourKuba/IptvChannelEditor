#include "StdAfx.h"
#include "uri_sharavoz.h"
#include "utils.h"

static constexpr auto URI_TEMPLATE_SHARAVOZ_HLS = "http://{SUBDOMAIN}/{ID}/index.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_SHARAVOZ_MPEG = "http://{SUBDOMAIN}/{ID}/mpegts?token={TOKEN}";
static constexpr auto EPG1_TEMPLATE_SHARAVOZ = "http://api.program.spr24.net/api/program?epg={:s}&date={:4d}-{:02d}-{:02d}";
static constexpr auto EPG2_TEMPLATE_SHARAVOZ = "http://epg.arlekino.tv/api/program?epg={:s}&date={:4d}-{:02d}-{:02d}";

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

std::string uri_sharavoz::get_templated(StreamSubType subType, int shift_back) const
{
	std::string uri_template;
	switch (subType)
	{
		case StreamSubType::enHLS:
			uri_template = URI_TEMPLATE_SHARAVOZ_HLS;
			break;
		case StreamSubType::enMPEGTS:
			uri_template = URI_TEMPLATE_SHARAVOZ_MPEG;
			break;
	}

	if (shift_back)
	{
		uri_template += fmt::format("&utc={:d}&lutc={:d}", shift_back, _time32(nullptr));
	}

	return uri_template;
}

std::string uri_sharavoz::get_epg1_uri(const std::string& id) const
{
	return fmt::format(EPG1_TEMPLATE_SHARAVOZ, id);
}

std::string uri_sharavoz::get_epg2_uri(const std::string& id) const
{
	COleDateTime dt = COleDateTime::GetCurrentTime();
	return fmt::format(EPG2_TEMPLATE_SHARAVOZ, id, dt.GetYear(), dt.GetMonth(), dt.GetDay());
}
