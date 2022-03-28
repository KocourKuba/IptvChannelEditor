#include "pch.h"
#include "uri_edem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto PLAYLIST_TEMPLATE1 = L"http://epg.it999.ru/edem_epg_ico.m3u8";
static constexpr auto PLAYLIST_TEMPLATE2 = L"http://epg.it999.ru/edem_epg_ico2.m3u8";
static constexpr auto URI_TEMPLATE = L"http://{SUBDOMAIN}/iptv/{TOKEN}/{ID}/index.m3u8";
static constexpr auto EPG1_TEMPLATE_JSON = L"http://technic.cf/epg-it999/epg_day?id={:s}&day={:04d}.{:02d}.{:02d}";

void uri_edem::parse_uri(const std::wstring& url)
{
	// http://localhost/iptv/00000000000000/204/index.m3u8

	static std::wregex re_url(LR"(^https?:\/\/(.+)\/iptv\/(.+)\/(\d+)\/.*\.m3u8$)");
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

std::wstring uri_edem::get_templated_stream(StreamSubType subType, const TemplateParams& params) const
{
	std::wstring url = is_template() ? URI_TEMPLATE : get_uri();

	if (params.shift_back)
	{
		append_archive(url);
	}

	replace_vars(url, params);

	return url;
}

std::wstring uri_edem::get_epg_uri_json(bool first, const std::wstring& id, time_t for_time /*= 0*/) const
{
	COleDateTime dt(for_time ? for_time : COleDateTime::GetCurrentTime());
	return fmt::format(EPG1_TEMPLATE_JSON, id, dt.GetYear(), dt.GetMonth(), dt.GetDay());
}

std::wstring uri_edem::get_playlist_template(const PlaylistTemplateParams& params) const
{
	return params.number == 0 ? PLAYLIST_TEMPLATE1 : PLAYLIST_TEMPLATE2;
}

const nlohmann::json& uri_edem::get_epg_root(bool first, const nlohmann::json& epg_data) const
{
	return epg_data["data"];
}

std::string uri_edem::get_epg_name(bool first, const nlohmann::json& val) const
{
	return get_json_value("title", val);
}

std::string uri_edem::get_epg_desc(bool first, const nlohmann::json& val) const
{
	return get_json_value("description", val);
}

time_t uri_edem::get_epg_time_start(bool first, const nlohmann::json& val) const
{
	return get_json_int_value("begin", val);
}

time_t uri_edem::get_epg_time_end(bool first, const nlohmann::json& val) const
{
	return get_json_int_value("end", val);
}
