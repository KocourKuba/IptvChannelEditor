#include "StdAfx.h"
#include "uri_oneott.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// http://{DOMAIN}/PinApi/{LOGIN}/{PASSWORD}
static constexpr auto ACCOUNT_TEMPLATE = L"http://list.1ott.net/PinApi/{:s}/{:s}";
static constexpr auto PLAYLIST_TEMPLATE = L"http://list.1ott.net/api/{:s}/high/ottplay.m3u8";
static constexpr auto URI_TEMPLATE_HLS = L"http://{SUBDOMAIN}/~{TOKEN}/{ID}/hls/pl.m3u8";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{SUBDOMAIN}/~{TOKEN}/{ID}";
static constexpr auto EPG1_TEMPLATE = L"http://epg.propg.net/{:s}/epg2/{:4d}-{:02d}-{:02d}";
static constexpr auto EPG2_TEMPLATE = L"http://epg.ott-play.com/php/show_prog.php?f=1ott/epg/{:s}.json";
static constexpr auto EPG2_TEMPLATE_JSON = L"http://epg.ott-play.com/1ott/epg/{:s}.json";


void uri_oneott::parse_uri(const std::wstring& url)
{
	//http://rr2.1ott.net/~109dab8c798d546s8dc9c41b3c3af80d59a/35985/hls/pl.m3u8

	static std::wregex re_url(LR"(^https?:\/\/(.+)\/~(.+)\/(.+)\/hls\/.+\.m3u8$)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url))
	{
		set_template(true);
		set_domain(m[1].str());
		set_token(m[2].str());
		set_id(m[3].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_oneott::get_templated(StreamSubType subType, const TemplateParams& params) const
{
	auto& url = get_uri();

	if (is_template())
	{
		switch (subType)
		{
			case StreamSubType::enHLS:
				url = URI_TEMPLATE_HLS;
				break;
			case StreamSubType::enMPEGTS:
				url = URI_TEMPLATE_MPEG;
				break;
		}
	}

	if (params.shift_back)
	{
		url += L"?utc={START}&lutc={NOW}";
	}
	ReplaceVars(url, params);

	return url;
}

std::wstring uri_oneott::get_epg1_uri(const std::wstring& id) const
{
	COleDateTime dt = COleDateTime::GetCurrentTime();
	return fmt::format(EPG1_TEMPLATE, id, dt.GetYear(), dt.GetMonth(), dt.GetDay());
}

std::wstring uri_oneott::get_epg2_uri(const std::wstring& id) const
{
	return fmt::format(EPG2_TEMPLATE, id);
}

std::wstring uri_oneott::get_epg1_uri_json(const std::wstring& id) const
{
	return get_epg1_uri(id);
}

std::wstring uri_oneott::get_epg2_uri_json(const std::wstring& id) const
{
	COleDateTime dt = COleDateTime::GetCurrentTime();
	return fmt::format(EPG2_TEMPLATE_JSON, id, dt.GetYear(), dt.GetMonth(), dt.GetDay());
}

std::wstring uri_oneott::get_access_url(const std::wstring& login, const std::wstring& password) const
{
	return fmt::format(ACCOUNT_TEMPLATE, login, password);
}

std::wstring uri_oneott::get_playlist_template(bool first /*= true*/) const
{
	return PLAYLIST_TEMPLATE;
}