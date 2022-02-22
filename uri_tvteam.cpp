#include "pch.h"
#include "uri_tvteam.h"
#include "UtilsLib\utils.h"
#include "PlayListEntry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto PLAYLIST_TEMPLATE = L"http://tv.team/pl/11/{:s}/playlist.m3u8";
static constexpr auto URI_TEMPLATE_HLS = L"http://{SUBDOMAIN}/{ID}/mono.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{SUBDOMAIN}/{ID}/mpegts?token={TOKEN}";
static constexpr auto URI_TEMPLATE_ARCH_HLS = L"http://{SUBDOMAIN}/{ID}/index-{START}-7200.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_ARCH_MPEG = L"http://{SUBDOMAIN}/{ID}/archive-{START}-7200.ts?token={TOKEN}";
static constexpr auto EPG1_TEMPLATE_JSON = L"http://tv.team/{:s}.json";
static constexpr auto EPG2_TEMPLATE_JSON = L"http://epg.ott-play.com/tvteam/epg/{:s}.json";

void uri_tvteam::parse_uri(const std::wstring& url)
{
	// http://3.troya.tv:34000/ch001/mono.m3u8?token=den8.lKT8tP92crF9WyqMVzXu61GkG3UQW0KmBDEitpUhz13ckoHfjmv22gjF

	static std::wregex re_url_hls(LR"(^https?:\/\/(.+)\/(.+)\/mono\.m3u8\?token=(.+)$)");

	std::wsmatch m;
	if (std::regex_match(url, m, re_url_hls))
	{
		templated = true;
		domain = std::move(m[1].str());
		id = std::move(m[2].str());
		token = std::move(m[3].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_tvteam::get_templated_stream(StreamSubType subType, const TemplateParams& params) const
{
	std::wstring url;

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
			default:
				break;
		}
	}
	else
	{
		url = get_uri();
		if (params.shift_back)
		{
			append_archive(url);
		}
	}

	replace_vars(url, params);

	return url;
}

std::wstring uri_tvteam::get_epg_uri_json(bool first, const std::wstring& id) const
{
	return fmt::format(first ? EPG1_TEMPLATE_JSON : EPG2_TEMPLATE_JSON, id);
}

std::wstring uri_tvteam::get_playlist_template(const PlaylistTemplateParams& params) const
{
	return fmt::format(PLAYLIST_TEMPLATE, params.password);
}
