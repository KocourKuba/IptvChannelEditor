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
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto API_COMMAND_AUTH = L"{:s}?userLogin={:s}&userPasswd={:s}";
static constexpr auto API_COMMAND_GET_URL = L"{:s}/?apiAction={:s}&sessionId={:s}";
static constexpr auto API_COMMAND_SET_URL = L"{:s}/?apiAction={:s}&{:s}={:s}&sessionId={:s}";
static constexpr auto SESSION_ID = L"session_id";

bool plugin_tvteam::get_api_token(TemplateParams& params)
{
	if (params.creds.login.empty())
	{
		return false;
	}

	const auto& session_id = get_file_cookie(SESSION_ID);
	if (!session_id.empty())
	{
		return true;
	}

	CWaitCursor cur;
	std::stringstream data;
	const std::wstring& url = fmt::format(API_COMMAND_AUTH, get_provider_api_url(), params.creds.get_login(), utils::utf8_to_utf16(utils::md5_hash_hex(params.creds.password)));
	if (download_url(url, data))
	{
		JSON_ALL_TRY;
		{
			const auto& parsed_json = nlohmann::json::parse(data.str());
			if (parsed_json.contains("status") && parsed_json["status"] == 1)
			{
				set_file_cookie(SESSION_ID, utils::get_json_string("sessionId", parsed_json["data"]), time(nullptr) + 86400);

				if (!params.creds.s_token.empty())
				{
					params.creds.s_token.clear();
				}
				return true;
			}
		}
		JSON_ALL_CATCH;
	}

	return false;
}

void plugin_tvteam::parse_account_info(TemplateParams& params)
{
	const auto& session_id = get_file_cookie(SESSION_ID);
	if (session_id.empty())
	{
		return;
	}

	if (account_info.empty())
	{
		CWaitCursor cur;
		const auto& url = fmt::format(API_COMMAND_GET_URL, get_provider_api_url(), L"getUserData", utils::utf8_to_utf16(session_id));
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
			else if (parsed_json.contains("data") && parsed_json["data"].contains("userData"))
			{
				const auto& js_account = parsed_json["data"]["userData"];
				set_json_info("userLogin", js_account, account_info);
				set_json_info("userEmail", js_account, account_info);
				set_json_info("userToken", js_account, account_info);
				set_json_info("groupId", js_account, account_info);
				set_json_info("userIsTimed", js_account, account_info);
				set_json_info("userBalance", js_account, account_info);

				success = true;
			}
		}
		JSON_ALL_CATCH;
		if (!success)
		{
			return;
		}
	}

	params.creds.set_s_token(account_info[L"userToken"]);
	params.server_id = utils::utf16_to_utf8(account_info[L"groupId"]);

}

void plugin_tvteam::fill_servers_list(TemplateParams& params)
{
	const auto& session_id = get_file_cookie(SESSION_ID);
	if (session_id.empty())
	{
		return;
	}


	if (!servers_list.empty())
	{
		sync_server_id(params);
		return;
	}

	const auto& url = fmt::format(API_COMMAND_GET_URL, get_provider_api_url(), L"getServersGroups", utils::utf8_to_utf16(session_id));

	CWaitCursor cur;

	std::vector<DynamicParamsInfo> servers;
	std::stringstream data;
	if (download_url(url, data))
	{
		JSON_ALL_TRY;
		{
			const auto& parsed_json = nlohmann::json::parse(data.str());
			if (parsed_json.contains("data") && parsed_json["data"].contains("serversGroupsList"))
			{
				int idx = 0;
				for (const auto& item : parsed_json["data"]["serversGroupsList"].items())
				{
					const auto& server = item.value();
					const auto& server_name = fmt::format("{:s} ({:s}", utils::get_json_string("streamDomainName", server), utils::get_json_string("portalDomainName", server));
					const auto& server_id = utils::get_json_string("groupId", server);
					DynamicParamsInfo info{ server_id, server_name };
					servers.emplace_back(info);
					++idx;
				}
			}
		}
		JSON_ALL_CATCH;
	}

	sync_server_id(params);
	set_servers_list(servers);
}

bool plugin_tvteam::set_server(TemplateParams& params)
{
	const auto& session_id = get_file_cookie(SESSION_ID);
	if (!servers_list.empty() && !session_id.empty())
	{

		const auto& url = fmt::format(API_COMMAND_SET_URL,
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

void plugin_tvteam::sync_server_id(TemplateParams& params)
{
	if (servers_list.empty() || params.server_id.empty())
	{
		return;
	}

	for (int i = 0; i < (int)servers_list.size(); ++i)
	{
		if (servers_list[i].id == params.server_id)
		{
			params.creds.server_id = i;
			break;
		}
	}
}
