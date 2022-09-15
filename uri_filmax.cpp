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
#include "uri_filmax.h"
#include "IPTVChannelEditor.h"

#include "UtilsLib\utils.h"
#include "UtilsLib\inet_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto PLAYLIST_TEMPLATE = L"http://lk.filmax-tv.ru/{:s}/{:s}/hls/p{:s}/playlist.m3u8";

uri_filmax::uri_filmax()
{
	provider_url = L"https://filmax-tv.ru/";
	access_type = AccountAccessType::enLoginPass;
	catchup_type = { CatchupType::cu_flussonic, CatchupType::cu_flussonic };

	uri_hls_template = L"http://{DOMAIN}:{PORT}/{INT_ID}/index.m3u8?token={TOKEN}";
	uri_hls_arc_template = L"http://{DOMAIN}:{PORT}/{INT_ID}/archive-{START}-{DURATION}.m3u8?token={TOKEN}";
	uri_mpeg_template = L"http://{DOMAIN}:{PORT}/{INT_ID}/mpegts?token={TOKEN}";
	uri_mpeg_arc_template = L"http://{DOMAIN}:{PORT}/{INT_ID}/archive-{START}-{DURATION}.ts?token={TOKEN}";

	auto& params = epg_params[0];
	params.epg_url = L"http://epg.esalecrm.net/filmax/epg/{ID}.json";

	for (int i = 0; i <= IDS_STRING_FILMAX_P12 - IDS_STRING_FILMAX_P1; i++)
	{
		ServersInfo info({ fmt::format(L"{:d}", i + 1), load_string_resource(IDS_STRING_FILMAX_P1 + i) });
		servers_list.emplace_back(info);
	}
}

void uri_filmax::parse_uri(const std::wstring& url)
{
	// http://eu1-filmax.pp.ru:8080/a841f5cc3252dac06a7964c3069b4483/index.m3u8?token=42hfcmtdNzs8fGfsbgRSVGs1VGxaeE1qaFY=
	// http://eu1-filmax.pp.ru:8080/a841f5cc3252dac06a7964c3069b4483/mpegts?token=42hfcmtdNzs8fGfsbgRSVGs1VGxaeE1qaFY=

	static std::wregex re_url(LR"(^https?:\/\/(.+):(.+)\/(.+)\/index\.m3u8\?token=(.+)$)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url))
	{
		templated = true;
		domain = std::move(m[1].str());
		port = std::move(m[2].str());
		int_id = std::move(m[3].str());
		token = std::move(m[4].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_filmax::get_playlist_url(TemplateParams& params)
{
	return fmt::format(PLAYLIST_TEMPLATE, params.login, params.password, servers_list[params.server].id);
}
