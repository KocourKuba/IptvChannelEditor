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
#include "plugin_iptvonline.h"
#include "Constants.h"
#include "AccountSettings.h"

#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto API_COMMAND_AUTH= L"{:s}/auth";
static constexpr auto API_COMMAND_REFRESH_TOKEN = L"{:s}/oauth2";
static constexpr auto API_COMMAND_INFO = L"{:s}/profile";
static constexpr auto API_COMMAND_DEVICE = L"{:s}/device/{:s}";
static constexpr auto API_COMMAND_PLAYLIST = L"{:s}/playlist/m3u8";

static constexpr const char* client_id = "TestAndroidAppV0";
static constexpr const char* client_secret = "kshdiouehruyiwuresuygr736t4763b7637";
static constexpr const char* device_id = "IPTV Channel Editor " STRPRODUCTVER;

void plugin_iptvonline::get_api_token(Credentials& creds)
{
	std::wstring url;
	bool expired = creds.expired <= time(nullptr) && !creds.token.empty();

	nlohmann::json json_request;

	if (expired)
	{
		url = fmt::format(API_COMMAND_REFRESH_TOKEN, get_provider_api_url());
		json_request["grant_type"] = "refresh_token";
		json_request["refresh_token"] = creds.token;
	}
	else if (creds.s_token.empty() || creds.token.empty())
	{
		url = fmt::format(API_COMMAND_AUTH, get_provider_api_url());
		json_request["login"] = creds.login;
		json_request["password"] = creds.password;
	}

	if (!url.empty())
	{
		json_request["client_id"] = client_id;
		json_request["client_secret"] = client_secret;
		json_request["device_id"] = device_id;
		const auto& post = json_request.dump();

		std::vector<std::string> headers;
		headers.emplace_back("accept: */*");
		headers.emplace_back("Content-Type: application/json; charset=utf-8");

		CWaitCursor cur;
		std::stringstream data;
		if (download_url(url, data, 0, &headers, true, post.c_str()))
		{
			JSON_ALL_TRY;
			{
				const auto& parsed_json = nlohmann::json::parse(data.str());
				creds.s_token = utils::get_json_string("access_token", parsed_json);
				creds.token = utils::get_json_string("refresh_token", parsed_json);
				creds.expired = utils::get_json_int("expires_time", parsed_json);
			}
			JSON_ALL_CATCH;
		}
	}
}

std::wstring plugin_iptvonline::get_playlist_url(const TemplateParams& params, std::wstring url /* = L"" */)
{
	url = get_playlist_info(params.playlist_idx).get_pl_template(); //-V763

	JSON_ALL_TRY;
	{
		const auto& parsed_json = server_request(fmt::format(API_COMMAND_PLAYLIST, get_provider_api_url()), params.creds, 0);
		if (utils::get_json_bool("success", parsed_json) == true && parsed_json.contains("data"))
		{
			const auto& server_url = utils::get_json_wstring("data", parsed_json);
			boost::wregex re(LR"(^.+\/(.+)\/m3u8)");
			boost::wsmatch m;
			if (boost::regex_match(server_url, m, re))
			{
				utils::string_replace_inplace<wchar_t>(url, REPL_PASSWORD, m[1]);
			}

		}
	}
	JSON_ALL_CATCH;

	return base_plugin::get_playlist_url(params, url);
}

void plugin_iptvonline::parse_account_info(Credentials& creds)
{
	if (account_info.empty())
	{
		const auto& url = fmt::format(API_COMMAND_INFO, get_provider_api_url());
		const auto& parsed_json = server_request(url, creds, 0);

		JSON_ALL_TRY;
		{
			if (utils::get_json_int("status", parsed_json) == 200 && parsed_json.contains("data"))
			{
				const auto& js_data = parsed_json["data"];
				set_json_info("login", js_data, account_info);
				set_json_info("balance", js_data, account_info);
				set_json_info("currency", js_data, account_info);
				set_json_info("server_name", js_data, account_info);
				set_json_info("playlist", js_data, account_info);

				if (js_data.contains("subscriptions"))
				{
					for (auto& item : js_data["subscriptions"].items())
					{
						const auto& val = item.value();
						const auto& name = utils::get_json_wstring("name", val);
						const auto& value = fmt::format(L"expired {:s}", utils::get_json_wstring("end_date", val));
						account_info.emplace(name, value);
					}
				}
				if (js_data.contains("selected_playlist"))
				{
					for (auto& item : js_data["selected_playlist"].items())
					{
						const auto& value = utils::get_json_wstring("title", item.value());
						account_info.emplace(L"name", value);
					}
				}
			}
		}
		JSON_ALL_CATCH;
	}
}

