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

class uri_sharaclub : public uri_stream
{
public:

	uri_sharaclub()
	{
		short_name = "sharaclub";
		provider_api_url = L"http://conf.playtv.pro/api/con8fig.php?source=dune_editor";;
		provider_vod_url = L"http://{SUBDOMAIN}/kino-full/{LOGIN}-{PASSWORD}";
	}

	void load_default() override
	{
		title = "Sharaclub TV";
		name = "sharaclub.tv";
		access_type = AccountAccessType::enLoginPass;

		provider_url = "https://shara.club/";
		playlist_template = "http://{SUBDOMAIN}/tv_live-m3u8/{LOGIN}-{PASSWORD}";
		uri_parse_template = R"(^https?:\/\/(?<domain>.+)\/live\/(?<token>.+)\/(?<id>.+)\/.+\.m3u8$)";

		streams_config[0].cu_type = CatchupType::cu_append;
		streams_config[0].uri_template = "http://{DOMAIN}/live/{TOKEN}/{ID}/video.m3u8";
		streams_config[0].uri_arc_template = "{CU_SUBST}={START}";

		streams_config[1].cu_type = CatchupType::cu_append;
		streams_config[1].cu_subst = "utc";
		streams_config[1].uri_template = "http://{DOMAIN}/live/{TOKEN}/{ID}.ts";
		streams_config[1].uri_arc_template = "{CU_SUBST}={START}";

		auto& params = epg_params[0];
		params.epg_root = "";
		params.epg_url = "http://{DOMAIN}/get/?type=epg&ch={ID}";
	}

	void get_playlist_url(std::wstring& url, TemplateParams& params) override;
	bool parse_access_info(TemplateParams& params, std::list<AccountInfo>& info_list) override;
	const std::vector<ServersInfo>& get_servers_list(TemplateParams& params) override;
	bool set_server(TemplateParams& params) override;
	const std::vector<ProfilesInfo>& get_profiles_list(TemplateParams& params) override;
	bool set_profile(TemplateParams& params) override;
};
