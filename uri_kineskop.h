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
		short_name = "kineskop";
		for (int i = 0; i <= IDS_STRING_KINESKOP_P4 - IDS_STRING_KINESKOP_P1; i++)
		{
			ServersInfo info({ fmt::format(L"{:d}", i + 1), load_string_resource(IDS_STRING_KINESKOP_P1 + i) });
			servers_list.emplace_back(info);
		}
	}

	void load_default() override
	{
		title = "Kineskop.Club";
		name = "kineskop";
		access_type = AccountAccessType::enLoginPass;

		provider_url = "http://kineskop.club/";
		playlist_template = "http://knkp.in/{LOGIN}/{PASSWORD}/{SERVER}/1";
		uri_parse_template = R"(^https?:\/\/(?<domain>.+)\/(?<host>.+)\/(?<id>.+)\/(?<token>.+)\.m3u8$)";

		per_channel_token = true;

		streams_config[0].uri_template = "http://{DOMAIN}/{HOST}/{ID}/{TOKEN}.m3u8";
		streams_config[0].uri_arc_template = "{CU_SUBST}={START}&lutc={NOW}";

		epg_params[0].epg_url = "http://epg.esalecrm.net/kineskop/epg/{ID}.json";
	}
};
