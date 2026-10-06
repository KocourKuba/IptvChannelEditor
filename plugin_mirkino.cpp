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
#include "plugin_mirkino.h"
#include "Constants.h"
#include "SettingsStorage.h"

#include "UtilsLib\utils.h"
#include "UtilsLib\md5.h"
#include "UtilsLib\inet_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr auto SESSION_TOKEN_TEMPLATE = "session_token_{:s}";
constexpr auto AUTH_HEADER = R"(Authorization: MediaBrowser Client="IPTV Channel Editor", Device="PC", DeviceId="{:s}", Version="{:s}")";
constexpr auto ACCEPT_JSON_HEADER = "Accept: application/json";
constexpr auto ITEM_MOVIE = "Movie";
constexpr auto ITEM_SERIES = "Series";
constexpr auto COLLECTION_MOVIES = "movies";
constexpr auto COLLECTION_TVSHOWS = "tvshows";
// Audio codecs played by Dune in MPEG-TS segments. Other codecs are transcoded by server to the first one
constexpr auto HLS_AUDIO_CODECS = L"aac,ac3,eac3,mp3,mp2,dts";
constexpr int PAGE_LIMIT = 500;
// RunTimeTicks are in 100 ns units
constexpr long long TICKS_PER_MINUTE = 600000000LL;

namespace
{
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

std::wstring format_rating(const nlohmann::json& node, const std::string& key)
{
	if (!node.contains(key) || !node[key].is_number()) return {};

	return std::format(L"{:.1f}", node[key].get<double>());
}

// Jellyfin reports container as ffprobe format names, for example "mov,mp4,m4a,3gp,3g2,mj2" or "matroska,webm"
std::wstring get_stream_extension(const std::wstring& container)
{
	static const std::pair<std::wstring, std::wstring> extensions[] = {
		{ L"mp4", L"mp4" },
		{ L"matroska", L"mkv" },
		{ L"mkv", L"mkv" },
		{ L"mov", L"mov" },
	};

	const auto& formats = utils::string_split(utils::wstring_tolower_l_copy(container), L',');
	for (const auto& [format, ext] : extensions)
	{
		if (std::find(formats.begin(), formats.end(), format) != formats.end())
		{
			return ext;
		}
	}

	return {};
}
}

bool plugin_mirkino::get_vod_api_token(TemplateParams& params, std::string& api_token)
{
	if (!login(params))
	{
		AfxMessageBox(L"plugin_mirkino: Login failed", MB_OK | MB_ICONERROR);
		return false;
	}

	api_token = access_token;
	return true;
}

void plugin_mirkino::clear_account_info()
{
	delete_file_cookie(session_file);
	access_token.clear();
	user_id.clear();
	return account_info.clear();
}

