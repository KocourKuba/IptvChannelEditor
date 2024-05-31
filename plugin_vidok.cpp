/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2024): sharky72 (https://github.com/KocourKuba)

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
#include "plugin_vidok.h"

#include "UtilsLib\md5.h"
#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// API documentation http://wiki.vidok.tv/index.php?title=SAPI

static constexpr auto API_COMMAND_GET_URL = L"{:s}/{:s}?token={:s}";
static constexpr auto API_COMMAND_SET_URL = L"{:s}/{:s}?token={:s}&{:s}={:s}";

std::wstring plugin_vidok::get_api_token(const Credentials& creds) const
{
	std::string login_a = utils::utf16_to_utf8(creds.get_login());
	std::string password_a = utils::utf16_to_utf8(creds.get_password());
	return utils::utf8_to_utf16(utils::md5_hash_hex(login_a + utils::md5_hash_hex(password_a)));
}

std::map<std::wstring, std::wstring, std::less<>> plugin_vidok::parse_access_info(const TemplateParams& params)
{
	std::map<std::wstring, std::wstring, std::less<>> info;

	Credentials creds;
	creds.set_login(params.login);
	creds.set_password(params.password);

	CWaitCursor cur;
	std::stringstream data;
	if (download_url(fmt::format(API_COMMAND_GET_URL, utils::utf8_to_utf16(provider_api_url), L"account", get_api_token(creds)), data))
	{
		JSON_ALL_TRY
		{
			const auto& parsed_json = nlohmann::json::parse(data.str());
			if (parsed_json.contains("account"))
			{
				const auto& js_account = parsed_json["account"];

				set_json_info("login", js_account, info);
				set_json_info("balance", js_account, info);

				if (js_account.contains("packages"))
				{
					for (auto& item : js_account["packages"].items())
					{
						COleDateTime dt(utils::char_to_int64(item.value().value("expire", "")));
						info.emplace(utils::utf8_to_utf16(item.value().value("name", "")),
									 utils::utf8_to_utf16(fmt::format("expired {:d}.{:d}.{:d}", dt.GetDay(), dt.GetMonth(), dt.GetYear())));
					}
				}
			}
		}
		JSON_ALL_CATCH;
	}

	return info;
}

void plugin_vidok::fill_servers_list(TemplateParams& params)
{
	if (params.login.empty() || params.password.empty() || !get_servers_list().empty())
		return;

	std::vector<DynamicParamsInfo> servers;

	Credentials creds;
	creds.set_login(params.login);
	creds.set_password(params.password);

	const auto& url = fmt::format(API_COMMAND_GET_URL, utils::utf8_to_utf16(provider_api_url), L"settings", get_api_token(creds));

	CWaitCursor cur;
	std::stringstream data;
	if (download_url(url, data))
	{
		JSON_ALL_TRY;
		{
			const auto& parsed_json = nlohmann::json::parse(data.str());
			if (parsed_json.contains("settings"))
			{
				const auto& settings = parsed_json["settings"];

				const auto& current = utils::get_json_wstring("id", settings["current"]["server"]);

				for (const auto& item : settings["lists"]["servers"].items())
				{
					const auto& server = item.value();
					DynamicParamsInfo info{ utils::get_json_string("id", server), utils::get_json_string("name", server) };
					if (info.get_id() == current)
						params.server_idx = (int)servers.size();

					servers.emplace_back(info);
				}
			}
		}
		JSON_ALL_CATCH;
	}

	set_servers_list(servers);
}

bool plugin_vidok::set_server(TemplateParams& params)
{
	if (servers_list.empty())
	{
		fill_servers_list(params);
	}

	if (!servers_list.empty())
	{
		Credentials creds;
		creds.set_login(params.login);
		creds.set_password(params.password);

		const auto& url = fmt::format(API_COMMAND_SET_URL,
									  utils::utf8_to_utf16(provider_api_url),
									  L"settings_set",
									  get_api_token(creds),
									  L"server",
									  servers_list[params.server_idx].get_id());

		CWaitCursor cur;
		std::stringstream data;
		if (download_url(url, data))
		{
			JSON_ALL_TRY;
			{
				const auto& parsed_json = nlohmann::json::parse(data.str());
				for (const auto& item : parsed_json["settings"].items())
				{
					const auto& server = item.value();
					return (utils::get_json_wstring("value", server) == servers_list[params.server_idx].get_id()); //-V612
				}
			}
			JSON_ALL_CATCH;
		}
	}

	return false;
}
