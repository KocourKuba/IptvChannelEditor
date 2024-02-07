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
#include "PlaylistParseJsonThread.h"
#include "PlayListEntry.h"

#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CPlaylistParseJsonThread, CWinThread)

BOOL CPlaylistParseJsonThread::InitInstance()
{
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	m_dl.SetUserAgent(m_parent_plugin->get_user_agent());
	m_dl.SetCacheTtl(m_config.m_cache_ttl * 3600);

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

		case PluginType::enSharavoz:
			ParseSharavoz();
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
		const auto& all_name = load_string_resource(IDS_STRING_ALL);
		auto all_category = std::make_shared<vod_category>(all_name);
		all_category->name = all_name;
		categories->set_back(all_name, all_category);

		std::stringstream data;
		if (!m_dl.DownloadFile(m_config.m_url, data)) break;

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
					const auto& title = utils::utf8_to_utf16(genre_item.value().get<std::string>());
					vod_genre genre({ title, title });

					movie->genres.set_back(title, genre);
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
				m_config.SendNotifyParent(WM_UPDATE_PROGRESS, cnt, cnt);
				if (::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0) break;
			}
		}
	} while (false);

	if (::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0)
	{
		categories.reset();
	}

	m_config.SendNotifyParent(WM_END_LOAD_JSON_PLAYLIST, (WPARAM)categories.release());
}

void CPlaylistParseJsonThread::ParseCbilling()
{
	auto categories = std::make_unique<vod_category_storage>();

	do
	{
		const auto& all_name = load_string_resource(IDS_STRING_ALL);
		auto all_category = std::make_shared<vod_category>(all_name);
		all_category->name = all_name;
		categories->set_back(all_name, all_category);

		std::stringstream info;
		if (!m_dl.DownloadFile(m_config.m_url, info)) break;

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
			categories->set_back(category->id, category);
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
				if (!m_dl.DownloadFile(cat_url, data) || data.bad())
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
						movie->rating = utils::get_json_wstring("rating", movie_item);
						movie->country = utils::get_json_wstring("country", movie_item);
						movie->year = utils::get_json_wstring("year", movie_item);

						for (const auto& genre_item : movie_item["genres"].items())
						{
							const auto& title = utils::get_json_wstring("title", genre_item.value());
							vod_genre genre({ title, title });
							movie->genres.set_back(title, genre);
						}

						category->movies.set_back(movie->id, movie);
					}
					JSON_ALL_CATCH;

					if (++cnt % 100 == 0)
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
	{
		categories.reset();
	}

	m_config.SendNotifyParent(WM_END_LOAD_JSON_PLAYLIST, (WPARAM)categories.release());
}

void CPlaylistParseJsonThread::ParseEdem()
{
	auto categories = std::make_unique<vod_category_storage>();

	do
	{
		const auto& all_name = load_string_resource(IDS_STRING_ALL);
		auto all_category = std::make_shared<vod_category>(all_name);
		all_category->name = all_name;
		categories->set_back(all_name, all_category);

		int cache_ttl = m_config.m_cache_ttl * 3600;

		boost::wregex re_url(LR"(^portal::\[key:(.+)\](.+)$)");
		boost::wsmatch m;
		if (!boost::regex_match(m_config.m_url, m, re_url)) break;

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
		if (!m_dl.DownloadFile(url, data, &headers, true, post.c_str())) break;

		JSON_ALL_TRY;
		{
			nlohmann::json parsed_json = nlohmann::json::parse(data.str());
			for (const auto& item_it : parsed_json["items"].items())
			{
				if (item_it.value().empty()) continue;
				const auto& item = item_it.value();

				auto category = std::make_shared<vod_category>();
				category->id = utils::get_json_wstring("fid", item["request"]);
				category->name = utils::get_json_wstring("title", item);
				categories->set_back(category->id, category);
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
					filters.set_back(id, filter);
				}
				categories->vec().front().second->filters.set_back(utils::utf8_to_utf16(id_tag), filters);
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
				if (!m_dl.DownloadFile(url, cat_data, &headers, true, cat_post.c_str())) break;

				const auto& data_str = cat_data.str();
				nlohmann::json movie_json = nlohmann::json::parse(data_str);
				total += utils::get_json_int("count", movie_json);
				m_config.SendNotifyParent(WM_INIT_PROGRESS, total, cnt);
				ATLTRACE("\ntotal movies: %d\n", total);

				int readed = 0;
				int offset = 0;
				int prev_offset = -1;

				for (;;)
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
						movie->rating = utils::get_json_wstring("rating", movie_item);
						movie->country = utils::get_json_wstring("country", movie_item);
						movie->year = utils::get_json_wstring("year", movie_item);
						movie->age = utils::get_json_wstring("agelimit", movie_item);
						movie->movie_time = utils::get_json_int("duration", movie_item);

						category.second->movies.set_back(movie->id, movie);

						readed++;
						if (++cnt % 100 == 0)
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
					if (!m_dl.DownloadFile(url, mov_data, &headers, true, json_request.dump().c_str())) break;

					movie_json = nlohmann::json::parse(mov_data.str());
				}

				if (::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0) break;
			}
		}
		JSON_ALL_CATCH;
	} while (false);

	if (::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0)
	{
		categories.reset();
	}

	m_config.SendNotifyParent(WM_END_LOAD_JSON_PLAYLIST, (WPARAM)categories.release());
}

