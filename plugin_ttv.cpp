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
#include "plugin_ttv.h"
#include "Constants.h"
#include "SettingsStorage.h"

#include "UtilsLib\utils.h"
#include "UtilsLib\md5.h"
#include "UtilsLib\inet_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// API documentation https://tapitv.com/v3/api_v3.html

constexpr auto API_COMMAND_AUTH = L"{{API_URL}}/auth.php?username={:s}&password={:s}&application=android&guid={:s}&typeresult=json";
constexpr auto API_COMMAND_URL = L"{{API_URL}}/{:s}.php?session={:s}&typeresult=json";
constexpr auto SESSION_TOKEN_TEMPLATE = "session_token_{:s}";
// error code of the expired or revoked session, new session must be requested
constexpr auto ERROR_INCORRECT = "incorrect";
// types of the film_filter request
constexpr auto TYPE_ALL = L"all";
constexpr auto TYPE_SERIAL = L"serial";
// part of the directors role name in the film_alt_info persons
constexpr auto ROLE_DIRECTOR = L"\u0435\u0436\u0438\u0441\u0441";
constexpr int PAGE_SIZE = 100;
// parallel requests for the movies of the genre
constexpr int PARALLEL_REQUESTS = 8;
// session does not expire while in use, refresh it once a week
constexpr time_t SESSION_LIFETIME = 7 * 24 * 3600;

namespace
{
std::wstring get_rating(const nlohmann::json& node)
{
	auto rating = utils::get_json_wstring("kp_rating", node);
	return rating.empty() ? utils::get_json_wstring("imdb_rating", node) : rating;
}

// 'BDRIP 1080P'
std::wstring get_quality_name(const nlohmann::json& file)
{
	auto quality = utils::get_json_wstring("quality", file);
	const auto& hd = utils::get_json_wstring("hd_standard", file);
	if (!hd.empty())
	{
		if (!quality.empty())
		{
			quality += L" ";
		}
		quality += hd;
	}

	std::ranges::transform(quality, quality.begin(), [](const wchar_t c) { return static_cast<wchar_t>(std::towupper(c)); });
	return quality;
}

// persons is a role keyed object: { "Directors": [ { "rus_name": "", "orig_name": "" }, ... ], ... }
std::wstring get_persons(const nlohmann::json& info, const std::wstring& role_stem)
{
	std::wstring result;
	if (!info.contains("persons") || !info["persons"].is_object()) return result;

	for (const auto& [role, persons] : info["persons"].items())
	{
		if (!persons.is_array() || utils::utf8_to_utf16(role).find(role_stem) == std::wstring::npos) continue;

		for (const auto& person : persons)
		{
			auto name = utils::get_json_wstring("rus_name", person);
			if (name.empty())
			{
				name = utils::get_json_wstring("orig_name", person);
			}

			if (name.empty()) continue;

			if (!result.empty())
			{
				result += L", ";
			}
			result += name;
		}
		break;
	}

	return result;
}

std::wstring join_json_array(const nlohmann::json& node, const std::string& key)
{
	std::wstring result;
	if (!node.contains(key) || !node[key].is_array()) return result;

	for (const auto& item : node[key])
	{
		const auto& value = utils::get_json_wstring("", item);
		if (value.empty()) continue;

		if (!result.empty())
		{
			result += L", ";
		}
		result += value;
	}

	return result;
}

// add file as quality variant, the files of the same quality are distinguished by the file name
void add_quality(const nlohmann::json& file, vod_episode_def& episode)
{
	vod_variant_def quality;
	quality.id = utils::get_json_wstring("tfile_id", file);
	quality.title = get_quality_name(file);
	quality.url = utils::get_json_wstring("url", file);
	if (quality.title.empty() || episode.qualities.contains(quality.title))
	{
		quality.title = utils::get_json_wstring("name", file);
	}

	if (!quality.url.empty() && !episode.qualities.contains(quality.title))
	{
		episode.qualities.set_back(quality.title, quality);
	}
}
}

