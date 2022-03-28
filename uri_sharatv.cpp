#include "pch.h"
#include "uri_sharatv.h"
#include "UtilsLib\utils.h"
#include "PlayListEntry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto PLAYLIST_TEMPLATE = L"http://tvfor.pro/g/{:s}:{:s}/1/playlist.m3u";
static constexpr auto URI_TEMPLATE = L"http://{SUBDOMAIN}/{ID}/{TOKEN}";
static constexpr auto EPG1_TEMPLATE_JSON = L"http://technic.cf/epg-shara-tv/epg_day?id={:s}&day={:04d}.{:02d}.{:02d}";

void uri_sharatv::parse_uri(const std::wstring& url)
{
	// http://messi.tvfor.pro/Perviykanal/a8dv285y29itx4ye4oj3cez5

	static std::wregex re_url(LR"(^https?:\/\/(.+)\/(.+)\/(.+)$)");
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

std::wstring uri_sharatv::get_templated_stream(StreamSubType subType, const TemplateParams& params) const
{
	std::wstring url = is_template() ? URI_TEMPLATE : get_uri();

	if (params.shift_back)
	{
		url += L"?utc={START}";
	}

	replace_vars(url, params);

	return url;
}

std::wstring uri_sharatv::get_epg_uri_json(bool first, const std::wstring& id, time_t for_time /*= 0*/) const
{
	COleDateTime dt(for_time ? for_time : COleDateTime::GetCurrentTime());
	return fmt::format(EPG1_TEMPLATE_JSON, id, dt.GetYear(), dt.GetMonth(), dt.GetDay());
}

std::wstring uri_sharatv::get_playlist_template(const PlaylistTemplateParams& params) const
{
	return fmt::format(PLAYLIST_TEMPLATE, params.login, params.password);
}

const nlohmann::json& uri_sharatv::get_epg_root(bool first, const nlohmann::json& epg_data) const
{
	return epg_data["data"];
}

std::string uri_sharatv::get_epg_name(bool first, const nlohmann::json& val) const
{
	return get_json_value("title", val);
}

std::string uri_sharatv::get_epg_desc(bool first, const nlohmann::json& val) const
{
	return get_json_value("description", val);
}

time_t uri_sharatv::get_epg_time_start(bool first, const nlohmann::json& val) const
{
	return get_json_int_value("begin", val);
}

time_t uri_sharatv::get_epg_time_end(bool first, const nlohmann::json& val) const
{
	return get_json_int_value("end", val);
}
