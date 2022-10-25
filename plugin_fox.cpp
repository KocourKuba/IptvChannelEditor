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
#include "plugin_fox.h"
#include "IPTVChannelEditor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

plugin_fox::plugin_fox()
{
	short_name = "fox";
}

void plugin_fox::load_default()
{
	title = "Fox TV";
	name = "fox-fun.tv";
	access_type = AccountAccessType::enLoginPass;

	provider_url = "http://info.fox-tv.fun/";

	PlaylistTemplateInfo vod_info;
	vod_info.set_name(load_string_resource(IDS_STRING_EDEM_STANDARD));
	vod_info.pl_template = "http://pl.fox-tv.fun/{LOGIN}/{PASSWORD}/vodall.m3u";
	vod_templates.emplace_back(vod_info);

	vod_info.set_name(load_string_resource(IDS_STRING_NO_ADULT));
	vod_info.pl_template = "http://pl.fox-tv.fun/{LOGIN}/{PASSWORD}/vod.m3u";
	vod_templates.emplace_back(vod_info);

	vod_parse_pattern = R"(^#EXTINF:.+tvg-logo=\"(?<logo>[^\"]+)\".+group-title="(?<category>[^\"]+)\".*,\s*(?<title>[^\/]+)\/(?<title_orig>.+)\s(?<year>\d+)$)";

	vod_support = true;
	vod_m3u = true;

	PlaylistTemplateInfo info;
	info.set_name(load_string_resource(IDS_STRING_EDEM_STANDARD));
	info.pl_template = "http://pl.fox-tv.fun/{LOGIN}/{PASSWORD}/tv.m3u";
	playlist_templates.emplace_back(info);

	uri_parse_pattern = R"(^https?:\/\/(?<domain>[^\/]+)\/(?<token>.+)$)";
	uri_id_parse_pattern = "^#EXTINF:.+CUID=\"(?<id>[^\"]+)\"";

	square_icons = true;
	per_channel_token = true;

	streams_config[0].uri_template = "http://{DOMAIN}/{TOKEN}";
	streams_config[0].uri_arc_template = "{LIVE_URL}?{CU_SUBST}={START}&lutc={NOW}";

	epg_params[0].epg_url = "http://epg.drm-play.ml/fox-tv/epg/{EPG_ID}.json";
}
