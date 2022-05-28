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
#include "uri_ottclub.h"

#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto PLAYLIST_TEMPLATE = L"http://myott.top/playlist/{:s}/m3u";
static constexpr auto URI_TEMPLATE_HLS = L"http://{DOMAIN}/stream/{TOKEN}/{ID}.m3u8";

uri_ottclub::uri_ottclub()
{
	epg_params[0].epg_url = L"http://myott.top/api/channel/{ID}";
	streams = { {StreamSubType::enHLS, L"HLS"} };
	provider_url = L"https://www.ottclub.cc/";
}

void uri_ottclub::parse_uri(const std::wstring& url)
{
	// http://myott.top/stream/S7NTAAORW5/131.m3u8

	static std::wregex re_url_hls(LR"(^https?:\/\/(.+)\/stream\/(.+)\/(.+)\.m3u8$)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url_hls))
	{
		templated = true;
		domain = std::move(m[1].str());
		token = std::move(m[2].str());
		id = std::move(m[3].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_ottclub::get_templated_stream(const StreamSubType subType, TemplateParams& params) const
{
	std::wstring url;

	if (is_template())
	{
		url = URI_TEMPLATE_HLS;
	}
	else
	{
		url = get_uri();
	}

	if (params.shift_back)
	{
		append_archive(url);
	}

	replace_vars(url, params);

	return url;
}

std::wstring uri_ottclub::get_playlist_url(TemplateParams& params)
{
	return fmt::format(PLAYLIST_TEMPLATE, params.password);
}
