/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2026): sharky72 (https://github.com/KocourKuba)

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
#include "plugin_ipstream.h"
#include "Constants.h"
#include "AccountSettings.h"

#include "UtilsLib\xxhash.hpp"

#include "UtilsLib\utils.h"
#include "UtilsLib\inet_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void plugin_ipstream::parse_vod(const ThreadConfig& config)
{
	auto categories = std::make_unique<vod_category_storage>();

	do
	{
		const auto& all_name = load_string_resource(IDS_STRING_ALL);
		auto all_category = std::make_shared<vod_category>(all_name);
		all_category->name = all_name;
		categories->set_back(all_name, all_category);

		utils::http_request req
		{
			.url = config.m_url,
			.cache_ttl = GetConfig().get_chrono(true, REG_MAX_CACHE_TTL)
		};
		if (!DownloadFile(req)) break;

		nlohmann::json parsed_json;
		JSON_ALL_TRY
		{
			parsed_json = nlohmann::json::parse(req.body.str());
		}
		JSON_ALL_CATCH

		if (parsed_json.empty()) break;

		utils::progress_info info{ .maxPos = static_cast<int>(parsed_json.size()) };
		config.progress_callback(info);
		info.type = utils::ProgressType::Progress;

		int cnt = 0;
		constexpr int limit = 200;
		for (const auto& item : parsed_json.items())
		{
			const auto& value = item.value();
			if (value.empty()) continue;

			std::shared_ptr<vod_category> category;
			std::wstring category_name;
			auto movie = std::make_shared<vod_movie_def>();

			JSON_ALL_TRY
			{
				category_name = utils::get_json_wstring("category", value);

				if (!categories->tryGet(category_name, category))
				{
					category = std::make_shared<vod_category>(category_name);
					category->name = category_name;
					categories->set_back(category_name, category);
				}

				movie->title = utils::get_json_wstring("name", value);

				if (value.contains("id"))
				{
					movie->id = utils::get_json_wstring("id", value);
				}
				else if (value.contains("series_id"))
				{
					movie->id = utils::get_json_wstring("series_id", value) + L"_serial";
					movie->is_series = true;
				}

				if (movie->id.empty())
				{
					movie->id = std::format(L"{:08x}", xxh::xxhash<32>(movie->title));
				}
				movie->url = utils::get_json_wstring("video", value);

				const auto& info = value["info"];
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
						const auto& genre_title = utils::get_json_wstring("", genre_item.value());
						vod_genre_def genre({ genre_title, genre_title });
						movie->genres.set_back(genre_title, genre);
					}

					std::string country;
					for (const auto& country_item : info["country"].items())
					{
						if (!country.empty())
						{
							country += ", ";
						}
						country += utils::get_json_string("", country_item.value());
					}
					movie->country = utils::utf8_to_utf16(country);
				}

				if (value.contains("seasons"))
				{
					for (const auto& season_it : value["seasons"].items())
					{
						const auto& season_item = season_it.value();
						vod_season_def season;
						season.id = utils::get_json_wstring("season", season_item);

						const auto& season_info = season_item["info"];
						season.number = utils::get_json_wstring("season", season_info);
						season.title = utils::get_json_wstring("name", season_info);
						if (season.title.empty())
						{
							season.title = load_string_resource(IDS_STRING_SEASON) + L" " + season.number;
						}

						season.year = utils::get_json_wstring("year", season_info);
						if (!season.year.empty())
						{
							season.title += std::format(L" ({:s})", season.year);
						}


						for (const auto& episode_it : season_item["episodes"].items())
						{
							const auto& episode_item = episode_it.value();

							vod_episode_def episode;
							episode.id = utils::get_json_wstring("id", episode_item);
							episode.number = utils::get_json_wstring("episode", episode_item);
							episode.url = utils::get_json_wstring("video", episode_item);

							season.episodes.set_back(episode.id, episode);
						}
						movie->seasons.set_back(season.id, season);
					}
				}

				category->movies.set_back(movie->id, movie);
			}
			JSON_ALL_CATCH

			if (++cnt % limit == 0)
			{
				info.curPos = info.value = cnt;
				config.progress_callback(info);

				if (::WaitForSingleObject(config.m_hStop, 0) == WAIT_OBJECT_0) break;
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

std::wstring plugin_ipstream::get_movie_url(const std::shared_ptr<Credentials>&, const movie_request& request, const vod_movie_def& movie)
{
	std::wstring url = movie.url;

	if (!movie.seasons.empty())
	{
		const auto& episodes = movie.seasons.front().episodes;
		if (!episodes.empty() && request.episode_idx != CB_ERR)
		{
			url = episodes[request.episode_idx].url;
		}
	}

	return url;
}
