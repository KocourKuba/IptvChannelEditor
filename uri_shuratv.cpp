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
#include "uri_shuratv.h"
#include "IPTVChannelEditor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto PLAYLIST_TEMPLATE = L"http://pl.tvshka.net/?uid={:s}&srv={:s}&type=halva";
static constexpr auto URI_TEMPLATE_HLS = L"http://{DOMAIN}/~{TOKEN}/{ID}/hls/pl.m3u8";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{DOMAIN}/~{TOKEN}/{ID}/";

uri_shuratv::uri_shuratv()
{
	provider_url = L"http://shura.tv/b/";
	access_type = AccountAccessType::enPin;

	auto& params1 = epg_params[0];
	params1.epg_url = L"http://epg.propg.net/{ID}/epg2/{DATE}";
	params1.epg_root = "";
	params1.epg_name = "epg";
	params1.epg_desc = "desc";
	params1.epg_start = "start";
	params1.epg_end = "stop";

	for (int i = 0; i <= IDS_STRING_SHURA_TV_P2 - IDS_STRING_SHURA_TV_P1; i++)
	{
		ServersInfo info({ load_string_resource(IDS_STRING_SHURA_TV_P1 + i), fmt::format(L"{:d}", i + 1) });
		servers_list.emplace_back(info);
	}
}

void uri_shuratv::parse_uri(const std::wstring& url)
{
	// http://bl2.provds.pw/~shsv45hh617fU/119/hls/pl.m3u8
	static std::wregex re_url(LR"(^https?:\/\/(.+)\/~(.+)\/(.+)\/hls\/.+\.m3u8$)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url))
	{
		templated = true;
		domain = std::move(m[1].str());
		token = std::move(m[2].str());
		id = std::move(m[3].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_shuratv::get_templated_stream(TemplateParams& params) const
{
	auto& url = get_uri();

	if (is_template())
	{
		switch (params.streamSubtype)
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

std::wstring uri_shuratv::get_playlist_url(TemplateParams& params)
{
	int server = params.server;
	if (params.server >= (int)servers_list.size())
		server = (int)servers_list.size() - 1;

	return fmt::format(PLAYLIST_TEMPLATE, params.password, servers_list[server].id);
}

std::wstring& uri_shuratv::append_archive(std::wstring& url) const
{
	if (url.rfind('?') != std::wstring::npos)
		url += '&';
	else
		url += '?';

	url += L"archive={START}&lutc={NOW}";

	return url;
}
