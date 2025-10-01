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
#include "plugin_sharavoz.h"
#include "Constants.h"
#include "AccountSettings.h"

#include "UtilsLib\utils.h"
#include "UtilsLib\inet_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr auto VOD_API_REQUEST = L"{:s}/player_api.php?username={{LOGIN}}&password={{PASSWORD}}";
constexpr auto VOD_API_ACTION = L"{:s}&action={:s}";
constexpr auto PARAM_FMT = L"&{:s}={:s}";

void plugin_sharavoz::parse_vod(const ThreadConfig& config)
{
	auto categories = std::make_unique<vod_category_storage>();

	do
	{
		const auto& all_name = load_string_resource(IDS_STRING_ALL);
		auto all_category = std::make_shared<vod_category>(all_name);
		all_category->name = all_name;
		categories->set_back(all_name, all_category);

		const auto& api_url = replace_params_vars(config.m_params, std::format(VOD_API_REQUEST, config.m_url));

		std::wstring request_url = std::format(VOD_API_ACTION, api_url, L"get_vod_categories");

		const auto& category_json = xtream_request(config, request_url);
		if (category_json.empty()) break;

		int cnt = 0;
		int total = 0;
		constexpr int limit = 200;
		utils::progress_info info;
		for (const auto& item : category_json.items())
		{
			const auto& val = item.value();
			if (val.empty()) continue;

			std::shared_ptr<vod_category> category;
			const auto& category_id = xtream_parse_category(val, category, categories);
			if (category_id.empty()) continue;


			request_url = std::format(VOD_API_ACTION, api_url, L"get_vod_streams");
			request_url += std::format(PARAM_FMT, L"category_id", category_id);

			const auto& streams_json = xtream_request(config, request_url);
			if (streams_json.empty()) continue;

			total += static_cast<int>(streams_json.size());
			info.type = utils::ProgressType::Initializing;
			info.curPos = cnt;
			info.maxPos = total;
			config.progress_callback(info);
			info.type = utils::ProgressType::Progress;

			for (const auto& item : streams_json.items())
			{
				const auto& val = item.value();
				if (val.empty()) continue;


				std::shared_ptr<vod_movie> movie;
				JSON_ALL_TRY
				{
					const auto& movie_id = utils::get_json_wstring("stream_id", val);
					if (!category->movies.tryGet(movie_id, movie))
					{
						movie = std::make_shared<vod_movie>();
						movie->id = movie_id;
						movie->title = utils::get_json_wstring("name", val);
						movie->poster_url.set_uri(utils::get_json_wstring("stream_icon", val));
						movie->rating = utils::get_json_wstring("rating", val);

						category->movies.set_back(movie_id, movie);
					}
				}
				JSON_ALL_CATCH

				movie->genres.set_back(category_id, category->genres.get(category_id));

				if (++cnt % limit == 0)
				{
					info.curPos = info.value = cnt;
					config.progress_callback(info);

					if (::WaitForSingleObject(config.m_hStop, 0) == WAIT_OBJECT_0) break;
				}
			}
		}

		request_url = std::format(VOD_API_ACTION, api_url, L"get_series_categories");

		const auto& series_cat_json = xtream_request(config, request_url);
		if (series_cat_json.empty()) break;

		for (const auto& item : series_cat_json.items())
		{
			const auto& val = item.value();
			if (val.empty()) continue;

			std::shared_ptr<vod_category> category;
			const auto& category_id = xtream_parse_category(val, category, categories);
			if (category_id.empty()) continue;

			request_url = std::format(VOD_API_ACTION, api_url, L"get_series");
			request_url += std::format(PARAM_FMT, L"category_id", category_id);

			const auto& series_json = xtream_request(config, request_url);
			if (series_json.empty()) break;

			for (const auto& item : series_json.items())
			{
				const auto& val = item.value();
				if (val.empty()) continue;

				std::shared_ptr<vod_movie> movie;
				JSON_ALL_TRY
				{
					const auto& movie_id = utils::get_json_wstring("series_id", val);
					if (!category->movies.tryGet(movie_id, movie))
					{
						movie = std::make_shared<vod_movie>();
						movie->id = movie_id;
						movie->title = utils::get_json_wstring("name", val);
						movie->is_series = true;

						category->movies.set_back(movie_id, movie);
					}
				}
				JSON_ALL_CATCH

				movie->genres.set_back(category_id, category->genres.get(category_id));

				if (++cnt % limit == 0)
				{
					info.curPos = info.value = static_cast<int>(cnt);
					config.progress_callback(info);

					if (::WaitForSingleObject(config.m_hStop, 0) == WAIT_OBJECT_0) break;
				}
			}
		}

	} while (false);

	if (::WaitForSingleObject(config.m_hStop, 0) == WAIT_OBJECT_0)
	{
		categories.reset();
	}

	utils::progress_info info{ .type = utils::ProgressType::Finalizing };
	config.progress_callback(info);
	SendNotifyParent(config.m_parent, WM_END_LOAD_JSON_PLAYLIST, (WPARAM)categories.release());
}

