#include "pch.h"
#include "uri_shuratv.h"

#include "UtilsLib\json.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto PLAYLIST_TEMPLATE = L"http://pl.tvshka.net/?uid={:s}&srv={:d}&type=halva";
static constexpr auto URI_TEMPLATE_HLS = L"http://{SUBDOMAIN}/~{TOKEN}/{ID}/hls/pl.m3u8";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{SUBDOMAIN}/~{TOKEN}/{ID}/";
static constexpr auto EPG1_TEMPLATE = L"http://s1.tvshka.net/{:s}/epg/range14-7.json";

void uri_shuratv::parse_uri(const std::wstring& url)
{
	// http://s1.tvshka.net/~shsv45hh617fU/119/hls/pl.m3u8
	static std::wregex re_url(LR"(^https?:\/\/(.+)\/~(.+)\/(.+)\/hls\/.+\.m3u8$)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url))
	{
		set_template(true);
		set_domain(m[1].str());
		set_token(m[2].str());
		set_id(m[3].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_shuratv::get_templated(StreamSubType subType, const TemplateParams& params) const
{
	auto& url = get_uri();

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
			default:
				break;
		}
	}

	if (params.shift_back)
	{
		AppendArchive(url);
	}

	ReplaceVars(url, params);

	return url;
}

std::wstring uri_shuratv::get_epg1_uri(const std::wstring& id) const
{
	COleDateTime dt = COleDateTime::GetCurrentTime();
	return fmt::format(EPG1_TEMPLATE, id, dt.GetYear(), dt.GetMonth(), dt.GetDay());
}

std::wstring uri_shuratv::get_epg1_uri_json(const std::wstring& id) const
{
	return get_epg1_uri(id);
}

std::wstring uri_shuratv::get_playlist_template(bool first /*= true*/) const
{
	return PLAYLIST_TEMPLATE;
}

std::wstring& uri_shuratv::AppendArchive(std::wstring& url) const
{
	if (url.rfind('?') != std::wstring::npos)
		url += '&';
	else
		url += '?';

	url += L"archive={START}&lutc={NOW}";

	return url;
}
