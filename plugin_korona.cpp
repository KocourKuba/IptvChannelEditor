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
#include "plugin_korona.h"
#include "Constants.h"
#include "AccountSettings.h"

#include "UtilsLib\utils.h"
#include "UtilsLib\md5.h"
#include "UtilsLib\inet_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr auto API_COMMAND_AUTH = L"{API_URL}/auth/";
constexpr auto API_COMMAND_INFO = L"{API_URL}/xapi10/accountinfo";
constexpr auto API_COMMAND_SERVERS = L"{API_URL}/xapi10/tv/servers";

constexpr auto PARAM_FMT = "{:s}={:s}";
constexpr auto SESSION_TOKEN_TEMPLATE = "session_token_{:s}";

std::string plugin_korona::get_api_token(TemplateParams& params)
{
	session_token_file = utils::utf8_to_utf16(std::format(SESSION_TOKEN_TEMPLATE,
														  utils::md5_hash_hex(params.creds.login + utils::md5_hash_hex(params.creds.password))));

	auto session_token = get_file_cookie(session_token_file);
	std::map<std::string, std::string> post_request;
	if (session_token.empty() && !params.creds.token.empty())
	{
		post_request["grant_type"] = "refresh_token";
		post_request["refresh_token"] = params.creds.token;
	}
	else if (params.creds.token.empty())
	{
		post_request["grant_type"] = "password";
		post_request["username"] = params.creds.login;
		post_request["password"] = params.creds.password;
	}
	else
	{
		return session_token;
	}

	utils::http_request req
	{
		.url = replace_params_vars(params, API_COMMAND_AUTH),
		.headers { "accept: */*",  "Content-Type: application/x-www-form-urlencoded" },
		.verb_post = true
	};

	for (const auto& [key, value] : post_request)
	{
		if (!req.post_data.empty())
		{
			req.post_data += "&";
		}
		req.post_data += std::format(PARAM_FMT, key, utils::encodeURIComponent(value));
	}


	if (utils::AsyncDownloadFile(req).get())
	{
		JSON_ALL_TRY
		{
			const auto& parsed_json = nlohmann::json::parse(req.body.str());
			if (utils::get_json_string("error", parsed_json).empty())
			{
				session_token = utils::get_json_string("access_token", parsed_json);
				set_file_cookie(session_token_file, session_token,
								 time(nullptr) + utils::get_json_int("expires_in", parsed_json));
				params.creds.token = utils::get_json_string("refresh_token", parsed_json);
				return session_token;
			}
			else
			{
				delete_file_cookie(session_token_file);
				params.creds.token.clear();
				session_token.clear();
			}
		}
		JSON_ALL_CATCH
	}
	else
	{
		LOG_PROTOCOL(std::format(L"plugin_korona: Failed to get token: {:s}", req.error_message));
	}

	return session_token;
}

void plugin_korona::parse_account_info(TemplateParams& params)
{
	if (account_info.empty())
	{
		const auto& js_data = server_request(replace_params_vars(params, API_COMMAND_INFO));

		JSON_ALL_TRY
		{
			set_json_info("balance", js_data, account_info);
			set_json_info("expiry_data", js_data, account_info);
			set_json_info("number_additional_accounts", js_data, account_info);
			set_json_info("active", js_data, account_info);

			if (js_data.contains("tariff"))
			{
				const auto& tariff = js_data["tariff"];

				set_json_info("period", tariff, account_info);
				set_json_info("currency", tariff, account_info);
				set_json_info("name", tariff, account_info);
				set_json_info("base_price", tariff, account_info);
				set_json_info("additional_account_price", tariff, account_info);
				set_json_info("full_price", tariff, account_info);
			}
		}
		JSON_ALL_CATCH
	}
}

void plugin_korona::fill_servers_list(TemplateParams& params)
{
	if (!get_servers_list().empty())
		return;

	std::vector<DynamicParamsInfo> servers;

	JSON_ALL_TRY
	{
		const auto& parsed_json = server_request(replace_params_vars(params, API_COMMAND_SERVERS));
		if (parsed_json.contains("data"))
		{
			int idx = 0;
			for (auto& item : parsed_json["data"].items())
			{
				const auto& server = item.value();
				DynamicParamsInfo info{ utils::get_json_string("id", server), utils::get_json_string("title", server) };
				if (utils::get_json_bool("selected", server))
				{
					params.creds.server_id = idx;
				}
				servers.emplace_back(info);
				idx++;
			}
		}
	}
	JSON_ALL_CATCH

	set_servers_list(servers);
}

