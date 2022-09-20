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

class uri_glanz	: public uri_stream
{
public:

	uri_glanz()
	{
		title = L"glanz TV";
		short_name = "glanz";
		name = "glanz.tv";
		provider_url = L"http://ottg.cc/";
		access_type = AccountAccessType::enLoginPass;

		playlist_template = L"http://pl.ottg.cc/get.php?username={LOGIN}&password={PASSWORD}&type=m3u&output=hls";
		provider_vod_url = L"http://api.ottg.cc/playlist/vod?login={LOGIN}&password={PASSWORD}";

		parser.uri_parse_template = LR"(^https?:\/\/(?<domain>.+)\/(?<id>.+)\/.+\?username=(?<login>.+)&password=(?<password>.+)&token=(?<token>.+)&ch_id=(?<int_id>\d+)&req_host=(?<host>.+)$)";

		streams_config[0].stream_type = StreamSubType::enHLS;
		streams_config[0].catchup_type = CatchupType::cu_flussonic;
		streams_config[0].uri_template = L"http://{DOMAIN}/{ID}/video.m3u8?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}";
		streams_config[0].uri_arc_template = L"http://{DOMAIN}/{ID}/{FLUSSONIC}-{START}-{DURATION}.m3u8?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}";
		streams_config[0].flussonic_replace = L"video";

		streams_config[1].stream_type = StreamSubType::enMPEGTS;
		streams_config[1].catchup_type = CatchupType::cu_flussonic;
		streams_config[1].uri_template = L"http://{DOMAIN}/{ID}/mpegts?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}";
		streams_config[1].uri_arc_template = L"http://{DOMAIN}/{ID}/{FLUSSONIC}-{START}-{DURATION}.ts?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}";

		auto& params1 = epg_params[0];
		params1.epg_url = L"http://epg.iptvx.one/api/id/{ID}.json";
		params1.epg_root = "ch_programme";
		params1.epg_name = "title";
		params1.epg_desc = "description";
		params1.epg_start = "start";
		params1.epg_end = "";
		params1.epg_time_format = "%d-%m-%Y %H:%M";
		params1.epg_tz = 3600 * 3; // iptvx.one uses moscow time (UTC+3)

		epg_params[1].epg_url = L"http://epg.drm-play.ml/iptvx.one/epg/{ID}.json";
	}
};