void plugin_iptvonline::fill_servers_list(TemplateParams& params)
{
	if (params.creds.s_token.empty() || !get_servers_list().empty())
		return;

	std::vector<DynamicParamsInfo> servers;

	const auto& url = fmt::format(API_COMMAND_DEVICE, get_provider_api_url(), L"info");

	JSON_ALL_TRY;
	{
		const auto& parsed_json = server_request(url, params.creds, 0);
		if (utils::get_json_int("status", parsed_json) == 200)
		{
			if (parsed_json.contains("device")
				&& parsed_json["device"].contains("settings")
				&& parsed_json["device"]["settings"].contains("server_location")
				&& parsed_json["device"]["settings"]["server_location"].contains("value")
				)
			{
				int idx = 0;
				for (auto& item : parsed_json["device"]["settings"]["server_location"]["value"].items())
				{
					const auto& server = item.value();
					DynamicParamsInfo info{ utils::get_json_string("id", server), utils::get_json_string("label", server) };
					if (utils::get_json_bool("selected", server))
					{
						params.server_idx = idx;
					}
					servers.emplace_back(info);
					idx++;
				}
			}
		}
	}
	JSON_ALL_CATCH;

	set_servers_list(servers);
}

bool plugin_iptvonline::set_server(TemplateParams& params)
{
	// {
	//   "server_location": 9,
	//   "user_playlists" : 0,
	//   "parental_control_enable" : true,
	//	 "parental_control_code" : "1234"
	// }

	if (servers_list.empty())
	{
		fill_servers_list(params);
	}

	if (!servers_list.empty())
	{
		nlohmann::json json_request;
		json_request["server_location"] = std::stoi(servers_list[params.server_idx].id);
		const auto& url = fmt::format(API_COMMAND_DEVICE, get_provider_api_url(), L"settings");

		const auto& parsed_json = server_request(url, params.creds, 0, true, json_request);

		JSON_ALL_TRY;
		{
			if (parsed_json.contains("device")
				&& parsed_json["device"].contains("settings")
				&& parsed_json["device"]["settings"].contains("server_location")
				&& parsed_json["device"]["settings"]["server_location"].contains("value")
				)
			{
				int idx = 0;
				for (auto& item : parsed_json["device"]["settings"]["server_location"]["value"].items())
				{
					if (utils::get_json_bool("selected", item.value()))
					{
						params.server_idx = idx;
						break;
					}
					idx++;
				}
			}

			return utils::get_json_int("status", parsed_json) == 200;
		}
		JSON_ALL_CATCH;
	}

	return false;
}

void plugin_iptvonline::parse_vod(const CThreadConfig& config)
{
	int cache_ttl = GetConfig().get_int(true, REG_MAX_CACHE_TTL) * 3600;
	auto categories = std::make_unique<vod_category_storage>();

	const auto& all_name = load_string_resource(IDS_STRING_ALL);
	auto all_category = std::make_shared<vod_category>(all_name);
	all_category->name = all_name;
	categories->set_back(all_name, all_category);

	collect_movies(L"movies", L"Movies", config, categories);
	collect_movies(L"serials", L"Serials", config, categories, true);

	if (::WaitForSingleObject(config.m_hStop, 0) == WAIT_OBJECT_0)
	{
		categories.reset();
	}

	config.SendNotifyParent(WM_END_LOAD_JSON_PLAYLIST, (WPARAM)categories.release());
}