bool plugin_mirkino::login(const TemplateParams& params, bool force /*= false*/)
{
	auto url = get_vod_url(params);
	while (!url.empty() && url.back() == L'/')
	{
		url.pop_back();
	}

	const auto& hash = utils::md5_hash_hex(utils::utf16_to_utf8(url) + params.creds->login + utils::md5_hash_hex(params.creds->password));
	const auto& session = utils::utf8_to_utf16(std::format(SESSION_TOKEN_TEMPLATE, hash));
	if (url != base_url || session != session_file)
	{
		base_url = url;
		session_file = session;
		access_token.clear();
		user_id.clear();
	}

	device_id = utils::utf8_to_utf16(utils::md5_hash_hex(params.creds->login));

	if (force)
	{
		delete_file_cookie(session_file);
		access_token.clear();
		user_id.clear();
	}

	if (!access_token.empty() && !user_id.empty())
	{
		return true;
	}

	// saved session: token|user_id
	const auto& saved = utils::string_split(get_file_cookie(session_file), '|');
	if (saved.size() == 2 && !saved[0].empty() && !saved[1].empty())
	{
		access_token = saved[0];
		user_id = saved[1];
		return true;
	}

	if (base_url.empty())
	{
		return false;
	}

	nlohmann::json json_request;
	json_request["Username"] = params.creds->login;
	json_request["Pw"] = params.creds->password;

	auto headers = build_headers(false);
	headers.emplace_back(CONTENT_TYPE_JSON);

	utils::http_request req
	{
		.url = base_url + L"/Users/AuthenticateByName",
		.request_headers = headers,
		.post_data = json_request.dump(),
		.verb_post = true,
		.timeouts = GetConfig().GetTimeouts(),
	};

	const auto& parsed_json = jellyfin_request(req);
	if (parsed_json.empty())
	{
		LOG_PROTOCOL(std::format(L"plugin_mirkino: Login failed: {:s}", req.error_message));
		return false;
	}

	JSON_ALL_TRY
	{
		access_token = utils::get_json_string("AccessToken", parsed_json);
		if (parsed_json.contains("User"))
		{
			user_id = utils::get_json_string("Id", parsed_json["User"]);
		}

		if (user_id.empty() && parsed_json.contains("SessionInfo"))
		{
			user_id = utils::get_json_string("UserId", parsed_json["SessionInfo"]);
		}
	}
	JSON_ALL_CATCH

	if (access_token.empty() || user_id.empty())
	{
		access_token.clear();
		user_id.clear();
		return false;
	}

	// jellyfin token does not expire until revoked, refresh it once a week
	set_file_cookie(session_file, access_token + "|" + user_id, time(nullptr) + 7 * 24 * 3600);
	return true;
}

std::vector<std::string> plugin_mirkino::build_headers(bool auth) const
{
	auto authorization = std::format(AUTH_HEADER, utils::utf16_to_utf8(device_id), STRPRODUCTVER);
	if (auth && !access_token.empty())
	{
		authorization += std::format(R"(, Token="{:s}")", access_token);
	}

	return { authorization, ACCEPT_JSON_HEADER };
}

nlohmann::json plugin_mirkino::jellyfin_request(utils::http_request& req) const
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

nlohmann::json plugin_mirkino::jellyfin_get(const TemplateParams& params, const std::wstring& path, bool use_cache /*= true*/)
{
	for (int attempt = 0; attempt < 2; attempt++)
	{
		// token revoked or expired, log in again and repeat once
		if (!login(params, attempt != 0)) break;

		utils::http_request req
		{
			.url = base_url + path,
			.cache_ttl = use_cache ? GetConfig().get_chrono(true, REG_MAX_CACHE_TTL) : std::chrono::seconds::zero(),
			.request_headers = build_headers(true),
			.timeouts = GetConfig().GetTimeouts(),
		};

		const auto& result = jellyfin_request(req);
		if (!result.empty())
		{
			return result;
		}

		LOG_PROTOCOL(std::format(L"plugin_mirkino: Can't get response ({:s}) on request: {:s}", req.error_message, req.url));
	}

	return {};
}

std::wstring plugin_mirkino::get_image_url(const std::wstring& item_id) const
{
	return std::format(L"{:s}/Items/{:s}/Images/Primary?format=Jpg&maxWidth=400", base_url, item_id);
}

std::wstring plugin_mirkino::get_stream_url(const std::wstring& item_id,
											const std::wstring& source_id,
											const std::wstring& container,
											bool direct,
											const std::wstring& session_id /*= L""*/) const
{
	// player can't send the Authorization header. ApiKey replaces the api_key parameter deprecated since 10.11
	auto query = std::format(L"MediaSourceId={:s}&ApiKey={:s}&DeviceId={:s}",
							 source_id.empty() ? item_id : source_id, utils::utf8_to_utf16(access_token), device_id);
	if (!session_id.empty())
	{
		query += L"&PlaySessionId=" + session_id;
	}

	// original file as is: no load on server, all audio tracks are selectable in player
	if (direct)
	{
		const auto& ext = get_stream_extension(container);
		return std::format(L"{:s}/Videos/{:s}/stream{:s}?static=true&{:s}", base_url, item_id, ext.empty() ? L"" : L"." + ext, query);
	}

	// direct stream not supported, play hls transcoded by server
	return std::format(L"{:s}/Videos/{:s}/master.m3u8?AudioCodec={:s}&{:s}", base_url, item_id, HLS_AUDIO_CODECS, query);
}

