#include "pch.h"
#include "uri_cbilling.h"
#include "PlayListEntry.h"

#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto PLAYLIST_TEMPLATE = L"http://cbilling.pw/playlist/{:s}_otp_dev1.m3u8";
static constexpr auto URI_TEMPLATE_HLS = L"http://{SUBDOMAIN}/s/{TOKEN}/{ID}.m3u8";
static constexpr auto URI_TEMPLATE_HLS2 = L"http://{SUBDOMAIN}/{ID}/video.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{SUBDOMAIN}/{ID}/mpegts?token={TOKEN}";
static constexpr auto EPG1_TEMPLATE = L"http://epg.ott-play.com/php/show_prog.php?f=cbilling/epg/{:s}.json";
static constexpr auto EPG1_TEMPLATE_JSON = L"http://epg.ott-play.com/cbilling/epg/{:s}.json";
static constexpr auto EPG2_TEMPLATE = L"https://api.iptvx.tv/epg/{:s}";
static constexpr auto EPG2_TEMPLATE_JSON = L"https://api.iptvx.tv/epg/{:s}";

void uri_cbilling::parse_uri(const std::wstring& url)
{
	// http://s01.iptvx.tv:8090/s/82s4fb5785dcf28dgd6ga681a94ba78f/pervyj.m3u8
	static std::wregex re_url_hls(LR"(^https?:\/\/(.+)\/s\/(.+)\/(.+)\.m3u8$)");

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

std::wstring uri_cbilling::get_templated(StreamSubType subType, TemplateParams& params) const
{
	std::wstring url;

	if (is_template())
	{
		std::wstring no_port(params.domain);
		if (auto pos = no_port.find(':'); pos != std::wstring::npos)
		{
			no_port = no_port.substr(0, pos);
		}

		switch (subType)
		{
			case StreamSubType::enHLS:
				url = URI_TEMPLATE_HLS;
				if (params.shift_back)
				{
					AppendArchive(url);
				}
				break;
			case StreamSubType::enHLS2:
				url = URI_TEMPLATE_HLS2;
				params.domain = no_port;
				if (params.shift_back)
				{
					utils::string_replace_inplace(url, L"video.m3u8", L"video-{START}-10800.m3u8");
				}
				break;
			case StreamSubType::enMPEGTS:
				url = URI_TEMPLATE_MPEG;
				params.domain = no_port;
				if (params.shift_back)
				{
					utils::string_replace_inplace(url, L"mpegts", L"archive-{START}-10800.ts");
				}
				break;
		}
	}
	else
	{
		url = get_uri();
	}

	ReplaceVars(url, params);
	return url;
}

std::wstring uri_cbilling::get_epg1_uri(const std::wstring& id) const
{
	return fmt::format(EPG1_TEMPLATE, id);
}

std::wstring uri_cbilling::get_epg1_uri_json(const std::wstring& id) const
{
	return fmt::format(EPG1_TEMPLATE_JSON, id);
}

std::wstring uri_cbilling::get_epg2_uri(const std::wstring& id) const
{
	return fmt::format(EPG2_TEMPLATE, id);
}

std::wstring uri_cbilling::get_epg2_uri_json(const std::wstring& id) const
{
	return fmt::format(EPG2_TEMPLATE_JSON, id);
}

std::wstring uri_cbilling::get_playlist_template(bool first /*= true*/) const
{
	return PLAYLIST_TEMPLATE;
}
