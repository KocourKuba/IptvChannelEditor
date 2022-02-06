#include "pch.h"
#include "uri_ottclub.h"

#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto PLAYLIST_TEMPLATE = L"http://myott.top/playlist/{:s}/m3u";
static constexpr auto URI_TEMPLATE_HLS = L"http://{SUBDOMAIN}/stream/{TOKEN}/{ID}.m3u8";
static constexpr auto EPG1_TEMPLATE = L"http://myott.top/api/channel/{:s}";
static constexpr auto EPG1_TEMPLATE_JSON = L"http://myott.top/api/channel/{:s}";

void uri_ottclub::parse_uri(const std::wstring& url)
{
	// http://myott.top/stream/S7NTAAORW5/131.m3u8

	static std::wregex re_url_hls(LR"(^https?:\/\/(.+)\/stream\/(.+)\/(.+)\.m3u8$)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url_hls))
	{
		set_template(true);
		set_domain(m[1].str());
		set_token(m[2].str());
		set_id(m[3].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_ottclub::get_templated(StreamSubType subType, const TemplateParams& params) const
{
	std::wstring url;

	if (is_template())
	{
		url = URI_TEMPLATE_HLS;
	}
	else
	{
		url = get_uri();
	}

	if (params.shift_back)
	{
		AppendArchive(url);
	}

	ReplaceVars(url, params);

	return url;
}

std::wstring uri_ottclub::get_epg1_uri(const std::wstring& id) const
{
	return fmt::format(EPG1_TEMPLATE, id);
}

std::wstring uri_ottclub::get_epg1_uri_json(const std::wstring& id) const
{
	return fmt::format(EPG1_TEMPLATE_JSON, id);
}

std::wstring uri_ottclub::get_playlist_template(bool first /*= true*/) const
{
	return PLAYLIST_TEMPLATE;
}
