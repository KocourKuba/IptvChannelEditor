#include "pch.h"
#include "uri_shuratv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto PLAYLIST_TEMPLATE = L"http://pl.tvshka.net/?uid={:s}&srv={:d}&type=halva";
static constexpr auto URI_TEMPLATE_HLS = L"http://{SUBDOMAIN}/~{TOKEN}/{ID}/hls/pl.m3u8";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{SUBDOMAIN}/~{TOKEN}/{ID}/";
static constexpr auto EPG1_TEMPLATE_JSON = L"http://s1.tvshka.net/{:s}/epg/range14-7.json";

void uri_shuratv::parse_uri(const std::wstring& url)
{
	// http://s1.tvshka.net/~shsv45hh617fU/119/hls/pl.m3u8
	static std::wregex re_url(LR"(^https?:\/\/(.+)\/~(.+)\/(.+)\/hls\/.+\.m3u8$)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url))
	{
		templated = true;
		domain = std::move(m[1].str());
		token = std::move(m[2].str());
		id = std::move(m[3].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_shuratv::get_templated_stream(StreamSubType subType, const TemplateParams& params) const
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
		append_archive(url);
	}

	replace_vars(url, params);

	return url;
}

std::wstring uri_shuratv::get_epg_uri_json(int epg_idx, const std::wstring& id, time_t for_time /*= 0*/) const
{
	COleDateTime dt = COleDateTime::GetCurrentTime();
	return fmt::format(EPG1_TEMPLATE_JSON, id, dt.GetYear(), dt.GetMonth(), dt.GetDay());
}

std::wstring uri_shuratv::get_playlist_template(const PlaylistTemplateParams& params) const
{
	return fmt::format(PLAYLIST_TEMPLATE, params.password, params.number);
}

nlohmann::json uri_shuratv::get_epg_root(int epg_idx, const nlohmann::json& epg_data) const
{
	return epg_idx == 0 ? epg_data : uri_stream::get_epg_root(epg_idx, epg_data);
}

std::string uri_shuratv::get_epg_name(int epg_idx, const nlohmann::json& val) const
{
	return epg_idx == 0 ? get_json_value("name", val) : uri_stream::get_epg_name(epg_idx, val);
}

std::string uri_shuratv::get_epg_desc(int epg_idx, const nlohmann::json& val) const
{
	return epg_idx == 0 ? get_json_value("text", val) : uri_stream::get_epg_desc(epg_idx, val);
}

time_t uri_shuratv::get_epg_time_start(int epg_idx, const nlohmann::json& val) const
{
	return epg_idx == 0 ? get_json_int_value("start_time", val) : uri_stream::get_epg_time_start(epg_idx, val);
}

time_t uri_shuratv::get_epg_time_end(int epg_idx, const nlohmann::json& val) const
{
	return epg_idx == 0 ? get_json_int_value("duration", val) : uri_stream::get_epg_time_end(epg_idx, val);
}

std::wstring& uri_shuratv::append_archive(std::wstring& url) const
{
	if (url.rfind('?') != std::wstring::npos)
		url += '&';
	else
		url += '?';

	url += L"archive={START}&lutc={NOW}";

	return url;
}
