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
#include "plugin_lightiptv.h"
#include "IPTVChannelEditor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

plugin_lightiptv::plugin_lightiptv()
{
	short_name = "lightiptv";
}

void plugin_lightiptv::load_default()
{
	title = "LightIPTV";
	name = "lightiptv";
	access_type = AccountAccessType::enPin;

	provider_url = "https://ottbill.cc/";

	PlaylistTemplateInfo info;
	info.set_name(load_string_resource(IDS_STRING_EDEM_STANDARD));
	info.pl_template = "http://lightiptv.cc/playlist/hls/{PASSWORD}.m3u";
	playlist_templates.emplace_back(info);

	uri_parse_pattern = R"(^https?:\/\/(?<domain>.+)\/(?<token>.+)\/video\.m3u8\?token=(?<password>.+)$)";
	uri_id_parse_pattern = "^#EXTINF:.+tvg-id=\"(?<id>[^\"]+)\"";

	square_icons = true;

	streams_config[0].cu_type = CatchupType::cu_flussonic;
	streams_config[0].cu_subst = "video";
	streams_config[0].uri_template = "http://{DOMAIN}/{TOKEN}/video.m3u8?token={PASSWORD}";
	streams_config[0].uri_arc_template = "http://{DOMAIN}/{TOKEN}/{CU_SUBST}-{START}-{DURATION}.m3u8?token={PASSWORD}";

	streams_config[1].cu_subst = "timeshift_abs";
	streams_config[1].uri_template = "http://{DOMAIN}/{TOKEN}/mpegts?token={PASSWORD}";
	streams_config[1].uri_arc_template = "http://{DOMAIN}/{TOKEN}/{CU_SUBST}-{START}-{DURATION}.ts?token={PASSWORD}";

	epg_params[0].epg_url = "http://epg.drm-play.ml/lightiptv/epg/{EPG_ID}.json";
	epg_params[1].epg_url = "http://epg.ott-play.com/lightiptv/epg/{EPG_ID}.json";
}
