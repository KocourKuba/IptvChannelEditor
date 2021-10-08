#include "StdAfx.h"
#include "uri_itv.h"
#include "utils.h"

static constexpr auto PLAYLIST_TEMPLATE = "https://itv.ooo/p/{:s}/hls.m3u8";
static constexpr auto URI_TEMPLATE_HLS = "http://{SUBDOMAIN}/{ID}/video.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_MPEG = "http://{SUBDOMAIN}/{ID}/mpegts?token={TOKEN}";
static constexpr auto URI_TEMPLATE_ARCH_HLS = "http://{SUBDOMAIN}/{ID}/archive-{START}-10800.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_ARCH_MPEG = "http://{SUBDOMAIN}/{ID}/archive-{START}-10800.ts?token={TOKEN}";
static constexpr auto EPG1_TEMPLATE = "http://api.itv.live/epg/{:s}";
static constexpr auto EPG1_TEMPLATE_JSON = EPG1_TEMPLATE;

void uri_itv::parse_uri(const std::string& url)
{
	// http://cloud15.05cdn.wf/ch378/video.m3u8?token=5bdbc7125f6ed805a5fd238b9f885d1a3c67a6594
	// http://cloud15.05cdn.wf/ch378/mpegts?token=5bdbc7125f6ed805a5fd238b9f885d1a3c67a6594
	static std::regex re_url_hls(R"(^https?:\/\/(.+)\/(.+)\/[^\?]+\?token=(.+)$)");
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

std::string uri_itv::get_templated(StreamSubType subType, const TemplateParams& params) const
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
		std::string subdomain;
		switch (subType)
		{
			case StreamSubType::enHLS:
				url = params.shift_back ? URI_TEMPLATE_ARCH_HLS : URI_TEMPLATE_HLS;
				break;
			case StreamSubType::enMPEGTS:
				url = params.shift_back ? URI_TEMPLATE_ARCH_MPEG : URI_TEMPLATE_MPEG;
				break;
		}

		// http://{SUBDOMAIN}/{ID}/video.m3u8?token={TOKEN}
		// http://{SUBDOMAIN}/{ID}/mpegts?token={TOKEN}
		// http://{SUBDOMAIN}/{ID}/archive-{START}-10800.m3u8?token={TOKEN}
		// http://{SUBDOMAIN}/{ID}/archive-{START}-10800.ts?token={TOKEN}
		utils::string_replace_inplace(url, "{SUBDOMAIN}", params.domain);
		utils::string_replace_inplace(url, "{TOKEN}", get_token());
		utils::string_replace_inplace(url, "{ID}", get_id());
		utils::string_replace_inplace(url, "{START}", utils::int_to_char(params.shift_back));
	}

	return url;
}

std::string uri_itv::get_epg1_uri(const std::string& id) const
{
	return fmt::format(EPG1_TEMPLATE, id);
}

std::string uri_itv::get_epg1_uri_json(const std::string& id) const
{
	return fmt::format(EPG1_TEMPLATE_JSON, id);
}

std::string uri_itv::get_playlist_template(bool first /*= true*/) const
{
	return PLAYLIST_TEMPLATE;
}
