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
#include "uri_tvteam.h"
#include "PlayListEntry.h"
#include "IPTVChannelEditor.h"

#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// API documentation https://tv.team/api/manual.php

static constexpr auto PLAYLIST_TEMPLATE = L"http://tv.team/pl/11/{:s}/playlist.m3u8";
static constexpr auto URI_TEMPLATE_HLS = L"http://{DOMAIN}/{ID}/mono.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_ARCH_HLS = L"http://{DOMAIN}/{ID}/index-{START}-7200.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{DOMAIN}/{ID}/mpegts?token={TOKEN}";
static constexpr auto URI_TEMPLATE_ARCH_MPEG = L"http://{DOMAIN}/{ID}/archive-{START}-7200.ts?token={TOKEN}";

uri_tvteam::uri_tvteam()
{
	epg_params[0].epg_url = L"http://tv.team/{ID}.json";
	provider_url = L"https://tv.team/";
	server_subst_type = ServerSubstType::enStream;
	servers_list = {
		{ load_string_resource(IDS_STRING_TV_TEAM_P1), L"3.troya.tv"  }, // All (except RU, BY)
		{ load_string_resource(IDS_STRING_TV_TEAM_P2), L"4.troya.tv"  }, // DE, RU
		{ load_string_resource(IDS_STRING_TV_TEAM_P3), L"9.troya.tv"  }, // DE, RU, BY, MD
		{ load_string_resource(IDS_STRING_TV_TEAM_P4), L"8.troya.tv"  }, // DE, UA, BY, MD
		{ load_string_resource(IDS_STRING_TV_TEAM_P5), L"7.troya.tv"  }, // FR, DE, RU, BY
		{ load_string_resource(IDS_STRING_TV_TEAM_P6), L"02.tv.team"  }, // NL
		{ load_string_resource(IDS_STRING_TV_TEAM_P7), L"10.troya.tv" }, // RU, BY
		{ load_string_resource(IDS_STRING_TV_TEAM_P8), L"01.tv.team"  }, // USA 1
		{ load_string_resource(IDS_STRING_TV_TEAM_P9), L"2.troya.tv"  }, // USA 2
	};
}

void uri_tvteam::parse_uri(const std::wstring& url)
{
	// http://3.troya.tv:34000/ch001/mono.m3u8?token=den8.lKT8tP92crF9WyqMVzXu61GkG3UQW0KmBDEitpUhz13ckoHfjmv22gjF

	static std::wregex re_url_hls(LR"(^https?:\/\/(.+)\/(.+)\/mono\.m3u8\?token=(.+)$)");

	std::wsmatch m;
	if (std::regex_match(url, m, re_url_hls))
	{
		templated = true;
		domain = std::move(m[1].str());
		id = std::move(m[2].str());
		token = std::move(m[3].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_tvteam::get_templated_stream(const StreamSubType subType, TemplateParams& params) const
{
	std::wstring url;

	if (is_template())
	{
		switch (subType)
		{
			case StreamSubType::enHLS:
				url = params.shift_back ? URI_TEMPLATE_ARCH_HLS : URI_TEMPLATE_HLS;
				break;
			case StreamSubType::enMPEGTS:
				url = params.shift_back ? URI_TEMPLATE_ARCH_MPEG : URI_TEMPLATE_MPEG;
				break;
			default:
				break;
		}
	}
	else
	{
		url = get_uri();
		if (params.shift_back)
		{
			append_archive(url);
		}
	}

	std::wstring domain(servers_list[params.server].id);
	if (auto pos = servers_list[params.server].id.find(':'); pos != std::wstring::npos)
	{
		domain = fmt::format(L"{:s}:{:s}", servers_list[params.server].id, servers_list[params.server].id.substr(pos + 1));
	}

	utils::string_replace_inplace<wchar_t>(url, REPL_DOMAIN, domain);
	replace_vars(url, params);

	return url;
}

std::wstring uri_tvteam::get_playlist_url(TemplateParams& params)
{
	return fmt::format(PLAYLIST_TEMPLATE, params.password);
}