bool plugin_ttv::get_api_token(TemplateParams& params, std::string& api_token)
{
	if (!login(params))
	{
		AfxMessageBox(L"plugin_ttv: Login failed", MB_OK | MB_ICONERROR);
		return false;
	}

	// playlist token is used by the playlist template
	params.creds->s_token = playlist_token;
	api_token = session;
	return true;
}

void plugin_ttv::clear_account_info()
{
	delete_file_cookie(session_file);
	session.clear();
	playlist_token.clear();
	return account_info.clear();
}

std::string plugin_ttv::make_guid(const std::string& login)
{
	// device id must be the same for every auth request
	const auto& hash = utils::md5_hash_hex("torrenttv:" + login);
	return std::format("{:s}-{:s}-{:s}-{:s}-{:s}", hash.substr(0, 8), hash.substr(8, 4), hash.substr(12, 4), hash.substr(16, 4), hash.substr(20, 12));
}

bool plugin_ttv::login(const TemplateParams& params, bool force /*= false*/)
{
	if (params.creds->login.empty() || params.creds->password.empty())
	{
		return false;
	}

	const auto& name = utils::utf8_to_utf16(std::format(SESSION_TOKEN_TEMPLATE,
														utils::md5_hash_hex(params.creds->login + utils::md5_hash_hex(params.creds->password))));
	if (name != session_file)
	{
		// credentials changed
		session_file = name;
		session.clear();
		playlist_token.clear();
	}

	if (force)
	{
		delete_file_cookie(session_file);
		session.clear();
		playlist_token.clear();
	}

	if (!session.empty() && !playlist_token.empty())
	{
		return true;
	}

	// saved session: session|playlist_token
	const auto& saved = utils::string_split(get_file_cookie(session_file), '|');
	if (saved.size() == 2 && !saved[0].empty() && !saved[1].empty())
	{
		session = saved[0];
		playlist_token = saved[1];
		return true;
	}

	utils::http_request req
	{
		.url = replace_params_vars(params, std::format(API_COMMAND_AUTH,
													   utils::utf8_to_utf16(params.creds->login),
													   utils::utf8_to_utf16(params.creds->password),
													   utils::utf8_to_utf16(make_guid(params.creds->login)))),
		.timeouts = GetConfig().GetTimeouts(),
	};

	const auto& auth = download_json(req);
	if (utils::get_json_int("success", auth) != 1 || utils::get_json_string("session", auth).empty())
	{
		LOG_PROTOCOL(std::format(L"plugin_ttv: Login failed: {:s} {:s}", utils::get_json_wstring("error", auth), req.error_message));
		return false;
	}

	session = utils::get_json_string("session", auth);

	// playlist token is returned only by the account info
	utils::http_request info_req
	{
		.url = replace_params_vars(params, std::format(API_COMMAND_URL, L"userinfo", utils::utf8_to_utf16(session))),
		.timeouts = GetConfig().GetTimeouts(),
	};

	const auto& info = download_json(info_req);
	playlist_token = utils::get_json_string("playlist_token", info);
	if (utils::get_json_int("success", info) != 1 || playlist_token.empty())
	{
		LOG_PROTOCOL(std::format(L"plugin_ttv: Failed to get playlist token: {:s} {:s}", utils::get_json_wstring("error", info), info_req.error_message));
		session.clear();
		playlist_token.clear();
		return false;
	}

	set_file_cookie(session_file, session + "|" + playlist_token, time(nullptr) + SESSION_LIFETIME);
	return true;
}

nlohmann::json plugin_ttv::download_json(utils::http_request& req) const
{
	nlohmann::json result;
	if (utils::DownloadFile(req))
	{
		JSON_ALL_TRY
		{
			result = nlohmann::json::parse(req.body.str());
		}
		JSON_ALL_CATCH
	}

	return result;
}

