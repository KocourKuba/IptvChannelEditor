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
#include "Constants.h"
#include "AccountSettings.h"

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
	url = get_playlist_info(params.playlist_idx).get_pl_template(); //-V763
	if (params.profile_idx != 0)
	{
		const auto& profiles = get_profiles_list();
		url += L"/" + profiles[params.profile_idx].get_id();
	}

	return base_plugin::get_playlist_url(params, url);
}

void plugin_sharaclub::parse_account_info(Credentials& creds)
{
	if (account_info.empty())
	{
		CWaitCursor cur;
		const auto& url = fmt::format(API_COMMAND_GET_URL, get_provider_api_url(), L"subscr_info", creds.get_login(), creds.get_password());
		std::stringstream data;
		if (download_url(url, data))
		{
			JSON_ALL_TRY;
			{
				const auto& parsed_json = nlohmann::json::parse(data.str());
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
				}
			}
			JSON_ALL_CATCH;
		}
	}
}

void plugin_sharaclub::fill_servers_list(TemplateParams& params)
{
	if (params.creds.login.empty() || params.creds.password.empty() || !get_servers_list().empty())
		return;

	std::vector<DynamicParamsInfo> servers;

	const auto& url = fmt::format(API_COMMAND_GET_URL,
									get_provider_api_url(),
									L"ch_cdn",
									params.creds.get_login(),
									params.creds.get_password());

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
									  params.creds.get_login(),
									  params.creds.get_password());

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
	if (!get_profiles_list().empty() || params.creds.login.empty() || params.creds.password.empty())
		return;

	const auto& url = fmt::format(API_COMMAND_GET_URL,
								  get_provider_api_url(),
								  L"list_profiles",
								  params.creds.get_login(),
								  params.creds.get_password());

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
									  params.creds.get_login(),
									  params.creds.get_password());

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

void plugin_sharaclub::parse_vod(const CThreadConfig& config)
{
	int cache_ttl = GetConfig().get_int(true, REG_MAX_CACHE_TTL) * 3600;
	auto categories = std::make_unique<vod_category_storage>();

	do
	{
		const auto& all_name = load_string_resource(IDS_STRING_ALL);
		auto all_category = std::make_shared<vod_category>(all_name);
		all_category->name = all_name;
		categories->set_back(all_name, all_category);

		std::stringstream data;
		if (!download_url(config.m_url, data, cache_ttl)) break;

		nlohmann::json parsed_json;
		JSON_ALL_TRY;
		parsed_json = nlohmann::json::parse(data.str());
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