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

class uri_kineskop : public uri_stream
{
public:

	uri_kineskop()
	{
		provider_url = L"http://kineskop.club/";
		epg_params[0].epg_url = L"http://epg.esalecrm.net/kineskop/epg/{ID}.json";
		access_type = AccountAccessType::enLoginPass;
		catchup.catchup_type = { CatchupType::cu_shift, CatchupType::cu_flussonic };
		support_streams = { {StreamSubType::enHLS, L"HLS"} };
		parser.per_channel_token = true;

		playlist_template = L"http://knkp.in/{LOGIN}/{PASSWORD}/{SERVER_ID}/1";
		uri_parse_template = LR"(^https?:\/\/(?<domain>.+)\/(?<host>.+)\/(?<id>.+)\/(?<token>.+)\.m3u8$)";
		catchup.uri_hls_template = L"http://{DOMAIN}/{HOST}/{ID}/{TOKEN}.m3u8";

		for (int i = 0; i <= IDS_STRING_KINESKOP_P4 - IDS_STRING_KINESKOP_P1; i++)
		{
			parser.id = load_string_resource(IDS_STRING_KINESKOP_P1 + i);
			ServersInfo info({ utils::wstring_tolower(parser.id), parser.id });
			servers_list.emplace_back(info);
		}
	}
};