void plugin_iptvonline::fetch_movie_info(const Credentials& creds, vod_movie& movie)
{
	TemplateParams params;
	params.creds = creds;
	update_provider_params(params);

	CWaitCursor cur;

	const auto& url = fmt::format(L"{:s}/{:s}/{:s}", get_vod_url(params), movie.category, movie.id);

	int cache_ttl = GetConfig().get_int(true, REG_MAX_CACHE_TTL) * 3600;
	nlohmann::json movies_json = server_request(url, creds, cache_ttl);
	if (movies_json.empty() || !movies_json.contains("data"))
	{
		return;
	}

	JSON_ALL_TRY;
	{
		const auto& value = movies_json["data"];

		movie.description = utils::get_json_wstring("plot", value);
		movie.director = utils::get_json_wstring("director", value);
		movie.casting = utils::get_json_wstring("cast", value);

		if (value.contains("seasons"))
		{
			for (const auto& season_it : value["seasons"].items())
			{
				const auto& season_item = season_it.value();
				vod_season_def season;
				season.id = utils::get_json_wstring("season", season_item);
				season.title = utils::get_json_wstring("title", season_item);
				season.season_id = utils::get_json_wstring("season", season_item);

				for (const auto& episode_it : season_item["episodes"].items())
				{
					const auto& episode_item = episode_it.value();

					vod_episode_def episode;
					episode.id = utils::get_json_wstring("episode", episode_item);
					episode.title = utils::get_json_wstring("title", episode_item);
					episode.episode_id = utils::get_json_wstring("episode", episode_item);
					episode.url = utils::get_json_wstring("url", episode_item);
					episode.audios.set_back(L"auto", vod_variant_def({ L"auto", episode.url }));

					if (episode_item.contains("audios"))
					{
						for (const auto& variant_it : episode_item["audios"].items())
						{
							const auto& audios = variant_it.value();

							const auto& vod_title = utils::get_json_wstring("translate", audios);
							const auto& q_url = utils::get_json_wstring("url", audios);

							if (!vod_title.empty() && !q_url.empty())
							{
								episode.audios.set_back(vod_title, vod_variant_def({ vod_title, q_url }));
							}
						}
					}

					season.episodes.set_back(episode.id, episode);
				}
				movie.seasons.set_back(season.id, season);
			}
		}
		else if (value.contains("medias"))
		{
			movie.url = utils::get_json_wstring("url", value["medias"]);
			movie.title = utils::get_json_wstring("title", value["medias"]);
			movie.audios.set_back(L"auto", vod_variant_def({ L"auto", movie.url }));

			if (value["medias"].contains("audios"))
			{
				for (const auto& variant_it : value["medias"]["audios"].items())
				{
					const auto& audios = variant_it.value();

					const auto& vod_title = utils::get_json_wstring("translate", audios);
					const auto& q_url = utils::get_json_wstring("url", audios);
					if (!vod_title.empty() && !q_url.empty())
					{
						movie.audios.set_back(vod_title, vod_variant_def({ vod_title, q_url }));
					}
				}
			}
		}
	}
	JSON_ALL_CATCH;
}

std::wstring plugin_iptvonline::get_movie_url(const Credentials& creds, const movie_request& request, const vod_movie& movie)
{
	std::wstring url = movie.url;

	if (!movie.audios.empty() && request.audio_idx != CB_ERR)
	{
		url = movie.audios[request.audio_idx].url;
	}
	else if (!movie.seasons.empty())
	{
		const auto& episodes = movie.seasons.front().episodes;
		if (!episodes.empty() && request.episode_idx != CB_ERR)
		{
			const auto& audio = episodes[request.episode_idx].audios;
			if (audio.empty())
			{
				url = episodes[request.episode_idx].url;
			}
			else
			{
				url = episodes[request.episode_idx].audios[request.audio_idx].url;
			}
		}
	}

	return url;
}

