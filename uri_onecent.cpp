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
#include "uri_onecent.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto PLAYLIST_TEMPLATE = L"http://only4.tv/pl/{:s}/102/only4tv.m3u8";
static constexpr auto URI_TEMPLATE_HLS = L"http://{DOMAIN}/{ID}/index.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_ARCH_HLS = L"http://{DOMAIN}/{ID}/index-{START}-10800.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{DOMAIN}/{ID}/mpegts?token={TOKEN}";
static constexpr auto URI_TEMPLATE_ARCH_MPEG = L"http://{DOMAIN}/{ID}/archive-{START}-10800.ts?token={TOKEN}";

uri_onecent::uri_onecent()
{
	provider_url = L"https://1cent.tv/";
	access_type = AccountAccessType::enPin;

	auto& params1 = epg_params[0];
	params1.epg_url = L"http://epg.iptvx.one/api/id/{ID}.json";
	params1.epg_root = "ch_programme";
	params1.epg_name = "title";
	params1.epg_desc = "description";
	params1.epg_start = "start";
	params1.epg_end = "";
	params1.epg_time_format = "%d-%m-%Y %H:%M";
	params1.epg_tz = 3600 * 3; // iptvx.one uses moscow time (UTC+3)

	secondary_epg = true;
	epg_params[1].epg_url = L"http://epg.drm-play.ml/iptvx.one/epg/{ID}.json";
}

void uri_onecent::parse_uri(const std::wstring& url)
{
	// http://cdn.only4.tv/20115/index.m3u8?token=MH1LeVsHSD

	static std::wregex re_url_hls(LR"(^https?:\/\/(.+)\/(.+)\/index\.m3u8\?token=(.+)$)");

	std::wsmatch m;
	if (std::regex_match(url, m, re_url_hls))
	{
		templated = true;
		domain = std::move(m[1].str());
		id = std::move(m[2].str());
		token = std::move(m[3].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_onecent::get_templated_stream(TemplateParams& params) const
{
	std::wstring url = get_uri();

	if (is_template())
	{
		switch (params.streamSubtype)
		{
			case StreamSubType::enHLS:
				url = params.shift_back ? URI_TEMPLATE_ARCH_HLS : URI_TEMPLATE_HLS;
				break;
			case StreamSubType::enMPEGTS:
				url = params.shift_back ? URI_TEMPLATE_ARCH_MPEG : URI_TEMPLATE_MPEG;
				break;
			default:
				break;
		}
	}
	else
	{
		if (params.shift_back)
		{
			append_archive(url);
		}
	}

	replace_vars(url, params);
	return url;
}

std::wstring uri_onecent::get_playlist_url(TemplateParams& params)
{
	return fmt::format(PLAYLIST_TEMPLATE, params.password);
}
