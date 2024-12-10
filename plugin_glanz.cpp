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
#include "plugin_glanz.h"
#include "AccountSettings.h"
#include "Constants.h"

#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void plugin_glanz::parse_vod(const CThreadConfig& config)
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
		{
			parsed_json = nlohmann::json::parse(data.str());
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
			{
				category_name = utils::get_json_wstring("category", val);
				if (category_name.empty())
				{
					category_name = L"Без категории";
				}

				if (!categories->tryGet(category_name, category))
				{
					category = std::make_shared<vod_category>(category_name);
					category->name = category_name;
					categories->set_back(category_name, category);
				}

				movie->title = utils::get_json_wstring("name", val);
				movie->id = utils::get_json_wstring("id", val);
				movie->url = utils::get_json_wstring("url", val);

				movie->poster_url.set_uri(utils::get_json_wstring("cover", val));
				movie->description = utils::get_json_wstring("description", val);
				movie->rating = utils::get_json_wstring("rating", val);
				movie->year = utils::get_json_wstring("year", val);
				movie->director = utils::get_json_wstring("director", val);
				movie->casting = utils::get_json_wstring("actors", val);
				//movie->age = utils::get_json_string("censored", val);
				movie->movie_time = val.value("duration_secs", 0);
				movie->country = utils::get_json_wstring("country", val);

				for (const auto& item : val["genres"].items())
				{
					const auto& genre_item = item.value();
					const auto& id = utils::get_json_wstring("id", genre_item);
					const auto& vod_title = utils::get_json_wstring("title", genre_item);
					vod_genre_def genre({ id, vod_title });

					movie->genres.set_back(vod_title, genre);
				}

				category->movies.set_back(movie->id, movie);
			}
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

void plugin_glanz::update_entry(PlaylistEntry& entry)
{
	entry.set_icon_uri(utils::string_replace<wchar_t>(entry.get_icon_uri().get_uri(), L"https://", L"http://"));
}
