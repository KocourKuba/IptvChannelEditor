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
static constexpr auto ACCOUNT_TEMPLATE = L"http://{:s}/api/dune-api5m.php?subscr={:s}-{:s}";
static constexpr auto PLAYLIST_TEMPLATE = L"http://{:s}/tv_live-m3u8/{:s}-{:s}";
static constexpr auto PLAYLIST_VOD_TEMPLATE = L"http://{:s}/kino-full/{:s}-{:s}";
static constexpr auto URI_TEMPLATE_HLS = L"http://{DOMAIN}/live/{TOKEN}/{ID}/video.m3u8";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{DOMAIN}/live/{TOKEN}/{ID}.ts";

uri_sharaclub::uri_sharaclub()
{
	auto& params = epg_params[0];
	params.epg_root = "";
	params.epg_url = L"http://{DOMAIN}/get/?type=epg&ch={ID}";
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

std::wstring uri_sharaclub::get_templated_stream(StreamSubType subType, const TemplateParams& params) const
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

std::wstring uri_sharaclub::get_playlist_url(const PlaylistTemplateParams& params) const
{
	return fmt::format(PLAYLIST_TEMPLATE, params.domain, params.login, params.password);
}

bool uri_sharaclub::parse_access_info(const PlaylistTemplateParams& params, std::list<AccountInfo>& info_list) const
{
	std::vector<BYTE> data;
	if (!utils::DownloadFile(fmt::format(ACCOUNT_TEMPLATE, params.domain, params.login, params.password), data) || data.empty())
	{
		return false;
	}

	JSON_ALL_TRY;
		nlohmann::json parsed_json = nlohmann::json::parse(data);
		if (parsed_json.contains("status"))
		{
			AccountInfo info{ L"state", utils::utf8_to_utf16(parsed_json.value("status", "")) };
			info_list.emplace_back(info);
		}

		nlohmann::json js_data = parsed_json["data"];
		put_account_info("login", js_data, info_list);
		put_account_info("money", js_data, info_list);
		put_account_info("money_need", js_data, info_list);
		put_account_info("abon", js_data, info_list);

		return true;

	JSON_ALL_CATCH;

	return false;
}
