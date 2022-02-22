#include "pch.h"
#include "uri_glanz.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto PLAYLIST_TEMPLATE = L"http://pl.ottglanz.tv/get.php?username={:s}&password={:s}&type=m3u&output=hls";
static constexpr auto URI_TEMPLATE_HLS = L"http://{SUBDOMAIN}/{ID}/video.m3u8?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{SUBDOMAIN}/{ID}/mpegts?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}";
static constexpr auto URI_TEMPLATE_ARCH_HLS = L"http://{SUBDOMAIN}/{ID}/video-{START}-10800.m3u8?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}";
static constexpr auto URI_TEMPLATE_ARCH_MPEG = L"http://{SUBDOMAIN}/{ID}/archive-{START}-10800.ts?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}";
static constexpr auto EPG1_TEMPLATE_JSON = L"http://epg.ott-play.com/ottg/epg/{:s}.json";

static constexpr auto REPL_LOGIN = L"{LOGIN}";
static constexpr auto REPL_PASSWORD = L"{PASSWORD}";
static constexpr auto REPL_INT_ID = L"{INT_ID}";
static constexpr auto REPL_HOST = L"{HOST}";

void uri_glanz::parse_uri(const std::wstring& url)
{
	// http://str01.ottg.cc/9195/video.m3u8?username=sharky72&password=F8D58856LWX&token=f5afea07cef148278ae074acaf67a547&ch_id=70&req_host=pkSx3BL
	// http://str01.ottg.cc/9195/mpegts?username=sharky72&password=F8D58856LWX&token=f5afea07cef148278ae074acaf67a547&ch_id=70&req_host=pkSx3BL

	static std::wregex re_url(LR"(^https?:\/\/(.+)\/(.+)\/.+\?username=(.+)&password=(.+)&token=(.+)&ch_id=(\d+)&req_host=(.+)$)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url))
	{
		templated = true;
		domain = std::move(m[1].str());
		id = std::move(m[2].str());
		login = std::move(m[3].str());
		password = std::move(m[4].str());
		token = std::move(m[5].str());
		int_id = std::move(m[6].str());
		host = std::move(m[7].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_glanz::get_templated_stream(StreamSubType subType, const TemplateParams& params) const
{
	std::wstring url;

	if (is_template())
	{
		switch (subType)
		{
			case StreamSubType::enHLS: // hls
				url = params.shift_back ? URI_TEMPLATE_ARCH_HLS : URI_TEMPLATE_HLS;
				break;
			case StreamSubType::enMPEGTS: // mpeg-ts
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

	// http://{SUBDOMAIN}/{ID}/video.m3u8?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}
	// http://{SUBDOMAIN}/{ID}/mpegts?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}
	utils::string_replace_inplace<wchar_t>(url, REPL_LOGIN, params.login);
	utils::string_replace_inplace<wchar_t>(url, REPL_PASSWORD, params.password);
	utils::string_replace_inplace<wchar_t>(url, REPL_INT_ID, get_int_id());
	utils::string_replace_inplace<wchar_t>(url, REPL_HOST, params.host);
	replace_vars(url, params);

	return url;
}

std::wstring uri_glanz::get_epg_uri_json(bool first, const std::wstring& id, time_t for_time /*= 0*/) const
{
	return fmt::format(EPG1_TEMPLATE_JSON, id);
}

std::wstring uri_glanz::get_playlist_template(const PlaylistTemplateParams& params) const
{
	return fmt::format(PLAYLIST_TEMPLATE, params.login, params.password);
}
