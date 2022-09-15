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
#include "uri_glanz.h"
#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto PLAYLIST_TEMPLATE = L"http://pl.ottglanz.tv/get.php?username={:s}&password={:s}&type=m3u&output=hls";

uri_glanz::uri_glanz()
{
	provider_url = L"http://ottg.tv/";
	provider_vod_url = L"http://api.ottg.tv/playlist/vod?login={:s}&password={:s}";
	access_type = AccountAccessType::enLoginPass;
	catchup_type = { CatchupType::cu_flussonic, CatchupType::cu_flussonic };
	vod_supported = true;

	uri_hls_template = L"http://{DOMAIN}/{ID}/video.m3u8?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}";
	uri_hls_arc_template = L"http://{DOMAIN}/{ID}/video-{START}-{DURATION}.m3u8?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}";
	uri_mpeg_template = L"http://{DOMAIN}/{ID}/mpegts?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}";
	uri_mpeg_arc_template = L"http://{DOMAIN}/{ID}/archive-{START}-{DURATION}.ts?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}";

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

void uri_glanz::parse_uri(const std::wstring& url)
{
	// http://str01.ottg.cc/9195/video.m3u8?username=sharky72&password=F8D58856LWX&token=f5afea07cef148278ae074acaf67a547&ch_id=70&req_host=pkSx3BL
	// http://str01.ottg.cc/9195/mpegts?username=sharky72&password=F8D58856LWX&token=f5afea07cef148278ae074acaf67a547&ch_id=70&req_host=pkSx3BL

	static std::wregex re_url(LR"(^https?:\/\/(.+)\/(.+)\/.+\?username=(.+)&password=(.+)&token=(.+)&ch_id=(\d+)&req_host=(.+)$)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url))
	{
		templated = true;
		domain = std::move(m[1].str());
		id = std::move(m[2].str());
		login = std::move(m[3].str());
		password = std::move(m[4].str());
		token = std::move(m[5].str());
		int_id = std::move(m[6].str());
		host = std::move(m[7].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_glanz::get_playlist_url(TemplateParams& params)
{
	return fmt::format(PLAYLIST_TEMPLATE, params.login, params.password);
}
