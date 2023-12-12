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
#include "plugin_glanz.h"
#include "IPTVChannelEditor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

plugin_glanz::plugin_glanz()
{
	type_name = "glanz";
	class_name = "glanz_config";
}

void plugin_glanz::load_default()
{
	base_plugin::load_default();

	title = "glanz TV";
	name = "glanz.tv";
	access_type = AccountAccessType::enLoginPass;

	provider_url = "http://ottg.cc/";

	vod_templates.clear();
	PlaylistTemplateInfo vod_info(IDS_STRING_EDEM_STANDARD);
	vod_info.pl_template = "http://api.{PL_DOMAIN}/playlist/vod?login={LOGIN}&password={PASSWORD}";
	vod_templates.emplace_back(vod_info);
	vod_support = true;
	vod_filter = true;

	PlaylistTemplateInfo info(IDS_STRING_EDEM_STANDARD);
	info.pl_template = "http://pl.{PL_DOMAIN}/get.php?username={LOGIN}&password={PASSWORD}&type=m3u&output=hls";
	info.pl_parse_regex = R"(^https?:\/\/.+\?username=(?<login>.+)&password=(?<password>[^&]+)&.*$)";
	info.parse_regex = R"(^(?<scheme>https?:\/\/)(?<domain>.+)\/(?<id>.+)\/.+\?username=(?<login>.+)&password=(?<password>.+)&token=(?<token>.+)&ch_id=(?<int_id>.+)&req_host=(?<host>.+)$)";
	playlist_templates.emplace_back(info);

	info.set_name(IDS_STRING_NO_ADULT);
	info.pl_template = "http://pl.{PL_DOMAIN}/get.php?username={LOGIN}&password={PASSWORD}&type=m3u&output=hls&censored=0";
	playlist_templates.emplace_back(info);

	square_icons = true;

	streams_config[0].cu_type = CatchupType::cu_flussonic;
	streams_config[0].uri_template = "{SCHEME}{DOMAIN}/{ID}/video.m3u8?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}";
	streams_config[0].uri_arc_template = "{SCHEME}{DOMAIN}/{ID}/video-{START}-{DURATION}.m3u8?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}";

	streams_config[1].cu_type = CatchupType::cu_flussonic;
	streams_config[1].uri_template = "{SCHEME}{DOMAIN}/{ID}/mpegts?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}";
	streams_config[1].uri_arc_template = "{SCHEME}{DOMAIN}/{ID}/archive-{START}-{DURATION}.ts?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}";

	epg_params[0].epg_url = "{EPG_DOMAIN}/ottg%2Fepg%2F{EPG_ID}.json";

	set_epg_preset(1, EpgPresets::enIptvxOne);
	epg_params[1].epg_domain = "http://epg.iptvx.one";
	epg_params[1].epg_url = "{EPG_DOMAIN}/api/id/{EPG_ID}.json";
}

void plugin_glanz::fill_domains_list(TemplateParams* params /*= nullptr*/)
{
	if (!get_domains_list().empty())
		return;

	std::vector<DynamicParamsInfo> domains;

	DynamicParamsInfo info;
	info.set_id(L"0");
	info.set_name(L"ottg.cc");
	domains.emplace_back(info);

	info.set_id(L"1");
	info.set_name(L"ottg.in");
	domains.emplace_back(info);

	info.set_id(L"2");
	info.set_name(L"ottg.tv");
	domains.emplace_back(info);

	info.set_id(L"3");
	info.set_name(L"ottg.space");
	domains.emplace_back(info);

	info.set_id(L"4");
	info.set_name(L"ottg.eu");
	domains.emplace_back(info);

	set_domains_list(domains);
}
