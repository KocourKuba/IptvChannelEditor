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

// API documentation http://protected-api.com/api/documentation

class uri_cbilling : public uri_stream
{
public:

	uri_cbilling()
	{
		provider_url = L"https://cbilling.eu/";
		access_type = AccountAccessType::enPin;
		catchup.catchup_type = { CatchupType::cu_shift, CatchupType::cu_flussonic };

		playlist_template = L"http://247on.cc/playlist/{PASSWORD}_otp_dev{SERVER_ID}.m3u8";
		uri_parse_template = LR"(^https?:\/\/(?<domain>.+):(?<port>.+)\/s\/(?<token>.+)\/(?<id>.+)\.m3u8$)";
		catchup.uri_hls_template = L"http://{DOMAIN}:{PORT}/s/{TOKEN}/{ID}.m3u8";
		catchup.uri_hls_arc_template = L"http://{DOMAIN}:{PORT}/s/{TOKEN}/{ID}.m3u8?utc={START}&lutc={NOW}";
		catchup.uri_mpeg_template = L"http://{DOMAIN}/{ID}/mpegts?token={TOKEN}";
		catchup.uri_mpeg_arc_template = L"http://{DOMAIN}/{ID}/{MPEG_FLUSSONIC}.ts?token={TOKEN}";

		auto& params1 = epg_params[0];
		params1.epg_url = L"http://protected-api.com/epg/{ID}/?date=";
		params1.epg_root = "";

		for (int i = 0; i <= IDS_STRING_CBILLING_TV_P3 - IDS_STRING_CBILLING_TV_P1; i++)
		{
			ServersInfo info({ fmt::format(L"{:d}", i + 1), load_string_resource(IDS_STRING_CBILLING_TV_P1 + i) });
			servers_list.emplace_back(info);
		}

		provider_vod_url = L"http://protected-api.com";
		vod_supported = true;
	}

	bool parse_access_info(TemplateParams& params, std::list<AccountInfo>& info_list) override;
};
