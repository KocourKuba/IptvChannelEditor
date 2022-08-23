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
#include "uri_sharatv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto PLAYLIST_TEMPLATE = L"http://tvfor.pro/g/{:s}:{:s}/1/playlist.m3u";
static constexpr auto URI_TEMPLATE = L"http://{DOMAIN}/{ID}/{TOKEN}";

uri_sharatv::uri_sharatv()
{
	streams = { {StreamSubType::enHLS, L"HLS"} };
	provider_url = L"https://shara-tv.org/";
	access_type = AccountAccessType::enLoginPass;

	epg_params[0].epg_url = L"http://epg.drm-play.ml/shara-tv/epg/{ID}.json";

	secondary_epg = true;
	auto& params2 = epg_params[1];
	params2.epg_use_mapper = true;
	params2.epg_url = L"http://technic.cf/epg-shara-tv/epg_day?id={ID}&day={DATE}";
	params2.epg_mapper_url = L"http://technic.cf/epg-shara-tv/channels";
	params2.epg_date_format = L"{:04d}.{:02d}.{:02d}";
	params2.epg_root = "data";
	params2.epg_name = "title";
	params2.epg_desc = "description";
	params2.epg_start = "begin";
	params2.epg_end = "end";
}

void uri_sharatv::parse_uri(const std::wstring& url)
{
	// http://messi.tvfor.pro/Perviykanal/a8dv285y29itx4ye4oj3cez5

	static std::wregex re_url(LR"(^https?:\/\/(.+)\/(.+)\/(.+)$)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url))
	{
		templated = true;
		domain = std::move(m[1].str());
		id = std::move(m[2].str());
		token = std::move(m[3].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_sharatv::get_templated_stream(TemplateParams& params) const
{
	std::wstring url = is_template() ? URI_TEMPLATE : get_uri();

	if (params.shift_back)
	{
		append_archive(url);
	}

	replace_vars(url, params);

	return url;
}

std::wstring uri_sharatv::get_playlist_url(TemplateParams& params)
{
	return fmt::format(PLAYLIST_TEMPLATE, params.login, params.password);
}
