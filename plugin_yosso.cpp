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
	type_name = "yosso";
}

void plugin_yosso::load_default()
{
	base_plugin::load_default();

	title = "Yosso TV";
	name = "yossotv";
	access_type = AccountAccessType::enLoginPass;

	provider_url = "https://streaming-elbrus.su/";

	PlaylistTemplateInfo info(IDS_STRING_EDEM_STANDARD);
	info.pl_template = "{PL_DOMAIN}/playlist/{LOGIN}/{PASSWORD}/{SERVER_ID}playlist.m3u8";
	info.pl_parse_regex = R"(^https?:\/\/.*\/playlist\/(?<login>.+)\/(?<password>.+)\/(?<server>.+)\/.*$)";
	info.parse_regex = R"(^(?<scheme>https?:\/\/)(?<domain>.+):(?<port>\d+)\/(?<var1>.+\/)?(?<id>.+)\/(?<var2>.+)\.m3u8\?token=(?<token>.+)$)";
	info.square_icons = true;
	playlist_templates.emplace_back(info);

	streams_config[0].cu_type = CatchupType::cu_flussonic;
	streams_config[0].uri_template = "{SCHEME}{DOMAIN}:{PORT}/{VAR1}{ID}/{VAR2}.m3u8?token={TOKEN}";
	streams_config[0].uri_arc_template = "{SCHEME}{DOMAIN}:{PORT}/{VAR1}{ID}/{VAR2}-{START}-{DURATION}.m3u8?token={TOKEN}";

	epg_params[0].epg_url = "{EPG_DOMAIN}/yosso%2Fepg%2F{EPG_ID}.json";

	static_servers = true;
}

void plugin_yosso::fill_servers_list(TemplateParams* params /*= nullptr*/)
{
	if (!get_servers_list().empty())
		return;

	std::vector<DynamicParamsInfo> servers;
	for (int i = 0; i <= IDS_STRING_YOSSO_ID10 - IDS_STRING_YOSSO_ID1; i++)
	{
		DynamicParamsInfo info;
		info.set_id(load_string_resource(1033, IDS_STRING_YOSSO_ID1 + i));
		info.set_name(load_string_resource(1033, IDS_STRING_YOSSO_P1 + i));
		servers.emplace_back(info);
	}

	set_servers_list(servers);
}

void plugin_yosso::fill_domains_list(TemplateParams* params /*= nullptr*/)
{
	if (!get_domains_list().empty())
		return;

	DynamicParamsInfo info;
	info.set_id(L"0");
	info.set_name(L"https://streaming-elbrus.su");

	std::vector<DynamicParamsInfo> domains;
	domains.emplace_back(info);

	set_domains_list(domains);
}
