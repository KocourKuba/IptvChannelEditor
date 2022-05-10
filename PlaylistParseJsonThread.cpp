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
#include "PlaylistParseJsonThread.h"
#include "PlayListEntry.h"
#include "vod_movie.h"

#include "UtilsLib\utils.h"
#include "UtilsLib\inet_utils.h"
#include "UtilsLib\vectormap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CPlaylistParseJsonThread, CWinThread)

BOOL CPlaylistParseJsonThread::InitInstance()
{
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	switch (m_config.m_parser)
	{
		case 1:
			Parse1();
			break;

		case 2:
			Parse2();
			break;

		default:
			break;
	}

	CoUninitialize();

	return FALSE;
}


void CPlaylistParseJsonThread::Parse1()
{
	auto categories = std::make_unique<utils::vectormap<std::wstring, std::shared_ptr<vod_category>>>();
	do
	{
		std::vector<BYTE> data;
		if (!utils::DownloadFile(m_config.m_url, data, m_config.m_use_cache) || data.empty()) break;

		nlohmann::json parsed_json;
		JSON_ALL_TRY
		{
			parsed_json = nlohmann::json::parse(data.begin(), data.end());
		}
		JSON_ALL_CATCH;

		if (parsed_json.empty()) break;

		m_config.SendNotifyParent(WM_INIT_PROGRESS, (int)parsed_json.size(), TRUE);

		int cnt = 0;
		for (const auto& item : parsed_json.items())
		{
			std::shared_ptr<vod_category> category;
			std::wstring category_name;
			auto movie = std::make_shared<vod_movie>();
			nlohmann::json val = item.value();
			if (val.empty()) continue;

			JSON_ALL_TRY
			{
				category_name = utils::get_json_value("category", val);

				if (!categories->tryGet(category_name, category))
				{
					category = std::make_shared<vod_category>(category_name);
					category->name = category_name;
					categories->set(category_name, category);
				}

				movie->title = utils::get_json_value("name", val);
				movie->id = utils::get_json_value("id", val);
				movie->url = utils::get_json_value("video", val);

				const auto& info = val["info"];
				if (!info.empty())
				{
					movie->poster_url.set_uri(utils::get_json_value("poster", info));
					movie->poster_url.set_schema(L"http://");
					movie->description = utils::get_json_value("plot", info);
					movie->rating = utils::get_json_value("rating", info);
					movie->year = utils::get_json_value("year", info);
					movie->director = utils::get_json_value("director", info);
					movie->casting = utils::get_json_value("cast", info);
					movie->age = utils::get_json_value("adult", info);
					movie->movie_time = info.value("duration_secs", 0);

					std::string genre;
					for (const auto& genre_item : info["genre"].items())
					{
						movie->genres.emplace(utils::utf8_to_utf16(genre_item.value().get<std::string>()));
					}

					std::string country;
					for (const auto& country_item : info["country"].items())
					{
						if (!country.empty())
							country += ", ";
						country += country_item.value().get<std::string>();
					}
					movie->country = utils::utf8_to_utf16(country);
				}

				category->movies.set(movie->id, movie);
			}
			JSON_ALL_CATCH;
			cnt++;
			if (cnt % 50 == 0)
			{
				m_config.PostNotifyParent(WM_UPDATE_PROGRESS, cnt, cnt);
				if (::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0)
				{
					categories.reset();
					break;
				}
			}
		}
	} while (false);

	m_config.SendNotifyParent(WM_END_LOAD_JSON_PLAYLIST, (WPARAM)categories.release());
}

void CPlaylistParseJsonThread::Parse2()
{
	auto categories = std::make_unique<utils::vectormap<std::wstring, std::shared_ptr<vod_category>>>();
	do
	{
		std::vector<BYTE> data;
		if (!utils::DownloadFile(m_config.m_url, data, m_config.m_use_cache) || data.empty()) break;

		nlohmann::json parsed_json;
		JSON_ALL_TRY
		{
			parsed_json = nlohmann::json::parse(data.begin(), data.end());
		}
		JSON_ALL_CATCH;

		for (const auto& item : parsed_json["data"].items())
		{
			if (item.value().empty()) continue;

			nlohmann::json movies_json;
			auto category = std::make_shared<vod_category>();
			JSON_ALL_TRY
			{
				category->id = utils::get_json_value("id", item.value());
				category->name = utils::get_json_value("name", item.value());
				categories->set(category->id, category);

				data.clear();
				const auto& cat_url = fmt::format(L"{:s}/cat/{:s}?per_page=10000", m_config.m_url, category->id);
				if (!utils::DownloadFile(cat_url, data, m_config.m_use_cache) || data.empty()) continue;

				movies_json = nlohmann::json::parse(data.begin(), data.end());
			}
			JSON_ALL_CATCH;

			if (movies_json.empty() || !movies_json.contains("data")) continue;

			const auto& json_data = movies_json["data"];
			m_config.SendNotifyParent(WM_INIT_PROGRESS, (int)json_data.size(), TRUE);

			int cnt = 0;
			for (const auto& movie_it : json_data.items())
			{
				const auto& movie_item = movie_it.value();

				JSON_ALL_TRY
				{
					auto movie = std::make_shared<vod_movie>();

					movie->id = utils::get_json_value("id", movie_item);
					movie->title = utils::get_json_value("name", movie_item);
					movie->poster_url.set_uri(utils::get_json_value("poster", movie_item));
					movie->poster_url.set_schema(L"http://");
					movie->rating = utils::get_json_value("rating", movie_item);
					movie->country = utils::get_json_value("country", movie_item);
					movie->year = utils::get_json_value("year", movie_item);

					for (const auto& genre_item : movie_item["genres"].items())
					{
						movie->genres.emplace(utils::get_json_value("title", genre_item.value()));
					}
					category->movies.set(movie->id, movie);

				}
				JSON_ALL_CATCH;

				cnt++;
				if (cnt % 50 == 0)
				{
					m_config.PostNotifyParent(WM_UPDATE_PROGRESS, cnt, cnt);
					if (::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0)
					{
						categories.reset();
						break;
					}
				}
			}
		}

	} while (false);

	m_config.SendNotifyParent(WM_END_LOAD_JSON_PLAYLIST, (WPARAM)categories.release());
}
