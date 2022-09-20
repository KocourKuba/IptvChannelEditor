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

class uri_filmax : public uri_stream
{
public:

	uri_filmax()
	{
		title = L"Filmax TV";
		short_name = "filmax";
		name = "filmax";
		provider_url = L"https://filmax-tv.ru/";
		access_type = AccountAccessType::enLoginPass;

		playlist_template = L"http://lk.filmax-tv.ru/{LOGIN}/{PASSWORD}/hls/p{SERVER_ID}/playlist.m3u8";

		uri_parse_template = LR"(^https?:\/\/(?<domain>.+):(?<port>.+)\/(?<int_id>.+)\/index\.m3u8\?token=(?<token>.+)$)";

		streams_config[0].enabled = true;
		streams_config[0].stream_sub_type = StreamSubType::enHLS;
		streams_config[0].catchup_type = CatchupType::cu_flussonic;
		streams_config[0].shift_replace = "archive";
		streams_config[0].uri_template = "http://{DOMAIN}:{PORT}/{INT_ID}/index.m3u8?token={TOKEN}";
		streams_config[0].uri_arc_template = "http://{DOMAIN}:{PORT}/{INT_ID}/{SHIFT_SUBST}-{START}-{DURATION}.m3u8?token={TOKEN}";

		streams_config[1].enabled = true;
		streams_config[1].stream_sub_type = StreamSubType::enMPEGTS;
		streams_config[1].catchup_type = CatchupType::cu_flussonic;
		streams_config[1].shift_replace = "archive";
		streams_config[1].uri_template = "http://{DOMAIN}:{PORT}/{INT_ID}/mpegts?token={TOKEN}";
		streams_config[1].uri_arc_template = "http://{DOMAIN}:{PORT}/{INT_ID}/{SHIFT_SUBST}-{START}-{DURATION}.ts?token={TOKEN}";

		epg_params[0].epg_url = "http://epg.esalecrm.net/filmax/epg/{ID}.json";

		for (int i = 0; i <= IDS_STRING_FILMAX_P12 - IDS_STRING_FILMAX_P1; i++)
		{
			ServersInfo info({ fmt::format(L"{:d}", i + 1), load_string_resource(IDS_STRING_FILMAX_P1 + i) });
			servers_list.emplace_back(info);
		}
	}
};
