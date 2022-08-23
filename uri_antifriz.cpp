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
#include "uri_antifriz.h"

#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// API documentation http://protected-api.com/api/documentation

static constexpr auto PLAYLIST_TEMPLATE = L"http://antifriz.tv/playlist/{:s}.m3u8";
static constexpr auto URI_TEMPLATE_HLS = L"http://{DOMAIN}/s/{TOKEN}/{ID}/video.m3u8";
static constexpr auto URI_TEMPLATE_ARCH_HLS = L"http://{DOMAIN}/{ID}/archive-{START}-10800.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{DOMAIN}/{ID}/mpegts?token={TOKEN}";
static constexpr auto URI_TEMPLATE_ARCH_MPEG = L"http://{DOMAIN}/{ID}/archive-{START}-10800.ts?token={TOKEN}";

uri_antifriz::uri_antifriz()
{
	provider_url = L"https://antifriztv.com/";
	provider_vod_url = L"http://protected-api.com";
	vod_supported = true;
	access_type = AccountAccessType::enPin;

	auto& params1 = epg_params[0];
	params1.epg_url = L"http://protected-api.com/epg/{ID}/?date=";
	params1.epg_root = "";
}

void uri_antifriz::parse_uri(const std::wstring& url)
{
	// http://tchaikovsky.antifriz.tv:1600/s/ibzsdpt2t/demo-4k/video.m3u8
	// http://tchaikovsky.antifriz.tv:80/demo-4k/mpegts?token=ibzsdpt2t

	static std::wregex re_url_hls(LR"(^https?:\/\/(.+)\/s\/(.+)\/(.+)\/video\.m3u8$)");
	static std::wregex re_url_mpeg(LR"(^https?:\/\/(.+)\/(.+)\/mpegts\?token=(.+)$)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url_hls))
	{
		templated = true;
		domain = std::move(m[1].str());
		token = std::move(m[2].str());
		id = std::move(m[3].str());
		return;
	}

	if (std::regex_match(url, m, re_url_mpeg))
	{
		templated = true;
		domain = std::move(m[1].str());
		id = std::move(m[2].str());
		token = std::move(m[3].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_antifriz::get_templated_stream(TemplateParams& params) const
{
	std::wstring url;

	if (is_template())
	{
		auto& new_params = const_cast<TemplateParams&>(params);
		std::wstring no_port(params.domain);
		if (auto pos = no_port.find(':'); pos != std::wstring::npos)
		{
			no_port = no_port.substr(0, pos);
		}

		switch (params.streamSubtype)
		{
			case StreamSubType::enHLS:
				url = params.shift_back ? URI_TEMPLATE_ARCH_HLS : URI_TEMPLATE_HLS;
				new_params.domain = params.shift_back ? no_port : params.domain;
				break;
			case StreamSubType::enMPEGTS:
				url = params.shift_back ? URI_TEMPLATE_ARCH_MPEG : URI_TEMPLATE_MPEG;
				new_params.domain = std::move(no_port);
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

std::wstring uri_antifriz::get_playlist_url(TemplateParams& params)
{
	return fmt::format(PLAYLIST_TEMPLATE, params.password);
}
