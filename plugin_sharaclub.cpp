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
#include "plugin_sharaclub.h"
#include "Constants.h"
#include "AccountSettings.h"

#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// API documentation https://list.playtv.pro/api/players.txt

constexpr auto API_COMMAND_URL = L"{{API_URL}}?a={:s}&u={{LOGIN}}-{{PASSWORD}}&source=dune_editor";
constexpr auto PARAM_FMT = L"&{:s}={:s}";

std::wstring plugin_sharaclub::get_playlist_url(const TemplateParams& params, std::wstring url /* = L"" */)
{
	url = get_playlist_info(params.playlist_idx).get_pl_template(); //-V763
	if (params.creds.profile_id != 0)
	{
		const auto& profiles = get_profiles_list();
		url += L"/" + profiles[params.creds.profile_id].get_id();
	}

	return base_plugin::get_playlist_url(params, url);
}

void plugin_sharaclub::parse_account_info(TemplateParams& params)
{
	if (account_info.empty())
	{
		const auto& url = std::format(API_COMMAND_URL, L"subscr_info");
		utils::http_request req{ replace_params_vars(params, url) };

		if (utils::DownloadFile(req))
		{
			JSON_ALL_TRY;
			{
				const auto& parsed_json = nlohmann::json::parse(req.body.str());
				if (parsed_json.contains("status"))
				{
					account_info.emplace(L"state", utils::utf8_to_utf16(parsed_json.value("status", "")));
				}

				if (parsed_json.contains("data"))
				{
					const auto& js_data = parsed_json["data"];
					set_json_info("login", js_data, account_info);
					set_json_info("money", js_data, account_info);
					set_json_info("money_need", js_data, account_info);
					set_json_info("abon", js_data, account_info);
					if (domains_list.empty()) {
						domains_list.resize(1);
					}
					domains_list[0] = {"0", js_data["listdomain"].get<std::string>()};
					epg_params[0].epg_domain = js_data["jsonEpgDomain"].get<std::string>();
				}
			}
			JSON_ALL_CATCH;
		}
		else
		{
			LogProtocol(std::format(L"plugin_sharaclub: Failed to get account info: {:s}", req.error_message));
		}
	}
}

