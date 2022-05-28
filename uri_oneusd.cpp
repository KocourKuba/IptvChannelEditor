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
#include "uri_oneusd.h"
#include "UtilsLib\utils.h"
#include "PlayListEntry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto PLAYLIST_TEMPLATE = L"http://1usd.tv/pl-{:s}-hls";
static constexpr auto URI_TEMPLATE_HLS = L"http://{DOMAIN}/{ID}/mono.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_ARCH_HLS = L"http://{DOMAIN}/{ID}/index-{START}-7200.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{DOMAIN}/{ID}/mpegts?token={TOKEN}";
static constexpr auto URI_TEMPLATE_ARCH_MPEG = L"http://{DOMAIN}/{ID}/archive-{START}-7200.ts?token={TOKEN}";

uri_oneusd::uri_oneusd()
{
	epg_params[0].epg_url = L"http://tv.team/{ID}.json";
	provider_url = L"http://1usd.tv/";
}

void uri_oneusd::parse_uri(const std::wstring& url)
{
	// http://1.1usd.tv:34000/ch001/mono.m3u8?token=1usdtv_roberto.VdOQFJwYXrBNhWugsGPRdx-HBrzPKotuzKtiuHlD2Eyc7d7GPdA4CT8plX8MekNzcvYSCKA9C7SsfNNryytxJg..
	// http://1.1usd.tv:34000/ch002/mono.m3u8?token=1usdtv_roberto.VdOQFJwYXrBNhWugsGPRdx-HBrzPKotuzKtiuHlD2EzWB4Sdv5ZgmYMp5dx5SZqpGpZdSDUI6YVkqEciNnrvBA..
	// http://1.1usd.tv:34000/ch025/mono.m3u8?token=1usdtv_roberto.VdOQFJwYXrBNhWugsGPRdx-HBrzPKotuzKtiuHlD2Ezbn-vBVpPlFCl2dNce4nnKhFT1o9jM6oFrM-csWTJQCA..

	static std::wregex re_url_hls(LR"(^https?:\/\/(.+)\/(.+)\/mono\.m3u8\?token=(.+)$)");

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

std::wstring uri_oneusd::get_templated_stream(const StreamSubType subType, TemplateParams& params) const
{
	std::wstring url;

	if (is_template())
	{
		switch (subType)
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
		url = get_uri();
		if (params.shift_back)
		{
			append_archive(url);
		}
	}

	replace_vars(url, params);
	return url;
}

std::wstring uri_oneusd::get_playlist_url(TemplateParams& params)
{
	return fmt::format(PLAYLIST_TEMPLATE, params.password);
}
