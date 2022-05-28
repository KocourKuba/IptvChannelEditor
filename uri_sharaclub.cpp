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
#include "uri_sharaclub.h"
#include "UtilsLib\utils.h"
#include "UtilsLib\inet_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto API_URL = L"http://conf.playtv.pro/api/con8fig.php?source=dune_editor";
static constexpr auto API_COMMAND_GET_URL = L"http://{:s}/api/players.php?a={:s}&u={:s}-{:s}&source=dune_editor";
static constexpr auto API_COMMAND_SET_URL = L"http://{:s}/api/players.php?a={:s}&{:s}={:s}&u={:s}-{:s}&source=dune_editor";
static constexpr auto ACCOUNT_TEMPLATE = L"http://{:s}/api/dune-api5m.php?subscr={:s}-{:s}";
static constexpr auto PLAYLIST_TEMPLATE = L"http://{:s}/tv_live-m3u8/{:s}-{:s}";
static constexpr auto PLAYLIST_VOD_TEMPLATE = L"http://{:s}/kino-full/{:s}-{:s}";
static constexpr auto EPG_TEMPLATE_URL = L"http://{DOMAIN}/get/?type=epg&ch={ID}";
static constexpr auto URI_TEMPLATE_HLS = L"http://{DOMAIN}/live/{TOKEN}/{ID}/video.m3u8";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{DOMAIN}/live/{TOKEN}/{ID}.ts";

uri_sharaclub::uri_sharaclub()
{
	auto& params = epg_params[0];
	params.epg_root = "";
	params.epg_url = EPG_TEMPLATE_URL;
	provider_url = L"https://shara.club/";
	provider_api_url = API_URL;
	provider_vod_url = PLAYLIST_VOD_TEMPLATE;
	vod_supported = true;
}