void plugin_korona::parse_vod(const ThreadConfig& config)
{
	auto categories = std::make_unique<vod_category_storage>();
	const auto& all_name = load_string_resource(IDS_STRING_ALL);
	auto all_category = std::make_shared<vod_category>(all_name);
	all_category->name = all_name;
	categories->set_back(all_name, all_category);

	do
	{
		const auto& api_url = get_vod_url(config.m_params);

		const auto& url = std::format(L"{:s}/cat", api_url);
		int total = 0;
		JSON_ALL_TRY
		{
			nlohmann::json parsed_json = server_request(url, true);
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
		JSON_ALL_CATCH

		SendNotifyParent(config.m_parent, WM_INIT_PROGRESS, total, 0);

		int cnt = 0;
		for (const auto& pair : categories->vec())
		{
			const auto& category = pair.second;

			for (;;)
			{
				const auto& cat_url = std::format(L"{:s}/cat/{:s}?page=1&per_page=99999999", api_url, category->id);
				nlohmann::json genres_json = server_request(cat_url, true);
				if (!genres_json.contains("data")) break;

				for (const auto& movie_it : genres_json["data"].items())
				{
					const auto& movie_item = movie_it.value();

					JSON_ALL_TRY
					{
						auto movie = std::make_shared<vod_movie>();

						movie->id = utils::get_json_wstring("id", movie_item);
						movie->title = utils::get_json_wstring("name", movie_item);
						movie->poster_url.set_uri(utils::get_json_wstring("poster", movie_item));
						movie->rating = utils::get_json_wstring("rating", movie_item);
						movie->country = utils::get_json_wstring("country", movie_item);
						movie->year = utils::get_json_wstring("year", movie_item);
						movie->movie_time = utils::get_json_int("time", movie_item);

						if (movie_item.contains("genres"))
						{
							for (const auto& genre_item : movie_item["genres"].items())
							{
								const auto& genre_id = utils::get_json_wstring("title", genre_item.value());
								const auto& genre_title = utils::get_json_wstring("title", genre_item.value());
								vod_genre_def genre({ genre_id, genre_title });
								movie->genres.set_back(genre_id, genre);
								category->genres.set_back(genre_id, genre);
							}
						}

						category->movies.set_back(movie->id, movie);
					}
					JSON_ALL_CATCH

					if (++cnt % 100 == 0)
					{
						SendNotifyParent(config.m_parent, WM_UPDATE_PROGRESS, cnt, cnt);
						if (::WaitForSingleObject(config.m_hStop, 0) == WAIT_OBJECT_0) break;
					}
				}

				break;
			}
		}

	} while (false);

	if (::WaitForSingleObject(config.m_hStop, 0) == WAIT_OBJECT_0)
	{
		categories.reset();
	}

	SendNotifyParent(config.m_parent, WM_END_LOAD_JSON_PLAYLIST, (WPARAM)categories.release());
}

void plugin_korona::fetch_movie_info(const Credentials& creds, vod_movie& movie)
{
	TemplateParams params
	{
		.creds = creds
	};
	update_provider_params(params);

	const auto& url = std::format(L"{:s}/video/{:s}", get_vod_url(params),  movie.id);

	nlohmann::json movies_json = server_request(url, true);
	if (movies_json.empty() || !movies_json.contains("data"))
	{
		return;
	}

	JSON_ALL_TRY
	{
		const auto& value = movies_json["data"];

		movie.title = utils::get_json_wstring("name", value);
		movie.title_orig = utils::get_json_wstring("original_name", value);
		movie.description = utils::get_json_wstring("description", value);
		movie.director = utils::get_json_wstring("director", value);
		movie.casting = utils::get_json_wstring("actors", value);
		movie.age = utils::get_json_wstring("age", value);
		movie.movie_time = utils::get_json_int("time", value);
		movie.country = utils::get_json_wstring("country", value);
		movie.year = utils::get_json_wstring("year", value);

		if (value.contains("seasons"))
		{
			for (const auto& season_it : value["seasons"].items())
			{
				const auto& season_item = season_it.value();
				vod_season_def season;
				season.id = utils::get_json_wstring("id", season_item);
				season.season_id = utils::get_json_wstring("number", season_item);
				season.title = utils::get_json_wstring("name", season_item);
				if (season.title.empty())
				{
					season.title = load_string_resource(IDS_STRING_SEASON) + L" " + season.season_id;
				}

				for (const auto& episode_it : season_item["series"].items())
				{
					const auto& episode_item = episode_it.value();

					vod_episode_def episode;
					episode.id = utils::get_json_wstring("id", episode_item);
					episode.episode_id = utils::get_json_wstring("number", episode_item);
					episode.title = utils::get_json_wstring("name", episode_item);
					if (episode.title.empty())
					{
						episode.title = std::format(L"Episode {:s}", episode.episode_id);
					}

					episode.url = utils::get_json_wstring("url", episode_item["files"][0]);
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
	JSON_ALL_CATCH
}

std::wstring plugin_korona::get_movie_url(const Credentials& creds, const movie_request& request, const vod_movie& movie)
{
	std::wstring url;

	if (movie.seasons.empty())
	{
		url = movie.url;
	}
	else
	{
		const auto& episodes = movie.seasons.front().episodes;
		if (!episodes.empty() && request.episode_idx != CB_ERR)
		{
			url = episodes[request.episode_idx].url;
		}
	}

	return url;
}

void plugin_korona::clear_account_info()
{
	delete_file_cookie(session_token_file);
	return account_info.clear();
}

nlohmann::json plugin_korona::server_request(const std::wstring& url, const bool use_cache_ttl /*= false*/)
{
	const auto& session_token = get_file_cookie(session_token_file);
	if (!session_token.empty())
	{
		utils::http_request req
		{
			.url = url,
			.cache_ttl = use_cache_ttl ? GetConfig().get_chrono(true, REG_MAX_CACHE_TTL) : std::chrono::seconds::zero(),
			.headers { "accept: */*",  "Content-Type: application/x-www-form-urlencoded", std::format("Authorization: Bearer {:s}", session_token) },
		};

		if (utils::AsyncDownloadFile(req).get())
		{
			JSON_ALL_TRY
			{
				return nlohmann::json::parse(req.body.str());
			}
			JSON_ALL_CATCH
		}
	}

	return {};
}