void CPlaylistParseJsonThread::ParseGlanz()
{
	auto categories = std::make_unique<vod_category_storage>();

	do
	{
		const auto& all_name = load_string_resource(IDS_STRING_ALL);
		auto all_category = std::make_shared<vod_category>(all_name);
		all_category->name = all_name;
		categories->set_back(all_name, all_category);

		std::stringstream data;
		if (!m_dl.DownloadFile(m_config.m_url, data)) break;

		nlohmann::json parsed_json;
		JSON_ALL_TRY;
		{
			parsed_json = nlohmann::json::parse(data.str());
		}
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
					const auto& title = utils::get_json_wstring("title", genre_item);
					vod_genre genre({ id, title });

					movie->genres.set_back(title, genre);
				}

				category->movies.set_back(movie->id, movie);
			}
			JSON_ALL_CATCH;

			if (++cnt % 100 == 0)
			{
				m_config.SendNotifyParent(WM_UPDATE_PROGRESS, cnt, cnt);
				if (::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0) break;
			}
		}
	} while (false);

	if (::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0)
	{
		categories.reset();
	}

	m_config.SendNotifyParent(WM_END_LOAD_JSON_PLAYLIST, (WPARAM)categories.release());
}

void CPlaylistParseJsonThread::ParseSharavoz()
{
	auto categories = std::make_unique<vod_category_storage>();

	do
	{
		const auto& all_name = load_string_resource(IDS_STRING_ALL);
		auto all_category = std::make_shared<vod_category>(all_name);
		all_category->name = all_name;
		categories->set_back(all_name, all_category);

		const auto& category_json = xtream_request(L"get_vod_categories");
		if (category_json.empty()) break;

		size_t cnt = 0;
		size_t total = 0;
		for (const auto& item : category_json.items())
		{
			const auto& val = item.value();
			if (val.empty()) continue;

			std::shared_ptr<vod_category> category;
			const auto& category_id = xtream_parse_category(val, category, categories);
			if (category_id.empty()) continue;

			const auto& streams_json = xtream_request(L"get_vod_streams", L"category_id", category_id);
			if (streams_json.empty()) continue;

			total += streams_json.size();
			m_config.SendNotifyParent(WM_INIT_PROGRESS, cnt, total);

			for (const auto& item : streams_json.items())
			{
				const auto& val = item.value();
				if (val.empty()) continue;


				std::shared_ptr<vod_movie> movie;
				JSON_ALL_TRY;
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
				JSON_ALL_CATCH;

				movie->genres.set_back(category_id, category->genres.get(category_id));

				if (++cnt % 100 == 0)
				{
					m_config.SendNotifyParent(WM_UPDATE_PROGRESS, cnt, cnt);
					if (::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0) break;
				}
			}
		}

		const auto& series_cat_json = xtream_request(L"get_series_categories");
		if (series_cat_json.empty()) break;

		for (const auto& item : series_cat_json.items())
		{
			const auto& val = item.value();
			if (val.empty()) continue;

			std::shared_ptr<vod_category> category;
			const auto& category_id = xtream_parse_category(val, category, categories);
			if (category_id.empty()) continue;

			const auto& series_json = xtream_request(L"get_series", L"category_id", category_id);
			if (series_json.empty()) break;

			for (const auto& item : series_json.items())
			{
				const auto& val = item.value();
				if (val.empty()) continue;

				std::shared_ptr<vod_movie> movie;
				JSON_ALL_TRY;
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
				JSON_ALL_CATCH;

				movie->genres.set_back(category_id, category->genres.get(category_id));

				if (++cnt % 100 == 0)
				{
					m_config.SendNotifyParent(WM_UPDATE_PROGRESS, cnt, cnt);
					if (::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0) break;
				}
			}
		}

	} while (false);

	if (::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0)
	{
		categories.reset();
	}

	m_config.SendNotifyParent(WM_END_LOAD_JSON_PLAYLIST, (WPARAM)categories.release());
}

std::wstring CPlaylistParseJsonThread::xtream_parse_category(const nlohmann::json& val,
															 std::shared_ptr<vod_category>& category,
															 std::unique_ptr<vod_category_storage>& categories)
{
	std::wstring category_id;
	JSON_ALL_TRY;
	{
		const auto& title = utils::get_json_wstring("category_name", val);
		category_id = utils::get_json_wstring("category_id", val);

		if (category_id.empty())
		{
			throw std::exception("empty category_id");
		}

		auto& pair = utils::string_split(title, L'|');
		utils::string_trim(pair[0]);

		utils::string_trim(pair[1]);
		vod_genre genre({ category_id, pair[1] });

		if (!categories->tryGet(pair[0], category))
		{
			category = std::make_shared<vod_category>(pair[0]);
			category->name = pair[0];
			categories->set_back(pair[0], category);
		}

		category->genres.set_back(category_id, genre);
	}
	JSON_ALL_CATCH;


	return category_id;
}

nlohmann::json CPlaylistParseJsonThread::xtream_request(const std::wstring& action, const std::wstring param /*= L""*/, std::wstring value /*= L""*/)
{
	auto& cat_url = fmt::format(L"{:s}/player_api.php?username={:s}&password={:s}&action={:s}", m_config.m_url, m_config.m_params.password, m_config.m_params.password, action);

	if (!param.empty())
	{
		cat_url += fmt::format(L"&{:s}={:s}", param, value);
	}

	nlohmann::json category_json;
	std::stringstream cat_data;
	if (m_dl.DownloadFile(cat_url, cat_data))
	{
		JSON_ALL_TRY;
		{
			category_json = nlohmann::json::parse(cat_data.str());
		}
		JSON_ALL_CATCH;
	}

	return category_json;
}
