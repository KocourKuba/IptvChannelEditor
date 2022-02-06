#include "pch.h"
#include "uri_sharatv.h"
#include "UtilsLib\utils.h"
#include "PlayListEntry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto PLAYLIST_TEMPLATE = L"http://tvfor.pro/g/{:s}:{:s}/1/playlist.m3u";
static constexpr auto URI_TEMPLATE = L"http://{SUBDOMAIN}/{ID}/{TOKEN}";
static constexpr auto EPG1_TEMPLATE = L"http://epg.ott-play.com/php/show_prog.php?f=shara-tv/epg/{:s}.json";
static constexpr auto EPG1_TEMPLATE_JSON = L"http://epg.ott-play.com/shara-tv/epg/{:s}.json";

void uri_sharatv::parse_uri(const std::wstring& url)
{
	// http://messi.tvfor.pro/Perviykanal/a8dv285y29itx4ye4oj3cez5

	static std::wregex re_url(LR"(^https?:\/\/(.+)\/(.+)\/(.+)$)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url))
	{
		set_template(true);
		set_domain(m[1].str());
		set_id(m[2].str());
		set_token(m[3].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_sharatv::get_templated(StreamSubType subType, const TemplateParams& params) const
{
	std::wstring url = is_template() ? URI_TEMPLATE : get_uri();

	if (params.shift_back)
	{
		url += L"?utc={START}";
	}

	ReplaceVars(url, params);

	return url;
}

std::wstring uri_sharatv::get_epg1_uri(const std::wstring& id) const
{
	return fmt::format(EPG1_TEMPLATE, id);
}

std::wstring uri_sharatv::get_epg1_uri_json(const std::wstring& id) const
{
	return fmt::format(EPG1_TEMPLATE_JSON, id);
}

std::wstring uri_sharatv::get_playlist_template(bool first /*= true*/) const
{
	return PLAYLIST_TEMPLATE;
}
