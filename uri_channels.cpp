#include "StdAfx.h"
#include "uri_channels.h"
#include "utils.h"

std::string uri_channels::get_id_translated_url() const
{
	// templated url changed, custom is unchanged
	// http://ts://{SUBDOMAIN}/iptv/{UID}/{ID}/index.m3u8 -> http://ts://{SUBDOMAIN}/iptv/{UID}/204/index.m3u8
	if (is_template())
	{
		return utils::string_replace(URI_TEMPLATE, "{ID}", get_id());
	}

	return get_uri();
}

std::string uri_channels::get_playable_url(const std::string& access_domain, const std::string& access_key) const
{
	// http://ts://{SUBDOMAIN}/iptv/{UID}/204/index.m3u8 -> http://{SUBDOMAIN}/iptv/{UID}/204/index.m3u8
	// http://ts://rtmp.api.rt.com/hls/rtdru.m3u8 -> http://rtmp.api.rt.com/hls/rtdru.m3u8

	std::string stream_url = get_id_translated_url();
	utils::string_replace_inplace(stream_url, "ts://", "");

	// http://ts://{SUBDOMAIN}/iptv/{UID}/205/index.m3u8 -> http://ts://domain.com/iptv/000000000000/205/index.m3u8

	utils::string_replace_inplace(stream_url, "{SUBDOMAIN}", access_domain);
	utils::string_replace_inplace(stream_url, "{UID}", access_key);

	return stream_url;
}

void uri_channels::parse_uri(const std::string& url)
{
	// http://ts://{SUBDOMAIN}/iptv/{UID}/205/index.m3u8

	static std::regex re(R"(http?:\/\/ts:\/\/\{SUBDOMAIN\}\/iptv\/\{UID\}\/(\d+)\/index.m3u8)");
	std::smatch m;
	if (std::regex_match(url, m, re))
	{
		set_template(true);
		id = m[1].str();
	}
	else
	{
		// http://ts://rtmp.api.rt.com/hls/rtdru.m3u8
		set_template(false);
		uri_base::set_uri(url);
	}
}
