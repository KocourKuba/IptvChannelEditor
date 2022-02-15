#include "pch.h"
#include "uri_iptvonline.h"

#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto PLAYLIST_TEMPLATE = L"http://iptv.online/play/{:s}/m3u8";
static constexpr auto URI_TEMPLATE_HLS = L"http://{SUBDOMAIN}/play/{ID}/{TOKEN}/video.m3u8";
static constexpr auto EPG1_TEMPLATE = L"http://epg.ott-play.com/php/show_prog.php?f=iptvx.one/epg/{:S}.json";
static constexpr auto EPG1_TEMPLATE_JSON = L"http://epg.ott-play.com/iptvx.one/epg/{:s}.json";

void uri_iptvonline::parse_uri(const std::wstring& url)
{
	// http://cz.iptv.monster/play/73/38798DB9DF4EA8F/video.m3u8

	static std::wregex re_url_hls(LR"(^https?:\/\/(.+)\/play\/(.+)\/(.+)\/video\.m3u8$)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url_hls))
	{
		templated = true;
		domain = std::move(m[1].str());
		id = std::move(m[2].str());
		token = std::move(m[3].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_iptvonline::get_templated(StreamSubType subType, const TemplateParams& params) const
{
	std::wstring url;

	if (is_template())
	{
		url = URI_TEMPLATE_HLS;
		switch (subType)
		{
			case StreamSubType::enHLS:
				if (params.shift_back)
				{
					utils::string_replace_inplace(url, L"video.m3u8", L"video-{START}-10800.m3u8");
				}
				break;
			case StreamSubType::enMPEGTS:
				utils::string_replace_inplace(url, L"video.m3u8", L"mpegts");
				if (params.shift_back)
				{
					utils::string_replace_inplace(url, L"mpegts", L"archive-{START}-10800.ts");
				}
				break;
			default:
				break;
		}
	}
	else
	{
		url = get_uri();
		if (params.shift_back)
		{
			AppendArchive(url);
		}

	}

	ReplaceVars(url, params);

	return url;
}

std::wstring uri_iptvonline::get_epg1_uri(const std::wstring& id) const
{
	return fmt::format(EPG1_TEMPLATE, id);
}

std::wstring uri_iptvonline::get_epg1_uri_json(const std::wstring& id) const
{
	return fmt::format(EPG1_TEMPLATE_JSON, id);
}

std::wstring uri_iptvonline::get_playlist_template(bool first /*= true*/) const
{
	return PLAYLIST_TEMPLATE;
}
