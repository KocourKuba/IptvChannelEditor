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
#include "plugin_sharatv.h"
#include "IPTVChannelEditor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

plugin_sharatv::plugin_sharatv()
{
	type_name = "sharatv";
}

void plugin_sharatv::load_default()
{
	base_plugin::load_default();

	title = "Shara TV";
	name = "shara.tv";
	access_type = AccountAccessType::enLoginPass;

	provider_url = "https://shara-tv.org/";

	PlaylistTemplateInfo info(IDS_STRING_EDEM_STANDARD);
	info.pl_domain = "http://tvfor.pro";
	info.pl_template = "{PL_DOMAIN}/g/{LOGIN}:{PASSWORD}/1/playlist.m3u";
	info.pl_parse_regex = R"(^https?:\/\/.*\/(?<login>.+):(?<password>.+)\/.+\/.*$)";
	info.parse_regex = R"(^https?:\/\/(?<domain>.+)\/(?<id>.+)\/(?<token>.+)$)";
	playlist_templates.emplace_back(info);

	streams_config[0].uri_template = "http://{DOMAIN}/{ID}/{TOKEN}";
	streams_config[0].uri_arc_template = "{LIVE_URL}?utc={START}&lutc={NOW}";

	epg_params[0].epg_domain = "http://epg.drm-play.com";
	epg_params[0].epg_url = "{EPG_DOMAIN}/shara-tv%2Fepg%2F{EPG_ID}.json";
}
