#include "StdAfx.h"
#include "uri_glanz.h"

static constexpr auto PLAYLIST_TEMPLATE = "http://pl.ottglanz.tv/get.php?username={:s}&password={:s}&type=m3u&output=hls";
static constexpr auto URI_TEMPLATE_HLS = "http://{SUBDOMAIN}/{ID}/video.m3u8?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}";
static constexpr auto URI_TEMPLATE_MPEG = "http://{SUBDOMAIN}/{ID}/mpegts?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}";
static constexpr auto URI_TEMPLATE_ARCH_HLS = "http://{SUBDOMAIN}/{ID}/video-{START}-10800.m3u8?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}";
static constexpr auto URI_TEMPLATE_ARCH_MPEG = "http://{SUBDOMAIN}/{ID}/archive-{START}-10800.ts?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}";
static constexpr auto EPG1_TEMPLATE = "http://epg.ott-play.com/php/show_prog.php?f=ottg/epg/{:s}.json";

void uri_glanz::parse_uri(const std::string& url)
{
	// http://str01.ottg.cc/9195/video.m3u8?username=sharky72&password=F8D58856LWX&token=f5afea07cef148278ae074acaf67a547&ch_id=70&req_host=pkSx3BL
	// http://str01.ottg.cc/9195/mpegts?username=sharky72&password=F8D58856LWX&token=f5afea07cef148278ae074acaf67a547&ch_id=70&req_host=pkSx3BL

	static std::regex re_url(R"(^https?:\/\/(.+)\/(.+)\/.+\?username=(.+)&password=(.+)&token=(.+)&ch_id=(\d+)&req_host=(.+)$)");
	std::smatch m;
	if (std::regex_match(url, m, re_url))
	{
		set_template(true);

		set_domain(m[1].str());
		set_id(m[2].str());
		set_login(m[3].str());
		set_password(m[4].str());
		set_token(m[5].str());
		set_int_id(m[6].str());
		set_host(m[7].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::string uri_glanz::get_templated(StreamSubType subType, const TemplateParams& params) const
{
	std::string url;

	if (!is_template())
	{
		url = get_uri();
		if (params.shift_back)
		{
			utils::string_replace_inplace(url, "{LOGIN}", params.login);

			url += fmt::format("&utc={:d}&lutc={:d}", params.shift_back, _time32(nullptr));
		}
	}
	else
	{
		switch (subType)
		{
			case StreamSubType::enHLS: // hls
				url = params.shift_back ? URI_TEMPLATE_ARCH_HLS : URI_TEMPLATE_HLS;
				break;
			case StreamSubType::enMPEGTS: // mpeg-ts
				url = params.shift_back ? URI_TEMPLATE_ARCH_MPEG : URI_TEMPLATE_MPEG;
				break;
		}

		// http://{SUBDOMAIN}/{ID}/video.m3u8?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}
		// http://{SUBDOMAIN}/{ID}/mpegts?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}
		utils::string_replace_inplace(url, "{SUBDOMAIN}", params.domain);
		utils::string_replace_inplace(url, "{ID}", get_id());
		utils::string_replace_inplace(url, "{LOGIN}", params.login);
		utils::string_replace_inplace(url, "{PASSWORD}", params.password);
		utils::string_replace_inplace(url, "{TOKEN}", params.token);
		utils::string_replace_inplace(url, "{INT_ID}", get_int_id());
		utils::string_replace_inplace(url, "{HOST}", params.host);
		utils::string_replace_inplace(url, "{START}", utils::int_to_char(params.shift_back));
	}

	return url;
}

std::string uri_glanz::get_epg1_uri(const std::string& id) const
{
	return fmt::format(EPG1_TEMPLATE, id);
}

std::string uri_glanz::get_playlist_template(bool first /*= true*/) const
{
	return PLAYLIST_TEMPLATE;
}
