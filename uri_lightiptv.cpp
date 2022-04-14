#include "pch.h"
#include "uri_lightiptv.h"
#include "PlayListEntry.h"

#include "UtilsLib\utils.h"
#include "UtilsLib\xxhash.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto PLAYLIST_TEMPLATE = L"http://lightiptv.cc/playlist/hls/{:s}.m3u";
static constexpr auto URI_TEMPLATE_HLS = L"http://{SUBDOMAIN}/{TOKEN}/video.m3u8?token={PASSWORD}";
static constexpr auto URI_TEMPLATE_HLS2 = L"http://{SUBDOMAIN}/{TOKEN}/index.m3u8?token={PASSWORD}";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{SUBDOMAIN}/{TOKEN}/mpegts?token={PASSWORD}";
static constexpr auto URI_TEMPLATE_ARCH_HLS = L"http://{SUBDOMAIN}/{TOKEN}/video-{START}-7200.m3u8?token={PASSWORD}";
static constexpr auto URI_TEMPLATE_ARCH_MPEG = L"http://{SUBDOMAIN}/{TOKEN}/timeshift_abs-{START}.ts?token={PASSWORD}";
static constexpr auto EPG1_TEMPLATE_JSON = L"http://epg.ott-play.com/lightiptv/epg/{:s}.json";
static constexpr auto EPG2_TEMPLATE_JSON = L"http://ott-epg.prog4food.eu.org/lightiptv/epg/{:d}.json";

uri_lightiptv::uri_lightiptv()
{
	epg_proxy[0] = true;
	epg2 = true;
}

void uri_lightiptv::parse_uri(const std::wstring& url)
{
	// http://de1light.pp.ru:8080/6e9751628dd0dbbdd6adf4232c135d83/video.m3u8?token=ds9fsahrrk

	static std::wregex re_url_hls(LR"(^https?:\/\/(.+)\/(.+)\/video\.m3u8\?token=(.+)$)");

	std::wsmatch m;
	if (std::regex_match(url, m, re_url_hls))
	{
		templated = true;
		domain = std::move(m[1].str());
		token = std::move(m[2].str());
		password = std::move(m[3].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_lightiptv::get_templated_stream(StreamSubType subType, const TemplateParams& params) const
{
	std::wstring url;

	if (is_template())
	{
		switch (subType)
		{
			case StreamSubType::enHLS:
				url = params.shift_back ? URI_TEMPLATE_ARCH_HLS : URI_TEMPLATE_HLS;
				break;
			case StreamSubType::enHLS2:
				url = params.shift_back ? URI_TEMPLATE_ARCH_HLS : URI_TEMPLATE_HLS2;
				break;
			case StreamSubType::enMPEGTS:
				url = params.shift_back ? URI_TEMPLATE_ARCH_MPEG : URI_TEMPLATE_MPEG;
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

std::wstring uri_lightiptv::get_epg_uri_json(int epg_idx, const std::wstring& id, time_t for_time /*= 0*/) const
{
	std::wstring url;
	switch (epg_idx)
	{
		case 0:
			url = fmt::format(EPG1_TEMPLATE_JSON, id);
			break;
		case 1:
			url = fmt::format(EPG2_TEMPLATE_JSON, xxh::xxhash<32>(utils::utf16_to_utf8(id)));
			break;
	}
	return url;
}

std::wstring uri_lightiptv::get_playlist_template(const PlaylistTemplateParams& params) const
{
	return fmt::format(PLAYLIST_TEMPLATE, params.password);
}
