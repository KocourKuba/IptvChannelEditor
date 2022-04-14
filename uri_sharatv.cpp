#include "pch.h"
#include "uri_sharatv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto PLAYLIST_TEMPLATE = L"http://tvfor.pro/g/{:s}:{:s}/1/playlist.m3u";
static constexpr auto URI_TEMPLATE = L"http://{SUBDOMAIN}/{ID}/{TOKEN}";

void uri_sharatv::parse_uri(const std::wstring& url)
{
	// http://messi.tvfor.pro/Perviykanal/a8dv285y29itx4ye4oj3cez5

	static std::wregex re_url(LR"(^https?:\/\/(.+)\/(.+)\/(.+)$)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url))
	{
		templated = true;
		domain = std::move(m[1].str());
		id = std::move(m[2].str());
		token = std::move(m[3].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_sharatv::get_templated_stream(StreamSubType subType, const TemplateParams& params) const
{
	std::wstring url = is_template() ? URI_TEMPLATE : get_uri();

	if (params.shift_back)
	{
		url += L"?utc={START}";
	}

	replace_vars(url, params);

	return url;
}

std::wstring uri_sharatv::get_playlist_template(const PlaylistTemplateParams& params) const
{
	return fmt::format(PLAYLIST_TEMPLATE, params.login, params.password);
}
