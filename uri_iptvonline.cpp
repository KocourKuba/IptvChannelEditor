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
#include "uri_iptvonline.h"

#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto PLAYLIST_TEMPLATE = L"http://iptv.online/play/{:s}/m3u8";
static constexpr auto URI_TEMPLATE_HLS = L"http://{DOMAIN}/play/{ID}/{TOKEN}/video.m3u8";
static constexpr auto URI_TEMPLATE_ARCHIVE_HLS = L"http://{DOMAIN}/play/{ID}/{TOKEN}/video-{START}-10800.m3u8";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{DOMAIN}/play/{ID}/{TOKEN}/mpegts";
static constexpr auto URI_TEMPLATE_ARCHIVE_MPEG = L"http://{DOMAIN}/play/{ID}/{TOKEN}/archive-{START}-10800.ts";

uri_iptvonline::uri_iptvonline()
{
	provider_url = L"https://iptv.online/";
}

void uri_iptvonline::parse_uri(const std::wstring& url)
{
	// http://cz.iptv.monster/play/73/38798DB9DF4EA8F/video.m3u8

	static std::wregex re_url_hls(LR"(^https?:\/\/(.+)\/play\/(.+)\/(.+)\/video\.m3u8$)");
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

std::wstring uri_iptvonline::get_templated_stream(StreamSubType subType, const TemplateParams& params) const
{
	std::wstring url;

	if (is_template())
	{
		url = URI_TEMPLATE_HLS;
		switch (subType)
		{
			case StreamSubType::enHLS:
				url = (params.shift_back) ? URI_TEMPLATE_ARCHIVE_HLS : URI_TEMPLATE_HLS;
				break;
			case StreamSubType::enMPEGTS:
				url = (params.shift_back) ? URI_TEMPLATE_ARCHIVE_MPEG : URI_TEMPLATE_MPEG;
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

std::wstring uri_iptvonline::get_playlist_url(const PlaylistTemplateParams& params) const
{
	return fmt::format(PLAYLIST_TEMPLATE, params.password);
}
