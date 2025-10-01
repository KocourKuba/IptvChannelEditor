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
#include "plugin_tvclub.h"
#include "Constants.h"

#include "UtilsLib\md5.h"
#include "UtilsLib\utils.h"
#include "UtilsLib\inet_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr auto API_COMMAND_URL = L"{{API_URL}}/{:s}?token={{S_TOKEN}}";
constexpr auto PARAM_FMT = L"&{:s}={:s}";

std::string plugin_tvclub::get_api_token(TemplateParams& params)
{
	params.creds.s_token = utils::md5_hash_hex(params.creds.login + utils::md5_hash_hex(params.creds.password));
	return params.creds.s_token;
}

void plugin_tvclub::parse_account_info(TemplateParams& params)
{
	if (account_info.empty())
	{
		utils::http_request req{ replace_params_vars(params, std::format(API_COMMAND_URL, L"account")) };
		if (utils::DownloadFile(req))
		{
			JSON_ALL_TRY
			{
				const auto& parsed_json = nlohmann::json::parse(req.body.str());
				if (parsed_json.contains("account"))
				{
					const auto& js_account = parsed_json["account"];
					if (js_account.contains("info"))
					{
						const auto& js_info = js_account["info"];
						set_json_info("login", js_info, account_info);
						set_json_info("balance", js_info, account_info);
					}

					if (js_account.contains("settings"))
					{
						const auto& js_settings = js_account["settings"];
						set_json_info("server_name", js_settings, account_info);
						set_json_info("tz_name", js_settings, account_info);
						set_json_info("tz_gmt", js_settings, account_info);
					}

					if (js_account.contains("services"))
					{
						for (auto& item : js_account["services"].items())
						{
							const auto& val = item.value();
							COleDateTime dt((time_t)val.value("expire", 0));
							const auto& value = std::format(L"expired {:d}.{:d}.{:d}", dt.GetDay(), dt.GetMonth(), dt.GetYear());
							const auto& name = utils::utf8_to_utf16(std::format("{:s} {:s}", val.value("name", ""), val.value("type", "")));
							account_info.emplace(name, value);
						}
					}
				}
			}
			JSON_ALL_CATCH
		}
		else
		{
			LOG_PROTOCOL(std::format(L"plugin_tvclub: Failed to get account info: {:s}", req.error_message));
		}
	}
}

void plugin_tvclub::fill_servers_list(TemplateParams& params)
{
	if (params.creds.login.empty() || params.creds.password.empty() || !get_servers_list().empty())
		return;

	std::vector<DynamicParamsInfo> servers;

	get_api_token(params);

	utils::http_request req{replace_params_vars(params, std::format(API_COMMAND_URL, L"settings"))};
	if (utils::DownloadFile(req))
	{
		JSON_ALL_TRY
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
		JSON_ALL_CATCH
	}

	set_servers_list(servers);
}

bool plugin_tvclub::set_server(TemplateParams& params)
{
	if (servers_list.empty())
	{
		fill_servers_list(params);
	}

	if (!servers_list.empty())
	{
		get_api_token(params);

		utils::http_request req{ replace_params_vars(params, std::format(API_COMMAND_URL, L"set") + std::format(PARAM_FMT, L"server", REPL_SERVER_ID)) };
		if (utils::DownloadFile(req))
		{
			JSON_ALL_TRY
			{
				const auto& parsed_json = nlohmann::json::parse(req.body.str());
				return utils::get_json_int("updated", parsed_json["settings"]) == 1;
			}
			JSON_ALL_CATCH
		}
	}

	return false;
}
