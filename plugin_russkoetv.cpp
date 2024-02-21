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
#include "plugin_russkoetv.h"
#include "IPTVChannelEditor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// API documentation http://protected-api.com/api/documentation

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
	provider_api_url = "http://protected-api.com";

	PlaylistTemplateInfo info(IDS_STRING_EDEM_STANDARD);
	info.pl_template = "{PL_DOMAIN}/play/{PASSWORD}.m3u8";
	info.pl_parse_regex = R"(^https?:\/\/.*\/(?<password>.+)\.m3u8?$)";
	info.parse_regex = R"(^(?<scheme>https?:\/\/)(?<domain>.+)\/s\/(?<token>.+)\/.+\.m3u8$)";
	info.tag_id_match = "tvg-id";
	playlist_templates.emplace_back(info);

	streams_config[0].uri_template = "{SCHEME}{DOMAIN}/s/{TOKEN}/{ID}.m3u8";
	streams_config[0].uri_arc_template = "{LIVE_URL}?utc={START}&lutc={NOW}";

	set_epg_preset(0, EpgPresets::enCbilling);
	epg_params[0].epg_domain = "";
	epg_params[0].epg_url = "{API_URL}/epg/{EPG_ID}/?date=";
}

void plugin_russkoetv::fill_domains_list(TemplateParams* params /*= nullptr*/)
{
	if (!get_domains_list().empty())
		return;

	DynamicParamsInfo info;
	info.set_id(L"0");
	info.set_name(L"https://russkoetv.tv");

	std::vector<DynamicParamsInfo> domains;
	domains.emplace_back(info);

	set_domains_list(domains);
}
