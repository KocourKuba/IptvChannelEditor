/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2024): sharky72 (https://github.com/KocourKuba)

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
#include "plugin_smile.h"
#include "IPTVChannelEditor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

plugin_smile::plugin_smile()
{
	type_name = "smile";
}

void plugin_smile::load_default()
{
	base_plugin::load_default();

	title = "Smile TV";
	name = "smiletv";
	access_type = AccountAccessType::enLoginPass;

	provider_url = "http://smile-tv.live/";

	vod_templates.clear();
	PlaylistTemplateInfo vod_info(IDS_STRING_EDEM_STANDARD);
	vod_info.pl_template = "{PL_DOMAIN}/{LOGIN}/{PASSWORD}/vodall.m3u?srv={SERVER_ID}";
	vod_info.parse_regex = R"((?<title>[^\(]*)\((?<country>[^\d]+)\s(?<year>\d+)\)$)";
	vod_templates.emplace_back(vod_info);

	vod_info.set_name(load_string_resource(IDS_STRING_NO_ADULT));
	vod_info.pl_template = "{PL_DOMAIN}/{LOGIN}/{PASSWORD}/vod.m3u?srv={SERVER_ID}";
	vod_info.parse_regex = R"((?<title>[^\(]*)\((?<country>[^\d]+)\s(?<year>\d+)\)$)";
	vod_templates.emplace_back(vod_info);

	vod_engine = VodEngine::enM3U;

	PlaylistTemplateInfo info(IDS_STRING_EDEM_STANDARD);
	info.pl_template = "{PL_DOMAIN}/{LOGIN}/{PASSWORD}/tv.m3u?srv={SERVER_ID}";
	info.pl_parse_regex = R"(^https?:\/\/.*\/(?<login>.+)\/(?<password>.+)\/.*$)";
	info.parse_regex = R"(^(?<scheme>https?:\/\/)(?<domain>[^\/]+)\/(?<token>.+)$)";
	info.tag_id_match = "CUID";
	playlist_templates.emplace_back(info);

	square_icons = true;
	static_servers = true;

	streams_config[0].uri_template = "{SCHEME}{DOMAIN}/{TOKEN}";
	streams_config[0].uri_arc_template = "{LIVE_URL}?utc={START}&lutc={NOW}";

	epg_params[0].epg_url = "{EPG_DOMAIN}/smile%2Fepg%2F{EPG_ID}.json";
}

void plugin_smile::fill_servers_list(TemplateParams* params /*= nullptr*/)
{
	if (!get_servers_list().empty())
		return;

	// [{`id`:`0`,`name`:`Auto Select`},{`id`:`1`,`name`:`Server CZ`},{`id`:`2`,`name`:`Server DE`},{`id`:`3`,`name`:`Server NL`},{`id`:`4`,`name`:`Server RU`}]
	std::vector<DynamicParamsInfo> servers;
	for (int i = 0; i <= IDS_STRING_SMILE_P4 - IDS_STRING_SMILE_P0; i++)
	{
		DynamicParamsInfo info;
		info.set_id(std::to_wstring(i));
		info.set_name(load_string_resource(1049, IDS_STRING_SMILE_P0 + i));
		servers.emplace_back(info);
	}

	set_servers_list(servers);
}

void plugin_smile::fill_domains_list(TemplateParams* params /*= nullptr*/)
{
	if (!get_domains_list().empty())
		return;

	DynamicParamsInfo info;
	info.set_id(L"0");
	info.set_name(L"http://pl.smile-tv.live");

	std::vector<DynamicParamsInfo> domains;
	domains.emplace_back(info);

	set_domains_list(domains);
}
