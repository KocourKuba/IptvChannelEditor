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

class uri_oneusd : public uri_stream
{
public:

	uri_oneusd()
	{
		short_name = "oneusd";
	}

	void load_default() override
	{
		title = "1USD TV";
		name = "oneusd.tv";
		access_type = AccountAccessType::enPin;

		provider_url = "http://1usd.tv/";
		playlist_template = "http://1usd.tv/pl-{PASSWORD}-hls";
		uri_parse_template = R"(^https?:\/\/(?<domain>.+)\/(?<id>.+)\/mono\.m3u8\?token=(?<token>.+)$)";

		per_channel_token = true;

		streams_config[0].cu_type = CatchupType::cu_flussonic;
		streams_config[0].cu_subst = "index";
		streams_config[0].uri_template = "http://{DOMAIN}/{ID}/mono.m3u8?token={TOKEN}";
		streams_config[0].uri_arc_template = "http://{DOMAIN}/{ID}/{CU_SUBST}-{START}-{DURATION}.m3u8?token={TOKEN}";

		streams_config[1].uri_template = "http://{DOMAIN}/{ID}/mpegts?token={TOKEN}";
		streams_config[1].uri_arc_template = "http://{DOMAIN}/{ID}/{CU_SUBST}-{START}-{DURATION}.ts?token={TOKEN}";

		epg_params[0].epg_url = "http://tv.team/{ID}.json";
	}
};
