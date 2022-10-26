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

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CPlaylistParseJsonThread, CWinThread)

BOOL CPlaylistParseJsonThread::InitInstance()
{
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	switch (m_parent_plugin->get_plugin_type())
	{
		case PluginType::enSharaclub:
			ParseSharaclub();
			break;

		case PluginType::enAntifriz:
		case PluginType::enCbilling:
			ParseCbilling();
			break;

		case PluginType::enEdem:
			ParseEdem();
			break;

		case PluginType::enGlanz:
			ParseGlanz();
			break;

		default:
			break;
	}

	CoUninitialize();

	return FALSE;
}


void CPlaylistParseJsonThread::ParseSharaclub()
{
	auto categories = std::make_unique<vod_category_storage>();
	do
	{
		std::stringstream data;
		if (!utils::CurlDownload(m_config.m_url, data, m_config.m_use_cache)) break;

		nlohmann::json parsed_json;
		JSON_ALL_TRY;
		parsed_json = nlohmann::json::parse(data.str());
		JSON_ALL_CATCH;

		if (parsed_json.empty()) break;

		m_config.SendNotifyParent(WM_INIT_PROGRESS, (int)parsed_json.size(), 0);

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
				categories->set(category_name, category);
			}

			movie->title = utils::get_json_wstring("name", val);
			movie->id = utils::get_json_wstring("id", val);
			movie->url = utils::get_json_wstring("video", val);

			const auto& info = val["info"];
			if (!info.empty())
			{
				movie->poster_url.set_uri(utils::get_json_wstring("poster", info));
				movie->poster_url.set_schema(L"http://");
				movie->description = utils::get_json_wstring("plot", info);
				movie->rating = utils::get_json_wstring("rating", info);
				movie->year = utils::get_json_wstring("year", info);
				movie->director = utils::get_json_wstring("director", info);
				movie->casting = utils::get_json_wstring("cast", info);
				movie->age = utils::get_json_wstring("adult", info);
				movie->movie_time = std::to_wstring(info.value("duration_secs", 0) / 60);

				for (const auto& genre_item : info["genre"].items())
				{
					const auto& title = utils::utf8_to_utf16(genre_item.value().get<std::string>());
					vod_genre genre({ title, title });

					movie->genres.set(title, genre);
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
			JSON_ALL_CATCH;

			cnt++;
			if (cnt % 100 == 0)
			{
				m_config.SendNotifyParent(WM_UPDATE_PROGRESS, cnt, cnt);
				if (::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0) break;
			}
		}
	} while (false);

	if (::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0)
		categories.reset();

	m_config.SendNotifyParent(WM_END_LOAD_JSON_PLAYLIST, (WPARAM)categories.release());
}