nlohmann::json plugin_ttv::api_request(const TemplateParams& params,
									   const std::wstring& command,
									   const std::wstring& query /*= L""*/,
									   bool use_cache /*= false*/)
{
	for (int attempt = 0; attempt < 2; attempt++)
	{
		// session revoked or expired, log in again and repeat once
		if (!login(params, attempt != 0)) break;

		utils::http_request req
		{
			.url = replace_params_vars(params, std::format(API_COMMAND_URL, command, utils::utf8_to_utf16(session))) + query,
			.cache_ttl = use_cache ? GetConfig().get_chrono(true, REG_MAX_CACHE_TTL) : std::chrono::seconds::zero(),
			.timeouts = GetConfig().GetTimeouts(),
		};

		const auto& result = download_json(req);
		if (utils::get_json_int("success", result) == 1)
		{
			return result;
		}

		// error response must not be taken from cache next time
		utils::ClearCachedUrl(req.url);

		const auto& error = utils::get_json_string("error", result);
		LOG_PROTOCOL(std::format(L"plugin_ttv: Bad response ({:s} {:s}) on request: {:s}", utils::utf8_to_utf16(error), req.error_message, command));
		if (error != ERROR_INCORRECT) break;
	}

	return {};
}

void plugin_ttv::parse_account_info(TemplateParams& params)
{
	if (!account_info.empty()) return;

	const auto& info = api_request(params, L"userinfo");
	if (info.empty()) return;

	JSON_ALL_TRY
	{
		set_json_info("email", info, account_info);
		set_json_info("balance", info, account_info);
		set_json_info("tsproxy_tariff", info, account_info);
		set_json_info("timezone_name", info, account_info);
		account_info.emplace(L"vip_status", utils::get_json_int("vip_status", info) != 0);
		account_info.emplace(L"tsproxy_access", utils::get_json_int("tsproxy_access", info) != 0);

		const auto expire = utils::get_json_number_value("tsproxy_date", info);
		if (expire != 0)
		{
			COleDateTime dt(static_cast<time_t>(expire));
			account_info.emplace(L"tsproxy_date", std::format(L"{:d}.{:d}.{:d}", dt.GetDay(), dt.GetMonth(), dt.GetYear()));
		}
	}
	JSON_ALL_CATCH
}

