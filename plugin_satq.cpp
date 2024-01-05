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
#include "plugin_satq.h"
#include "IPTVChannelEditor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

plugin_satq::plugin_satq()
{
	type_name = "satq";
}

void plugin_satq::load_default()
{
	base_plugin::load_default();

	title = "Satq TV";
	name = "satq";
	access_type = AccountAccessType::enPin;

	provider_url = "https://satq.tv/";

	PlaylistTemplateInfo info(IDS_STRING_EDEM_STANDARD);
	info.pl_template = "{PL_DOMAIN}/1/{PASSWORD}.m3u8";
	info.pl_parse_regex = R"(^https?:\/\/(?<password>.+)\.m3u8?$)";
	info.parse_regex = R"(^(?<scheme>https?:\/\/)(?<domain>.+)\/(?<id>.+)\/(?<token>.+)\/(video\.m3u8|mpegts)$)";
	playlist_templates.emplace_back(info);

	streams_config[0].cu_type = CatchupType::cu_flussonic;
	streams_config[0].uri_template = "{SCHEME}{SERVER_ID}/{ID}/{TOKEN}/video.m3u8";
	streams_config[0].uri_arc_template = "{SCHEME}{SERVER_ID}/{ID}/index-{START}-{DURATION}.m3u8?token={TOKEN}";

	streams_config[1].uri_template = "{SCHEME}{SERVER_ID}/{ID}/{TOKEN}/mpegts";
	streams_config[1].uri_arc_template = "{SCHEME}{SERVER_ID}/{ID}/archive-{START}-{DURATION}.ts?token={TOKEN}";

	epg_params[0].epg_domain = "";
	epg_params[1].epg_domain = "";

	static_servers = true;
}

void plugin_satq::fill_servers_list(TemplateParams* params /*= nullptr*/)
{
	if (!get_servers_list().empty())
		return;

	std::vector<DynamicParamsInfo> servers =
	{
		{ "cdn.iptv11.com",   utils::utf16_to_utf8(load_string_resource(IDS_STRING_SATQ_P0)) },
		{ "serv2.iptv11.com", utils::utf16_to_utf8(load_string_resource(IDS_STRING_SATQ_P1)) },
		{ "serv6.iptv11.com", utils::utf16_to_utf8(load_string_resource(IDS_STRING_SATQ_P2)) },
		{ "serv1.iptv11.com", utils::utf16_to_utf8(load_string_resource(IDS_STRING_SATQ_P3)) },
		{ "serv5.iptv11.com", utils::utf16_to_utf8(load_string_resource(IDS_STRING_SATQ_P4)) },
		{ "serv7.iptv11.com", utils::utf16_to_utf8(load_string_resource(IDS_STRING_SATQ_P5)) },
		{ "serv3.iptv11.com", utils::utf16_to_utf8(load_string_resource(IDS_STRING_SATQ_P6)) },
		{ "serv4.iptv11.com", utils::utf16_to_utf8(load_string_resource(IDS_STRING_SATQ_P7)) },
	};

	set_servers_list(servers);
}

void plugin_satq::fill_domains_list(TemplateParams* params /*= nullptr*/)
{
	if (!get_domains_list().empty())
		return;

	std::vector<DynamicParamsInfo> domains;

	DynamicParamsInfo info;
	info.set_id(L"0");
	info.set_name(L"http://iptv11.com");
	domains.emplace_back(info);

	set_domains_list(domains);
}
