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
static constexpr auto EPG1_TEMPLATE = L"http://sapi.ott.st/v2.4/xml/epg2?cid={:s}&token={:s}";
static constexpr auto EPG1_TEMPLATE_JSON = L"http://sapi.ott.st/v2.4/json/epg2?cid={:s}&token={:s}";

void uri_vidok::parse_uri(const std::wstring& url)
{
	// http://bddpv.hdme.top/p/7508af3ccf8194bd2339897708c06eda/1
	static std::wregex re_url_hls(LR"(^https?:\/\/(.+)\/p\/(.+)\/(.+)$)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url_hls))
	{
		set_template(true);
		set_domain(m[1].str());
		set_token(m[2].str());
		set_id(m[3].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_vidok::get_templated(StreamSubType subType, const TemplateParams& params) const
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
		AppendArchive(url);
	}

	ReplaceVars(url, params);

	return url;
}

std::wstring uri_vidok::get_epg1_uri(const std::wstring& id) const
{
	return fmt::format(EPG1_TEMPLATE, id, token);
}

std::wstring uri_vidok::get_epg1_uri_json(const std::wstring& id) const
{
	return fmt::format(EPG1_TEMPLATE_JSON, id, token);
}

std::wstring uri_vidok::get_playlist_template(bool first /*= true*/) const
{
	return PLAYLIST_TEMPLATE;
}

std::wstring uri_vidok::get_access_url(const std::wstring& login, const std::wstring& password) const
{
	std::string login_a = utils::string_tolower(utils::utf16_to_utf8(login));
	std::string password_a = utils::utf16_to_utf8(password);
	const auto& token = utils::utf8_to_utf16(utils::md5_hash_hex(login_a + utils::md5_hash_hex(password_a)));

	return fmt::format(ACCOUNT_TEMPLATE, token);
}

bool uri_vidok::parse_access_info(const std::vector<BYTE>& json_data, std::list<AccountParams>& params) const
{
	using json = nlohmann::json;

	try
	{
		json js = json::parse(json_data);
		json js_account = js["account"];

		PutAccountParameter("login", js_account, params);
		PutAccountParameter("balance", js_account, params);

		for (auto& item : js_account["packages"].items())
		{
			json val = item.value();
			COleDateTime dt(utils::char_to_int64(val.value("expire", "")));
			const auto& value = fmt::format("expired {:d}.{:d}.{:d}", dt.GetDay(), dt.GetMonth(), dt.GetYear());
			params.emplace_back(utils::utf8_to_utf16(val.value("name", "")), utils::utf8_to_utf16(value));
		}

		return true;
	}
	catch (const json::parse_error&)
	{
		// parse errors are ok, because input may be random bytes
	}
	catch (const json::out_of_range&)
	{
		// out of range errors may happen if provided sizes are excessive
	}
	catch (const json::type_error&)
	{
		// type errors may happen if provided sizes are excessive
	}

	return false;
}
