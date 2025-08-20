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
#include "plugin_cbilling.h"
#include "vod_movie.h"
#include "AccountSettings.h"
#include "Constants.h"
#include "UtilsLib\inet_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr auto API_COMMAND_AUTH = L"{API_URL}/auth/info";
constexpr auto ACCOUNT_HEADER_TEMPLATE = "x-public-key: {:s}";

void plugin_cbilling::parse_account_info(TemplateParams& params)
{
	/*
	{
		"data": {
			"public_token": "2f5787bd53fcaeeae27ba3ed3669babc",
			"private_token": "5acf87d0206da05b73f8923a703gf666",
			"end_time": 1706129968,
			"end_date": "2024-01-24 23:59:28",
			"devices_num": 1,
			"server": "s01.wsbof.com",
			"vod": true,
			"ssl": false,
			"disable_adult": false
	}
}	*/

	if (account_info.empty())
	{
		utils::http_request req{ replace_params_vars(params, API_COMMAND_AUTH) };
		req.headers.emplace_back("accept: */*");
		req.headers.emplace_back(std::format(ACCOUNT_HEADER_TEMPLATE, params.creds.password));
		if (utils::AsyncDownloadFile(req).get())
		{
			JSON_ALL_TRY
			{
				const auto& parsed_json = nlohmann::json::parse(req.body.str());
				if (parsed_json.contains("data"))
				{
					const auto& js_data = parsed_json["data"];

					set_json_info("package", js_data, account_info);
					set_json_info("end_date", js_data, account_info);
					set_json_info("devices_num", js_data, account_info);
					set_json_info("server", js_data, account_info);
					set_json_info("vod", js_data, account_info);
					set_json_info("ssl", js_data, account_info);
					set_json_info("public_token", js_data, account_info);
					set_json_info("private_token", js_data, account_info);

					params.creds.set_subdomain(account_info[L"server"]);
					params.creds.set_s_token(account_info[L"private_token"]);
				}
			}
			JSON_ALL_CATCH;
		}
	}
	else
	{
		LogProtocol(L"plugin_cbilling: Failed to account info: account info is empty");
	}
}

void plugin_cbilling::parse_vod(ThreadConfig config)
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

		int total = 0;
		JSON_ALL_TRY;
		{
			const auto& parsed_json = nlohmann::json::parse(req.body.str());
			for (const auto& item_it : parsed_json["data"].items())
			{
				if (item_it.value().empty()) continue;
				const auto& item = item_it.value();

				auto category = std::make_shared<vod_category>();
				category->id = utils::get_json_wstring("id", item);
				category->name = utils::get_json_wstring("name", item);
				total += item["count"].get<int>();
				categories->set_back(category->id, category);
			}
		}
		JSON_ALL_CATCH;

		SendNotifyParent(config.m_parent, WM_INIT_PROGRESS, total, 0);

		int cnt = 0;
		for (const auto& pair : categories->vec())
		{
			const auto& category = pair.second;

			int page = 1;
			int retry = 0;
			for (;;)
			{
				if (::WaitForSingleObject(config.m_hStop, 0) == WAIT_OBJECT_0 || retry > 2) break;

				utils::http_request jreq{ std::format(L"{:s}/cat/{:s}?page={:d}&per_page=200", config.m_url, category->id, page), cache_ttl };
				if (!utils::AsyncDownloadFile(jreq).get())
				{
					retry++;
					continue;
				}

				if (req.body.bad())
				{
					retry++;
					continue;
				}

				nlohmann::json movies_json;
				JSON_ALL_TRY;
				{
					movies_json = nlohmann::json::parse(req.body.str());
				}
				JSON_ALL_CATCH;

				if (movies_json.empty() || !movies_json.contains("data"))
				{
					retry++;
					continue;
				}

				for (const auto& movie_it : movies_json["data"].items())
				{
					const auto& movie_item = movie_it.value();

					JSON_ALL_TRY;
					{
						auto movie = std::make_shared<vod_movie>();

						movie->id = utils::get_json_wstring("id", movie_item);
						movie->title = utils::get_json_wstring("name", movie_item);
						movie->poster_url.set_uri(utils::get_json_wstring("poster", movie_item));
						movie->rating = utils::get_json_wstring("rating", movie_item);
						movie->country = utils::get_json_wstring("country", movie_item);
						movie->year = utils::get_json_wstring("year", movie_item);

						for (const auto& genre_item : movie_item["genres"].items())
						{
							const auto& vod_title = utils::get_json_wstring("title", genre_item.value());
							vod_genre_def genre({ vod_title, vod_title });
							movie->genres.set_back(vod_title, genre);
						}

						category->movies.set_back(movie->id, movie);
					}
					JSON_ALL_CATCH;

					if (++cnt % 100 == 0)
					{
						SendNotifyParent(config.m_parent, WM_UPDATE_PROGRESS, cnt, cnt);
						if (::WaitForSingleObject(config.m_hStop, 0) == WAIT_OBJECT_0) break;
					}
				}

				nlohmann::json meta;
				int last = 0;
				JSON_ALL_TRY;
				{
					meta = movies_json["meta"];
					last = utils::get_json_int("last_page", meta);
				}
				JSON_ALL_CATCH;

				if (page >= last) break;
				page++;
			}
		}

	} while (false);

	if (::WaitForSingleObject(config.m_hStop, 0) == WAIT_OBJECT_0)
	{
		categories.reset();
	}

	SendNotifyParent(config.m_parent, WM_END_LOAD_JSON_PLAYLIST, (WPARAM)categories.release());
}

