/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2022): sharky72 (https://github.com/KocourKuba)

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
#include "uri_viplime.h"
#include "IPTVChannelEditor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

uri_viplime::uri_viplime()
{
	short_name = "viplime";
}

void uri_viplime::fill_quality_list(TemplateParams& /*params*/)
{
	if (!get_quality_list().empty())
		return;

	struct Info
	{
		std::wstring id;
		UINT res;
	};

	std::vector<Info> infos =
	{
		{ L"high",   IDS_STRING_VIPLIME_P1 },
		{ L"middle", IDS_STRING_VIPLIME_P2 },
		{ L"low",    IDS_STRING_VIPLIME_P3 },
		{ L"variant",IDS_STRING_VIPLIME_P4 },
		{ L"hls",    IDS_STRING_VIPLIME_P5 },
	}; //-V826

	std::vector<DynamicParamsInfo> quality;
	for (const auto& item : infos)
	{
		DynamicParamsInfo info;
		info.set_id(item.id);
		info.set_name(load_string_resource(1049, item.res));
		quality.emplace_back(info);
	}

	set_quality_list(quality);
}

void uri_viplime::load_default()
{
	title = "VipLime TV";
	name = "viplime.fun.tv";
	access_type = AccountAccessType::enPin;

	provider_url = "http://viplime.fun/";
	playlist_template = "http://cdntv.online/high/{PASSWORD}/playlist.m3u8";
	uri_parse_pattern = R"(^https?:\/\/(?<domain>.+)\/(?<quality>.+)\/(?<token>.+)\/(?<id>.+).m3u8$)";

	streams_config[0].uri_template = "http://{DOMAIN}/{QUALITY_ID}/{TOKEN}/{ID}.m3u8";
	streams_config[0].uri_arc_template = "{CU_SUBST}={START}&lutc={NOW}";

	streams_config[1].cu_type = CatchupType::cu_shift;
	streams_config[1].cu_subst = "utc";
	streams_config[1].uri_template = "http://{DOMAIN}/{QUALITY_ID}/{TOKEN}/{ID}.mpeg";
	streams_config[1].uri_arc_template = "{CU_SUBST}={START}&lutc={NOW}";

	epg_params[0].epg_url = "http://epg.drm-play.ml/viplime/epg/{ID}.json";
}
