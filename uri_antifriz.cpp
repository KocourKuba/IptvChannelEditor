#include "pch.h"
#include "uri_antifriz.h"

#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto PLAYLIST_TEMPLATE = L"http://antifriz.tv/playlist/{:s}.m3u8";
static constexpr auto URI_TEMPLATE_HLS = L"http://{SUBDOMAIN}/s/{TOKEN}/{ID}/video.m3u8";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{SUBDOMAIN}/{ID}/mpegts?token={TOKEN}";
static constexpr auto URI_TEMPLATE_ARCH_HLS = L"http://{SUBDOMAIN}/{ID}/archive-{START}-10800.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_ARCH_MPEG = L"http://{SUBDOMAIN}/{ID}/archive-{START}-10800.ts?token={TOKEN}";
static constexpr auto EPG1_TEMPLATE = L"http://epg.ott-play.com/php/show_prog.php?f=antifriz/epg/{:s}.json";
static constexpr auto EPG1_TEMPLATE_JSON = L"http://epg.ott-play.com/antifriz/epg/{:s}.json";

void uri_antifriz::parse_uri(const std::wstring& url)
{
	// http://tchaikovsky.antifriz.tv:1600/s/ibzsdpt2t/demo-4k/video.m3u8
	// http://tchaikovsky.antifriz.tv:80/demo-4k/mpegts?token=ibzsdpt2t

	static std::wregex re_url_hls(LR"(^https?:\/\/(.+)\/s\/(.+)\/(.+)\/video\.m3u8$)");
	static std::wregex re_url_mpeg(LR"(^https?:\/\/(.+)\/(.+)\/mpegts\?token=(.+)$)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url_hls))
	{
		set_template(true);
		set_domain(m[1].str());
		set_token(m[2].str());
		set_id(m[3].str());
		return;
	}

	if (std::regex_match(url, m, re_url_mpeg))
	{
		set_template(true);
		set_domain(m[1].str());
		set_id(m[2].str());
		set_token(m[3].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_antifriz::get_templated(StreamSubType subType, TemplateParams& params) const
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
				url = params.shift_back ? URI_TEMPLATE_ARCH_HLS : URI_TEMPLATE_HLS;
				params.domain = params.shift_back ? no_port : params.domain;
				break;
			case StreamSubType::enMPEGTS:
				url = params.shift_back ? URI_TEMPLATE_ARCH_MPEG : URI_TEMPLATE_MPEG;
				params.domain = std::move(no_port);
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

std::wstring uri_antifriz::get_epg1_uri(const std::wstring& id) const
{
	return fmt::format(EPG1_TEMPLATE, id);
}

std::wstring uri_antifriz::get_epg1_uri_json(const std::wstring& id) const
{
	return fmt::format(EPG1_TEMPLATE_JSON, id);
}

std::wstring uri_antifriz::get_playlist_template(bool first /*= true*/) const
{
	return PLAYLIST_TEMPLATE;
}
