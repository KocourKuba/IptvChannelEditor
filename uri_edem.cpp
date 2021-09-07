#include "StdAfx.h"
#include "uri_edem.h"

static constexpr auto URI_TEMPLATE_EDEM_HLS = "http://{SUBDOMAIN}/iptv/{TOKEN}/{ID}/index.m3u8";
static constexpr auto URI_TEMPLATE_EDEM_MPEG = "http://{SUBDOMAIN}/iptv/{TOKEN}/{ID}/index.m3u8";

static constexpr auto EPG1_TEMPLATE_EDEM = "http://epg.ott-play.com/edem/epg/%s.json";
static constexpr auto EPG2_TEMPLATE_EDEM = "http://www.teleguide.info/kanal{:d}_{:4d}{:02d}{:02d}.html";

void uri_edem::parse_uri(const std::string& url)
{
	// http://localhost/iptv/00000000000000/204/index.m3u8

	static std::regex re_url(R"(^https?:\/\/(.+)\/iptv\/(.+)\/(\d+)\/.*\.m3u8$)");
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

std::string uri_edem::get_templated(StreamSubType subType, int shift_back) const
{
	std::string uri_template;
	switch (subType)
	{
		case StreamSubType::enHLS:
			uri_template = URI_TEMPLATE_EDEM_HLS;
			break;
		case StreamSubType::enMPEGTS:
			uri_template = URI_TEMPLATE_EDEM_MPEG;
			break;
	}

	if (shift_back)
	{
		uri_template += fmt::format("&utc={:d}&lutc={:d}", shift_back, _time32(nullptr));
	}

	return uri_template;
}

std::string uri_edem::get_epg1_uri(const std::string& id) const
{
	return fmt::format(EPG1_TEMPLATE_EDEM, id);
}

std::string uri_edem::get_epg2_uri(const std::string& id) const
{
	return fmt::format(EPG2_TEMPLATE_EDEM, id);
}
