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

#pragma once
#include "uri_stream.h"

class uri_iptvonline : public uri_stream
{
public:

	uri_iptvonline()
	{
		title = L"IPTV Online";
		short_name = "iptvonline";
		name = "iptvonline";
		provider_url = L"https://iptv.online/";
		access_type = AccountAccessType::enPin;

		playlist_template = L"http://iptv.online/play/{PASSWORD}/m3u8";

		uri_parse_template = LR"(^https?:\/\/(?<domain>.+)\/play\/(?<id>.+)\/(?<token>.+)\/video\.m3u8$)";

		streams_config[0].enabled = true;
		streams_config[0].stream_sub_type = StreamSubType::enHLS;
		streams_config[0].catchup_type = CatchupType::cu_flussonic;
		streams_config[0].shift_replace = "video";
		streams_config[0].uri_template = "http://{DOMAIN}/play/{ID}/{TOKEN}/video.m3u8";
		streams_config[0].uri_arc_template = "http://{DOMAIN}/play/{ID}/{TOKEN}/{SHIFT_SUBST}-{START}-{DURATION}.m3u8";

		streams_config[1].enabled = true;
		streams_config[1].stream_sub_type = StreamSubType::enMPEGTS;
		streams_config[1].catchup_type = CatchupType::cu_flussonic;
		streams_config[1].shift_replace = "archive";
		streams_config[1].uri_template = "http://{DOMAIN}/play/{ID}/{TOKEN}/mpegts";
		streams_config[1].uri_arc_template = "http://{DOMAIN}/play/{ID}/{TOKEN}/{SHIFT_SUBST}-{START}-{DURATION}.ts";

		auto& params1 = epg_params[0];
		params1.epg_url = "http://epg.iptvx.one/api/id/{ID}.json";
		params1.epg_root = "ch_programme";
		params1.epg_name = "title";
		params1.epg_desc = "description";
		params1.epg_start = "start";
		params1.epg_end = "";
		params1.epg_time_format = "%d-%m-%Y %H:%M";
		params1.epg_tz = 3; // iptvx.one uses moscow time (UTC+3)

		epg_params[1].epg_url = "http://epg.drm-play.ml/iptvx.one/epg/{ID}.json";
	}
};
