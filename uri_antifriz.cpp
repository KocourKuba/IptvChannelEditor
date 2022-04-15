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
static constexpr auto EPG1_TEMPLATE_JSON = L"http://protected-api.com/epg/{:s}/?date=";

void uri_antifriz::parse_uri(const std::wstring& url)
{
	// http://tchaikovsky.antifriz.tv:1600/s/ibzsdpt2t/demo-4k/video.m3u8
	// http://tchaikovsky.antifriz.tv:80/demo-4k/mpegts?token=ibzsdpt2t

	static std::wregex re_url_hls(LR"(^https?:\/\/(.+)\/s\/(.+)\/(.+)\/video\.m3u8$)");
	static std::wregex re_url_mpeg(LR"(^https?:\/\/(.+)\/(.+)\/mpegts\?token=(.+)$)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url_hls))
	{
		templated = true;
		domain = std::move(m[1].str());
		token = std::move(m[2].str());
		id = std::move(m[3].str());
		return;
	}

	if (std::regex_match(url, m, re_url_mpeg))
	{
		templated = true;
		domain = std::move(m[1].str());
		id = std::move(m[2].str());
		token = std::move(m[3].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_antifriz::get_templated_stream(StreamSubType subType, const TemplateParams& params) const
{
	std::wstring url;

	if (is_template())
	{
		auto& new_params = const_cast<TemplateParams&>(params);
		std::wstring no_port(params.domain);
		if (auto pos = no_port.find(':'); pos != std::wstring::npos)
		{
			no_port = no_port.substr(0, pos);
		}

		switch (subType)
		{
			case StreamSubType::enHLS:
				url = params.shift_back ? URI_TEMPLATE_ARCH_HLS : URI_TEMPLATE_HLS;
				new_params.domain = params.shift_back ? no_port : params.domain;
				break;
			case StreamSubType::enMPEGTS:
				url = params.shift_back ? URI_TEMPLATE_ARCH_MPEG : URI_TEMPLATE_MPEG;
				new_params.domain = std::move(no_port);
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

std::wstring uri_antifriz::get_epg_uri_json(int epg_idx, const std::wstring& id, time_t for_time /*= 0*/) const
{
	COleDateTime dt(for_time ? for_time : COleDateTime::GetCurrentTime());
	return fmt::format(EPG1_TEMPLATE_JSON, id, dt.GetYear(), dt.GetMonth(), dt.GetDay());
}

std::wstring uri_antifriz::get_playlist_template(const PlaylistTemplateParams& params) const
{
	return fmt::format(PLAYLIST_TEMPLATE, params.password);
}

nlohmann::json uri_antifriz::get_epg_root(int epg_idx, const nlohmann::json& epg_data) const
{
	return epg_data;
}
