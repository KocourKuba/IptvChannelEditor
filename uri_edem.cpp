#include "StdAfx.h"
#include "uri_edem.h"

static constexpr auto URI_TEMPLATE_EDEM = "http://{SUBDOMAIN}/iptv/{TOKEN}/{ID}/index.m3u8";

static constexpr auto EPG1_TEMPLATE_EDEM = "http://epg.ott-play.com/edem/epg/{:s}.json";
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

std::string uri_edem::get_templated(StreamSubType /*subType*/, const TemplateParams& params) const
{
	std::string url;

	if (!is_template())
	{
		url = get_uri();
	}
	else
	{
		std::string uri_template;

		// http://{SUBDOMAIN}/iptv/{TOKEN}/{ID}/index.m3u8
		url = fmt::format(URI_TEMPLATE_EDEM,
						  fmt::arg("SUBDOMAIN", params.domain),
						  fmt::arg("TOKEN", params.token),
						  fmt::arg("ID", get_id())
		);

	}

	if (params.shift_back)
	{
		url += fmt::format("&utc={:d}&lutc={:d}", params.shift_back, _time32(nullptr));
	}

	return url;
}

std::string uri_edem::get_epg1_uri(const std::string& id) const
{
	return fmt::format(EPG1_TEMPLATE_EDEM, id);
}

std::string uri_edem::get_epg2_uri(const std::string& id) const
{
	return fmt::format(EPG2_TEMPLATE_EDEM, id);
}
