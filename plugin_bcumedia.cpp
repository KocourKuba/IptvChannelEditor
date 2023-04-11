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
#include "plugin_bcumedia.h"
#include "IPTVChannelEditor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

plugin_bcumedia::plugin_bcumedia()
{
	type_name = "bcumedia";
}

void plugin_bcumedia::load_default()
{
	base_plugin::load_default();

	title = "BCU Media";
	name = "bcumedia";
	access_type = AccountAccessType::enPin;

	provider_url = "https://bcumedia.pro/";

	PlaylistTemplateInfo info(IDS_STRING_EDEM_STANDARD);
	info.pl_template = "https://bcumedia.pro/playlist/hls/{PASSWORD}.m3u";
	info.pl_parse_regex = R"(^https?:\/\/.*\/playlist\/hls\/(?<password>.+)\.m3u8?$)";
	info.parse_regex = R"(^https?:\/\/(?<domain>.+)\/(?<token>.+)\/video\.m3u8\?token=(?<password>.+)$)";
	info.tag_id_match = "tvg-id";
	playlist_templates.emplace_back(info);

	per_channel_token = true;
	square_icons = true;

	streams_config[0].cu_type = CatchupType::cu_flussonic;
	streams_config[0].uri_template = "http://{DOMAIN}/{TOKEN}/video.m3u8?token={PASSWORD}";
	streams_config[0].uri_arc_template = "http://{DOMAIN}/{TOKEN}/video-{START}-{DURATION}.m3u8?token={PASSWORD}";

	streams_config[1].uri_template = "http://{DOMAIN}/{TOKEN}/mpegts?token={PASSWORD}";
	streams_config[1].uri_arc_template = "http://{DOMAIN}/{TOKEN}/archive-{START}-{DURATION}.ts?token={PASSWORD}";

	epg_params[0].epg_url = "http://epg.drm-play.ml/bcu%2Fepg%2F{EPG_ID}.json";
}