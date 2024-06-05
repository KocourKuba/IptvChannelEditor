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
#include "plugin_edem.h"
#include "AccountSettings.h"
#include "Constants.h"

#include "UtilsLib\utils.h"
#include "UtilsLib\json_wrapper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void plugin_edem::parse_vod(const CThreadConfig& config)
{
	int cache_ttl = GetConfig().get_int(true, REG_MAX_CACHE_TTL) * 3600;
	auto categories = std::make_unique<vod_category_storage>();

	do
	{
		const auto& all_name = load_string_resource(IDS_STRING_ALL);
		auto all_category = std::make_shared<vod_category>(all_name);
		all_category->name = all_name;
		categories->set_back(all_name, all_category);

		boost::wregex re_url(LR"(^portal::\[key:(.+)\](.+)$)");
		boost::wsmatch m;
		const auto& vportal = config.m_params.creds.get_portal();
		if (!boost::regex_match(vportal, m, re_url)) break;

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
		if (!download_url(url, data, cache_ttl, &headers, true, post.c_str())) break;

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
				if (filter_name == L"∆‡Ì")
				{
					id_tag = "genre";
				}
				else if (filter_name == L"√Ó‰")
				{
					id_tag = "years";
				}
				else
				{
					ASSERT(false);
					continue;
				}

				utils::vectormap<std::wstring, vod_filter_def> filters;
				for (const auto& filter_sub_it : filter["items"].items())
				{
					const auto& filter_sub = filter_sub_it.value();
					const auto& id = utils::get_json_wstring(id_tag, filter_sub["request"]);
					const auto& title = utils::get_json_wstring("title", filter_sub);
					vod_filter_def filter({ id, title });
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
				if (!download_url(url, cat_data, cache_ttl, &headers, true, cat_post.c_str())) break;

				const auto& data_str = cat_data.str();
				nlohmann::json movie_json = nlohmann::json::parse(data_str);
				total += utils::get_json_int("count", movie_json);
				config.SendNotifyParent(WM_INIT_PROGRESS, total, cnt);
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
							config.SendNotifyParent(WM_UPDATE_PROGRESS, cnt, cnt);
							if (::WaitForSingleObject(config.m_hStop, 0) == WAIT_OBJECT_0) break;
						}
					}

					ATLTRACE("\nreaded: %d\n", readed);

					if (offset == prev_offset || ::WaitForSingleObject(config.m_hStop, 0) == WAIT_OBJECT_0) break;

					prev_offset = offset;
					json_request["offset"] = offset;
					ATLTRACE("\noffset: %d\n", offset);

					std::stringstream mov_data;
					if (!download_url(url, mov_data, cache_ttl, &headers, true, json_request.dump().c_str())) break;

					movie_json = nlohmann::json::parse(mov_data.str());
				}

				if (::WaitForSingleObject(config.m_hStop, 0) == WAIT_OBJECT_0) break;
			}
		}
		JSON_ALL_CATCH;
	} while (false);

	if (::WaitForSingleObject(config.m_hStop, 0) == WAIT_OBJECT_0)
	{
		categories.reset();
	}

	config.SendNotifyParent(WM_END_LOAD_JSON_PLAYLIST, (WPARAM)categories.release());
}

void plugin_edem::fetch_movie_info(const Credentials& creds, vod_movie& movie)
{
	CWaitCursor cur;
	do
	{
		boost::wregex re_url(LR"(^portal::\[key:(.+)\](.+)$)");
		boost::wsmatch m;
		const auto& vportal = creds.get_portal();
		if (!boost::regex_match(vportal, m, re_url)) break;

		const auto& key = m[1].str();
		const auto& url = m[2].str();

		nlohmann::json json_request;
		json_request["cmd"] = "flick";
		json_request["limit"] = 300;
		json_request["offset"] = 0;
		json_request["key"] = utils::utf16_to_utf8(key);
		json_request["mac"] = "000000000000";
		json_request["app"] = "IPTV ChannelEditor";
		json_request["fid"] = utils::char_to_int(movie.id);

		const auto& post = json_request.dump();
		ATLTRACE("\n%s\n", post.c_str());

		std::vector<std::string> headers;
		headers.emplace_back("accept: */*");
		headers.emplace_back("Content-Type: application/json");

		std::stringstream data;
		if (!download_url(url, data, 0, &headers, true, post.c_str())) break;

		JSON_ALL_TRY;

		const auto& json_data = nlohmann::json::parse(data.str());
		const auto& type = json_data["type"];
		if (type == "multistream")
		{
			if (movie.seasons.empty())
			{
				movie.seasons.set_back(L"season", vod_season_def());
			}
			auto& season = movie.seasons.get(L"season");

			for (const auto& items_it : json_data["items"].items())
			{

				const auto& item = items_it.value();
				vod_episode_def episode;
				episode.title = utils::get_json_wstring("title", item);
				episode.url = utils::get_json_wstring("url", item);
				episode.id = utils::get_json_wstring("fid", item);
				if (episode.id.empty())
				{
					episode.id = utils::get_json_wstring("fid", item["request"]);
				}

				json_request["fid"] = utils::char_to_int(episode.id);
				const auto& item_post = json_request.dump();
				ATLTRACE("\n%s\n", post.c_str());

				std::stringstream var_data;
				if (download_url(url, var_data, 0, &headers, true, item_post.c_str()))
				{
					const auto& variants_data = nlohmann::json::parse(var_data.str());
					if (variants_data.contains("variants"))
					{
						for (const auto& variant_it : variants_data["variants"].items())
						{
							const auto& title = utils::utf8_to_utf16(variant_it.key());
							const auto& q_url = utils::utf8_to_utf16(variant_it.value().get<std::string>());

							episode.qualities.set_back(title, vod_variant_def({ title, q_url }));
						}
					}
				}

				season.episodes.set_back(episode.id, episode);
			}
		}
		else
		{
			movie.url = utils::get_json_wstring("url", json_data);
			if (json_data.contains("variants"))
			{
				for (const auto& variant_it : json_data["variants"].items())
				{
					const auto& title = utils::utf8_to_utf16(variant_it.key());
					const auto& q_url = utils::utf8_to_utf16(variant_it.value().get<std::string>());

					movie.quality.set_back(title, vod_variant_def({ title, q_url }));
				}
			}
		}
		JSON_ALL_CATCH;
	} while (false);
}

std::wstring plugin_edem::get_movie_url(const Credentials& creds, const movie_request& request, const vod_movie& movie)
{
	std::wstring url = movie.url;

	if (!movie.quality.empty() && request.quality_idx != CB_ERR)
	{
		url = movie.quality[request.quality_idx].url;
	} else if (!movie.seasons.empty())
	{
		const auto& episodes = movie.seasons.front().episodes;
		if (!episodes.empty() && request.episode_idx != CB_ERR)
		{
			const auto& quality = episodes[request.episode_idx].qualities;
			if (quality.empty())
				url = episodes[request.episode_idx].url;
			else
				url = episodes[request.episode_idx].qualities[request.quality_idx].url;
		}
	}

	return url;
}

void plugin_edem::update_entry(PlaylistEntry& entry)
{
	const auto& pl_info = get_current_playlist_info();
	if (!pl_info.get_square_icons())
	{
		entry.set_icon_uri(utils::string_replace<wchar_t>(entry.get_icon_uri().get_uri(), L"/img/", L"/img2/"));
	}
}