void plugin_sharaclub::fill_servers_list(TemplateParams& params)
{
	if (params.creds.login.empty() || params.creds.password.empty() || !get_servers_list().empty())
		return;

	std::vector<DynamicParamsInfo> servers;

	const auto& url = std::format(API_COMMAND_URL, L"ch_cdn");

	utils::http_request req{ replace_params_vars(params, url) };

	if (utils::AsyncDownloadFile(req).get())
	{
		JSON_ALL_TRY;
		{
			const auto& parsed_json = nlohmann::json::parse(req.body.str());
			if (utils::get_json_int("status", parsed_json) == 1)
			{
				params.creds.server_id = utils::get_json_int("current", parsed_json);
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
		auto url = std::format(API_COMMAND_URL, L"ch_cdn");
		url += std::format(PARAM_FMT, L"num", REPL_SERVER_ID);

		utils::http_request req{ replace_params_vars(params, url) };
		if (utils::AsyncDownloadFile(req).get())
		{
			JSON_ALL_TRY;
			{
				const auto& parsed_json = nlohmann::json::parse(req.body.str());
				return utils::get_json_wstring("status", parsed_json) == L"1";
			}
			JSON_ALL_CATCH;
		}
	}

	return false;
}

void plugin_sharaclub::fill_profiles_list(TemplateParams& params)
{
	if (!get_profiles_list().empty() || params.creds.login.empty() || params.creds.password.empty())
		return;

	utils::http_request req{ replace_params_vars(params, std::format(API_COMMAND_URL, L"list_profiles")) };

	if (!utils::AsyncDownloadFile(req).get())
	{
		return;
	}

	JSON_ALL_TRY;
	{
		const auto& parsed_json = nlohmann::json::parse(req.body.str());
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
						params.creds.profile_id = (int)profiles.size();

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
		auto url = std::format(API_COMMAND_URL, L"list_profiles");
		url += std::format(PARAM_FMT, L"num", REPL_PROFILE_ID);

		utils::http_request req{ replace_params_vars(params, url) };
		if (utils::AsyncDownloadFile(req).get())
		{
			JSON_ALL_TRY;
			{
				const auto& parsed_json = nlohmann::json::parse(req.body.str());
				return utils::get_json_wstring("status", parsed_json) == L"1";
			}
			JSON_ALL_CATCH;
		}
	}

	return false;
}

void plugin_sharaclub::parse_vod(const CThreadConfig& config)
{
	auto categories = std::make_unique<vod_category_storage>();

	do
	{
		const auto& all_name = load_string_resource(IDS_STRING_ALL);
		auto all_category = std::make_shared<vod_category>(all_name);
		all_category->name = all_name;
		categories->set_back(all_name, all_category);

		auto cache_ttl = GetConfig().get_chrono(true, REG_MAX_CACHE_TTL);
		utils::http_request req{ config.m_url, cache_ttl };
		if (!utils::AsyncDownloadFile(req).get()) break;

		nlohmann::json parsed_json;
		JSON_ALL_TRY;
		{
			parsed_json = nlohmann::json::parse(req.body.str());
		}
		JSON_ALL_CATCH;

		if (parsed_json.empty()) break;

		config.SendNotifyParent(WM_INIT_PROGRESS, (int)parsed_json.size(), 0);

		int cnt = 0;
		for (const auto& item : parsed_json.items())
		{
			const auto& val = item.value();
			if (val.empty()) continue;

			std::shared_ptr<vod_category> category;
			std::wstring category_name;
			auto movie = std::make_shared<vod_movie>();

			JSON_ALL_TRY;
			category_name = utils::get_json_wstring("category", val);

			if (!categories->tryGet(category_name, category))
			{
				category = std::make_shared<vod_category>(category_name);
				category->name = category_name;
				categories->set_back(category_name, category);
			}

			movie->title = utils::get_json_wstring("name", val);
			movie->id = utils::get_json_wstring("id", val);
			movie->url = utils::get_json_wstring("video", val);

			const auto& info = val["info"];
			if (!info.empty())
			{
				movie->poster_url.set_uri(utils::get_json_wstring("poster", info));
				movie->description = utils::get_json_wstring("plot", info);
				movie->rating = utils::get_json_wstring("rating", info);
				movie->year = utils::get_json_wstring("year", info);
				movie->director = utils::get_json_wstring("director", info);
				movie->casting = utils::get_json_wstring("cast", info);
				movie->age = utils::get_json_wstring("adult", info);
				movie->movie_time = info.value("duration_secs", 0) / 60;

				for (const auto& genre_item : info["genre"].items())
				{
					const auto& vod_title = utils::utf8_to_utf16(genre_item.value().get<std::string>());
					vod_genre_def genre({ vod_title, vod_title });

					movie->genres.set_back(vod_title, genre);
				}

				std::string country;
				for (const auto& country_item : info["country"].items())
				{
					if (!country.empty())
					{
						country += ", ";
					}
					country += country_item.value().get<std::string>();
				}
				movie->country = utils::utf8_to_utf16(country);
			}

			category->movies.set_back(movie->id, movie);
			JSON_ALL_CATCH;

			if (++cnt % 100 == 0)
			{
				config.SendNotifyParent(WM_UPDATE_PROGRESS, cnt, cnt);
				if (::WaitForSingleObject(config.m_hStop, 0) == WAIT_OBJECT_0) break;
			}
		}
	} while (false);

	if (::WaitForSingleObject(config.m_hStop, 0) == WAIT_OBJECT_0)
	{
		categories.reset();
	}

	config.SendNotifyParent(WM_END_LOAD_JSON_PLAYLIST, (WPARAM)categories.release());
}