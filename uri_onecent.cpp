#include "StdAfx.h"
#include "uri_onecent.h"
#include "utils.h"
#include "PlayListEntry.h"

static constexpr auto PLAYLIST_TEMPLATE = L"http://only4.tv/pl/{:s}/102/only4tv.m3u8";
static constexpr auto URI_TEMPLATE_HLS = L"http://{SUBDOMAIN}/{ID}/index.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{SUBDOMAIN}/{ID}/mpegts?token={TOKEN}";
static constexpr auto URI_TEMPLATE_ARCH_HLS = L"http://{SUBDOMAIN}/{ID}/index-{START}-10800.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_ARCH_MPEG = L"http://{SUBDOMAIN}/{ID}/archive-{START}-10800.ts?token={TOKEN}";
static constexpr auto EPG1_TEMPLATE = L"http://epg.ott-play.com/php/show_prog.php?f=only4/epg/{:s}.json";
static constexpr auto EPG1_TEMPLATE_JSON = L"http://epg.ott-play.com/only4/epg/{:s}.json";

void uri_onecent::parse_uri(const std::wstring& url)
{
	// http://cdn.only4.tv/20115/index.m3u8?token=MH1LeVsHSD

	static std::wregex re_url_hls(LR"(^https?:\/\/(.+)\/(.+)\/index\.m3u8\?token=(.+)$)");

	std::wsmatch m;
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

std::wstring uri_onecent::get_templated(StreamSubType subType, const TemplateParams& params) const
{
	std::wstring url = get_uri();

	if (is_template())
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
	}
	else
	{
		if (params.shift_back)
		{
			url += L"&utc={START}&lutc={NOW}";
		}
	}

	ReplaceVars(url, params);
	return url;
}

std::wstring uri_onecent::get_epg1_uri(const std::wstring& id) const
{
	return fmt::format(EPG1_TEMPLATE, id);
}

std::wstring uri_onecent::get_epg1_uri_json(const std::wstring& id) const
{
	return fmt::format(EPG1_TEMPLATE_JSON, id);
}

std::wstring uri_onecent::get_playlist_template(bool first /*= true*/) const
{
	return PLAYLIST_TEMPLATE;
}