void plugin_mirkino::fill_qualities(const std::wstring& item_id, const nlohmann::json& media_sources, vod_episode_def& episode) const
{
	if (!media_sources.is_array()) return;

	for (const auto& source : media_sources)
	{
		const auto& source_id = utils::get_json_wstring("Id", source);
		if (source_id.empty()) continue;

		const auto& container = utils::get_json_wstring("Container", source);
		const auto& url = get_stream_url(item_id, source_id, container, true);
		if (source_id == item_id)
		{
			// default media source
			episode.url = url;
		}

		if (!source.contains("MediaStreams")) continue;

		for (const auto& stream : source["MediaStreams"])
		{
			if (utils::get_json_string("Type", stream) != "Video") continue;

			vod_variant_def quality;
			quality.id = source_id;
			quality.title = utils::get_json_wstring("DisplayTitle", stream);
			if (quality.title.empty())
			{
				quality.title = utils::get_json_wstring("Name", source);
			}
			quality.url = url;
			episode.qualities.set_back(quality.title, quality);
			break;
		}
	}

	if (episode.url.empty())
	{
		episode.url = episode.qualities.empty() ? get_stream_url(item_id, L"", L"", true) : episode.qualities[0].url;
	}
}

void plugin_mirkino::parse_vod(const ThreadConfig& config)
{
	auto categories = std::make_unique<vod_category_storage>();

	do
	{
		const auto& all_name = load_string_resource(IDS_STRING_ALL);
		auto all_category = std::make_shared<vod_category>(all_name);
		all_category->name = all_name;
		categories->set_back(all_name, all_category);

		const auto& params = config.m_params;
		if (!login(params)) break;

		const auto& views = jellyfin_get(params, std::format(L"/UserViews?userId={:s}", utils::utf8_to_utf16(user_id)));
		if (!views.contains("Items")) break;

		int cnt = 0;
		utils::progress_info info{ .type = utils::ProgressType::Initializing };
		config.progress_callback(info);
		info.type = utils::ProgressType::Progress;

		for (const auto& collection : views["Items"])
		{
			if (::WaitForSingleObject(config.m_hStop, 0) == WAIT_OBJECT_0) break;

			JSON_ALL_TRY
			{
				if (utils::get_json_string("Type", collection) != "CollectionFolder") continue;

				const auto& collection_type = utils::get_json_string("CollectionType", collection);
				std::wstring item_types;
				if (collection_type == COLLECTION_TVSHOWS)
				{
					item_types = utils::utf8_to_utf16(ITEM_SERIES);
				}
				else if (collection_type.empty() || collection_type == COLLECTION_MOVIES)
				{
					item_types = utils::utf8_to_utf16(ITEM_MOVIE);
				}
				else
				{
					continue;
				}

				const auto& collection_id = utils::get_json_wstring("Id", collection);
				const auto& name = utils::get_json_wstring("Name", collection);
				if (collection_id.empty() || name.empty()) continue;

				std::shared_ptr<vod_category> category;
				if (!categories->tryGet(name, category))
				{
					category = std::make_shared<vod_category>(name);
					category->name = name;
					categories->set_back(name, category);
				}

				for (int start = 0;; start += PAGE_LIMIT)
				{
					const auto& path = std::format(L"/Items?userId={:s}&ParentId={:s}&Recursive=true&IncludeItemTypes={:s}"
												   L"&Fields=Genres&SortBy=SortName&StartIndex={:d}&Limit={:d}",
												   utils::utf8_to_utf16(user_id), collection_id, item_types, start, PAGE_LIMIT);

					const auto& items = jellyfin_get(params, path);
					if (!items.contains("Items") || items["Items"].empty()) break;

					info.maxPos = utils::get_json_int("TotalRecordCount", items) + cnt;
					for (const auto& item : items["Items"])
					{
						const auto& movie_id = utils::get_json_wstring("Id", item);
						if (movie_id.empty() || category->movies.contains(movie_id)) continue;

						auto movie = std::make_shared<vod_movie_def>();
						movie->id = movie_id;
						movie->title = utils::get_json_wstring("Name", item);
						movie->is_series = utils::get_json_string("Type", item) == ITEM_SERIES;
						movie->year = utils::get_json_wstring("ProductionYear", item);
						movie->rating = format_rating(item, "CommunityRating");
						movie->age = utils::get_json_wstring("OfficialRating", item);
						movie->poster_url.set_uri(get_image_url(movie_id));
						movie->category = name;

						if (item.contains("Genres"))
						{
							for (const auto& genre_item : item["Genres"])
							{
								const auto& genre = utils::get_json_wstring("", genre_item);
								if (!genre.empty())
								{
									movie->genres.set_back(genre, vod_genre_def({ genre, genre }));
								}
							}
						}

						category->movies.set_back(movie_id, movie);
						++cnt;
					}

					info.curPos = info.value = cnt;
					config.progress_callback(info);

					if (::WaitForSingleObject(config.m_hStop, 0) == WAIT_OBJECT_0) break;
					if (static_cast<int>(items["Items"].size()) < PAGE_LIMIT) break;
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

void plugin_mirkino::fetch_movie_info(const TemplateParams& params, vod_movie_def& movie)
{
	if (!login(params)) return;

	const auto& w_user_id = utils::utf8_to_utf16(user_id);
	const auto& movie_item = jellyfin_get(params, std::format(L"/Items/{:s}?userId={:s}", movie.id, w_user_id));
	if (movie_item.empty()) return;

	JSON_ALL_TRY
	{
		movie.title_orig = utils::get_json_wstring("OriginalTitle", movie_item);
		movie.description = utils::get_json_wstring("Overview", movie_item);
		movie.country = join_json_array(movie_item, "ProductionLocations");
		if (movie_item.contains("RunTimeTicks") && movie_item["RunTimeTicks"].is_number())
		{
			movie.movie_time = static_cast<int>(movie_item["RunTimeTicks"].get<long long>() / TICKS_PER_MINUTE);
		}

		std::wstring directors;
		std::wstring actors;
		if (movie_item.contains("People"))
		{
			for (const auto& person : movie_item["People"])
			{
				const auto& type = utils::get_json_string("Type", person);
				const auto& name = utils::get_json_wstring("Name", person);
				if (name.empty()) continue;

				auto* target = (type == "Director") ? &directors : (type == "Actor") ? &actors : nullptr;
				if (!target) continue;

				if (!target->empty())
				{
					*target += L", ";
				}
				*target += name;
			}
		}
		movie.director = directors;
		movie.casting = actors;

		if (utils::get_json_string("Type", movie_item) != ITEM_SERIES)
		{
			fill_qualities(movie.id, movie_item.contains("MediaSources") ? movie_item["MediaSources"] : nlohmann::json(), movie);
			return;
		}

		const auto& seasons = jellyfin_get(params, std::format(L"/Shows/{:s}/Seasons?userId={:s}", movie.id, w_user_id));
		if (!seasons.contains("Items")) return;

		int season_idx = 0;
		for (const auto& season_item : seasons["Items"])
		{
			++season_idx;
			vod_season_def season;
			season.id = utils::get_json_wstring("Id", season_item);
			if (season.id.empty()) continue;

			season.number = utils::get_json_wstring("IndexNumber", season_item);
			if (season.number.empty())
			{
				season.number = std::to_wstring(season_idx);
			}
			season.title = utils::get_json_wstring("Name", season_item);

			// episodes list contains media sources, no need to request every episode separately
			const auto& episodes = jellyfin_get(params, std::format(L"/Shows/{:s}/Episodes?userId={:s}&seasonId={:s}&fields=MediaSources,Overview&sortBy=IndexNumber",
																	movie.id, w_user_id, season.id));
			if (!episodes.contains("Items")) continue;

			for (const auto& episode_item : episodes["Items"])
			{
				vod_episode_def episode;
				episode.id = utils::get_json_wstring("Id", episode_item);
				if (episode.id.empty()) continue;

				episode.title = utils::get_json_wstring("Name", episode_item);
				episode.number = utils::get_json_wstring("IndexNumber", episode_item);

				nlohmann::json media_sources;
				if (episode_item.contains("MediaSources"))
				{
					media_sources = episode_item["MediaSources"];
				}
				else
				{
					// older servers may not return media sources in the episodes list
					const auto& episode_info = jellyfin_get(params, std::format(L"/Items/{:s}?userId={:s}", episode.id, w_user_id));
					if (episode_info.contains("MediaSources"))
					{
						media_sources = episode_info["MediaSources"];
					}
				}

				fill_qualities(episode.id, media_sources, episode);
				season.episodes.set_back(episode.id, episode);
			}

			movie.seasons.set_back(season.id, season);
		}
	}
	JSON_ALL_CATCH
}

std::wstring plugin_mirkino::get_movie_url(const std::shared_ptr<Credentials>& creds, const movie_request& request, const vod_movie_def& movie)
{
	// selected movie or episode
	const vod_episode_def* source = &movie;
	if (request.season_idx >= 0 && request.season_idx < (int)movie.seasons.size())
	{
		const auto& episodes = movie.seasons[request.season_idx].episodes;
		if (request.episode_idx >= 0 && request.episode_idx < (int)episodes.size())
		{
			source = &episodes[request.episode_idx];
		}
	}

	const auto& default_url = get_variant_url(request, movie);
	if (source->id.empty() || (source == &movie && movie.is_series))
	{
		return default_url;
	}

	std::wstring source_id;
	if (request.quality_idx >= 0 && request.quality_idx < (int)source->qualities.size())
	{
		source_id = source->qualities[request.quality_idx].id;
	}

	TemplateParams params{ .creds = creds };
	update_provider_params(params);
	if (!login(params))
	{
		return default_url;
	}

	// PlaybackInfo registers the play session, server serves the stream within it
	nlohmann::json json_request;
	json_request["UserId"] = user_id;
	if (!source_id.empty())
	{
		json_request["MediaSourceId"] = utils::utf16_to_utf8(source_id);
	}

	auto headers = build_headers(true);
	headers.emplace_back(CONTENT_TYPE_JSON);

	utils::http_request req
	{
		.url = std::format(L"{:s}/Items/{:s}/PlaybackInfo", base_url, source->id),
		.request_headers = headers,
		.post_data = json_request.dump(),
		.verb_post = true,
		.timeouts = GetConfig().GetTimeouts(),
	};

	const auto& info = jellyfin_request(req);
	if (info.empty())
	{
		return default_url;
	}

	std::wstring url = default_url;
	JSON_ALL_TRY
	{
		const auto& session_id = utils::get_json_wstring("PlaySessionId", info);
		if (!info.contains("MediaSources") || info["MediaSources"].empty())
		{
			return default_url;
		}

		// for requested media source the response contains only this source, otherwise the one selected by server
		const auto& media_source = info["MediaSources"][0];
		if (source_id.empty())
		{
			source_id = utils::get_json_wstring("Id", media_source);
		}

		url = get_stream_url(source->id,
							 source_id,
							 utils::get_json_wstring("Container", media_source),
							 utils::get_json_bool("SupportsDirectStream", media_source),
							 session_id);
	}
	JSON_ALL_CATCH

	return url;
}
