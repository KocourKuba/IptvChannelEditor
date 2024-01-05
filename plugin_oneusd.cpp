/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2024): sharky72 (https://github.com/KocourKuba)

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
#include "plugin_oneusd.h"
#include "IPTVChannelEditor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

plugin_oneusd::plugin_oneusd()
{
	type_name = "oneusd";
}

void plugin_oneusd::load_default()
{
	base_plugin::load_default();

	title = "1USD TV";
	name = "oneusd.tv";
	access_type = AccountAccessType::enPin;

	provider_url = "http://1usd.tv/";

	PlaylistTemplateInfo info(IDS_STRING_EDEM_STANDARD);
	info.pl_template = "{PL_DOMAIN}/pl-{PASSWORD}-hls";
	info.pl_parse_regex = R"(^https?:\/\/.*\/pl-(?<password>.+)-hls$)";
	info.parse_regex = R"(^(?<scheme>https?:\/\/)(?<domain>.+)\/(?<id>.+)\/mono\.m3u8\?token=(?<token>.+)$)";
	playlist_templates.emplace_back(info);

	square_icons = true;

	streams_config[0].cu_type = CatchupType::cu_flussonic;
	streams_config[0].uri_template = "{SCHEME}{DOMAIN}/{ID}/mono.m3u8?token={TOKEN}";
	streams_config[0].uri_arc_template = "{SCHEME}{DOMAIN}/{ID}/index-{START}-{DURATION}.m3u8?token={TOKEN}";

	streams_config[1].uri_template = "{SCHEME}{DOMAIN}/{ID}/mpegts?token={TOKEN}";
	streams_config[1].uri_arc_template = "{SCHEME}{DOMAIN}/{ID}/archive-{START}-{DURATION}.ts?token={TOKEN}";

	epg_params[0].epg_domain = "http://tv.team";
	epg_params[0].epg_url = "{EPG_DOMAIN}/{EPG_ID}.json";

	epg_params[1].epg_url = "{EPG_DOMAIN}/tvteam%2Fepg%2F{EPG_ID}.json";
}

void plugin_oneusd::fill_domains_list(TemplateParams* params /*= nullptr*/)
{
	if (!get_domains_list().empty())
		return;

	DynamicParamsInfo info;
	info.set_id(L"0");
	info.set_name(L"http://1usd.tv");

	std::vector<DynamicParamsInfo> domains;
	domains.emplace_back(info);

	set_domains_list(domains);
}
