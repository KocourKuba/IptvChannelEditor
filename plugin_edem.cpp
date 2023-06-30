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
#include "plugin_edem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

plugin_edem::plugin_edem()
{
	type_name = "edem";
	class_name = "edem_config";
}

void plugin_edem::load_default()
{
	base_plugin::load_default();

	vod_support = true;
	vod_filter = true;

	PlaylistTemplateInfo info(IDS_STRING_EDEM_STANDARD);
	info.pl_domain = "http://epg.it999.ru";
	info.pl_template = "{PL_DOMAIN}/edem_epg_ico.m3u8";
	info.pl_parse_regex = R"(^https?:\/.*\/playlists\/uplist\/.*\/playlist\.m3u8$)";
	info.parse_regex = R"(^https?:\/\/(?<subdomain>.+)\/iptv\/(?<token>.+)\/(?<id>.+)\/.*\.m3u8$)";
	playlist_templates.emplace_back(info);

	info.set_name(IDS_STRING_EDEM_THEMATIC);
	info.pl_template = "{PL_DOMAIN}/edem_epg_ico2.m3u8";
	playlist_templates.emplace_back(info);

	PlaylistTemplateInfo vod_info;
	vod_info.set_name(load_string_resource(0, IDS_STRING_EDEM_STANDARD));
	vod_info.pl_template = "{SUBDOMAIN}";
	vod_templates.emplace_back(vod_info);

	title = "iEdem/iLook TV";
	name = "iedem.tv";
	access_type = AccountAccessType::enOtt;

	provider_url = "https://ilook.tv/";

	streams_config[0].uri_template = "http://{SUBDOMAIN}/iptv/{TOKEN}/{ID}/index.m3u8";
	streams_config[0].uri_arc_template = "{LIVE_URL}?utc={START}&lutc={NOW}";
	streams_config[0].dune_params = "hls_forced_type:event";

	epg_params[0].epg_url = "{EPG_DOMAIN}/edem%2Fepg%2F{EPG_ID}.json";
}
