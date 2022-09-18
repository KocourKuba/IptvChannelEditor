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

class uri_itv : public uri_stream
{
public:

	uri_itv()
	{
		provider_url = L"https://itv.live/";
		access_type = AccountAccessType::enPin;
		catchup.catchup_type = { CatchupType::cu_flussonic, CatchupType::cu_flussonic };
		parser.per_channel_token = true;

		playlist_template = L"http://itv.ooo/p/{PASSWORD}/hls.m3u8";
		uri_parse_template = LR"(^https?:\/\/(?<domain>.+)\/(?<id>.+)\/[^\?]+\?token=(?<token>.+)$)";

		catchup.uri_hls_template = L"http://{DOMAIN}/{ID}/video.m3u8?token={TOKEN}";
		catchup.uri_hls_arc_template = L"http://{DOMAIN}/{ID}/{HLS_FLUSSONIC}-{START}-{DURATION}.m3u8?token={TOKEN}";

		catchup.uri_mpeg_template = L"http://{DOMAIN}/{ID}/mpegts?token={TOKEN}";
		catchup.uri_mpeg_arc_template = L"http://{DOMAIN}/{ID}/{MPEG_FLUSSONIC}-{START}-{DURATION}.ts?token={TOKEN}";

		auto& params = epg_params[0];
		params.epg_url = L"http://api.itv.live/epg/{ID}";
		params.epg_root = "res";
		params.epg_name = "title";
		params.epg_desc = "desc";
		params.epg_start = "startTime";
		params.epg_end = "stopTime";
	}

	bool parse_access_info(TemplateParams& params, std::list<AccountInfo>& info_list) override;
};
