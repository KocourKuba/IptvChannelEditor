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
#include "plugin_sharavoz.h"
#include "IPTVChannelEditor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

plugin_sharavoz::plugin_sharavoz()
{
	type_name = "sharavoz";
	class_name = "sharavoz_config";
}

void plugin_sharavoz::load_default()
{
	base_plugin::load_default();

	title = "Sharavoz TV";
	name = "sharavoz.tv";
	access_type = AccountAccessType::enPin;

	provider_url = "https://www.sharavoz.tv/";
	provider_api_url = "http://app.media24.cc/json/c";

	vod_templates.clear();
	PlaylistTemplateInfo vod_info(IDS_STRING_EDEM_STANDARD);
	vod_info.pl_template = "{API_URL}";
	vod_templates.emplace_back(vod_info);
	vod_engine = VodEngine::enXC;

	PlaylistTemplateInfo info(IDS_STRING_EDEM_STANDARD);
	info.pl_template = "{PL_DOMAIN}/iptv/p/{PASSWORD}/Sharavoz.Tv.navigator-ott.m3u";
	info.pl_parse_regex = R"(^https?:\/\/.*\/iptv\/p\/(?<password>.+)\/.*$)";
	info.parse_regex = R"(^(?<scheme>https?:\/\/)(?<domain>.+)\/(?<id>.+)\/(?:mpegts|index\.m3u8)\?token=(?<token>.+)$)";
	info.epg_id_from_id = true;
	playlist_templates.emplace_back(info);

	info.set_name(IDS_STRING_SHARAVOZ_DIRECT);
	info.pl_template = "{PL_DOMAIN}/iptv/p/{PASSWORD}/Playlist.navigator-ott.m3u";
	playlist_templates.emplace_back(info);

	streams_config[0].cu_type = CatchupType::cu_flussonic;
	streams_config[0].uri_template = "{SCHEME}{SERVER_ID}/{ID}/index.m3u8?token={TOKEN}";
	streams_config[0].uri_arc_template = "{SCHEME}{SERVER_ID}/{ID}/index-{START}-{DURATION}.m3u8?token={TOKEN}";

	streams_config[1].uri_template = "{SCHEME}{SERVER_ID}/{ID}/mpegts?token={TOKEN}";
	streams_config[1].uri_arc_template = "{SCHEME}{SERVER_ID}/{ID}/archive-{START}-{DURATION}.ts?token={TOKEN}";

	epg_params[0].epg_domain = "http://api.program.spr24.net";
	epg_params[0].epg_url = "{EPG_DOMAIN}/api/program?epg={EPG_ID}&date={DATE}";
	epg_params[0].epg_date_format = "{YEAR}-{MONTH}-{DAY}";

	epg_params[1].epg_domain = "http://epg.arlekino.tv";
	epg_params[1].epg_url = "{EPG_DOMAIN}/api/program?epg={EPG_ID}&date={DATE}";
	epg_params[1].epg_date_format = "{YEAR}-{MONTH}-{DAY}";

	static_servers = true;
}

void plugin_sharavoz::fill_servers_list(TemplateParams* params /*= nullptr*/)
{
	if (!get_servers_list().empty())
		return;

	std::vector<DynamicParamsInfo> servers =
	{
		{ "ru01.spr24.net", utils::utf16_to_utf8(load_string_resource(IDS_STRING_SHARAVOZ_P1)) },
		{ "ru02.spr24.net", utils::utf16_to_utf8(load_string_resource(IDS_STRING_SHARAVOZ_P2)) },
		{ "nl01.spr24.net", utils::utf16_to_utf8(load_string_resource(IDS_STRING_SHARAVOZ_P3)) },
		{ "am01.spr24.net", utils::utf16_to_utf8(load_string_resource(IDS_STRING_SHARAVOZ_P4)) },
		{ "fr01.spr24.net", utils::utf16_to_utf8(load_string_resource(IDS_STRING_SHARAVOZ_P5)) },
		{ "ch01.spr24.net", utils::utf16_to_utf8(load_string_resource(IDS_STRING_SHARAVOZ_P6)) },
		{ "eu01.spr24.net", utils::utf16_to_utf8(load_string_resource(IDS_STRING_SHARAVOZ_P7)) },
		{ "pl01.spr24.net", utils::utf16_to_utf8(load_string_resource(IDS_STRING_SHARAVOZ_P8)) },
	};

	set_servers_list(servers);
}

void plugin_sharavoz::fill_domains_list(TemplateParams* params /*= nullptr*/)
{
	if (!get_domains_list().empty())
		return;

	std::vector<DynamicParamsInfo> domains;

	DynamicParamsInfo info;
	info.set_id(L"0");
	info.set_name(L"http://www.sharavoz.tv");
	domains.emplace_back(info);

	set_domains_list(domains);
}
