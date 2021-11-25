#include "StdAfx.h"
#include "uri_viplime.h"
#include "utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto PLAYLIST_TEMPLATE = L"http://cdntv.online/high/{:s}/playlist.m3u8";
static constexpr auto URI_TEMPLATE_HLS = L"http://{SUBDOMAIN}/high/{TOKEN}/{ID}.m3u8";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{SUBDOMAIN}/high/{TOKEN}/{ID}.mpeg";
static constexpr auto EPG1_TEMPLATE = L"http://epg.ott-play.com/php/show_prog.php?f=viplime/epg/{:s}.json";
static constexpr auto EPG1_TEMPLATE_JSON = L"http://epg.ott-play.com/viplime/epg/{:s}.json";

void uri_viplime::parse_uri(const std::wstring& url)
{
	// http://cdntv.online/high/z3sf8hueit/1.m3u8
	// http://cdntv.online/high/z3sf8hueit/1.mpeg
	static std::wregex re_url_hls(LR"(^https?:\/\/(.+)\/(.+)\/(.+)\/(.+).m3u8$)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url_hls))
	{
		set_template(true);
		set_domain(m[1].str());
		set_token(m[3].str());
		set_id(m[4].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_viplime::get_templated(StreamSubType subType, const TemplateParams& params) const
{
	std::wstring url;

	if (!is_template())
	{
		url = get_uri();
	}
	else
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

std::wstring uri_viplime::get_epg1_uri(const std::wstring& id) const
{
	return fmt::format(EPG1_TEMPLATE, id);
}

std::wstring uri_viplime::get_epg1_uri_json(const std::wstring& id) const
{
	return fmt::format(EPG1_TEMPLATE_JSON, id);
}

std::wstring uri_viplime::get_playlist_template(bool first /*= true*/) const
{
	return PLAYLIST_TEMPLATE;
}
