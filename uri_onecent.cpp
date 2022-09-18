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
#include "uri_onecent.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

uri_onecent::uri_onecent()
{
	provider_url = L"https://1cent.tv/";
	access_type = AccountAccessType::enPin;
	catchup_type = { CatchupType::cu_shift, CatchupType::cu_none };
	support_streams = { {StreamSubType::enHLS, L"HLS"} };

	playlist_template = L"http://only4.tv/pl/{PASSWORD}/102/only4tv.m3u8";
	uri_parse_template = LR"(^https?:\/\/(?<domain>.+)\/(?<id>.+)\/index\.m3u8\?token=(?<token>.+)$)";
	uri_hls_template = L"http://{DOMAIN}/{ID}/index.m3u8?token={TOKEN}";

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
