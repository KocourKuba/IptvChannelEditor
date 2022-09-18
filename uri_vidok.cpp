/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2022): sharky72 (https://github.com/KocourKuba)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "pch.h"
#include "uri_vidok.h"

#include "UtilsLib\md5.h"
#include "UtilsLib\utils.h"
#include "UtilsLib\inet_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// API documentation http://wiki.vidok.tv/index.php?title=SAPI

static constexpr auto API_COMMAND_GET_URL = L"http://sapi.ott.st/v2.4/json/{:s}?token={:s}";
static constexpr auto API_COMMAND_SET_URL = L"http://sapi.ott.st/v2.4/json/{:s}?token={:s}&{:s}={:s}";

uri_vidok::uri_vidok()
{
	provider_url = L"https://vidok.tv/";
	access_type = AccountAccessType::enLoginPass;
	catchup_type = { CatchupType::cu_append, CatchupType::cu_none };
	support_streams = { {StreamSubType::enHLS, L"HLS"} };
	per_channel_token = true;

	playlist_template = L"http://vidok.tv/p/{TOKEN}";
	uri_parse_template = LR"(^https?:\/\/(?<domain>.+)\/p\/(?<token>.+)\/(?<id>.+)$)";
	uri_hls_template = L"http://{DOMAIN}/p/{TOKEN}/{ID}";

	auto& params = epg_params[0];
	params.epg_url = L"http://sapi.ott.st/v2.4/json/epg2?cid={ID}&token={TOKEN}";
	params.epg_root = "epg";
	params.epg_name = "title";
	params.epg_desc = "description";
	params.epg_start = "start";
	params.epg_end = "end";
}

std::wstring uri_vidok::get_api_token(const std::wstring& login, const std::wstring& password) const
{
	std::string login_a = utils::string_tolower(utils::utf16_to_utf8(login));
	std::string password_a = utils::utf16_to_utf8(password);
	return utils::utf8_to_utf16(utils::md5_hash_hex(login_a + utils::md5_hash_hex(password_a)));
}

bool uri_vidok::parse_access_info(TemplateParams& params, std::list<AccountInfo>& info_list)
{
	std::vector<BYTE> data;
	if (!utils::DownloadFile(fmt::format(API_COMMAND_GET_URL, L"account", get_api_token(params.login, params.password)), data) || data.empty())
	{
		return false;
	}

	JSON_ALL_TRY
	{
		const auto& parsed_json = nlohmann::json::parse(data.begin(), data.end());
		if (parsed_json.contains("account"))
		{
			const auto& js_account = parsed_json["account"];

			put_account_info("login", js_account, info_list);
			put_account_info("balance", js_account, info_list);

			if (js_account.contains("packages"))
			{
				for (auto& item : js_account["packages"].items())
				{
					const auto& val = item.value();
					COleDateTime dt(utils::char_to_int64(val.value("expire", "")));
					const auto& value = fmt::format("expired {:d}.{:d}.{:d}", dt.GetDay(), dt.GetMonth(), dt.GetYear());
					AccountInfo info{ utils::utf8_to_utf16(val.value("name", "")), utils::utf8_to_utf16(value) };
					info_list.emplace_back(info);
				}
			}

			return true;
		}
	}
	JSON_ALL_CATCH;

	return false;
}

const std::vector<ServersInfo>& uri_vidok::get_servers_list(TemplateParams& params)
{
	if (servers_list.empty())
	{
		const auto& url = fmt::format(API_COMMAND_GET_URL, L"settings", get_api_token(params.login, params.password));
		std::vector<BYTE> data;
		if (utils::DownloadFile(url, data) || data.empty())
		{
			JSON_ALL_TRY;
			{
				const auto& parsed_json = nlohmann::json::parse(data.begin(), data.end());
				if (parsed_json.contains("settings"))
				{
					const auto& settings = parsed_json["settings"];

					const auto& current = utils::get_json_string("id", settings["current"]["server"]);

					for (const auto& item : settings["lists"]["servers"].items())
					{
						const auto& server = item.value();
						ServersInfo info({ utils::get_json_string("id", server), utils::get_json_string("name", server) });
						if (info.id == current)
							params.server = servers_list.size();

						servers_list.emplace_back(info);
					}
				}
			}
			JSON_ALL_CATCH;
		}
	}
	return servers_list;
}

bool uri_vidok::set_server(TemplateParams& params)
{
	const auto& servers = get_servers_list(params);
	if (!servers.empty())
	{
		const auto& url = fmt::format(API_COMMAND_SET_URL,
									  L"settings_set",
									  get_api_token(params.login, params.password),
									  L"server",
									  servers[params.server].id);

		std::vector<BYTE> data;
		if (utils::DownloadFile(url, data) || data.empty())
		{
			JSON_ALL_TRY;
			{
				const auto& parsed_json = nlohmann::json::parse(data.begin(), data.end());
				for (const auto& item : parsed_json["settings"].items())
				{
					const auto& server = item.value();
					return (utils::get_json_string("value", server) == servers[params.server].id); //-V612
				}
			}
			JSON_ALL_CATCH;
		}
	}

	return false;
}
