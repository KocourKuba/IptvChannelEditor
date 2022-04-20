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

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto PLAYLIST_TEMPLATE = L"http://cdntv.online/high/{:s}/playlist.m3u8";
static constexpr auto URI_TEMPLATE_HLS = L"http://{SUBDOMAIN}/high/{TOKEN}/{ID}.m3u8";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{SUBDOMAIN}/high/{TOKEN}/{ID}.mpeg";

uri_viplime::uri_viplime()
{
	auto& params = epg_params[0];
//	params.epg_url = L"http://epg.esalecrm.net/viplime/epg/{ID}.json";
//	params.epg_use_id_hash = true;
	params.epg_url = L"http://epg.ott-play.com/viplime/epg/{ID}.json";
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

std::wstring uri_viplime::get_templated_stream(StreamSubType subType, const TemplateParams& params) const
{
	std::wstring url;

	if (!is_template())
	{
		url = get_uri();
	}
	else
	{
		switch (subType)
		{
			case StreamSubType::enHLS:
				url = URI_TEMPLATE_HLS;
				break;
			case StreamSubType::enMPEGTS:
				url = URI_TEMPLATE_MPEG;
				break;
			default:
				break;
		}
	}

	if (params.shift_back)
	{
		append_archive(url);
	}

	replace_vars(url, params);

	return url;
}

std::wstring uri_viplime::get_playlist_url(const PlaylistTemplateParams& params) const
{
	return fmt::format(PLAYLIST_TEMPLATE, params.password);
}
