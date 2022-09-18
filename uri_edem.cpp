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
#include "uri_edem.h"
#include "IptvChannelEditor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

uri_edem::uri_edem()
{
	provider_url = L"https://ilook.tv/";
	access_type = AccountAccessType::enOtt;
	catchup_type = { CatchupType::cu_shift, CatchupType::cu_none };
	support_streams = { {StreamSubType::enHLS, L"HLS"} };
	vod_supported = true;

	uri_hls_template = L"http://{SUBDOMAIN}/iptv/{TOKEN}/{ID}/index.m3u8";
	uri_parse_template = LR"(^https?:\/\/(?<domain>.+)\/iptv\/(?<token>.+)\/(?<id>\d+)\/.*\.m3u8$)";

	auto& params = epg_params[0];
	params.epg_url = L"http://epg.drm-play.ml/edem/epg/{ID}.json";

	playlists.clear();

	PlaylistInfo info;
	info.name = load_string_resource(IDS_STRING_EDEM_STANDARD);
	playlists.emplace_back(info);

	info.name = load_string_resource(IDS_STRING_EDEM_THEMATIC);
	playlists.emplace_back(info);

	info.name = load_string_resource(IDS_STRING_CUSTOM_PLAYLIST);
	info.is_custom = true;
	playlists.emplace_back(info);
}

void uri_edem::get_playlist_url(std::wstring& url, TemplateParams& params)
{
	static constexpr auto PLAYLIST_TEMPLATE1 = L"http://epg.it999.ru/edem_epg_ico.m3u8";
	static constexpr auto PLAYLIST_TEMPLATE2 = L"http://epg.it999.ru/edem_epg_ico2.m3u8";

	url = params.number == 0 ? PLAYLIST_TEMPLATE1 : PLAYLIST_TEMPLATE2;
}
