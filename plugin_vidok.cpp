/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2025): sharky72 (https://github.com/KocourKuba)

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
#include "Constants.h"

#include "UtilsLib\md5.h"
#include "UtilsLib\utils.h"
#include "UtilsLib\inet_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// API documentation http://wiki.vidok.tv/index.php?title=SAPI

constexpr auto API_COMMAND_URL = L"{{API_URL}}/{:s}?token={{S_TOKEN}}";
constexpr auto PARAM_FMT = L"&{:s}={:s}";

std::string plugin_vidok::get_api_token(TemplateParams& params)
{
	params.creds.s_token = utils::md5_hash_hex(params.creds.login + utils::md5_hash_hex(params.creds.password));

	return params.creds.s_token;
}

void plugin_vidok::parse_account_info(TemplateParams& params)
{
	if (account_info.empty())
	{
		const auto& url = std::format(API_COMMAND_URL, L"account");
		utils::http_request req{ replace_params_vars(params, url) };
		if (utils::AsyncDownloadFile(req).get())
		{
			JSON_ALL_TRY
			{
				const auto& parsed_json = nlohmann::json::parse(req.body.str());
				if (parsed_json.contains("account"))
				{
					const auto& js_account = parsed_json["account"];

					set_json_info("login", js_account, account_info);
					set_json_info("balance", js_account, account_info);

					if (js_account.contains("packages"))
					{
						for (auto& item : js_account["packages"].items())
						{
							COleDateTime dt(utils::char_to_int64(item.value().value("expire", "")));
							account_info.emplace(utils::utf8_to_utf16(item.value().value("name", "")),
												 utils::utf8_to_utf16(std::format("expired {:d}.{:d}.{:d}", dt.GetDay(), dt.GetMonth(), dt.GetYear())));
						}
					}
				}
			}
			JSON_ALL_CATCH;
		}
		else
		{
			LogProtocol(std::format(L"plugin_vidok: Failed to get account info: {:s}", req.error_message));
		}
	}
}

void plugin_vidok::fill_servers_list(TemplateParams& params)
{
	if (params.creds.login.empty() || params.creds.password.empty() || !get_servers_list().empty())
		return;

	std::vector<DynamicParamsInfo> servers;

	get_api_token(params);

	const auto& url = std::format(API_COMMAND_URL, L"settings");
	utils::http_request req{ replace_params_vars(params, url) };
	if (utils::AsyncDownloadFile(req).get())
	{
		JSON_ALL_TRY;
		{
			const auto& parsed_json = nlohmann::json::parse(req.body.str());
			if (parsed_json.contains("settings"))
			{
				const auto& settings = parsed_json["settings"];

				const auto& current = utils::get_json_wstring("id", settings["current"]["server"]);

				for (const auto& item : settings["lists"]["servers"].items())
				{
					const auto& server = item.value();
					DynamicParamsInfo info{ utils::get_json_string("id", server), utils::get_json_string("name", server) };
					if (info.get_id() == current)
						params.creds.server_id = (int)servers.size();

					servers.emplace_back(info);
				}
			}
		}
		JSON_ALL_CATCH;
	}
	else
	{
		LogProtocol(std::format(L"plugin_vidok: Failed to get account info: {:s}", req.error_message));
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
		get_api_token(params);

		auto url = std::format(API_COMMAND_URL, L"settings_set");
		url += std::format(PARAM_FMT, L"server", REPL_SERVER_ID);

		utils::http_request req{ replace_params_vars(params, url) };
		if (utils::AsyncDownloadFile(req).get())
		{
			JSON_ALL_TRY;
			{
				const auto& parsed_json = nlohmann::json::parse(req.body.str());
				for (const auto& item : parsed_json["settings"].items())
				{
					const auto& server = item.value();
					return (utils::get_json_wstring("value", server) == servers_list[params.creds.server_id].get_id()); //-V612
				}
			}
			JSON_ALL_CATCH;
		}
		else
		{
			LogProtocol(std::format(L"plugin_vidok: Failed to set server: {:s}", req.error_message));
		}
	}

	return false;
}
