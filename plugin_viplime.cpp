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
#include "plugin_viplime.h"
#include "IPTVChannelEditor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

plugin_viplime::plugin_viplime()
{
	type_name = "viplime";
	class_name = "viplime_config";
}

void plugin_viplime::load_default()
{
	base_plugin::load_default();

	title = "VipLime TV";
	name = "viplime.fun.tv";
	access_type = AccountAccessType::enPin;

	provider_url = "http://viplime.fun/";

	PlaylistTemplateInfo info(IDS_STRING_EDEM_STANDARD);
	info.pl_domain = "http://cdntv.online";
	info.pl_template = "{PL_DOMAIN}/high/{PASSWORD}/playlist.m3u8";
	info.pl_parse_regex = R"(^https?:\/\/.*\/.*\/(?<password>.+)\/.*$)";
	info.parse_regex = R"(^https?:\/\/(?<domain>.+)\/(?<quality>.+)\/(?<token>.+)\/(?<id>.+).m3u8$)";
	playlist_templates.emplace_back(info);

	streams_config[0].uri_template = "http://{DOMAIN}/{QUALITY_ID}/{TOKEN}/{ID}.m3u8";
	streams_config[0].uri_arc_template = "{LIVE_URL}?utc={START}&lutc={NOW}";

	streams_config[1].cu_type = CatchupType::cu_shift;
	streams_config[1].uri_template = "http://{DOMAIN}/{QUALITY_ID}/{TOKEN}/{ID}.mpeg";
	streams_config[1].uri_arc_template = "{LIVE_URL}?utc={START}&lutc={NOW}";

	epg_params[0].epg_domain = "http://epg.drm-play.com";
	epg_params[0].epg_url = "{EPG_DOMAIN}/viplime%2Fepg%2F{EPG_ID}.json";

	static_qualities = true;
	fill_qualities_list();
}

void plugin_viplime::fill_qualities_list(TemplateParams* params /*= nullptr*/)
{
	if (!get_qualities_list().empty())
		return;

	struct Info
	{
		std::wstring id;
		UINT res;
	};

	std::vector<Info> infos = //-V826
	{
		{ L"high",   IDS_STRING_VIPLIME_P1 },
		{ L"middle", IDS_STRING_VIPLIME_P2 },
		{ L"low",    IDS_STRING_VIPLIME_P3 },
		{ L"variant",IDS_STRING_VIPLIME_P4 },
		{ L"hls",    IDS_STRING_VIPLIME_P5 },
	};

	std::vector<DynamicParamsInfo> quality;
	for (const auto& item : infos)
	{
		DynamicParamsInfo info;
		info.set_id(item.id);
		info.set_name(load_string_resource(1049, item.res));
		quality.emplace_back(info);
	}

	set_qualities_list(quality);
}
