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

#include "pch.h"
#include "uri_shuratv.h"
#include "IPTVChannelEditor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

uri_shuratv::uri_shuratv()
{
	short_name = "shuratv";
}

void uri_shuratv::load_default()
{
	title = "Shura TV";
	name = "shura.tv";
	access_type = AccountAccessType::enPin;

	provider_url = "http://shura.tv/b/";
	playlist_template = "http://pl.tvshka.net/?uid={PASSWORD}&srv={SERVER_ID}&type=halva";
	uri_parse_pattern = R"(^https?:\/\/(?<domain>.+)\/~(?<token>.+)\/(?<id>.+)\/hls\/.+\.m3u8$)";

	streams_config[0].cu_subst = "archive";
	streams_config[0].uri_template = "http://{DOMAIN}/~{TOKEN}/{ID}/hls/pl.m3u8";
	streams_config[0].uri_arc_template = "{CU_SUBST}={START}&lutc={NOW}";

	auto& params1 = epg_params[0];
	params1.epg_url = "http://epg.propg.net/{EPG_ID}/epg2/{DATE}";
	params1.epg_date_format = "%Y-%m-%d";
	params1.epg_root = "";
	params1.epg_name = "epg";
	params1.epg_desc = "desc";
	params1.epg_start = "start";
	params1.epg_end = "stop";

	static_servers = true;
	fill_servers_list(TemplateParams());
}

void uri_shuratv::fill_servers_list(TemplateParams& /*params*/)
{
	if (!get_servers_list().empty())
		return;

	std::vector<DynamicParamsInfo> servers;
	for (int i = 0; i <= IDS_STRING_SHURA_TV_P2 - IDS_STRING_SHURA_TV_P1; i++)
	{
		DynamicParamsInfo info;
		info.set_id(std::to_wstring(i));
		info.set_name(load_string_resource(1049, IDS_STRING_SHURA_TV_P1 + i));
		servers.emplace_back(info);
	}

	set_servers_list(servers);
}
