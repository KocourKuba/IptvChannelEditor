#include "pch.h"
#include "uri_tvclub.h"

#include "UtilsLib\md5.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto ACCOUNT_TEMPLATE = L"http://api.iptv.so/0.9/json/account?token={:s}";
static constexpr auto PLAYLIST_TEMPLATE = L"http://celn.shott.top/p/{:s}";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{SUBDOMAIN}/p/{TOKEN}/{ID}";
static constexpr auto EPG1_TEMPLATE_JSON = L"http://api.iptv.so/0.9/json/epg?token={:s}&channels={:s}&time={:d}&period=24";

void uri_tvclub::parse_uri(const std::wstring& url)
{
	// http://celn.shott.top/p/8d7b03a5df9b265e7d21bg876678cc0a/1
	static std::wregex re_url_hls(LR"(^https?:\/\/(.+)\/p\/(.+)\/(.+)$)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url_hls))
	{
		templated = true;
		domain = std::move(m[1].str());
		token = std::move(m[2].str());
		id = std::move(m[3].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_tvclub::get_templated_stream(StreamSubType subType, const TemplateParams& params) const
{
	std::wstring url;

	if (!is_template())
	{
		url = get_uri();
	}
	else
	{
		url = URI_TEMPLATE_MPEG;
	}

	if (params.shift_back)
	{
		append_archive(url);
	}

	replace_vars(url, params);

	return url;
}

std::wstring uri_tvclub::get_epg_uri_json(bool /*first*/, const std::wstring& id, time_t for_time /*= 0*/) const
{
	tm lt = {};
	localtime_s(&lt, &for_time);
	lt.tm_hour = 0;
	lt.tm_min = 0;
	lt.tm_sec = 0;
	time_t start = mktime(&lt);

	return fmt::format(EPG1_TEMPLATE_JSON, token, id, start);
}

std::wstring uri_tvclub::get_playlist_template(const PlaylistTemplateParams& params) const
{
	return fmt::format(PLAYLIST_TEMPLATE, get_api_token(params.login, params.password));
}

std::wstring uri_tvclub::get_api_token(const std::wstring& login, const std::wstring& password) const
{
	std::string login_a = utils::string_tolower(utils::utf16_to_utf8(login));
	std::string password_a = utils::utf16_to_utf8(password);
	return utils::utf8_to_utf16(utils::md5_hash_hex(login_a + utils::md5_hash_hex(password_a)));
}

bool uri_tvclub::parse_access_info(const PlaylistTemplateParams& params, std::list<AccountInfo>& info_list) const
{
	std::vector<BYTE> data;
	const auto& url = fmt::format(ACCOUNT_TEMPLATE, get_api_token(params.login, params.password));
	if (!utils::DownloadFile(url, data) || data.empty())
	{
		return false;
	}

	JSON_ALL_TRY
	{
		nlohmann::json parsed_json = nlohmann::json::parse(data);

		nlohmann::json js_info = parsed_json["account"]["info"];
		put_account_info("login", js_info, info_list);
		put_account_info("balance", js_info, info_list);

		nlohmann::json js_settings = parsed_json["account"]["settings"];
		put_account_info("server_name", js_settings, info_list);
		put_account_info("tz_name", js_settings, info_list);
		put_account_info("tz_gmt", js_settings, info_list);

		for (auto& item : parsed_json["account"]["services"].items())
		{
			nlohmann::json val = item.value();
			COleDateTime dt((time_t)val.value("expire", 0));
			const auto& value = fmt::format(L"expired {:d}.{:d}.{:d}", dt.GetDay(), dt.GetMonth(), dt.GetYear());
			const auto& name = utils::utf8_to_utf16(fmt::format("{:s} {:s}", val.value("name", ""), val.value("type", "")));

			AccountInfo info{ name, value };
			info_list.emplace_back(info);
		}

		return true;
	}
	JSON_ALL_CATCH;

	return false;
}

const nlohmann::json& uri_tvclub::get_epg_root(bool first, const nlohmann::json& epg_data) const
{
	return epg_data["epg"]["channels"][0]["epg"];
}

std::string uri_tvclub::get_epg_name(bool first, const nlohmann::json& val) const
{
	return get_json_value("text", val);
}

std::string uri_tvclub::get_epg_desc(bool first, const nlohmann::json& val) const
{
	return get_json_value("description", val);
}

time_t uri_tvclub::get_epg_time_start(bool first, const nlohmann::json& val) const
{
	return get_json_int_value("start", val);
}

time_t uri_tvclub::get_epg_time_end(bool first, const nlohmann::json& val) const
{
	return get_json_int_value("end", val);
}

std::wstring& uri_tvclub::append_archive(std::wstring& url) const
{
	if (url.rfind('?') != std::wstring::npos)
		url += '&';
	else
		url += '?';

	url += L"utc={START}";

	return url;
}
