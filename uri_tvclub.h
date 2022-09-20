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

// API documentation http://wiki.tvclub.cc/index.php?title=API_v0.9

class uri_tvclub : public uri_stream
{
public:

	uri_tvclub()
	{
		title = L"TV Club";
		short_name = "tvclub";
		name = "tv_club";
		provider_url = L"https://tvclub.cc/";
		access_type = AccountAccessType::enLoginPass;

		playlist_template = L"http://celn.shott.top/p/{TOKEN}";

		parser.uri_parse_template = LR"(^https?:\/\/(?<domain>.+)\/p\/(?<token>.+)\/(?<id>.+)$)";

		streams_config[1].stream_type = StreamSubType::enMPEGTS;
		streams_config[1].catchup_type = CatchupType::cu_shift;
		streams_config[1].uri_template = L"http://{SUBDOMAIN}/p/{TOKEN}/{ID}";

		auto& params = epg_params[0];
		params.epg_url = L"http://api.iptv.so/0.9/json/epg?token={TOKEN}&channels={ID}&time={TIME}&period=24";
		params.epg_name = "text";
		params.epg_desc = "description";
		params.epg_start = "start";
		params.epg_end = "end";
	}

	std::wstring get_api_token(const Credentials& creds) const override;
	bool parse_access_info(TemplateParams& params, std::list<AccountInfo>& info_list) override;
	nlohmann::json get_epg_root(int epg_idx, const nlohmann::json& epg_data) const override;
	const std::vector<ServersInfo>& get_servers_list(TemplateParams& params) override;
	bool set_server(TemplateParams& params) override;
};
