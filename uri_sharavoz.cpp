#include "StdAfx.h"
#include "uri_sharavoz.h"
#include "utils.h"

std::string uri_sharavoz::get_id_translated_url() const
{
	// templated url changed, custom is unchanged
	// http://{SUBDOMAIN}/{ID}/index.m3u8?token={UID} -> http://{SUBDOMAIN}/204/index.m3u8?token={UID}
	if (is_template())
	{
		return utils::string_replace(URI_TEMPLATE, "{ID}", get_id());
	}

	return get_uri();
}

std::string uri_sharavoz::get_playable_url(const std::string& access_domain, const std::string& access_key) const
{
	std::string stream_url = get_id_translated_url();

	// http://{SUBDOMAIN}/204/index.m3u8?token={UID} -> http://domain.com/204/index.m3u8?token=adsdaSDFJKHKJd

	utils::string_replace_inplace(stream_url, "{SUBDOMAIN}", access_domain);
	utils::string_replace_inplace(stream_url, "{UID}", access_key);

	return stream_url;
}

void uri_sharavoz::parse_uri(const std::string& url)
{
	// http://domain.com/204/index.m3u8?token=adsdaSDFJKHKJd
	// http://domain.com/204/mpegts?token=adsdaSDFJKHKJd

	static std::regex re_url(R"(http?:\/\/([0-9a-z\.]+)\/(\d+)\/(?:mpegts|index.m3u8)\?token=([0-9A-Za-z]+)");
	std::smatch m;
	if (std::regex_match(url, m, re_url))
	{
		set_template(true);
		domain = m[1].str();
		uid = m[3].str();
		id = m[2].str();
	}
	else
	{
		// http://rtmp.api.rt.com/hls/rtdru.m3u8
		set_template(false);
		uri_base::set_uri(url);
	}
}
