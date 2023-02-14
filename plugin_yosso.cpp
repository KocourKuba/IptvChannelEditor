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
#include "plugin_yosso.h"
#include "PlayListEntry.h"
#include "IPTVChannelEditor.h"

#include "UtilsLib\inet_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

plugin_yosso::plugin_yosso()
{
	short_name = "yosso";
}

void plugin_yosso::load_default()
{
	title = "Yosso TV";
	name = "yossotv";
	access_type = AccountAccessType::enLoginPass;

	provider_url = "https://streaming-elbrus.su/";

	PlaylistTemplateInfo info(IDS_STRING_EDEM_STANDARD);
	info.pl_template = "https://streaming-elbrus.su/playlist/{LOGIN}/{PASSWORD}/RHLS/playlist.m3u8";
	info.parse_regex = R"(^https?:\/\/(?<domain>.+):(?<port>.+)\/(?<int_id>.+)\/(?<id>.+)\/video\.m3u8\?token=(?<token>.+)$)";
	playlist_templates.emplace_back(info);

	square_icons = true;

	streams_config[0].cu_type = CatchupType::cu_flussonic;
	streams_config[0].uri_template = "http://{DOMAIN}:{PORT}/{INT_ID}/{ID}/video.m3u8?token={TOKEN}";
	streams_config[0].uri_arc_template = "http://{DOMAIN}:{PORT}/{INT_ID}/{ID}/video-{START}-{DURATION}.m3u8?token={TOKEN}";

	auto& params1 = epg_params[0];
	epg_params[0].epg_url = "http://epg.drm-play.ml/yosso/epg/{EPG_ID}.json";
}
