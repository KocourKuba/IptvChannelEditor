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

class uri_oneott : public uri_stream
{
public:

	uri_oneott()
	{
		provider_url = L"http://1ott.net/";
		access_type = AccountAccessType::enLoginPass;
		playlist_template = L"http://list.1ott.net/api/{TOKEN}/high/ottplay.m3u8";

		parser.uri_parse_template = LR"(^https?:\/\/(?<domain>.+)\/~(?<token>.+)\/(?<id>.+)\/hls\/.+\.m3u8$)";

		streams_config[0].stream_type = StreamSubType::enHLS;
		streams_config[0].catchup_type = CatchupType::cu_shift;
		streams_config[0].uri_template = L"http://{DOMAIN}/~{TOKEN}/{ID}/hls/pl.m3u8";

		streams_config[1].catchup_type = CatchupType::cu_shift;
		streams_config[1].stream_type = StreamSubType::enMPEGTS;
		streams_config[1].uri_template = L"http://{DOMAIN}/~{TOKEN}/{ID}";

		auto& params1 = epg_params[0];
		params1.epg_url = L"http://epg.propg.net/{ID}/epg2/{DATE}";
		params1.epg_root = "";
		params1.epg_name = "epg";
		params1.epg_desc = "desc";
		params1.epg_start = "start";
		params1.epg_end = "stop";

		epg_params[1].epg_url = L"http://epg.drm-play.ml/1ott/epg/{ID}.json";
	}

	bool parse_access_info(TemplateParams& params, std::list<AccountInfo>& info_list) override;
};
