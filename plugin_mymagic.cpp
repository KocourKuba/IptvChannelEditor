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
#include "plugin_mymagic.h"
#include "IptvChannelEditor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

plugin_mymagic::plugin_mymagic()
{
	short_name = "mymagic";
}

void plugin_mymagic::load_default()
{
	title = "MyMagic TV";
	name = "mymagic";
	access_type = AccountAccessType::enLoginPass;

	provider_url = "http://mymagic.tv/";

	PlaylistTemplateInfo info(IDS_STRING_EDEM_STANDARD);
	info.pl_template = "http://pl.mymagic.tv/srv/{SERVER_ID}/{QUALITY_ID}/{LOGIN}/{PASSWORD}/tv.m3u";
	info.parse_regex = R"(^https?:\/\/(?<domain>[^\/]+)\/(?<token>.+)$)";
	info.tag_id_match = "CUID";
	playlist_templates.emplace_back(info);

	square_icons = true;
	per_channel_token = true;

	streams_config[0].uri_template = "http://{DOMAIN}/{TOKEN}";
	streams_config[0].uri_arc_template = "{LIVE_URL}?utc={START}&lutc={NOW}";

	epg_params[0].epg_url = "http://epg.drm-play.ml/magic/epg/{EPG_ID}.json";
	epg_params[1].epg_url = "http://epg.esalecrm.net/magic/epg/{EPG_ID}.json";

	static_servers = true;
	static_qualities = true;
	fill_servers_list(TemplateParams());
	fill_qualities_list(TemplateParams());
}

void plugin_mymagic::fill_servers_list(TemplateParams& /*params*/)
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

void plugin_mymagic::fill_qualities_list(TemplateParams& /*params*/)
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