void uri_sharaclub::parse_uri(const std::wstring& url)
{
	// http://em.gazoni1.com:80/live/s.277258.1d25esee4e77f0419432d2ed8eb0ee525/pervyHD/video.m3u8

	static std::wregex re_url(LR"(^https?:\/\/(.+)\/live\/(.+)\/(.+)\/.+\.m3u8$)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url))
	{
		templated = true;
		domain = std::move(m[1].str());
		token = std::move(m[2].str());
		id = std::move(m[3].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_sharaclub::get_templated_stream(const StreamSubType subType, TemplateParams& params) const
{
	auto& url = get_uri();

	if (is_template())
	{
		switch (subType)
		{
			case StreamSubType::enHLS:
				url = URI_TEMPLATE_HLS;
				break;
			case StreamSubType::enMPEGTS:
				url = URI_TEMPLATE_MPEG;
				break;
			default:
				break;
		}
	}

	if (params.shift_back)
	{
		append_archive(url);
	}

	replace_vars(url, params);

	return url;
}

std::wstring uri_sharaclub::get_playlist_url(TemplateParams& params)
{
	auto& url = fmt::format(PLAYLIST_TEMPLATE, params.domain, params.login, params.password);
	if (params.profile != 0)
	{
		const auto& profiles = get_profiles_list(params);
		url += L"/" + profiles[params.profile].id;
	}

	return url;
}

bool uri_sharaclub::parse_access_info(TemplateParams& params, std::list<AccountInfo>& info_list)
{
	std::vector<BYTE> data;
	if (!utils::DownloadFile(fmt::format(ACCOUNT_TEMPLATE, params.domain, params.login, params.password), data) || data.empty())
	{
		return false;
	}

	JSON_ALL_TRY;
	{
		const auto& parsed_json = nlohmann::json::parse(data);
		if (parsed_json.contains("status"))
		{
			AccountInfo info{ L"state", utils::utf8_to_utf16(parsed_json.value("status", "")) };
			info_list.emplace_back(info);
		}

		if (parsed_json.contains("data"))
		{
			const auto& js_data = parsed_json["data"];
			put_account_info("login", js_data, info_list);
			put_account_info("money", js_data, info_list);
			put_account_info("money_need", js_data, info_list);
			put_account_info("abon", js_data, info_list);
		}

		return true;
	}
	JSON_ALL_CATCH;

	return false;
}

const std::vector<ServersInfo>& uri_sharaclub::get_servers_list(TemplateParams& params)
{
	if (servers_list.empty())
	{
		const auto& url = fmt::format(API_COMMAND_GET_URL,
									  params.domain,
									  L"ch_cdn",
									  params.login,
									  params.password);
		std::vector<BYTE> data;
		if (utils::DownloadFile(url, data) || data.empty())
		{
			JSON_ALL_TRY;
			{
				const auto& parsed_json = nlohmann::json::parse(data);
				if (utils::get_json_int("status", parsed_json) == 1)
				{
					params.server = utils::get_json_int("current", parsed_json);
					const auto& js_data = parsed_json["allow_nums"];
					for (const auto& item : js_data.items())
					{
						const auto& server = item.value();
						ServersInfo info({ utils::get_json_string("name", server), utils::get_json_string("id", server) });
						servers_list.emplace_back(info);
					}
				}
			}
			JSON_ALL_CATCH;
		}
	}
	return servers_list;
}

bool uri_sharaclub::set_server(TemplateParams& params)
{
	const auto& servers = get_servers_list(params);
	if (!servers.empty())
	{
		const auto& url = fmt::format(API_COMMAND_SET_URL,
									  params.domain,
									  L"ch_cdn",
									  L"num",
									  servers[params.server].id,
									  params.login,
									  params.password);

		std::vector<BYTE> data;
		if (utils::DownloadFile(url, data) || data.empty())
		{
			JSON_ALL_TRY;
			{
				const auto& parsed_json = nlohmann::json::parse(data);
				return utils::get_json_string("status", parsed_json) == L"1";
			}
			JSON_ALL_CATCH;
		}
	}

	return false;
}

const std::vector<ProfilesInfo>& uri_sharaclub::get_profiles_list(TemplateParams& params)
{
	if (profiles_list.empty())
	{
		const auto& url = fmt::format(API_COMMAND_GET_URL,
									  params.domain,
									  L"list_profiles",
									  params.login,
									  params.password);
		std::vector<BYTE> data;
		if (utils::DownloadFile(url, data) || data.empty())
		{
			JSON_ALL_TRY;
			{
				const auto& parsed_json = nlohmann::json::parse(data);
				if (utils::get_json_int("status", parsed_json) == 1)
				{
					const auto& current = utils::get_json_string("current", parsed_json);
					ProfilesInfo none_info({ L"None", L"0"});
					profiles_list.emplace_back(none_info);
					if (parsed_json.contains("profiles"))
					{
						for (const auto& item : parsed_json["profiles"].items())
						{
							const auto& profile = item.value();
							ProfilesInfo info({ utils::get_json_string("name", profile), utils::get_json_string("id", profile) });
							if (info.id == current)
								params.profile = profiles_list.size();

							profiles_list.emplace_back(info);
						}
					}
				}
			}
			JSON_ALL_CATCH;
		}
	}
	return profiles_list;
}

bool uri_sharaclub::set_profile(TemplateParams& params)
{
	const auto& profiles = get_profiles_list(params);
	if (!profiles.empty())
	{
		const auto& url = fmt::format(API_COMMAND_SET_URL,
									  params.domain,
									  L"list_profiles",
									  L"num",
									  profiles[params.profile].id,
									  params.login,
									  params.password);

		std::vector<BYTE> data;
		if (utils::DownloadFile(url, data) || data.empty())
		{
			JSON_ALL_TRY;
			{
				const auto& parsed_json = nlohmann::json::parse(data);
				return utils::get_json_string("status", parsed_json) == L"1";
			}
			JSON_ALL_CATCH;
		}
	}

	return false;
}