void plugin_ttv::parse_vod(const ThreadConfig& config)
{
	auto categories = std::make_unique<vod_category_storage>();
	genre_movies_cache.clear();

	do
	{
		const auto& all_name = load_string_resource(IDS_STRING_ALL);
		auto all_category = std::make_shared<vod_category>(all_name);
		all_category->name = all_name;
		categories->set_back(all_name, all_category);

		const auto& params = config.m_params;

		// sections: film, serial, animation
		const auto& sections = api_request(params, L"film_alt_genres", L"", true);
		if (!sections.contains("data") || !sections["data"].is_array()) break;

		// total count of the catalog for progress
		const auto& total = api_request(params, L"film_filter", std::format(L"&type={:s}&page=1&page_size=1", TYPE_ALL), true);

		int cnt = 0;
		utils::progress_info info{ .type = utils::ProgressType::Initializing };
		if (total.contains("data"))
		{
			info.maxPos = utils::get_json_int("count", total["data"]);
		}
		config.progress_callback(info);
		info.type = utils::ProgressType::Progress;

		for (const auto& section : sections["data"])
		{
			if (::WaitForSingleObject(config.m_hStop, 0) == WAIT_OBJECT_0) break;

			JSON_ALL_TRY
			{
				const auto& type = utils::get_json_wstring("type", section);
				const auto& name = utils::get_json_wstring("name", section);
				if (type.empty() || name.empty() || categories->contains(type)) continue;

				auto category = std::make_shared<vod_category>(type);
				category->name = name;
				categories->set_back(type, category);

				// movies list does not contain genres, movies of the genre are requested by get_genre_movies.
				// Genre without id is 'All genres', -1 is user favorites
				if (section.contains("genres"))
				{
					for (const auto& genre : section["genres"])
					{
						const auto genre_id = genre.contains("query") ? utils::get_json_int("genre_id", genre["query"]) : 0;
						const auto& genre_name = utils::get_json_wstring("name", genre);
						if (genre_id <= 0 || genre_name.empty()) continue;

						vod_genre_def genre_def({ std::to_wstring(genre_id), genre_name });
						category->genres.set_back(genre_def.id, genre_def);
						all_category->genres.set_back(genre_def.id, genre_def);
					}
				}

				for (int page = 1;; page++)
				{
					const auto& films = api_request(params, L"film_filter", std::format(L"&type={:s}&page={:d}&page_size={:d}", type, page, PAGE_SIZE), true);
					if (!films.contains("data") || !films["data"].contains("result")) break;

					const auto& data = films["data"];
					for (const auto& item : data["result"])
					{
						const auto& movie_id = utils::get_json_wstring("id", item);
						if (movie_id.empty() || category->movies.contains(movie_id)) continue;

						auto movie = std::make_shared<vod_movie_def>();
						movie->id = movie_id;
						movie->title = utils::get_json_wstring("name", item);
						movie->is_series = (type == TYPE_SERIAL);
						movie->year = utils::get_json_wstring("year", item);
						movie->rating = get_rating(item);
						movie->country = utils::get_json_wstring("country", item);
						// first 255 chars, full description is loaded with movie info
						movie->description = utils::get_json_wstring("description", item);
						movie->movie_time = utils::char_to_int(utils::get_json_wstring("length", item)); // '119 min. / 01:59'
						movie->poster_url.set_uri(utils::get_json_wstring("poster", item));
						movie->category = type;

						category->movies.set_back(movie_id, movie);
						++cnt;
					}

					info.curPos = info.value = cnt;
					config.progress_callback(info);

					if (::WaitForSingleObject(config.m_hStop, 0) == WAIT_OBJECT_0) break;
					if (page >= utils::get_json_int("pages", data)) break;
				}
			}
			JSON_ALL_CATCH
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

void plugin_ttv::fetch_movie_info(const TemplateParams& params, vod_movie_def& movie)
{
	const auto& film_query = std::format(L"&film_id={:s}", movie.id);

	JSON_ALL_TRY
	{
		const auto& info = api_request(params, L"film_alt_info", film_query, true);
		if (info.contains("data"))
		{
			const auto& data = info["data"];
			const auto& description = utils::get_json_wstring("description", data);
			if (!description.empty())
			{
				movie.description = description;
			}

			movie.country = utils::get_json_wstring("country", data);
			movie.casting = join_json_array(data, "actors");
			movie.director = get_persons(data, ROLE_DIRECTOR);

			if (data.contains("genres") && data["genres"].is_array())
			{
				for (const auto& genre_item : data["genres"])
				{
					const auto& genre = utils::get_json_wstring("", genre_item);
					if (!genre.empty())
					{
						movie.genres.set_back(genre, vod_genre_def({ genre, genre }));
					}
				}
			}
		}

		// playable files. Series has season and episode number for every file
		const auto& files_json = api_request(params, L"film_alt_http", film_query, true);
		if (!files_json.contains("data") || !files_json["data"].is_array() || files_json["data"].empty()) return;

		const auto& files = files_json["data"];
		const auto duration = utils::get_json_int("duration", files[0]);
		if (duration)
		{
			movie.movie_time = duration / 60;
		}

		// season number is false for the movie
		std::map<int, std::map<int, std::vector<nlohmann::json>>> seasons;
		for (const auto& file : files)
		{
			if (file.contains("season") && file["season"].is_number())
			{
				seasons[file["season"].get<int>()][utils::get_json_int("episode", file)].emplace_back(file);
			}
		}

		if (seasons.empty())
		{
			movie.is_series = false;
			movie.url = utils::get_json_wstring("url", files[0]);
			if (files.size() > 1)
			{
				for (const auto& file : files)
				{
					add_quality(file, movie);
				}
			}
			return;
		}

		movie.is_series = true;
		for (const auto& [season_num, episodes] : seasons)
		{
			vod_season_def season;
			season.id = std::to_wstring(season_num);
			season.number = season.id;
			season.title = load_string_resource(IDS_STRING_SEASON) + L" " + season.number;

			for (const auto& [episode_num, episode_files] : episodes)
			{
				const auto& first = episode_files.front();

				vod_episode_def episode;
				episode.id = utils::get_json_wstring("tfile_id", first);
				if (episode.id.empty())
				{
					episode.id = std::format(L"{:d}_{:d}", season_num, episode_num);
				}
				episode.number = std::to_wstring(episode_num);
				episode.title = utils::get_json_wstring("name", first);
				episode.url = utils::get_json_wstring("url", first);

				if (episode_files.size() > 1)
				{
					for (const auto& file : episode_files)
					{
						add_quality(file, episode);
					}
				}

				season.episodes.set_back(episode.id, episode);
			}

			movie.seasons.set_back(season.id, season);
		}
	}
	JSON_ALL_CATCH
}

bool plugin_ttv::get_genre_movies(const TemplateParams& params, const std::wstring& category_id, const std::wstring& genre_id, std::set<std::wstring>& movie_ids)
{
	// category id is the type of the film_filter request, 'All' category contains all types
	const auto& type = (category_id == load_string_resource(IDS_STRING_ALL)) ? std::wstring(TYPE_ALL) : category_id;
	const auto& key = type + L"|" + genre_id;
	if (const auto& it = genre_movies_cache.find(key); it != genre_movies_cache.end())
	{
		movie_ids = it->second;
		return true;
	}

	const auto& query = std::format(L"&type={:s}&genres_include[]={:s}&page_size={:d}&page=", type, genre_id, PAGE_SIZE);
	const auto add_movies = [&movie_ids](const nlohmann::json& films)
	{
		for (const auto& item : films["data"]["result"])
		{
			const auto& movie_id = utils::get_json_wstring("id", item);
			if (!movie_id.empty())
			{
				movie_ids.emplace(movie_id);
			}
		}
	};

	bool loaded = false;
	JSON_ALL_TRY
	{
		// first page checks the session and gives the pages count
		const auto& first = api_request(params, L"film_filter", query + L"1", true);
		if (first.contains("data") && first["data"].contains("result"))
		{
			loaded = true;
			add_movies(first);

			// big genre has dozens of pages, they are downloaded in parallel
			const auto pages = utils::get_json_int("pages", first["data"]);
			for (int batch = 2; batch <= pages; batch += PARALLEL_REQUESTS)
			{
				std::vector<utils::http_request> requests(std::min(PARALLEL_REQUESTS, pages - batch + 1));
				std::vector<std::future<bool>> results;
				for (size_t i = 0; i < requests.size(); i++)
				{
					requests[i].url = replace_params_vars(params, std::format(API_COMMAND_URL, L"film_filter", utils::utf8_to_utf16(session)))
						+ query + std::to_wstring(batch + i);
					requests[i].cache_ttl = GetConfig().get_chrono(true, REG_MAX_CACHE_TTL);
					requests[i].timeouts = GetConfig().GetTimeouts();
					results.emplace_back(utils::AsyncDownloadFile(requests[i]));
				}

				for (size_t i = 0; i < requests.size(); i++)
				{
					nlohmann::json films;
					if (results[i].get())
					{
						JSON_ALL_TRY
						{
							films = nlohmann::json::parse(requests[i].body.str());
						}
						JSON_ALL_CATCH
					}

					if (utils::get_json_int("success", films) == 1 && films.contains("data") && films["data"].contains("result"))
					{
						add_movies(films);
					}
					else
					{
						LOG_PROTOCOL(std::format(L"plugin_ttv: Bad response ({:s}) on request: {:s}", requests[i].error_message, requests[i].url));
						utils::ClearCachedUrl(requests[i].url);
						loaded = false;
					}
				}
			}
		}
	}
	JSON_ALL_CATCH

	// incomplete list is not cached, it will be requested again on next selection
	if (loaded)
	{
		genre_movies_cache.emplace(key, movie_ids);
	}

	return true;
}
