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
#include "uri_lightiptv.h"
#include "PlayListEntry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto PLAYLIST_TEMPLATE = L"http://lightiptv.cc/playlist/hls/{:s}.m3u";
static constexpr auto URI_TEMPLATE_HLS = L"http://{DOMAIN}/{TOKEN}/video.m3u8?token={PASSWORD}";
static constexpr auto URI_TEMPLATE_ARCH_HLS = L"http://{DOMAIN}/{TOKEN}/video-{START}-7200.m3u8?token={PASSWORD}";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{DOMAIN}/{TOKEN}/mpegts?token={PASSWORD}";
static constexpr auto URI_TEMPLATE_ARCH_MPEG = L"http://{DOMAIN}/{TOKEN}/timeshift_abs-{START}.ts?token={PASSWORD}";

uri_lightiptv::uri_lightiptv()
{
	auto& params = epg_params[0];
//	params.epg_url = L"http://epg.esalecrm.net/lightiptv/epg/{ID}.json";
	params.epg_url = L"http://epg.ott-play.com/lightiptv/epg/{ID}.json";
	provider_url = L"https://ottbill.cc/";
	access_type = AccountAccessType::enPin;
}

void uri_lightiptv::parse_uri(const std::wstring& url)
{
	// http://de1light.pp.ru:8080/6e9751628dd0dbbdd6adf4232c135d83/video.m3u8?token=ds9fsahrrk

	static std::wregex re_url_hls(LR"(^https?:\/\/(.+)\/(.+)\/video\.m3u8\?token=(.+)$)");

	std::wsmatch m;
	if (std::regex_match(url, m, re_url_hls))
	{
		templated = true;
		domain = std::move(m[1].str());
		token = std::move(m[2].str());
		password = std::move(m[3].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_lightiptv::get_templated_stream(TemplateParams& params) const
{
	std::wstring url;

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
		}
	}
	else
	{
		url = get_uri();
		if (params.shift_back)
		{
			append_archive(url);
		}
	}

	replace_vars(url, params);
	return url;
}

std::wstring uri_lightiptv::get_playlist_url(TemplateParams& params)
{
	return fmt::format(PLAYLIST_TEMPLATE, params.password);
}
