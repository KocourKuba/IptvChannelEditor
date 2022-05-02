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
#include "uri_fox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto PLAYLIST_TEMPLATE = L"http://pl.fox-tv.fun/{:s}/{:s}/tv.m3u";
static constexpr auto URI_TEMPLATE = L"http://{DOMAIN}/{TOKEN}";

uri_fox::uri_fox() : epg_technic({ L"fox", L"fox" })
{
	streams = { {StreamSubType::enHLS, L"HLS"} };
	provider_url = L"http://info.fox-tv.fun/";
}

void uri_fox::parse_uri(const std::wstring& url)
{
	// http://ost.fox-tv.fun/vLm0zdTg_dG9rZW49W3N0Yl90b2tlbl0iLCJhIjoiaHR0cDovL3N0cjIuZm94LXR2LmZ1bjo5OTg1L1BlcnZpeWthbmFsL3ZpZGVvLXRpbWVzaGlmdF9hYnMtW3RpbWVfc3RhcnRdLm0zdTg_dG9rZW49W3N0Yl90b2tlbl1bY3Vycl90aW1lXSIsImwiOiI2NTgxMWQwZCIsInAiOiI2NTgxMWQwZDNjMTRjMTFlIiwiYyI6IjEiLCJ0IjoiN2FiMDJjOTk4MmY4NjI4NGU1ODhkYTliZjc0YmU4YTgiLCJkIjoiMjk2NjciLCJyIjoiMTI5NjY4In0eyJ1IjoiaHR0cDovL3N0cjIuZm94LXR2LmZ1bjo5OTg2L1BlcnZpeWthbmFsL3ZpZGV/video.m3u8
	// http://ost.fox-tv.fun/0cz90b2tlbj1bc3RiX3Rva2VuXSIsImEiOiJodHRwOi8vc3RyMi5mb3gtdHYuZnVuOjk5ODUvUGVydml5a2FuYWwvdGltZXNoaWZ0X2Ficy1bdGltZV9zdGFydF0udHM_dG9rZW49W3N0Yl90b2tlbl0iLCJsIjoiNjU4MTFkMGQiLCJwIjoiNjU4MTFkMGQzYzE0YzExZSIsImMiOiIxIiwidCI6IjdhYjAyYzk5ODJmODYyODRlNTg4ZGE5YmY3NGJlOGE4IiwiZCI6IjI5NjY3IiwiciI6IjEyOTY2OCJ9eyJ1IjoiaHR0cDovL3N0cjIuZm94LXR2LmZ1bjo5OTg2L1BlcnZpeWthbmFsL21wZWd

	static std::wregex re_url(LR"(^https?:\/\/([^\/]+)\/(.+)$)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url))
	{
		templated = true;
		domain = std::move(m[1].str());
		token = std::move(m[2].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_fox::get_templated_stream(StreamSubType subType, const TemplateParams& params) const
{
	std::wstring url = is_template() ? URI_TEMPLATE : get_uri();

	if (params.shift_back)
	{
		append_archive(url);
	}

	replace_vars(url, params);

	return url;
}

std::wstring uri_fox::get_playlist_url(const PlaylistTemplateParams& params) const
{
	return fmt::format(PLAYLIST_TEMPLATE, params.login, params.password);
}
