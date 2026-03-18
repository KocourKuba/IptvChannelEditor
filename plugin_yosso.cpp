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
#include "plugin_yosso.h"
#include "Constants.h"
#include "AccountSettings.h"
#include "UtilsLib\utils.h"
#include "UtilsLib\inet_utils.h"

#include "UtilsLib\md5.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr auto VOD_API_REQUEST = L"{:s}/{:s}";
constexpr auto VOD_API_REQUEST_ID = L"{:s}/{:s}/{:s}";
constexpr auto VOD_API_REQUEST_ID2 = L"{:s}/{:s}/{:s}/{:s}";
constexpr auto PARAM_FMT = "{:s}={:s}";
constexpr auto JF_APP_VERSION = "1.0.0";

// Function to generate UUID v3 (MD5-based, deterministic)
std::string genUserUUID(const std::string& name) {
	static std::string editorUID = "52F7B990-1EB3-47F3-A8D5-FA640E7B2142";
	constexpr char hexval[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	DWORD volumeSerialNumber = 0;
	GetVolumeInformationA( "C:\\", nullptr, 0, &volumeSerialNumber, nullptr, nullptr, nullptr, 0);

	const auto& compID = std::format("{:08x}", volumeSerialNumber);

	std::string combined(compID + editorUID + name);
	// Hash with MD5
	auto hash = utils::md5_hash(combined.c_str(), combined.size());

	// Set version (3) and variant (RFC 4122)
	hash[6] = (hash[6] & 0x0F) | 0x30; // Version 3
	hash[8] = (hash[8] & 0x3F) | 0x80; // Variant

	// Convert to UUID string
	std::string hex;
	for (size_t i = 0; i < hash.size(); ++i)
	{
		const auto val = hash[i];
		hex.push_back(hexval[((val >> 4) & 0xF)]);
		hex.push_back(hexval[val & 0x0F]);
		if (i == 3 || i == 5 || i == 7 || i == 9)
		{
			hex.push_back('-');
		}
	}

	return hex;
}


bool plugin_yosso::get_vod_api_token(TemplateParams& params, std::string& api_token)
{
	if (!params.creds->s_token.empty())
	{
		return true;
	}

	std::wstring login_url = std::format(VOD_API_REQUEST, get_vod_url(params), L"Users/AuthenticateByName");
	const auto& auth_json = jellyfin_request(params, login_url, {}, false);
	if (auth_json.empty())
	{
		return false;
	}

	params.creds->s_token = utils::get_json_string("AccessToken", auth_json);

	api_token = params.creds->s_token;
	return !api_token.empty();
}

void plugin_yosso::parse_vod(const ThreadConfig& config)
{
	auto categories = std::make_unique<vod_category_storage>();

	do
	{
		const auto& all_name = load_string_resource(IDS_STRING_ALL);
		auto all_category = std::make_shared<vod_category>(all_name);
		all_category->name = all_name;
		categories->set_back(all_name, all_category);

		const auto& api_url = get_vod_url(config.m_params);

		const auto& user_view_request_url = std::format(VOD_API_REQUEST, api_url, L"UserViews");
		const auto& category_json = jellyfin_request(config.m_params, user_view_request_url);
		if (category_json.empty()) break;

		int cnt = 0;
		int total = 0;
		constexpr int limit = 200;
		utils::progress_info info;
		if (!category_json.contains("Items")) break;

		const auto& items_request_url = std::format(VOD_API_REQUEST, api_url, L"Items");
		for (const auto& category_item : category_json["Items"].items())
		{
			// collection
			const auto& val = category_item.value();
			if (val.empty()) continue;

			const auto& type = utils::get_json_string("Type", val);
			if (type != "CollectionFolder") continue;

			const auto& category_id = utils::get_json_string("Id", val);
			if (category_id.empty()) continue;

			const auto& collection_type = utils::get_json_string("CollectionType", val);
			bool is_movies = true;
			if (collection_type == "tvshows") {
				is_movies = false;
			}

			const auto& category_id_w = utils::utf8_to_utf16(category_id);
			auto category = std::make_shared<vod_category>(category_id_w);
			category->name = utils::get_json_wstring("Name", val);

			std::map<std::string, std::string> query_params = {
				{"ParentId", category_id},
				{"recursive", "true"}
			};

			const auto& filters_request_url = std::format(VOD_API_REQUEST, api_url, L"Items/Filters");
			const auto& filters_json = jellyfin_request(config.m_params, filters_request_url, query_params);
			if (filters_json.contains("Genres"))
			{
				for (const auto& item : filters_json["Genres"])
				{
					const auto& genre_name = utils::get_json_wstring("", item);

					category->genres.set_back(genre_name, { genre_name, genre_name });
				}
			}

			if (filters_json.contains("Years"))
			{
				for (const auto& item : filters_json["Years"])
				{
					const auto& year_name = utils::get_json_wstring("", item);

					category->years.set_back(year_name, { year_name, year_name });
				}
			}

			int idx = 0;
			query_params.clear();
			query_params = {
				{"ParentId", category_id},
				{"Limit", "500"},
				{"IncludeItemTypes", is_movies ? "Movies" : "Series"},
			};

			for (;;)
			{
				query_params["StartIndex"] = std::to_string(idx * 500);
				const auto& movies_json = jellyfin_request(config.m_params, items_request_url, query_params);
				if (movies_json.empty() || !movies_json.contains("Items") || movies_json["Items"].empty()) break;

				for (const auto& item : movies_json["Items"].items())
				{
					const auto& val = item.value();
					if (val.empty()) continue;


					std::shared_ptr<vod_movie_def> movie;
					const auto& movie_id = utils::get_json_wstring("Id", val);
					if (movie_id.empty()) continue;

					if (!category->movies.tryGet(movie_id, movie))
					{
						movie = std::make_shared<vod_movie_def>();
						movie->id = movie_id;
						movie->title = utils::get_json_wstring("Name", val);
						movie->year = utils::get_json_wstring("ProductionYear", val);
						movie->is_series = !is_movies;
						movie->poster_url.set_uri(std::format(L"{:s}/{:s}/Images/Primary?maxWidth=400&format=Png", items_request_url, movie_id));

						category->movies.set_back(movie_id, movie);
					}

					if (++cnt % limit == 0)
					{
						info.curPos = info.value = cnt;
						config.progress_callback(info);

						if (::WaitForSingleObject(config.m_hStop, 0) == WAIT_OBJECT_0) break;
					}
				}
				idx++;
			}
			if (!category->movies.empty())
			{
				categories->set_back(category_id_w, category);
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

void plugin_yosso::fetch_movie_info(const Credentials& creds, vod_movie_def& movie)
{
	TemplateParams params;
	params.creds = std::make_shared<Credentials>(creds);
	update_provider_params(params);

	const auto& api_url = get_vod_url(params);
	const auto& items_request_url = std::format(VOD_API_REQUEST, api_url, L"Items");
	const auto& movie_request_url = std::format(VOD_API_REQUEST_ID, api_url, L"Items", movie.id);

	const auto& movie_json = jellyfin_request(params, movie_request_url);
	if (movie_json.empty()) return;

	JSON_ALL_TRY
	{
		if (movie.is_series)
		{
			const auto& seasons_request_url = std::format(VOD_API_REQUEST_ID2, api_url, L"Shows", movie.id, L"Seasons");
			const auto& shows_json = jellyfin_request(params, seasons_request_url);
			for (const auto& season_it : shows_json["Items"].items())
			{
				const auto season_id = utils::get_json_string("Id", season_it.value());
				if (season_id.empty()) continue;
				const auto& season_wid = utils::utf8_to_utf16(season_id);
				vod_season_def season;
				season.id = season_wid;
				season.number = season_wid;
				season.title = utils::get_json_wstring("Name", season_it.value());


				std::map<std::string, std::string> query_params = {
					{"SeasonId", season_id},
					{"sortBy", "IndexNumber"}
				};

				const auto& episodes_request_url = std::format(VOD_API_REQUEST_ID2, api_url, L"Shows", movie.id,  L"Episodes");
				const auto& episodes_json = jellyfin_request(params, episodes_request_url, query_params);

				for (auto& episode_it : episodes_json["Items"].items())
				{
					const auto& episode_id = utils::get_json_wstring("Id", episode_it.value());

					vod_episode_def episode;
					episode.id = episode_id;
					episode.title = utils::get_json_wstring("Name", episode_it.value());

					const auto& episode_request_url = std::format(VOD_API_REQUEST_ID, api_url, L"Items", episode_id);
					const auto& episode_json = jellyfin_request(params, episode_request_url);

					collect_qualities(episode, items_request_url, creds, episode_json["MediaSources"]);
					season.episodes.set_back(episode_id, episode);
				}

				movie.seasons.set_back(season_wid, season);
			}
		}
		else
		{
			collect_qualities(movie, items_request_url, creds, movie_json["MediaSources"]);
		}

		std::map<std::string, std::wstring> people;
		for (const auto& person_it : movie_json["People"].items())
		{
			const auto& person_type = utils::get_json_string("Type", person_it.value());
			const auto& person_name = utils::get_json_wstring("Name", person_it.value());
			if (!person_type.empty() && !person_name.empty())
			{
				if (!empty(people[person_type]))
				{
					people[person_type] += L", ";
				}
				people[person_type] += person_name;
			}

		}

		for (const auto& genre_item : movie_json["Genres"].items())
		{
			const auto& genre_title = utils::get_json_wstring("", genre_item.value());
			vod_genre_def genre({ genre_title , genre_title });
			movie.genres.set_back(genre_title, genre);
		}

		movie.title_orig = utils::get_json_wstring("OriginalTitle", movie_json);
		movie.description = utils::get_json_wstring("Overview", movie_json);
		movie.country = utils::get_json_wstring("ProductionLocations", movie_json);
		movie.casting = people["Actor"];
		movie.director = people["Director"];
	}
	JSON_ALL_CATCH
}

void plugin_yosso::collect_qualities(vod_episode_def& movie, const std::wstring& items_request_url, const Credentials& creds, const nlohmann::json& item) const
{
	for (auto& it : item.items())
	{
		const auto& media_sources = it.value();
		const auto& stream_id = utils::get_json_wstring("Id", media_sources);
		if (stream_id.empty()) continue;

		for (auto& streams_it : media_sources["MediaStreams"].items())
		{
			const auto& type = utils::get_json_string("Type", streams_it.value());
			if (utils::string_tolower_copy(type) != "video") continue;

			vod_variant_def quality;
			quality.title = utils::get_json_wstring("DisplayTitle", streams_it.value());
			quality.url = std::format(L"{:s}/{:s}/Download?apiKey={:s}", items_request_url, stream_id, creds.get_s_token());

			movie.qualities.set_back(quality.title, quality);
		}

	}
}

std::wstring plugin_yosso::get_movie_url(const Credentials& creds, const movie_request& request, const vod_movie_def& movie)
{
	std::wstring url = movie.url;

	if (!movie.qualities.empty() && request.quality_idx != CB_ERR)
	{
		url = movie.qualities[request.quality_idx].url;
	}
	else if (!movie.seasons.empty())
	{
		const auto& episodes = movie.seasons[request.season_idx].episodes;
		if (!episodes.empty() && request.episode_idx != CB_ERR)
		{
			const auto& quality = episodes[request.episode_idx].qualities;
			if (quality.empty())
			{
				url = episodes[request.episode_idx].url;
			}
			else
			{
				url = episodes[request.episode_idx].qualities[request.quality_idx].url;
			}
		}
	}

	return url;
}

std::wstring plugin_yosso::jellyfin_parse_category(const nlohmann::json& val,
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

nlohmann::json plugin_yosso::jellyfin_request(const TemplateParams& params, const std::wstring& url, const std::map<std::string, std::string>& query_params /*= {}*/, bool authorized /*= true*/)
{
	utils::http_request req
	{
		.url = url,
		.cache_ttl = GetConfig().get_chrono(true, REG_MAX_CACHE_TTL),
		.headers{"Content-Type: application/json; charset=utf-8", "Accept: application/json", build_auth_header(*params.creds, authorized)},
	};

	if (!authorized)
	{
		nlohmann::json json_request;
		json_request["Username"] = params.creds->login;
		json_request["Pw"] = params.creds->password;

		req.post_data = json_request.dump();
		req.verb_post = true;
	}

	std::string query;
	for (const auto& param : query_params)
	{
		if (!query.empty())
		{
			query += "&";
		}
		query += std::format(PARAM_FMT, param.first, param.second);
	}

	if (!query.empty())
	{
		req.url += L"?" + utils::utf8_to_utf16(query);
	}

	nlohmann::json response_json;
	if (utils::DownloadFile(req))
	{
		JSON_ALL_TRY
		{
			response_json = nlohmann::json::parse(req.body.str());
		}
		JSON_ALL_CATCH
	}

	return response_json;
}

std::string plugin_yosso::build_auth_header(const Credentials& creds, bool auth /*= true*/)
{
	const auto& device_id = genUserUUID(creds.login);
	auto auth_hdr = std::format(R"(Authorization: MediaBrowser Client="IPTV Channels Editor",Device="windows",DeviceId="{:s}",Version="{:s}")", device_id, JF_APP_VERSION);

	if (auth && !creds.s_token.empty())
	{
		auth_hdr += std::format(R"(,Token="{:s}")", creds.s_token);
	}

	return auth_hdr;

}
