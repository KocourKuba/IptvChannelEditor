#include "pch.h"
#include "uri_sharavoz.h"
#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto PLAYLIST_TEMPLATE = L"http://sharavoz.tk/iptv/p/{:s}/Sharavoz.Tv.navigator-ott.m3u";
static constexpr auto URI_TEMPLATE_HLS = L"http://{SUBDOMAIN}/{ID}/index.m3u8?token={TOKEN}";
static constexpr auto EPG1_TEMPLATE_JSON = L"http://api.program.spr24.net/api/program?epg={:s}&date={:4d}-{:02d}-{:02d}";
static constexpr auto EPG2_TEMPLATE_JSON = L"http://epg.arlekino.tv/api/program?epg={:s}&date={:4d}-{:02d}-{:02d}";

void uri_sharavoz::parse_uri(const std::wstring& url)
{
	// http://domain.com/204/index.m3u8?token=adsdaSDFJKHKJd
	// http://domain.com/204/mpegts?token=adsdaSDFJKHKJd

	static std::wregex re_url(LR"(^https?:\/\/(.+)\/(\d+)\/(?:mpegts|index\.m3u8)\?token=(.+)$)");
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

std::wstring uri_sharavoz::get_templated_stream(StreamSubType subType, const TemplateParams& params) const
{
	std::wstring url = is_template() ? URI_TEMPLATE_HLS : get_uri();

	if (subType == StreamSubType::enMPEGTS)
	{
		utils::string_replace_inplace(url, L"index.m3u8", L"mpegts");
	}

	if (params.shift_back)
	{
		append_archive(url);
	}

	replace_vars(url, params);

	return url;
}

std::wstring uri_sharavoz::get_epg_uri_json(int epg_idx, const std::wstring& id, time_t for_time /*= 0*/) const
{
	COleDateTime dt = COleDateTime::GetCurrentTime();
	return fmt::format(epg_idx == 0 ? EPG1_TEMPLATE_JSON : EPG2_TEMPLATE_JSON, id, dt.GetYear(), dt.GetMonth(), dt.GetDay());
}

std::wstring uri_sharavoz::get_playlist_template(const PlaylistTemplateParams& params) const
{
	return fmt::format(PLAYLIST_TEMPLATE, params.password);
}