void plugin_iptvonline::collect_movies(const std::wstring& id,
									   const std::wstring& category_name,
									   const CThreadConfig& config,
									   std::unique_ptr<vod_category_storage>& categories,
									   bool is_serial /*= false*/)
{
	auto movie_category = std::make_shared<vod_category>(id);
	movie_category->name = category_name;

	std::wstring cat_url = fmt::format(L"{:s}/{:s}/?limit=100&page=1", get_vod_url(config.m_params), id);
	int cache_ttl = GetConfig().get_int(true, REG_MAX_CACHE_TTL) * 3600;
	const auto& meta_info_json = server_request(cat_url, config.m_params.creds, cache_ttl);

	if (meta_info_json.empty()
		|| !utils::get_json_bool("success", meta_info_json)
		|| utils::get_json_int("status", meta_info_json) != 200
		|| !meta_info_json.contains("data") || !meta_info_json["data"].contains("pagination"))
	{
		return;
	}

	int total = utils::get_json_int("total_items", meta_info_json["data"]["pagination"]);
	int last = utils::get_json_int("pages", meta_info_json["data"]["pagination"]);;

	config.SendNotifyParent(WM_INIT_PROGRESS, total, 0);

	int cnt = 0;
	int page = 1;
	for (;;)
	{
		if (::WaitForSingleObject(config.m_hStop, 0) == WAIT_OBJECT_0) break;

		cat_url = fmt::format(L"{:s}/{:s}/?limit=100&page={:d}", config.m_url, movie_category->id, page);
		nlohmann::json movies_json = server_request(cat_url, config.m_params.creds, cache_ttl);
		if (movies_json.empty() || !movies_json.contains("data") || !movies_json["data"].contains("items"))
		{
			continue;
		}

		for (const auto& movie_it : movies_json["data"]["items"].items())
		{
			const auto& movie_item = movie_it.value();

			JSON_ALL_TRY;
			{
				auto movie = std::make_shared<vod_movie>();

				movie->is_series = is_serial;
				movie->id = utils::get_json_wstring("id", movie_item);
				movie->category = movie_category->id;
				movie->title = utils::get_json_wstring("ru_title", movie_item);
				movie->title_orig = utils::get_json_wstring("orig_title", movie_item);
				movie->movie_time = utils::get_json_int("duration", movie_item) / 60;
				if (movie_item.contains("posters"))
				{
					movie->poster_url.set_uri(utils::get_json_wstring("medium", movie_item["posters"]));
				}
				movie->rating = utils::get_json_wstring("imdb_rating", movie_item);
				if (movie_item.contains("countries"))
				{
					for (const auto& item : movie_item["countries"])
					{
						if (!movie->country.empty())
						{
							movie->country += L", ";
						}
						movie->country += utils::utf8_to_utf16(item.get<std::string>());
					}
				}
				movie->year = utils::get_json_wstring("year", movie_item);

				for (const auto& genre_item : movie_item["genres"].items())
				{
					const auto& genre_value = genre_item.value();
					const auto& vod_title = utils::utf8_to_utf16(genre_value.get<std::string>());
					vod_genre_def genre({ vod_title, vod_title });
					movie->genres.set_back(vod_title, genre);
				}

				movie_category->movies.set_back(movie->id, movie);
			}
			JSON_ALL_CATCH;

			if (++cnt % 100 == 0)
			{
				config.SendNotifyParent(WM_UPDATE_PROGRESS, cnt, cnt);
				if (::WaitForSingleObject(config.m_hStop, 0) == WAIT_OBJECT_0) break;
			}
		}

		if (page >= last) break;
		page++;
	}

	categories->set_back(movie_category->id, movie_category);
}

nlohmann::json plugin_iptvonline::server_request(const std::wstring& url, const Credentials& creds, int cache_ttl, bool web_post /*= false*/, const nlohmann::json& post_data /*= {}*/)
{
	std::vector<std::string> headers;
	headers.emplace_back("accept: */*");
	headers.emplace_back("Content-Type: application/json; charset=utf-8");
	headers.emplace_back(fmt::format("Authorization: Bearer {:s}", creds.s_token));

	CWaitCursor cur;
	std::stringstream data;
	const auto& post = post_data.dump();
	if (download_url(url, data, cache_ttl, &headers, web_post, post.c_str()))
	{
		JSON_ALL_TRY;
		{
			return nlohmann::json::parse(data.str());
		}
		JSON_ALL_CATCH;
	}

	return {};
}
