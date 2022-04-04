#include "pch.h"
#include "uri_vidok.h"

#include "UtilsLib\md5.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto ACCOUNT_TEMPLATE = L"http://sapi.ott.st/v2.4/json/account?token={:s}";
static constexpr auto PLAYLIST_TEMPLATE = L"http://vidok.tv/p/{:s}";
static constexpr auto URI_TEMPLATE_HLS = L"http://{SUBDOMAIN}/p/{TOKEN}/{ID}";
static constexpr auto EPG1_TEMPLATE_JSON = L"http://sapi.ott.st/v2.4/json/epg2?cid={:s}&token={:s}";

void uri_vidok::parse_uri(const std::wstring& url)
{
	// http://bddpv.hdme.top/p/7508af3ccf8194bd2339897708c06eda/1
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

std::wstring uri_vidok::get_templated_stream(StreamSubType subType, const TemplateParams& params) const
{
	std::wstring url;

	if (!is_template())
	{
		url = get_uri();
	}
	else
	{
		switch (subType)
		{
			case StreamSubType::enHLS:
				url = URI_TEMPLATE_HLS;
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

std::wstring uri_vidok::get_epg_uri_json(bool first, const std::wstring& id, time_t for_time /*= 0*/) const
{
	return fmt::format(EPG1_TEMPLATE_JSON, id, token);
}

std::wstring uri_vidok::get_playlist_template(const PlaylistTemplateParams& params) const
{
	return fmt::format(PLAYLIST_TEMPLATE, get_api_token(params.login, params.password));
}

std::wstring uri_vidok::get_api_token(const std::wstring& login, const std::wstring& password) const
{
	std::string login_a = utils::string_tolower(utils::utf16_to_utf8(login));
	std::string password_a = utils::utf16_to_utf8(password);
	return utils::utf8_to_utf16(utils::md5_hash_hex(login_a + utils::md5_hash_hex(password_a)));
}

bool uri_vidok::parse_access_info(const PlaylistTemplateParams& params, std::list<AccountInfo>& info_list) const
{
	std::vector<BYTE> data;
	if (!utils::DownloadFile(fmt::format(ACCOUNT_TEMPLATE, get_api_token(params.login, params.password)), data) || data.empty())
	{
		return false;
	}

	JSON_ALL_TRY
	{
		nlohmann::json parsed_json = nlohmann::json::parse(data);
		nlohmann::json js_account = parsed_json["account"];

		put_account_info("login", js_account, info_list);
		put_account_info("balance", js_account, info_list);

		for (auto& item : js_account["packages"].items())
		{
			nlohmann::json val = item.value();
			COleDateTime dt(utils::char_to_int64(val.value("expire", "")));
			const auto& value = fmt::format("expired {:d}.{:d}.{:d}", dt.GetDay(), dt.GetMonth(), dt.GetYear());
			AccountInfo info { utils::utf8_to_utf16(val.value("name", "")), utils::utf8_to_utf16(value) };
			info_list.emplace_back(info);
		}

		return true;
	}
	JSON_ALL_CATCH;

	return false;
}

nlohmann::json uri_vidok::get_epg_root(bool first, const nlohmann::json& epg_data) const
{
	return epg_data["epg"];
}

std::string uri_vidok::get_epg_name(bool first, const nlohmann::json& val) const
{
	return get_json_value("title", val);
}

std::string uri_vidok::get_epg_desc(bool first, const nlohmann::json& val) const
{
	return get_json_value("description", val);
}

time_t uri_vidok::get_epg_time_start(bool first, const nlohmann::json& val) const
{
	return get_json_int_value("start", val);
}

time_t uri_vidok::get_epg_time_end(bool first, const nlohmann::json& val) const
{
	return get_json_int_value("end", val);
}
