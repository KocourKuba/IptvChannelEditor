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
	provider_url = L"http://viplime.fun/";
	access_type = AccountAccessType::enPin;
	catchup_type = { CatchupType::cu_shift, CatchupType::cu_shift };
	server_subst_type = ServerSubstType::enStream;

	playlist_template = L"http://cdntv.online/high/{PASSWORD}/playlist.m3u8";
	uri_hls_template = L"http://{DOMAIN}/{QUALITY}/{TOKEN}/{ID}.m3u8";
	uri_mpeg_template = L"http://{DOMAIN}/{QUALITY}/{TOKEN}/{ID}.mpeg";

	epg_params[0].epg_url = L"http://epg.drm-play.ml/viplime/epg/{ID}.json";

	quality_list = {
		{ L"high",   load_string_resource(IDS_STRING_VIPLIME_P1) },
		{ L"middle", load_string_resource(IDS_STRING_VIPLIME_P2) },
		{ L"low",    load_string_resource(IDS_STRING_VIPLIME_P3) },
		{ L"variant",load_string_resource(IDS_STRING_VIPLIME_P4) },
		{ L"hls",    load_string_resource(IDS_STRING_VIPLIME_P5) },
	};
}

void uri_viplime::parse_uri(const std::wstring& url)
{
	// http://cdntv.online/high/z3sf8hueit/1.m3u8
	// http://cdntv.online/high/z3sf8hueit/1.mpeg
	static std::wregex re_url_hls(LR"(^https?:\/\/(.+)\/(.+)\/(.+)\/(.+).m3u8$)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url_hls))
	{
		templated = true;
		domain = std::move(m[1].str());
		token = std::move(m[3].str());
		id = std::move(m[4].str());
		return;
	}

	uri_stream::parse_uri(url);
}
