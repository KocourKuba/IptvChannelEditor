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
#include "IPTVChannelEditor.h"

class uri_shuratv : public uri_stream
{
public:

	uri_shuratv()
	{
		provider_url = L"http://shura.tv/b/";
		access_type = AccountAccessType::enPin;
		playlist_template = L"http://pl.tvshka.net/?uid={PASSWORD}&srv={SERVER_ID}&type=halva";

		parser.uri_parse_template = LR"(^https?:\/\/(?<domain>.+)\/~(?<token>.+)\/(?<id>.+)\/hls\/.+\.m3u8$)";

		streams_config[0].stream_type = StreamSubType::enHLS;
		streams_config[0].catchup_type = CatchupType::cu_shift;
		streams_config[0].uri_template = L"http://{DOMAIN}/~{TOKEN}/{ID}/hls/pl.m3u8";
		streams_config[0].shift_replace = L"archive";

		auto& params1 = epg_params[0];
		params1.epg_url = L"http://epg.propg.net/{ID}/epg2/{DATE}";
		params1.epg_root = "";
		params1.epg_name = "epg";
		params1.epg_desc = "desc";
		params1.epg_start = "start";
		params1.epg_end = "stop";

		for (int i = 0; i <= IDS_STRING_SHURA_TV_P2 - IDS_STRING_SHURA_TV_P1; i++)
		{
			ServersInfo info({ fmt::format(L"{:d}", i + 1), load_string_resource(IDS_STRING_SHURA_TV_P1 + i) });
			servers_list.emplace_back(info);
		}
	}
};