void CPlaylistParseJsonThread::ParseCbilling()
{
	auto categories = std::make_unique<utils::vectormap<std::wstring, std::shared_ptr<vod_category>>>();
	do
	{
		std::stringstream info;
		if (!utils::CurlDownload(m_config.m_url, info, m_config.m_use_cache)) break;

		int total = 0;
		JSON_ALL_TRY;
		const auto& parsed_json = nlohmann::json::parse(info.str());
		for (const auto& item_it : parsed_json["data"].items())
		{
			if (item_it.value().empty()) continue;
			const auto& item = item_it.value();

			auto category = std::make_shared<vod_category>();
			category->id = utils::get_json_wstring("id", item);
			category->name = utils::get_json_wstring("name", item);
			total += item["count"].get<int>();
			categories->set(category->id, category);
		}
		JSON_ALL_CATCH;

		m_config.SendNotifyParent(WM_INIT_PROGRESS, total, 0);

		int cnt = 0;
		for (const auto& pair : categories->vec())
		{
			const auto& category = pair.second;

			int page = 1;
			int retry = 0;
			for(;;)
			{
				if (::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0 || retry > 2) break;

				std::stringstream data;
				const auto& cat_url = fmt::format(L"{:s}/cat/{:s}?page={:d}&per_page=200", m_config.m_url, category->id, page);
				if (!utils::CurlDownload(cat_url, data, m_config.m_use_cache) || data.bad())
				{
					retry++;
					continue;
				}

				nlohmann::json movies_json;
				JSON_ALL_TRY;
				{
					movies_json = nlohmann::json::parse(data.str());
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
						movie->poster_url.set_schema(L"http://");
						movie->rating = utils::get_json_wstring("rating", movie_item);
						movie->country = utils::get_json_wstring("country", movie_item);
						movie->year = utils::get_json_wstring("year", movie_item);

						for (const auto& genre_item : movie_item["genres"].items())
						{
							const auto& title = utils::get_json_wstring("title", genre_item.value());
							vod_genre genre({ title, title });
							movie->genres.set(title, genre);
						}

						category->movies.set(movie->id, movie);
					}
					JSON_ALL_CATCH;

					cnt++;
					if (cnt % 100 == 0)
					{
						m_config.SendNotifyParent(WM_UPDATE_PROGRESS, cnt, cnt);
						if (::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0) break;
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

	if (::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0)
		categories.reset();

	m_config.SendNotifyParent(WM_END_LOAD_JSON_PLAYLIST, (WPARAM)categories.release());
}

void CPlaylistParseJsonThread::ParseEdem()
{
	auto categories = std::make_unique<utils::vectormap<std::wstring, std::shared_ptr<vod_category>>>();
	do
	{
		std::wregex re_url(LR"(^portal::\[key:(.+)\](.+)$)");
		std::wsmatch m;
		if (!std::regex_match(m_config.m_url, m, re_url)) break;

		const auto& key = m[1].str();
		const auto& url = m[2].str();

		std::vector<std::string> headers;
		headers.emplace_back("accept: */*");
		headers.emplace_back("Content-Type: application/json");

		nlohmann::json json_request;
		json_request["key"] = utils::utf16_to_utf8(key);
		json_request["mac"] = "000000000000";
		json_request["app"] = "IPTV ChannelEditor";
		const auto& post = json_request.dump();

		std::stringstream data;
		if (!utils::CurlDownload(url, data, m_config.m_use_cache, &headers, true, post.c_str())) break;

		JSON_ALL_TRY;
		nlohmann::json parsed_json = nlohmann::json::parse(data.str());
		for (const auto& item_it : parsed_json["items"].items())
		{
			if (item_it.value().empty()) continue;
			const auto& item = item_it.value();

			auto category = std::make_shared<vod_category>();
			category->id = utils::get_json_wstring("fid", item["request"]);
			category->name = utils::get_json_wstring("title", item);
			categories->set(category->id, category);
		}

		for (const auto& filter_it : parsed_json["controls"]["filters"].items())
		{
			const auto& filter = filter_it.value();
			const auto& filter_name = utils::get_json_wstring("title", filter);
			std::string id_tag;
			if (filter_name == L"Жанр")
			{
				id_tag = "genre";
			}
			else if (filter_name == L"Год")
			{
				id_tag = "years";
			}
			else
			{
				ASSERT(false);
				continue;
			}

			utils::vectormap<std::wstring, vod_filter> filters;
			for (const auto& filter_sub_it : filter["items"].items())
			{
				const auto& filter_sub = filter_sub_it.value();
				const auto& id = utils::get_json_wstring(id_tag, filter_sub["request"]);
				const auto& title = utils::get_json_wstring("title", filter_sub);
				vod_filter filter({ id, title });
				filters.set(id, filter);
			}
			categories->vec().front().second->filters.set(utils::utf8_to_utf16(id_tag), filters);
		}

		json_request["cmd"] = "flicks";
		json_request["limit"] = 300;
		int cnt = 0;
		int total = 0;
		if (categories->empty()) break;

		for (auto& category : categories->vec())
		{
			json_request["fid"] = utils::char_to_int(category.second->id);
			json_request["offset"] = 0;
			const auto& cat_post = json_request.dump();

			std::stringstream cat_data;
			if (!utils::CurlDownload(url, cat_data, m_config.m_use_cache, &headers, true, cat_post.c_str())) break;

			const auto& data_str = cat_data.str();
			nlohmann::json movie_json = nlohmann::json::parse(data_str);
			total += utils::get_json_int("count", movie_json);
			m_config.SendNotifyParent(WM_INIT_PROGRESS, total, cnt);
			ATLTRACE("\ntotal movies: %d\n", total);

			int readed = 0;
			int offset = 0;
			int prev_offset = -1;

			for(;;)
			{
				//std::string dump(data.begin(), data.end());
				for (const auto& item_it : movie_json["items"].items())
				{
					const auto& movie_item = item_it.value();

					auto movie = std::make_shared<vod_movie>();

					if (utils::get_json_wstring("type", movie_item) == L"next")
					{
						offset = utils::get_json_int("offset", movie_item["request"]);
						continue;
					}

					movie->id = utils::get_json_wstring("fid", movie_item["request"]);
					movie->title = utils::get_json_wstring("title", movie_item);
					movie->description = utils::get_json_wstring("description", movie_item);
					movie->poster_url.set_uri(utils::get_json_wstring("img", movie_item));
					movie->poster_url.set_schema(L"http://");
					movie->rating = utils::get_json_wstring("rating", movie_item);
					movie->country = utils::get_json_wstring("country", movie_item);
					movie->year = utils::get_json_wstring("year", movie_item);
					movie->age = utils::get_json_wstring("agelimit", movie_item);
					movie->movie_time = utils::get_json_wstring("duration", movie_item);

					category.second->movies.set(movie->id, movie);

					readed++;
					cnt++;
					if (cnt % 100 == 0)
					{
						m_config.SendNotifyParent(WM_UPDATE_PROGRESS, cnt, cnt);
						if (::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0) break;
					}
				}

				ATLTRACE("\nreaded: %d\n", readed);

				if (offset == prev_offset || ::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0) break;

				prev_offset = offset;
				json_request["offset"] = offset;
				ATLTRACE("\noffset: %d\n", offset);

				std::stringstream mov_data;
				if (!utils::CurlDownload(url, mov_data, m_config.m_use_cache, &headers, true, json_request.dump().c_str())) break;

				movie_json = nlohmann::json::parse(mov_data.str());
			}

			if (::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0) break;
		}
		JSON_ALL_CATCH;
	} while (false);

	if (::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0)
		categories.reset();

	m_config.SendNotifyParent(WM_END_LOAD_JSON_PLAYLIST, (WPARAM)categories.release());
}

void CPlaylistParseJsonThread::ParseGlanz()
{
	auto categories = std::make_unique<vod_category_storage>();
	do
	{
		std::stringstream data;
		if (!utils::CurlDownload(m_config.m_url, data, m_config.m_use_cache)) break;

		nlohmann::json parsed_json;
		JSON_ALL_TRY;
		parsed_json = nlohmann::json::parse(data.str());
		JSON_ALL_CATCH;

		if (parsed_json.empty()) break;

		m_config.SendNotifyParent(WM_INIT_PROGRESS, (int)parsed_json.size(), 0);

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
			if (category_name.empty())
				category_name = L"Без категории";

			if (!categories->tryGet(category_name, category))
			{
				category = std::make_shared<vod_category>(category_name);
				category->name = category_name;
				categories->set(category_name, category);
			}

			movie->title = utils::get_json_wstring("name", val);
			movie->id = utils::get_json_wstring("id", val);
			movie->url = utils::get_json_wstring("url", val);

			movie->poster_url.set_uri(utils::get_json_wstring("cover", val));
			movie->poster_url.set_schema(L"http://");
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
				const auto& title = utils::get_json_wstring("title", genre_item);
				vod_genre genre({ id, title });

				movie->genres.set(title, genre);
			}


			category->movies.set(movie->id, movie);
			JSON_ALL_CATCH;

			cnt++;
			if (cnt % 100 == 0)
			{
				m_config.SendNotifyParent(WM_UPDATE_PROGRESS, cnt, cnt);
				if (::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0) break;
			}
		}
	} while (false);

	if (::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0)
		categories.reset();

	m_config.SendNotifyParent(WM_END_LOAD_JSON_PLAYLIST, (WPARAM)categories.release());
}
