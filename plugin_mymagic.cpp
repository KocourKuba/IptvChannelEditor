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
#include "plugin_mymagic.h"
#include "IptvChannelEditor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

plugin_mymagic::plugin_mymagic()
{
	type_name = "mymagic";
}

void plugin_mymagic::load_default()
{
	base_plugin::load_default();

	title = "MyMagic TV";
	name = "mymagic";
	access_type = AccountAccessType::enLoginPass;

	provider_url = "http://mymagic.tv/";

	PlaylistTemplateInfo info(IDS_STRING_EDEM_STANDARD);
	info.pl_domain = "http://pl.mymagic.tv";
	info.pl_template = "{PL_DOMAIN}/srv/{SERVER_ID}/{QUALITY_ID}/{LOGIN}/{PASSWORD}/tv.m3u";
	info.pl_parse_regex = R"(^https?:\/\/.*\/srv\/(?<server>.+)\/(?<quality>.+)\/(?<login>.+)\/(?<password>.+)\/.*$)";
	info.parse_regex = R"(^(?<scheme>https?):\/\/(?<domain>[^\/]+)\/(?<token>.+)$)";
	info.tag_id_match = "CUID";
	info.per_channel_token = true;
	playlist_templates.emplace_back(info);

	square_icons = true;
	static_servers = true;
	static_qualities = true;

	streams_config[0].uri_template = "{SCHEME}://{DOMAIN}/{TOKEN}";
	streams_config[0].uri_arc_template = "{LIVE_URL}?utc={START}&lutc={NOW}";

	epg_params[0].epg_url = "{EPG_DOMAIN}/magic%2Fepg%2F{EPG_ID}.json";

	fill_servers_list();
	fill_qualities_list();
}

void plugin_mymagic::fill_servers_list(TemplateParams* params /*= nullptr*/)
{
	if (!get_servers_list().empty())
		return;

	std::vector<DynamicParamsInfo> servers;
	for (int i = 0; i <= IDS_STRING_MYMAGIC_P6 - IDS_STRING_MYMAGIC_P0; i++)
	{
		DynamicParamsInfo info;
		info.set_id(std::to_wstring(i));
		info.set_name(load_string_resource(1049, IDS_STRING_MYMAGIC_P0 + i));
		servers.emplace_back(info);
	}

	set_servers_list(servers);
}

void plugin_mymagic::fill_qualities_list(TemplateParams* params /*= nullptr*/)
{
	if (!get_qualities_list().empty())
		return;

	std::vector<DynamicParamsInfo> quality;
	for (int i = 0; i <= IDS_STRING_MYMAGIC_Q2 - IDS_STRING_MYMAGIC_Q1; i++)
	{
		DynamicParamsInfo info;
		info.set_id(std::to_wstring(i));
		info.set_name(load_string_resource(1049, IDS_STRING_MYMAGIC_Q1 + i));
		quality.emplace_back(info);
	}

	set_qualities_list(quality);
}
