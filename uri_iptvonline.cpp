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

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

uri_iptvonline::uri_iptvonline()
{
	short_name = "iptvonline";
}

void uri_iptvonline::load_default()
{
	title = "IPTV Online";
	name = "iptvonline";
	access_type = AccountAccessType::enPin;

	provider_url = "https://iptv.online/";
	playlist_template = "http://iptv.online/play/{PASSWORD}/m3u8";
	uri_parse_pattern = R"(^https?:\/\/(?<domain>.+)\/play\/(?<id>.+)\/(?<token>.+)\/video\.m3u8$)";

	square_icons = true;

	streams_config[0].cu_type = CatchupType::cu_flussonic;
	streams_config[0].cu_subst = "video";
	streams_config[0].uri_template = "http://{DOMAIN}/play/{ID}/{TOKEN}/video.m3u8";
	streams_config[0].uri_arc_template = "http://{DOMAIN}/play/{ID}/{TOKEN}/{CU_SUBST}-{START}-{DURATION}.m3u8";

	streams_config[1].uri_template = "http://{DOMAIN}/play/{ID}/{TOKEN}/mpegts";
	streams_config[1].uri_arc_template = "http://{DOMAIN}/play/{ID}/{TOKEN}/{CU_SUBST}-{START}-{DURATION}.ts";

	auto& params1 = epg_params[0];
	params1.epg_url = "http://epg.iptvx.one/api/id/{ID}.json";
	params1.epg_root = "ch_programme";
	params1.epg_name = "title";
	params1.epg_desc = "description";
	params1.epg_start = "start";
	params1.epg_end = "";
	params1.epg_time_format = "{DAY}-{MONTH}-{YEAR} {HOUR}:{MIN}"; // "%d-%m-%Y %H:%M";
	params1.epg_timezone = 3; // iptvx.one uses moscow time (UTC+3)

	epg_params[1].epg_url = "http://epg.drm-play.ml/iptvx.one/epg/{ID}.json";
}
