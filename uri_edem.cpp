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
static constexpr auto EPG1_TEMPLATE = L"http://epg.ott-play.com/php/show_prog.php?f=edem/epg/{:s}.json";
static constexpr auto EPG1_TEMPLATE_JSON = L"http://epg.ott-play.com/edem/epg/{:s}.json";

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

std::wstring uri_edem::get_templated(StreamSubType subType, const TemplateParams& params) const
{
	std::wstring url = is_template() ? URI_TEMPLATE : get_uri();

	if (params.shift_back)
	{
		AppendArchive(url);
	}

	ReplaceVars(url, params);

	return url;
}

std::wstring uri_edem::get_epg1_uri(const std::wstring& id) const
{
	return fmt::format(EPG1_TEMPLATE, id);
}

std::wstring uri_edem::get_epg1_uri_json(const std::wstring& id) const
{
	return fmt::format(EPG1_TEMPLATE_JSON, id);
}

std::wstring uri_edem::get_playlist_template(bool first /*= true*/) const
{
	return first ? PLAYLIST_TEMPLATE1 : PLAYLIST_TEMPLATE2;
}
