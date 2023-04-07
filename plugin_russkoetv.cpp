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
#include "plugin_russkoetv.h"
#include "IPTVChannelEditor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

plugin_russkoetv::plugin_russkoetv()
{
	type_name = "russkoetv";
}

void plugin_russkoetv::load_default()
{
	base_plugin::load_default();

	title = "Russkoe TV";
	name = "russkoetv";
	access_type = AccountAccessType::enPin;

	provider_url = "https://russkoetv.tv/";

	PlaylistTemplateInfo info;
	info.set_name(load_string_resource(IDS_STRING_EDEM_STANDARD));
	info.pl_template = "http://russkoetv.tv/play/{PASSWORD}.m3u8";
	info.pl_parse_regex = R"(^https?:\/\/.*\/(?<password>.+)\.m3u8?$)";
	info.parse_regex = R"(^https?:\/\/(?<domain>.+)\/s\/(?<token>.+)\/(?<id>.+)\.m3u8$)";
	playlist_templates.emplace_back(info);

	streams_config[0].uri_template = "http://{DOMAIN}/s/{TOKEN}/{ID}.m3u8";
	streams_config[0].uri_arc_template = "{LIVE_URL}?utc={START}&lutc={NOW}";

	epg_params[0].epg_url = "http://protected-api.com/epg/{EPG_ID}/?date=";
	epg_params[0].epg_root = "";
}
