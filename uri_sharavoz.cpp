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
#include "uri_sharavoz.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

uri_sharavoz::uri_sharavoz()
{
	short_name = "sharavoz";
}

void uri_sharavoz::load_default()
{
	title = "Sharavoz TV";
	name = "sharavoz.tv";
	access_type = AccountAccessType::enPin;

	provider_url = "https://www.sharavoz.tv/";
	playlist_template = "http://www.spr24.net/iptv/p/{PASSWORD}/Sharavoz.Tv.navigator-ott.m3u";
	uri_parse_template = R"(^https?:\/\/(?<domain>.+)\/(?<id>\d+)\/(?:mpegts|index\.m3u8)\?token=(?<token>.+)$)";

	streams_list[0].cu_type = CatchupType::cu_flussonic;
	streams_list[0].cu_subst = "index";
	streams_list[0].uri_template = "http://{DOMAIN}/{ID}/index.m3u8?token={TOKEN}";
	streams_list[0].uri_arc_template = "http://{DOMAIN}/{ID}/{CU_SUBST}-{START}-{DURATION}.m3u8?token={TOKEN}";

	streams_list[1].uri_template = "http://{DOMAIN}/{ID}/mpegts?token={TOKEN}";
	streams_list[1].uri_arc_template = "http://{DOMAIN}/{ID}/{CU_SUBST}-{START}-{DURATION}.ts?token={TOKEN}";

	epg_params[0].epg_url = "http://api.program.spr24.net/api/program?epg={ID}";
	epg_params[1].epg_url = "http://epg.arlekino.tv/api/program?epg={ID}";
}
