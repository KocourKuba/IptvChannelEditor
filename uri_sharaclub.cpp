#include "StdAfx.h"
#include "uri_sharaclub.h"

static constexpr auto ACCOUNT_TEMPLATE = L"http://list.playtv.pro/api/dune-api5m.php?subscr={:s}-{:s}";
static constexpr auto PLAYLIST_TEMPLATE = L"http://list.playtv.pro/tv_live-m3u8/{:s}-{:s}";
static constexpr auto URI_TEMPLATE_HLS = L"http://{SUBDOMAIN}/live/{TOKEN}/{ID}/video.m3u8";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{SUBDOMAIN}/live/{TOKEN}/{ID}.ts";
static constexpr auto EPG1_TEMPLATE = L"http://api.sramtv.com/get/?type=epg&ch={:s}&date=&date={:4d}-{:02d}-{:02d}";
static constexpr auto EPG2_TEMPLATE = L"http://api.gazoni1.com/get/?type=epg&ch={:s}&date={:4d}-{:02d}-{:02d}";


void uri_sharaclub::parse_uri(const std::wstring& url)
{
	// http://em.gazoni1.com:80/live/s.277258.1d25esee4e77f0419432d2ed8eb0ee525/pervyHD/video.m3u8

	static std::wregex re_url(LR"(^https?:\/\/(.+)\/live\/(.+)\/(.+)\/.+\.m3u8$)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url))
	{
		set_template(true);
		domain = m[1].str();
		token = m[2].str();
		id = m[3].str();
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_sharaclub::get_templated(StreamSubType subType, const TemplateParams& params) const
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

	// http://{SUBDOMAIN}/live/{TOKEN}/{ID}/video.m3u8
	// http://{SUBDOMAIN}/live/{TOKEN}/{ID}.ts
	utils::string_replace_inplace<wchar_t>(url, REPL_TOKEN, params.token);
	ReplaceVars(url, params);

	return url;
}

std::wstring uri_sharaclub::get_epg1_uri(const std::wstring& id) const
{
	COleDateTime dt = COleDateTime::GetCurrentTime();
	return fmt::format(EPG1_TEMPLATE, id, dt.GetYear(), dt.GetMonth(), dt.GetDay());
}

std::wstring uri_sharaclub::get_epg2_uri(const std::wstring& id) const
{
	COleDateTime dt = COleDateTime::GetCurrentTime();
	return fmt::format(EPG2_TEMPLATE, id, dt.GetYear(), dt.GetMonth(), dt.GetDay());
}

std::wstring uri_sharaclub::get_epg1_uri_json(const std::wstring& id) const
{
	return get_epg1_uri(id);
}

std::wstring uri_sharaclub::get_epg2_uri_json(const std::wstring& id) const
{
	return get_epg2_uri(id);
}

std::wstring uri_sharaclub::get_access_url(const std::wstring& login, const std::wstring& password) const
{
	return fmt::format(ACCOUNT_TEMPLATE, login.c_str(), password.c_str());
}

std::wstring uri_sharaclub::get_playlist_template(bool first /*= true*/) const
{
	return PLAYLIST_TEMPLATE;
}
