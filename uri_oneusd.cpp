#include "pch.h"
#include "uri_oneusd.h"
#include "UtilsLib\utils.h"
#include "PlayListEntry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto PLAYLIST_TEMPLATE = L"http://1usd.tv/pl-{:s}-hls";
static constexpr auto URI_TEMPLATE_HLS = L"http://{SUBDOMAIN}/{ID}/mono.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{SUBDOMAIN}/{ID}/mpegts?token={TOKEN}";
static constexpr auto URI_TEMPLATE_ARCH_HLS = L"http://{SUBDOMAIN}/{ID}/index-{START}-7200.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_ARCH_MPEG = L"http://{SUBDOMAIN}/{ID}/archive-{START}-7200.ts?token={TOKEN}";
static constexpr auto EPG1_TEMPLATE_JSON = L"http://tv.team/{:s}.json";

void uri_oneusd::parse_uri(const std::wstring& url)
{
	// http://1.1usd.tv:34000/ch001/mono.m3u8?token=1usdtv_roberto.VdOQFJwYXrBNhWugsGPRdx-HBrzPKotuzKtiuHlD2Eyc7d7GPdA4CT8plX8MekNzcvYSCKA9C7SsfNNryytxJg..
	// http://1.1usd.tv:34000/ch002/mono.m3u8?token=1usdtv_roberto.VdOQFJwYXrBNhWugsGPRdx-HBrzPKotuzKtiuHlD2EzWB4Sdv5ZgmYMp5dx5SZqpGpZdSDUI6YVkqEciNnrvBA..
	// http://1.1usd.tv:34000/ch025/mono.m3u8?token=1usdtv_roberto.VdOQFJwYXrBNhWugsGPRdx-HBrzPKotuzKtiuHlD2Ezbn-vBVpPlFCl2dNce4nnKhFT1o9jM6oFrM-csWTJQCA..

	static std::wregex re_url_hls(LR"(^https?:\/\/(.+)\/(.+)\/mono\.m3u8\?token=(.+)$)");

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

std::wstring uri_oneusd::get_templated_stream(StreamSubType subType, const TemplateParams& params) const
{
	std::wstring url;

	if (is_template())
	{
		switch (subType)
		{
			case StreamSubType::enHLS:
				url = params.shift_back ? URI_TEMPLATE_ARCH_HLS : URI_TEMPLATE_HLS;
				break;
			case StreamSubType::enMPEGTS:
				url = params.shift_back ? URI_TEMPLATE_ARCH_MPEG : URI_TEMPLATE_MPEG;
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
			append_archive(url);
		}
	}

	replace_vars(url, params);
	return url;
}

std::wstring uri_oneusd::get_epg_uri_json(bool first, const std::wstring& id, time_t for_time /*= 0*/) const
{
	return fmt::format(EPG1_TEMPLATE_JSON, id);
}

std::wstring uri_oneusd::get_playlist_template(const PlaylistTemplateParams& params) const
{
	return fmt::format(PLAYLIST_TEMPLATE, params.password);
}
