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
#include "uri_sharaclub.h"
#include "UtilsLib\utils.h"
#include "UtilsLib\inet_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// API documentation https://list.playtv.pro/api/players.txt

static constexpr auto API_COMMAND_GET_URL = L"http://{:s}/api/players.php?a={:s}&u={:s}-{:s}&source=dune_editor";
static constexpr auto API_COMMAND_SET_URL = L"http://{:s}/api/players.php?a={:s}&{:s}={:s}&u={:s}-{:s}&source=dune_editor";

void uri_sharaclub::get_playlist_url(std::wstring& url, TemplateParams& params)
{
	url = playlist_template;
	if (params.profile != 0)
	{
		const auto& profiles = get_profiles_list(params);
		url += L"/" + profiles[params.profile].id;
	}

	uri_stream::get_playlist_url(url, params);
}

bool uri_sharaclub::parse_access_info(TemplateParams& params, std::list<AccountInfo>& info_list)
{
	static constexpr auto ACCOUNT_TEMPLATE = L"http://{:s}/api/dune-api5m.php?subscr={:s}-{:s}";
	std::vector<BYTE> data;
	if (!utils::DownloadFile(fmt::format(ACCOUNT_TEMPLATE, params.subdomain, params.login, params.password), data) || data.empty())
	{
		return false;
	}

	JSON_ALL_TRY;
	{
		const auto& parsed_json = nlohmann::json::parse(data.begin(), data.end());
		if (parsed_json.contains("status"))
		{
			AccountInfo info{ L"state", utils::utf8_to_utf16(parsed_json.value("status", "")) };
			info_list.emplace_back(info);
		}

		if (parsed_json.contains("data"))
		{
			const auto& js_data = parsed_json["data"];
			put_account_info("login", js_data, info_list);
			put_account_info("money", js_data, info_list);
			put_account_info("money_need", js_data, info_list);
			put_account_info("abon", js_data, info_list);
		}

		return true;
	}
	JSON_ALL_CATCH;

	return false;
}

const std::vector<ServersInfo>& uri_sharaclub::get_servers_list(TemplateParams& params)
{
	if (servers_list.empty())
	{
		const auto& url = fmt::format(API_COMMAND_GET_URL,
									  params.subdomain,
									  L"ch_cdn",
									  params.login,
									  params.password);
		std::vector<BYTE> data;
		if (utils::DownloadFile(url, data) || data.empty())
		{
			JSON_ALL_TRY;
			{
				const auto& parsed_json = nlohmann::json::parse(data.begin(), data.end());
				if (utils::get_json_int("status", parsed_json) == 1)
				{
					params.server = utils::get_json_int("current", parsed_json);
					const auto& js_data = parsed_json["allow_nums"];
					for (const auto& item : js_data.items())
					{
						const auto& server = item.value();
						ServersInfo info({ utils::get_json_string("id", server), utils::get_json_string("name", server) });
						servers_list.emplace_back(info);
					}
				}
			}
			JSON_ALL_CATCH;
		}
	}
	return servers_list;
}

bool uri_sharaclub::set_server(TemplateParams& params)
{
	const auto& servers = get_servers_list(params);
	if (!servers.empty())
	{
		const auto& url = fmt::format(API_COMMAND_SET_URL,
									  params.subdomain,
									  L"ch_cdn",
									  L"num",
									  servers[params.server].id,
									  params.login,
									  params.password);

		std::vector<BYTE> data;
		if (utils::DownloadFile(url, data) || data.empty())
		{
			JSON_ALL_TRY;
			{
				const auto& parsed_json = nlohmann::json::parse(data.begin(), data.end());
				return utils::get_json_string("status", parsed_json) == L"1";
			}
			JSON_ALL_CATCH;
		}
	}

	return false;
}

const std::vector<ProfilesInfo>& uri_sharaclub::get_profiles_list(TemplateParams& params)
{
	if (profiles_list.empty())
	{
		const auto& url = fmt::format(API_COMMAND_GET_URL,
									  params.subdomain,
									  L"list_profiles",
									  params.login,
									  params.password);
		std::vector<BYTE> data;
		if (utils::DownloadFile(url, data) || data.empty())
		{
			JSON_ALL_TRY;
			{
				const auto& parsed_json = nlohmann::json::parse(data.begin(), data.end());
				if (utils::get_json_int("status", parsed_json) == 1)
				{
					const auto& current = utils::get_json_string("current", parsed_json);
					ProfilesInfo none_info({ L"0", L"None" });
					profiles_list.emplace_back(none_info);
					if (parsed_json.contains("profiles"))
					{
						for (const auto& item : parsed_json["profiles"].items())
						{
							const auto& profile = item.value();
							ProfilesInfo info({ utils::get_json_string("id", profile), utils::get_json_string("name", profile) });
							if (info.id == current)
								params.profile = profiles_list.size();

							profiles_list.emplace_back(info);
						}
					}
				}
			}
			JSON_ALL_CATCH;
		}
	}
	return profiles_list;
}

bool uri_sharaclub::set_profile(TemplateParams& params)
{
	const auto& profiles = get_profiles_list(params);
	if (!profiles.empty())
	{
		const auto& url = fmt::format(API_COMMAND_SET_URL,
									  params.subdomain,
									  L"list_profiles",
									  L"num",
									  profiles[params.profile].id,
									  params.login,
									  params.password);

		std::vector<BYTE> data;
		if (utils::DownloadFile(url, data) || data.empty())
		{
			JSON_ALL_TRY;
			{
				const auto& parsed_json = nlohmann::json::parse(data.begin(), data.end());
				return utils::get_json_string("status", parsed_json) == L"1";
			}
			JSON_ALL_CATCH;
		}
	}

	return false;
}
