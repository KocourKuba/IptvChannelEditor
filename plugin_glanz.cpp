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
#include "plugin_glanz.h"
#include "IPTVChannelEditor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

plugin_glanz::plugin_glanz()
{
	short_name = "glanz";
}

void plugin_glanz::load_default()
{
	title = "glanz TV";
	name = "glanz.tv";
	access_type = AccountAccessType::enLoginPass;

	provider_url = "http://ottg.cc/";

	PlaylistTemplateInfo vod_info;
	vod_info.set_name(load_string_resource(0, IDS_STRING_EDEM_STANDARD));
	vod_info.pl_template = "http://api.ottg.cc/playlist/vod?login={LOGIN}&password={PASSWORD}";
	vod_templates.emplace_back(vod_info);
	vod_support = true;
	vod_filter = true;

	PlaylistTemplateInfo info(IDS_STRING_EDEM_STANDARD);
	info.pl_template = "http://pl.ottg.cc/get.php?username={LOGIN}&password={PASSWORD}&type=m3u&output=hls";
	playlist_templates.emplace_back(info);

	info.set_name(IDS_STRING_NO_ADULT);
	info.pl_template = "http://pl.ottg.cc/get.php?username={LOGIN}&password={PASSWORD}&type=m3u&output=hls&censored=0";
	info.parse_regex = R"(^https?:\/\/(?<domain>.+)\/(?<id>.+)\/.+\?username=(?<login>.+)&password=(?<password>.+)&token=(?<token>.+)&ch_id=(?<int_id>.+)&req_host=(?<host>.+)$)";
	playlist_templates.emplace_back(info);

	square_icons = true;

	streams_config[0].cu_type = CatchupType::cu_flussonic;
	streams_config[0].cu_subst = "video";
	streams_config[0].uri_template = "http://{DOMAIN}/{ID}/video.m3u8?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}";
	streams_config[0].uri_arc_template = "http://{DOMAIN}/{ID}/{CU_SUBST}-{START}-{DURATION}.m3u8?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}";

	streams_config[1].cu_type = CatchupType::cu_flussonic;
	streams_config[1].uri_template = "http://{DOMAIN}/{ID}/mpegts?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}";
	streams_config[1].uri_arc_template = "http://{DOMAIN}/{ID}/{CU_SUBST}-{START}-{DURATION}.ts?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}";

	auto& params1 = epg_params[0];
	params1.epg_url = "http://epg.iptvx.one/api/id/{EPG_ID}.json";
	params1.epg_root = "ch_programme";
	params1.epg_name = "title";
	params1.epg_desc = "description";
	params1.epg_start = "start";
	params1.epg_end = "";
	params1.epg_time_format = "{DAY}-{MONTH}-{YEAR} {HOUR}:{MIN}"; // "%d-%m-%Y %H:%M";
	params1.epg_timezone = 3; // iptvx.one uses moscow time (UTC+3)
}
