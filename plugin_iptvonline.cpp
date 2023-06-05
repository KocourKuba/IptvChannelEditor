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
#include "plugin_iptvonline.h"
#include "IPTVChannelEditor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

plugin_iptvonline::plugin_iptvonline()
{
	type_name = "iptvonline";
}

void plugin_iptvonline::load_default()
{
	base_plugin::load_default();

	title = "IPTV Online";
	name = "iptvonline";
	access_type = AccountAccessType::enPin;

	provider_url = "https://iptv.online/";

	PlaylistTemplateInfo info(IDS_STRING_EDEM_STANDARD);
	info.pl_domain = "http://iptv.online";
	info.pl_template = "{PL_DOMAIN}/play/{PASSWORD}/m3u8";
	info.pl_parse_regex = R"(^https?:\/\/.+\/play\/(?<password>.+)\/m3u8?$)";
	info.parse_regex = R"(^https?:\/\/(?<domain>.+)\/play\/(?<id>.+)\/(?<token>.+)\/video\.m3u8$)";
	playlist_templates.emplace_back(info);

	square_icons = true;

	streams_config[0].cu_type = CatchupType::cu_flussonic;
	streams_config[0].uri_template = "http://{DOMAIN}/play/{ID}/{TOKEN}/video.m3u8";
	streams_config[0].uri_arc_template = "http://{DOMAIN}/play/{ID}/{TOKEN}/video-{START}-{DURATION}.m3u8";

	streams_config[1].uri_template = "http://{DOMAIN}/play/{ID}/{TOKEN}/mpegts";
	streams_config[1].uri_arc_template = "http://{DOMAIN}/play/{ID}/{TOKEN}/archive-{START}-{DURATION}.ts";

	epg_params[0].epg_url = "{EPG_DOMAIN}/iptvx.one%2Fepg%2F{EPG_ID}.json";

	set_epg_preset(1, EpgPresets::enIptvxOne);
	epg_params[1].epg_domain = "http://epg.iptvx.one";
	epg_params[1].epg_url = "{EPG_DOMAIN}/api/id/{EPG_ID}.json";
}
