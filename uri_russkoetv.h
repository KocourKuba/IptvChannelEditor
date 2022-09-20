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

class uri_russkoetv : public uri_stream
{
public:

	uri_russkoetv()
	{
		title = L"Russkoe TV";
		short_name = "russkoetv";
		name = "russkoetv";
		provider_url = L"https://russkoetv.tv/";
		access_type = AccountAccessType::enPin;

		playlist_template = L"http://russkoetv.tv/play/{PASSWORD}.m3u8";

		parser.uri_parse_template = LR"(^https?:\/\/(?<domain>.+)\/s\/(?<token>.+)\/(?<id>.+)\.m3u8$)";

		streams_config[0].stream_type = StreamSubType::enHLS;
		streams_config[0].catchup_type = CatchupType::cu_shift;
		streams_config[0].uri_template = L"http://{DOMAIN}/s/{TOKEN}/{ID}.m3u8";

		epg_params[0].epg_url = L"http://protected-api.com/epg/{ID}/?date=";
		epg_params[0].epg_root = "";
	}
};
