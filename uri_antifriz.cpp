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
#include "uri_antifriz.h"

#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// API documentation http://protected-api.com/api/documentation

uri_antifriz::uri_antifriz()
{
	provider_url = L"https://antifriztv.com/";
	provider_vod_url = L"http://protected-api.com";
	access_type = AccountAccessType::enPin;
	catchup_type = { CatchupType::cu_flussonic, CatchupType::cu_flussonic };
	vod_supported = true;

	playlist_template = L"http://antifriz.tv/playlist/{PASSWORD}.m3u8";
	uri_parse_template = LR"(^https?:\/\/(?<domain>.+):(?<port>.+)\/s\/(?<token>.+)\/(?<id>.+)\/video\.m3u8$)";
	uri_hls_template = L"http://{DOMAIN}:{PORT}/s/{TOKEN}/{ID}/video.m3u8";
	uri_hls_arc_template = L"http://{DOMAIN}/{ID}/archive-{START}-{DURATION}.m3u8?token={TOKEN}";
	uri_mpeg_template = L"http://{DOMAIN}:{PORT}/{ID}/mpegts?token={TOKEN}";
	uri_mpeg_arc_template = L"http://{DOMAIN}/{ID}/archive-{START}-{DURATION}.ts?token={TOKEN}";

	auto& params1 = epg_params[0];
	params1.epg_url = L"http://protected-api.com/epg/{ID}/?date=";
	params1.epg_root = "";
}
