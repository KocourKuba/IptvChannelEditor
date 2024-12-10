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
#include "plugin_tvteam.h"

#include "UtilsLib\md5.h"
#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static constexpr auto API_COMMAND_AUTH = L"{:s}?userLogin={:s}&userPasswd={:s}";

std::string plugin_tvteam::get_api_token(TemplateParams& params)
{
	if (params.creds.login.empty() || params.creds.password.empty())
	{
		return {};
	}

	session_id_name = utils::utf8_to_utf16(fmt::format("session_{:s}", utils::md5_hash_hex(params.creds.login)));

	auto session_id = get_file_cookie(session_id_name);
	if (!session_id.empty())
	{
		return session_id;
	}

	CWaitCursor cur;
	std::stringstream data;
	const std::wstring& url = fmt::format(API_COMMAND_AUTH,
										  get_provider_api_url(),
										  params.creds.get_login(),
										  utils::utf8_to_utf16(utils::md5_hash_hex(params.creds.password)));
	if (download_url(url, data))
	{
		JSON_ALL_TRY;
		{
			const auto& parsed_json = nlohmann::json::parse(data.str());
			if (parsed_json.contains("status") && parsed_json["status"] == 1)
			{
				session_id = utils::get_json_string("sessionId", parsed_json["data"]);
				set_file_cookie(session_id_name, session_id, time(nullptr) + 86400*6);
			}
			else
			{
				session_id.clear();
				delete_file_cookie(session_id_name);
			}
		}
		JSON_ALL_CATCH;
	}

	return session_id;
}

void plugin_tvteam::parse_account_info(TemplateParams& params)
{
	const auto& session_id = get_file_cookie(session_id_name);
	if (session_id.empty())
	{
		return;
	}

	if (!account_info.empty())
	{
		if (params.creds.s_token.empty())
		{
			params.creds.set_s_token(account_info[L"userToken"]);
		}
	}
	else
	{
		CWaitCursor cur;
		const auto& url = fmt::format(L"{:s}/?apiAction={:s},{:s},{:s}&sessionId={:s}",
									  get_provider_api_url(),
									  L"getUserData",
									  L"getServersGroups",
									  L"getUserPackages",
									  utils::utf8_to_utf16(session_id));
		std::stringstream data;
		if (!download_url(url, data))
		{
			return;
		}

		bool success = false;
		JSON_ALL_TRY;
		{
			const auto& parsed_json = nlohmann::json::parse(data.str());
			if (parsed_json.contains("status") && parsed_json["status"] == 0)
			{
				set_json_info("error", parsed_json, account_info);
			}
			else
			{
				std::string selected_server_id;
				if (parsed_json.contains("data") && parsed_json["data"].contains("userData"))
				{
					const auto& js_account = parsed_json["data"]["userData"];
					set_json_info("userLogin", js_account, account_info);
					set_json_info("userEmail", js_account, account_info);
					set_json_info("userBalance", js_account, account_info);
					set_json_info("showPorno", js_account, account_info);
					params.creds.s_token = utils::get_json_string("userToken", js_account);
					selected_server_id = utils::get_json_string("groupId", js_account);
				}

				if (parsed_json.contains("data") && parsed_json["data"].contains("userPackagesList"))
				{
					for (const auto& item : parsed_json["data"]["userPackagesList"].items())
					{
						const auto& package = item.value();
						set_json_info("packageName", package, account_info);
						set_json_info("fromDate", package, account_info);
						set_json_info("toDate", package, account_info);
						set_json_info("salePrice", package, account_info);
					}
				}

				std::vector<DynamicParamsInfo> servers;
				if (parsed_json.contains("data") && parsed_json["data"].contains("serversGroupsList"))
				{
					int idx = 0;
					for (const auto& item : parsed_json["data"]["serversGroupsList"].items())
					{
						const auto& server = item.value();
						const auto& status = utils::get_json_string("showStatus", server);
						const auto& server_id = utils::get_json_string("groupId", server);
						const auto& server_name = fmt::format("{:s} ({:s})",
															  utils::get_json_string("groupCountry", server),
															  utils::get_json_string("streamDomainName", server));

						if (status != "1" && server_id != selected_server_id) continue;

						if (server_id == selected_server_id)
						{
							params.creds.server_id = idx;
						}

						DynamicParamsInfo info{ server_id, server_name };
						servers.emplace_back(info);
						++idx;
					}
					set_servers_list(servers);
				}


				success = true;
			}
		}
		JSON_ALL_CATCH;
	}
}

bool plugin_tvteam::set_server(TemplateParams& params)
{
	const auto& session_id = get_file_cookie(session_id_name);
	if (!servers_list.empty() && !session_id.empty())
	{

		const auto& url = fmt::format(L"{:s}/?apiAction={:s}&{:s}={:s}&sessionId={:s}",
									  get_provider_api_url(),
									  L"updateUserData",
									  L"groupId",
									  servers_list[params.creds.server_id].get_id(),
									  utils::utf8_to_utf16(session_id));

		CWaitCursor cur;
		std::stringstream data;
		if (download_url(url, data))
		{
			JSON_ALL_TRY;
			{
				const auto& parsed_json = nlohmann::json::parse(data.str());
				if (utils::get_json_int("status", parsed_json) == 1)
				{
					return true;
				}

				params.error_string = utils::get_json_wstring("error", parsed_json);
			}
			JSON_ALL_CATCH;
		}
	}

	return false;
}
