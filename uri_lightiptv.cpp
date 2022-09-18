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
#include "uri_lightiptv.h"
#include "PlayListEntry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

uri_lightiptv::uri_lightiptv()
{
	provider_url = L"https://ottbill.cc/";
	access_type = AccountAccessType::enPin;
	catchup_type = { CatchupType::cu_flussonic, CatchupType::cu_flussonic };

	playlist_template = L"http://lightiptv.cc/playlist/hls/{PASSWORD}.m3u";
	uri_hls_template = L"http://{DOMAIN}/{TOKEN}/video.m3u8?token={PASSWORD}";
	uri_hls_arc_template = L"http://{DOMAIN}/{TOKEN}/video-{START}-{DURATION}.m3u8?token={PASSWORD}";
	uri_mpeg_template = L"http://{DOMAIN}/{TOKEN}/mpegts?token={PASSWORD}";
	uri_mpeg_arc_template = L"http://{DOMAIN}/{TOKEN}/timeshift_abs-{START}.ts?token={PASSWORD}";

	epg_params[0].epg_url = L"http://epg.drm-play.ml/lightiptv/epg/{ID}.json";

	secondary_epg = true;
	epg_params[1].epg_url = L"http://epg.ott-play.com/lightiptv/epg/{ID}.json";
}

void uri_lightiptv::parse_uri(const std::wstring& url)
{
	// http://de1light.pp.ru:8080/6e9751628dd0dbbdd6adf4232c135d83/video.m3u8?token=ds9fsahrrk

	static std::wregex re_url_hls(LR"(^https?:\/\/(.+)\/(.+)\/video\.m3u8\?token=(.+)$)");

	std::wsmatch m;
	if (std::regex_match(url, m, re_url_hls))
	{
		templated = true;
		domain = std::move(m[1].str());
		token = std::move(m[2].str());
		password = std::move(m[3].str());
		return;
	}

	uri_stream::parse_uri(url);
}
