#include "StdAfx.h"
#include "uri_sharavoz.h"
#include "utils.h"

static constexpr auto PLAYLIST_TEMPLATE = L"http://sharavoz.tk/iptv/p/{:s}/Sharavoz.Tv.navigator-ott.m3u";
static constexpr auto URI_TEMPLATE_HLS = L"http://{SUBDOMAIN}/{ID}/index.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{SUBDOMAIN}/{ID}/mpegts?token={TOKEN}";
static constexpr auto EPG1_TEMPLATE = L"http://api.program.spr24.net/api/program?epg={:s}&date={:4d}-{:02d}-{:02d}";
static constexpr auto EPG2_TEMPLATE = L"http://epg.arlekino.tv/api/program?epg={:s}&date={:4d}-{:02d}-{:02d}";

void uri_sharavoz::parse_uri(const std::wstring& url)
{
	// http://domain.com/204/index.m3u8?token=adsdaSDFJKHKJd
	// http://domain.com/204/mpegts?token=adsdaSDFJKHKJd

	static std::wregex re_url(LR"(^https?:\/\/(.+)\/(\d+)\/(?:mpegts|index\.m3u8)\?token=(.+)$)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url))
	{
		set_template(true);
		domain = m[1].str();
		id = m[2].str();
		token = m[3].str();
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_sharavoz::get_templated(StreamSubType subType, const TemplateParams& params) const
{
	std::wstring url = get_uri();

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
		url += L"&utc={START}&lutc={NOW}";
	}

	// http://{SUBDOMAIN}/{ID}/index.m3u8?token={TOKEN}
	// http://{SUBDOMAIN}/{ID}/mpegts?token={TOKEN}
	utils::string_replace_inplace<wchar_t>(url, REPL_TOKEN, params.token);
	ReplaceVars(url, params);

	return url;
}

std::wstring uri_sharavoz::get_epg1_uri(const std::wstring& id) const
{
	COleDateTime dt = COleDateTime::GetCurrentTime();
	return fmt::format(EPG1_TEMPLATE, id, dt.GetYear(), dt.GetMonth(), dt.GetDay());
}

std::wstring uri_sharavoz::get_epg2_uri(const std::wstring& id) const
{
	COleDateTime dt = COleDateTime::GetCurrentTime();
	return fmt::format(EPG2_TEMPLATE, id, dt.GetYear(), dt.GetMonth(), dt.GetDay());
}

std::wstring uri_sharavoz::get_epg1_uri_json(const std::wstring& id) const
{
	return get_epg1_uri(id);
}

std::wstring uri_sharavoz::get_epg2_uri_json(const std::wstring& id) const
{
	return get_epg2_uri(id);
}

std::wstring uri_sharavoz::get_playlist_template(bool first /*= true*/) const
{
	return PLAYLIST_TEMPLATE;
}
