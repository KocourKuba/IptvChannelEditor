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
#include "plugin_crdtv.h"
#include "IPTVChannelEditor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

plugin_crdtv::plugin_crdtv()
{
	type_name = "crdtv";
}

void plugin_crdtv::load_default()
{
	base_plugin::load_default();

	title = "CRD TV";
	name = "crdtv";
	access_type = AccountAccessType::enPin;

	provider_url = "https://crdtv.net/";

	PlaylistTemplateInfo info(IDS_STRING_EDEM_STANDARD);
	info.pl_template = "{PL_DOMAIN}/m/{PASSWORD}/crdtv.m3u";
	info.pl_parse_regex = R"(^https?:\/\/.+\/m\/(?<password>.+)\/.*$)";
	info.parse_regex = R"(^https?:\/\/(?<domain>.+)\/(?<id>.+)\/(?<password>.+)\/.*$)";
	playlist_templates.emplace_back(info);

	square_icons = true;

	streams_config[0].uri_template = "{SCHEME}{DOMAIN}/{ID}/{PASSWORD}/live.m3u8";
	streams_config[0].uri_arc_template = "{LIVE_URL}?utc={START}&lutc={NOW}";

	epg_params[0].epg_url = "";
}

void plugin_crdtv::fill_domains_list(TemplateParams* params /*= nullptr*/)
{
	if (!get_domains_list().empty())
		return;

	DynamicParamsInfo info;
	info.set_id(L"0");
	info.set_name(L"http://crdtv.net");

	std::vector<DynamicParamsInfo> domains;
	domains.emplace_back(info);

	set_domains_list(domains);
}