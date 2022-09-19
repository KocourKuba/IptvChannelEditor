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
#include "IptvChannelEditor.h"

class uri_mymagic : public uri_stream
{
public:

	uri_mymagic()
	{
		provider_url = L"http://mymagic.tv/";
		access_type = AccountAccessType::enLoginPass;
		playlist_template = L"http://pl.mymagic.tv/srv/{SERVER_ID}/{QUALITY}/{LOGIN}/{PASSWORD}/tv.m3u";

		parser.uri_parse_template = LR"(^https?:\/\/(?<domain>[^\/]+)\/(?<token>.+)$)";
		parser.per_channel_token = true;

		streams_config[0].stream_type = StreamSubType::enHLS;
		streams_config[0].catchup_type = CatchupType::cu_shift;
		streams_config[0].uri_template = L"http://{DOMAIN}/{TOKEN}";

		epg_params[0].epg_url = L"http://epg.drm-play.ml/magic/epg/{ID}.json";
		epg_params[1].epg_url = L"http://epg.esalecrm.net/magic/epg/{ID}.json";

		servers_list = {
			{ L"0", load_string_resource(IDS_STRING_MYMAGIC_P0) },
			{ L"1", load_string_resource(IDS_STRING_MYMAGIC_P1) },
			{ L"2", load_string_resource(IDS_STRING_MYMAGIC_P2) },
			{ L"3", load_string_resource(IDS_STRING_MYMAGIC_P3) },
			{ L"4", load_string_resource(IDS_STRING_MYMAGIC_P4) },
			{ L"5", load_string_resource(IDS_STRING_MYMAGIC_P5) },
			{ L"6", load_string_resource(IDS_STRING_MYMAGIC_P6) },
		};

		quality_list = {
			{ L"0", load_string_resource(IDS_STRING_MYMAGIC_Q1) },
			{ L"1", load_string_resource(IDS_STRING_MYMAGIC_Q2) },
		};
	}
};
