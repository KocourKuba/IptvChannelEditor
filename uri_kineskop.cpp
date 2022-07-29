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
#include "uri_kineskop.h"
#include "IPTVChannelEditor.h"

#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto PLAYLIST_TEMPLATE = L"http://knkp.in/{:s}/{:s}/{:s}/1";
static constexpr auto URI_TEMPLATE_HLS = L"http://{DOMAIN}/{HOST}/{ID}/{TOKEN}.m3u8";
static constexpr auto REPL_HOST = L"{HOST}";

uri_kineskop::uri_kineskop()
{
	per_channel_token = true;
	provider_url = L"http://kineskop.club/";
	epg_params[0].epg_url = L"http://epg.esalecrm.net/kineskop/epg/{ID}.json";
	access_type = AccountAccessType::enLoginPass;

	for (int i = 0; i <= IDS_STRING_KINESKOP_P4 - IDS_STRING_KINESKOP_P1; i++)
	{
		auto str = id = load_string_resource(IDS_STRING_KINESKOP_P1 + i);
		ServersInfo info({ str, utils::wstring_tolower(id) });
		servers_list.emplace_back(info);
	}
}

void uri_kineskop::parse_uri(const std::wstring& url)
{
	// http://de.kineskop.tv/site2/119/1113391_541b6bc57cc71771_0_0_fs2.m3u8

	static std::wregex re_url(LR"(^https?:\/\/(.+)\/(.+)\/(.+)\/(.+)\.m3u8$)");
	std::wsmatch m;
	if (std::regex_match(url, m, re_url))
	{
		templated = true;
		domain = std::move(m[1].str());
		host = std::move(m[2].str());
		id = std::move(m[3].str());
		token = std::move(m[4].str());
		return;
	}

	uri_stream::parse_uri(url);
}

std::wstring uri_kineskop::get_templated_stream(TemplateParams& params) const
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

	utils::string_replace_inplace<wchar_t>(url, REPL_HOST, params.host);
	replace_vars(url, params);

	return url;
}

std::wstring uri_kineskop::get_playlist_url(TemplateParams& params)
{
	return fmt::format(PLAYLIST_TEMPLATE, params.login, params.password, servers_list[params.server].id);
}
