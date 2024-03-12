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
#include "plugin_sharaclub.h"

#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// API documentation https://list.playtv.pro/api/players.txt

static constexpr auto API_COMMAND_GET_URL = L"{:s}/api/players.php?a={:s}&u={:s}-{:s}&source=dune_editor";
static constexpr auto API_COMMAND_SET_URL = L"{:s}/api/players.php?a={:s}&{:s}={:s}&u={:s}-{:s}&source=dune_editor";

void plugin_sharaclub::configure_provider_plugin()
{
	base_plugin::configure_provider_plugin();

	CWaitCursor cur;
	std::stringstream data;

	if (provider_api_url.empty())
	{
		if (download_url(L"http://conf.playtv.pro/api/con8fig.php?source=dune_editor", data))
		{
			JSON_ALL_TRY;
			const auto& parsed_json = nlohmann::json::parse(data.str());
			provider_api_url = "http://" + parsed_json["listdomain"].get<std::string>();
			epg_params[0].epg_domain = "http://" + parsed_json["jsonEpgDomain"].get<std::string>();
			JSON_ALL_CATCH;
		}
		else
		{
			AfxMessageBox(get_download_error().c_str(), MB_ICONERROR | MB_OK);
		}
	}
}

std::wstring plugin_sharaclub::get_playlist_url(const TemplateParams& params, std::wstring url /* = L"" */)
{
	url = get_playlist_info(params.playlist_idx).get_pl_template();
	if (params.profile_idx != 0)
	{
		const auto& profiles = get_profiles_list();
		url += L"/" + profiles[params.profile_idx].get_id();
	}

	return base_plugin::get_playlist_url(params, url);
}

std::map<std::wstring, std::wstring, std::less<>> plugin_sharaclub::parse_access_info(const TemplateParams& params)
{
	const auto& url = fmt::format(API_COMMAND_GET_URL, get_provider_api_url(), L"subscr_info", params.login, params.password);

	std::map<std::wstring, std::wstring, std::less<>> info;

	CWaitCursor cur;
	std::stringstream data;
	if (download_url(url, data))
	{
		JSON_ALL_TRY;
		{
			const auto& parsed_json = nlohmann::json::parse(data.str());
			if (parsed_json.contains("status"))
			{
				info.emplace(L"state", utils::utf8_to_utf16(parsed_json.value("status", "")));
			}

			if (parsed_json.contains("data"))
			{
				const auto& js_data = parsed_json["data"];
				set_json_info("login", js_data, info);
				set_json_info("money", js_data, info);
				set_json_info("money_need", js_data, info);
				set_json_info("abon", js_data, info);
			}
		}
		JSON_ALL_CATCH;
	}

	return info;
}

void plugin_sharaclub::fill_servers_list(TemplateParams& params)
{
	if (params.login.empty() || params.password.empty() || !get_servers_list().empty())
		return;

	std::vector<DynamicParamsInfo> servers;

	const auto& url = fmt::format(API_COMMAND_GET_URL,
									get_provider_api_url(),
									L"ch_cdn",
									params.login,
									params.password);

	CWaitCursor cur;
	std::stringstream data;
	if (download_url(url, data))
	{
		JSON_ALL_TRY;
		{
			const auto& parsed_json = nlohmann::json::parse(data.str());
			if (utils::get_json_int("status", parsed_json) == 1)
			{
				params.server_idx = utils::get_json_int("current", parsed_json);
				const auto& js_data = parsed_json["allow_nums"];
				for (const auto& item : js_data.items())
				{
					const auto& server = item.value();
					DynamicParamsInfo info{ utils::get_json_string("id", server), utils::get_json_string("name", server) };
					servers.emplace_back(info);
				}
			}
		}
		JSON_ALL_CATCH;
	}

	set_servers_list(servers);
}

bool plugin_sharaclub::set_server(TemplateParams& params)
{
	if (servers_list.empty())
	{
		fill_servers_list(params);
	}

	if (!servers_list.empty())
	{
		const auto& url = fmt::format(API_COMMAND_SET_URL,
									  get_provider_api_url(),
									  L"ch_cdn",
									  L"num",
									  servers_list[params.server_idx].get_id(),
									  params.login,
									  params.password);

		CWaitCursor cur;
		std::stringstream data;
		if (download_url(url, data))
		{
			JSON_ALL_TRY;
			{
				const auto& parsed_json = nlohmann::json::parse(data.str());
				return utils::get_json_wstring("status", parsed_json) == L"1";
			}
			JSON_ALL_CATCH;
		}
	}

	return false;
}

void plugin_sharaclub::fill_profiles_list(TemplateParams& params)
{
	if (!get_profiles_list().empty() || params.login.empty() || params.password.empty())
		return;

	const auto& url = fmt::format(API_COMMAND_GET_URL,
								  get_provider_api_url(),
								  L"list_profiles",
								  params.login,
								  params.password);

	CWaitCursor cur;
	std::stringstream data;
	if (!download_url(url, data))
		return;

	JSON_ALL_TRY;
	{
		const auto& parsed_json = nlohmann::json::parse(data.str());
		if (utils::get_json_int("status", parsed_json) == 1)
		{
			const auto& current = utils::get_json_wstring("current", parsed_json);
			std::vector<DynamicParamsInfo> profiles;
			DynamicParamsInfo none_info({ "0", "None" });
			profiles.emplace_back(none_info);
			if (parsed_json.contains("profiles"))
			{
				for (const auto& item : parsed_json["profiles"].items())
				{
					const auto& profile = item.value();
					DynamicParamsInfo info;
					info.set_id(utils::get_json_wstring("id", profile));
					info.set_name(utils::get_json_wstring("name", profile));
					if (info.get_id() == current)
						params.profile_idx = (int)profiles.size();

					profiles.emplace_back(info);
				}
				set_profiles_list(profiles);
			}
		}
	}
	JSON_ALL_CATCH;
}

bool plugin_sharaclub::set_profile(TemplateParams& params)
{
	if (!profiles_list.empty())
	{
		const auto& url = fmt::format(API_COMMAND_SET_URL,
									  get_provider_api_url(),
									  L"list_profiles",
									  L"num",
									  profiles_list[params.profile_idx].get_id(),
									  params.login,
									  params.password);

		CWaitCursor cur;
		std::stringstream data;
		if (download_url(url, data))
		{
			JSON_ALL_TRY;
			{
				const auto& parsed_json = nlohmann::json::parse(data.str());
				return utils::get_json_wstring("status", parsed_json) == L"1";
			}
			JSON_ALL_CATCH;
		}
	}

	return false;
}
