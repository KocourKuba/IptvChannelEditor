/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2023): sharky72 (https://github.com/KocourKuba)

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
#include "plugin_filmax.h"
#include "IPTVChannelEditor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

plugin_filmax::plugin_filmax()
{
	short_name = "filmax";
}

void plugin_filmax::load_default()
{
	title = "Filmax TV";
	name = "filmax";
	access_type = AccountAccessType::enLoginPass;

	provider_url = "https://filmax-tv.ru/";

	PlaylistTemplateInfo info(IDS_STRING_EDEM_STANDARD);
	info.pl_template = "http://lk.filmax-tv.ru/{LOGIN}/{PASSWORD}/hls/p{SERVER_ID}/playlist.m3u8";
	info.parse_regex = R"(^https?:\/\/(?<domain>.+):(?<port>.+)\/(?<int_id>.+)\/index\.m3u8\?token=(?<token>.+)$)";
	info.tag_id_match = "tvg-name";
	playlist_templates.emplace_back(info);

	streams_config[0].cu_type = CatchupType::cu_flussonic;
	streams_config[0].uri_template = "http://{DOMAIN}:{PORT}/{INT_ID}/index.m3u8?token={TOKEN}";
	streams_config[0].uri_arc_template = "http://{DOMAIN}:{PORT}/{INT_ID}/archive-{START}-{DURATION}.m3u8?token={TOKEN}";

	streams_config[1].uri_template = "http://{DOMAIN}:{PORT}/{INT_ID}/mpegts?token={TOKEN}";
	streams_config[1].uri_arc_template = "http://{DOMAIN}:{PORT}/{INT_ID}/archive-{START}-{DURATION}.ts?token={TOKEN}";

	epg_params[0].epg_url = "http://epg.esalecrm.net/filmax/epg/{EPG_ID}.json";

	static_servers = true;
	fill_servers_list();
}

void plugin_filmax::fill_servers_list(TemplateParams* params /*= nullptr*/)
{
	if (!get_servers_list().empty())
		return;

	std::vector<DynamicParamsInfo> servers;
	for (int i = 0; i <= IDS_STRING_FILMAX_P12 - IDS_STRING_FILMAX_P1; i++)
	{
		DynamicParamsInfo info;
		info.set_id(std::to_wstring(i + 1));
		info.set_name(load_string_resource(1049, IDS_STRING_FILMAX_P1 + i));
		servers.emplace_back(info);
	}

	set_servers_list(servers);
}
