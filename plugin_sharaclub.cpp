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
#include "plugin_sharaclub.h"
#include "IPTVChannelEditor.h"

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

plugin_sharaclub::plugin_sharaclub()
{
	short_name = "sharaclub";
	provider_api_url = L"http://conf.playtv.pro/api/con8fig.php?source=dune_editor";;
}

void plugin_sharaclub::load_default()
{
	title = "Sharaclub TV";
	name = "sharaclub.tv";
	access_type = AccountAccessType::enLoginPass;

	provider_url = "https://shara.club/";

	PlaylistTemplateInfo vod_info;
	vod_info.set_name(load_string_resource(IDS_STRING_EDEM_STANDARD));
	vod_info.pl_template = "http://{SUBDOMAIN}/kino-full/{LOGIN}-{PASSWORD}";
	vod_templates.emplace_back(vod_info);

	vod_support = true;

	PlaylistTemplateInfo info;
	info.set_name(load_string_resource(IDS_STRING_EDEM_STANDARD));
	info.pl_template = "http://{SUBDOMAIN}/tv_live-m3u8/{LOGIN}-{PASSWORD}";
	playlist_templates.emplace_back(info);

	uri_parse_pattern = R"(^https?:\/\/(?<domain>.+)\/live\/(?<token>.+)\/(?<id>.+)\/.+\.m3u8$)";

	streams_config[0].cu_type = CatchupType::cu_append;
	streams_config[0].uri_template = "http://{DOMAIN}/live/{TOKEN}/{ID}/video.m3u8";
	streams_config[0].uri_arc_template = "{CU_SUBST}={START}";

	streams_config[1].cu_type = CatchupType::cu_append;
	streams_config[1].cu_subst = "utc";
	streams_config[1].uri_template = "http://{DOMAIN}/live/{TOKEN}/{ID}.ts";
	streams_config[1].uri_arc_template = "{CU_SUBST}={START}";

	auto& params = epg_params[0];
	params.epg_root = "";
	params.epg_url = "http://{DOMAIN}/get/?type=epg&ch={EPG_ID}";
}

std::wstring plugin_sharaclub::get_playlist_url(TemplateParams& params, std::wstring /*url = L""*/)
{
	auto& url = get_current_playlist_template();
	if (params.profile_idx != 0)
	{
		fill_profiles_list(params);
		const auto& profiles = get_profiles_list();
		url += L"/" + profiles[params.profile_idx].get_id();
	}

	return base_plugin::get_playlist_url(params, url);
}

bool plugin_sharaclub::parse_access_info(TemplateParams& params, std::list<AccountInfo>& info_list)
{
	static constexpr auto ACCOUNT_TEMPLATE = L"http://{:s}/api/dune-api5m.php?subscr={:s}-{:s}";
	std::stringstream data;
	if (!utils::CurlDownload(fmt::format(ACCOUNT_TEMPLATE, params.subdomain, params.login, params.password), data))
	{
		return false;
	}

	JSON_ALL_TRY;
	{
		const auto& parsed_json = nlohmann::json::parse(data.str());
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

void plugin_sharaclub::fill_servers_list(TemplateParams& params)
{
	if (params.login.empty() || params.password.empty() || !get_servers_list().empty())
		return;

	std::vector<DynamicParamsInfo> servers;

	const auto& url = fmt::format(API_COMMAND_GET_URL,
									params.subdomain,
									L"ch_cdn",
									params.login,
									params.password);
	std::stringstream data;
	if (utils::CurlDownload(url, data))
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
									  params.subdomain,
									  L"ch_cdn",
									  L"num",
									  servers_list[params.server_idx].get_id(),
									  params.login,
									  params.password);

		std::stringstream data;
		if (utils::CurlDownload(url, data))
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
									params.subdomain,
									L"list_profiles",
									params.login,
									params.password);
	std::stringstream data;
	if (!utils::CurlDownload(url, data))
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
						params.profile_idx = profiles.size();

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
									  params.subdomain,
									  L"list_profiles",
									  L"num",
									  profiles_list[params.profile_idx].get_id(),
									  params.login,
									  params.password);

		std::stringstream data;
		if (utils::CurlDownload(url, data))
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