void plugin_cbilling::fetch_movie_info(const Credentials& creds, vod_movie& movie)
{
	TemplateParams params;
	update_provider_params(params);

	const auto& url = std::format(L"{:s}/video/{:s}", get_vod_url(params), movie.id);
	auto cache_ttl = GetConfig().get_chrono(true, REG_MAX_CACHE_TTL);
	utils::http_request req{ url, cache_ttl };

	if (!utils::AsyncDownloadFile(req).get())
	{
		return;
	}

	JSON_ALL_TRY;
	const auto& parsed_json = nlohmann::json::parse(req.body.str());

	if (parsed_json.contains("data"))
	{
		const auto& value = parsed_json["data"];

		movie.description = utils::get_json_wstring("description", value);
		movie.director = utils::get_json_wstring("director", value);
		movie.casting = utils::get_json_wstring("actors", value);
		movie.movie_time = utils::get_json_int("time", value);
		const auto& adult = utils::get_json_wstring("adult", value);
		if (adult == L"1")
			movie.age += L" 18+";

		if (value.contains("seasons"))
		{
			for (const auto& season_it : value["seasons"].items())
			{
				const auto& season_item = season_it.value();
				vod_season_def season;
				season.id = utils::get_json_wstring("id", season_item);
				season.title = utils::get_json_wstring("name", season_item);
				season.season_id = utils::get_json_wstring("number", season_item);

				for (const auto& episode_it : season_item["series"].items())
				{
					const auto& episode_item = episode_it.value();

					vod_episode_def episode;
					episode.id = utils::get_json_wstring("id", episode_item);
					episode.title = utils::get_json_wstring("name", episode_item);
					episode.episode_id = utils::get_json_wstring("number", episode_item);

					if (episode_item["files"].is_array())
					{
						episode.url = utils::get_json_wstring("url", episode_item["files"].front());
					}

					season.episodes.set_back(episode.id, episode);
				}
				movie.seasons.set_back(season.id, season);
			}
		}
		else
		{
			movie.url = utils::get_json_wstring("url", value["files"][0]);
		}
	}
	JSON_ALL_CATCH;
}

std::wstring plugin_cbilling::get_movie_url(const Credentials& creds, const movie_request& request, const vod_movie& movie)
{
	std::wstring url;
	if (movie.url.empty() && request.season_idx != CB_ERR && request.episode_idx != CB_ERR)
	{
		const auto& season = movie.seasons[request.season_idx];
		url = season.episodes[request.episode_idx].url;
	}
	else
	{
		url = movie.url;
	}

	return std::format(L"http://{:s}{:s}?token={:s}", creds.get_subdomain(), url, creds.get_s_token());
}
