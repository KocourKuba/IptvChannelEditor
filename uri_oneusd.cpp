#include "StdAfx.h"
#include "uri_oneusd.h"
#include "utils.h"
#include "PlayListEntry.h"

static constexpr auto PLAYLIST_TEMPLATE = "http://1usd.tv/pl-{:s}-hls";
static constexpr auto URI_TEMPLATE_HLS = "http://{SUBDOMAIN}/{ID}/mono.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_MPEG = "http://{SUBDOMAIN}/{ID}/mpegts?token={TOKEN}";
static constexpr auto URI_TEMPLATE_ARCH_HLS = "http://{SUBDOMAIN}/{ID}/index-{START}-7200.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_ARCH_MPEG = "http://{SUBDOMAIN}/{ID}/index-{START}-7200.ts?token={TOKEN}";
static constexpr auto EPG1_TEMPLATE = "http://epg.ott-play.com/php/show_prog.php?f=1usd/epg/{:s}.json";

void uri_oneusd::parse_uri(const std::string& url)
{
	// http://1.1usd.tv:34000/ch001/mono.m3u8?token=1usdtv_robert21224.VdOQFJwYXrBNhWugsGPRdx-HBrzPKotuzKtiuHlD2EzeRAFw9-bIam9oFxeFb0Va
	// http://2.1usd.tv:34000/ch001/mono.m3u8?token=1usdtv_robert21224.VdOQFJwYXrBNhWugsGPRdx-HBrzPKotuzKtiuHlD2EzwI7un-s5g5Fb3RXYL5YSb
	// http://1.1usd.tv:34000/ch054/mono.m3u8?token=1usdtv_robert21224.VdOQFJwYXrBNhWugsGPRdx-HBrzPKotuzKtiuHlD2EzIYa-46CRxneMeOaVVgd87

	static std::regex re_url_hls(R"(^https?:\/\/(.+)\/(.+)\/mono\.m3u8\?token=(.+)$)");

	std::smatch m;
	if (std::regex_match(url, m, re_url_hls))
	{
		set_template(true);
		domain = m[1].str();
		id = m[2].str();
		token = m[3].str();
		return;
	}

	uri_stream::parse_uri(url);
}

std::string uri_oneusd::get_templated(StreamSubType subType, const TemplateParams& params) const
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
		switch (subType)
		{
			case StreamSubType::enHLS:
				url = params.shift_back ? URI_TEMPLATE_ARCH_HLS : URI_TEMPLATE_HLS;
				break;
			case StreamSubType::enMPEGTS:
				url = params.shift_back ? URI_TEMPLATE_ARCH_MPEG : URI_TEMPLATE_MPEG;
				break;
		}

		utils::string_replace_inplace(url, "{SUBDOMAIN}", params.domain);
		utils::string_replace_inplace(url, "{TOKEN}", get_token());
		utils::string_replace_inplace(url, "{ID}", get_id());
		utils::string_replace_inplace(url, "{START}", utils::int_to_char(params.shift_back));
	}

	return url;
}

std::string uri_oneusd::get_epg1_uri(const std::string& id) const
{
	return fmt::format(EPG1_TEMPLATE, id);
}

std::string uri_oneusd::get_playlist_template(bool first /*= true*/) const
{
	return PLAYLIST_TEMPLATE;
}
