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

// API documentation http://wiki.vidok.tv/index.php?title=SAPI

class uri_vidok : public uri_stream
{
public:

	uri_vidok()
	{
		provider_url = L"https://vidok.tv/";
		access_type = AccountAccessType::enLoginPass;
		playlist_template = L"http://vidok.tv/p/{TOKEN}";

		parser.uri_parse_template = LR"(^https?:\/\/(?<domain>.+)\/p\/(?<token>.+)\/(?<id>.+)$)";

		streams_config[0].stream_type = StreamSubType::enHLS;
		streams_config[0].catchup_type = CatchupType::cu_append;
		streams_config[0].uri_template = L"http://{DOMAIN}/p/{TOKEN}/{ID}";

		auto& params = epg_params[0];
		params.epg_url = L"http://sapi.ott.st/v2.4/json/epg2?cid={ID}&token={TOKEN}";
		params.epg_root = "epg";
		params.epg_name = "title";
		params.epg_desc = "description";
		params.epg_start = "start";
		params.epg_end = "end";
	}

	std::wstring get_api_token(const Credentials& creds) const override;
	bool parse_access_info(TemplateParams& params, std::list<AccountInfo>& info_list) override;
	const std::vector<ServersInfo>& get_servers_list(TemplateParams& params) override;
	bool set_server(TemplateParams& params) override;
};
