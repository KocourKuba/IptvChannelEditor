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
#include "uri_iptvonline.h"

#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

uri_iptvonline::uri_iptvonline()
{
	provider_url = L"https://iptv.online/";
	access_type = AccountAccessType::enPin;
	catchup_type = { CatchupType::cu_flussonic, CatchupType::cu_flussonic };

	playlist_template = L"http://iptv.online/play/{PASSWORD}/m3u8";
	uri_hls_template = L"http://{DOMAIN}/play/{ID}/{TOKEN}/video.m3u8";
	uri_hls_template = L"http://{DOMAIN}/play/{ID}/{TOKEN}/video-{START}-{DURATION}.m3u8";
	uri_mpeg_arc_template = L"http://{DOMAIN}/play/{ID}/{TOKEN}/mpegts";
	uri_mpeg_arc_template = L"http://{DOMAIN}/play/{ID}/{TOKEN}/archive-{START}-{DURATION}.ts";

	auto& params1 = epg_params[0];
	params1.epg_url = L"http://epg.iptvx.one/api/id/{ID}.json";
	params1.epg_root = "ch_programme";
	params1.epg_name = "title";
	params1.epg_desc = "description";
	params1.epg_start = "start";
	params1.epg_end = "";
	params1.epg_time_format = "%d-%m-%Y %H:%M";
	params1.epg_tz = 3600 * 3; // iptvx.one uses moscow time (UTC+3)

	secondary_epg = true;
	epg_params[1].epg_url = L"http://epg.drm-play.ml/iptvx.one/epg/{ID}.json";
}

void uri_iptvonline::parse_uri(const std::wstring& url)
{
	// http://cz.iptv.monster/play/73/38798DB9DF4EA8F/video.m3u8

	static std::wregex re_url_hls(LR"(^https?:\/\/(.+)\/play\/(.+)\/(.+)\/video\.m3u8$)");
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
