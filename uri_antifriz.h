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

// API documentation http://protected-api.com/api/documentation

class uri_antifriz : public uri_stream
{
public:

	uri_antifriz()
	{
		title = L"Antifriz TV";
		short_name = "antifriz";
		name = "antifriz.tv";
		provider_url = L"https://antifriztv.com/";
		access_type = AccountAccessType::enPin;

		provider_vod_url = L"http://protected-api.com";
		playlist_template = L"http://antifriz.tv/playlist/{PASSWORD}.m3u8";

		parser.uri_parse_template = LR"(^https?:\/\/(?<domain>.+):(?<port>.+)\/s\/(?<token>.+)\/(?<id>.+)\/video\.m3u8$)";

		streams_config[0].stream_type = StreamSubType::enHLS;
		streams_config[0].catchup_type = CatchupType::cu_flussonic;
		streams_config[0].uri_template = L"http://{DOMAIN}:{PORT}/s/{TOKEN}/{ID}/video.m3u8";
		streams_config[0].uri_arc_template = L"http://{DOMAIN}/{ID}/{FLUSSONIC}-{START}-{DURATION}.m3u8?token={TOKEN}";

		streams_config[1].stream_type = StreamSubType::enMPEGTS;
		streams_config[1].catchup_type = CatchupType::cu_flussonic;
		streams_config[1].uri_template = L"http://{DOMAIN}:{PORT}/{ID}/mpegts?token={TOKEN}";
		streams_config[1].uri_arc_template = L"http://{DOMAIN}/{ID}/{FLUSSONIC}-{START}-{DURATION}.ts?token={TOKEN}";

		epg_params[0].epg_url = L"http://protected-api.com/epg/{ID}/?date=";
		epg_params[0].epg_root = "";
	}
};
