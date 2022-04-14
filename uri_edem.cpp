#include "pch.h"
#include "uri_edem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto PLAYLIST_TEMPLATE1 = L"http://epg.it999.ru/edem_epg_ico.m3u8";
static constexpr auto PLAYLIST_TEMPLATE2 = L"http://epg.it999.ru/edem_epg_ico2.m3u8";
static constexpr auto URI_TEMPLATE = L"http://{SUBDOMAIN}/iptv/{TOKEN}/{ID}/index.m3u8";

void uri_edem::parse_uri(const std::wstring& url)
{
	// http://localhost/iptv/00000000000000/204/index.m3u8

	static std::wregex re_url(LR"(^https?:\/\/(.+)\/iptv\/(.+)\/(\d+)\/.*\.m3u8$)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url))
	{
		templated = true;
		domain = std::move(m[1].str());
		token = std::move(m[2].str());
		id = std::move(m[3].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_edem::get_templated_stream(StreamSubType subType, const TemplateParams& params) const
{
	std::wstring url = is_template() ? URI_TEMPLATE : get_uri();

	if (params.shift_back)
	{
		append_archive(url);
	}

	replace_vars(url, params);

	return url;
}

std::wstring uri_edem::get_playlist_template(const PlaylistTemplateParams& params) const
{
	return params.number == 0 ? PLAYLIST_TEMPLATE1 : PLAYLIST_TEMPLATE2;
}