void plugin_sharavoz::fetch_movie_info(const Credentials& creds, vod_movie& movie)
{
	TemplateParams params;
	update_provider_params(params);

	const auto& api_url = get_vod_url(params);
	const auto& token = creds.get_password();
	auto url = std::format(L"{:s}/player_api.php?username={:s}&password={:s}", api_url, token, token);
	if (movie.is_series)
	{
		url += std::format(L"&action=get_series_info&series_id={:s}", movie.id);
	}
	else
	{
		url += std::format(L"&action=get_vod_info&vod_id={:s}", movie.id);
	}

	utils::http_request req{ url, GetConfig().get_chrono(true, REG_MAX_CACHE_TTL) };
	if (!utils::DownloadFile(req))
	{
		return;
	}

	JSON_ALL_TRY
	{
		const auto& parsed_json = nlohmann::json::parse(req.body.str());

		if (!parsed_json.contains("info"))
		{
			return;
		}

		const auto& value = parsed_json["info"];
		if (movie.is_series)
		{
			movie.poster_url.set_uri(utils::get_json_wstring("cover", value));
			movie.casting = utils::get_json_wstring("cast", value);
			movie.rating = utils::get_json_wstring("rating", value);
			movie.movie_time = utils::get_json_int("episode_run_time", value);
			movie.year = utils::get_json_wstring("releaseDate", value);
			if (parsed_json.contains("episodes"))
			{
				for (const auto& season_it : parsed_json["episodes"].items())
				{
					vod_season_def season;
					season.id = season.season_id = utils::utf8_to_utf16(season_it.key());
					for (auto& episode_item : season_it.value())
					{
						vod_episode_def episode;
						episode.id = utils::get_json_wstring("id", episode_item);
						episode.title = utils::get_json_wstring("title", episode_item);
						episode.episode_id = utils::get_json_wstring("episode_num", episode_item);
						auto ext = utils::get_json_wstring("container_extension", episode_item);
						if (!ext.empty())
						{
							ext = L"." + ext;
						}

						episode.url = std::format(L"{:s}/movie/{:s}/{:s}/{:s}", api_url, token, token, episode.id + ext);

						season.episodes.set_back(episode.id, episode);
						movie.seasons.set_back(season.id, season);
					}
				}
			}
		}
		else
		{
			movie.title_orig = utils::get_json_wstring("o_name", value);
			movie.casting = utils::get_json_wstring("actors", value);
			movie.age = utils::get_json_wstring("age", value);
			movie.movie_time = utils::get_json_int("duration", value);
			movie.year = utils::get_json_wstring("releasedate", value);
			auto ext = utils::get_json_wstring("container_extension", value);
			if (!ext.empty())
			{
				ext = L"." + ext;
			}
			movie.url = std::format(L"{:s}/movie/{:s}/{:s}/{:s}", api_url, token, token, movie.id + ext);
		}

		movie.description = utils::get_json_wstring("plot", value);
		movie.director = utils::get_json_wstring("director", value);
	}
	JSON_ALL_CATCH
}

std::wstring plugin_sharavoz::get_movie_url(const Credentials& creds, const movie_request& request, const vod_movie& movie)
{
	if (!movie.seasons.empty() && request.season_idx != CB_ERR)
	{
		const auto& episodes = movie.seasons[request.season_idx].episodes;
		if (!episodes.empty() && request.episode_idx != CB_ERR)
		{
			return episodes[request.episode_idx].url;
		}
	}

	return movie.url;
}

std::wstring plugin_sharavoz::xtream_parse_category(const nlohmann::json& val,
													std::shared_ptr<vod_category>& category,
													std::unique_ptr<vod_category_storage>& categories)
{
	std::wstring category_id;
	JSON_ALL_TRY
	{
		const auto& title = utils::get_json_wstring("category_name", val);
		category_id = utils::get_json_wstring("category_id", val);

		if (category_id.empty())
		{
			throw std::exception("empty category_id");
		}

		auto pair = utils::string_split(title, L'|');
		utils::string_trim(pair[0]);
		utils::string_trim(pair[1]);
		vod_genre_def genre({ category_id, pair[1] });

		if (!categories->tryGet(pair[0], category))
		{
			category = std::make_shared<vod_category>(pair[0]);
			category->name = pair[0];
			categories->set_back(pair[0], category);
		}

		category->genres.set_back(category_id, genre);
	}
	JSON_ALL_CATCH


	return category_id;
}

nlohmann::json plugin_sharavoz::xtream_request(const ThreadConfig& config, const std::wstring& url)
{
	nlohmann::json category_json;

	utils::http_request req{ url, GetConfig().get_chrono(true, REG_MAX_CACHE_TTL) };
	if (utils::DownloadFile(req))
	{
		JSON_ALL_TRY
		{
			category_json = nlohmann::json::parse(req.body.str());
		}
		JSON_ALL_CATCH
	}

	return category_json;
}
