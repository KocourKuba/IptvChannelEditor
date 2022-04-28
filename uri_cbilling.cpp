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
#include "uri_cbilling.h"
#include "PlayListEntry.h"

#include "UtilsLib\utils.h"
#include "UtilsLib\inet_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto ACCOUNT_TEMPLATE = L"http://protected-api.com/auth/info";
static constexpr auto ACCOUNT_HEADER_TEMPLATE = L"accept: */*\r\nx-public-key: {:s}";
static constexpr auto PLAYLIST_TEMPLATE = L"http://247on.cc/playlist/{:s}_otp_dev{:d}.m3u8";
static constexpr auto URI_TEMPLATE_HLS = L"http://{SUBDOMAIN}/s/{TOKEN}/{ID}.m3u8";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{SUBDOMAIN}/{ID}/mpegts?token={TOKEN}";

uri_cbilling::uri_cbilling()
{
	auto& params = epg_params[0];
	params.epg_url = L"http://protected-api.com/epg/{ID}/?date=";
	params.epg_root = "";
	provider_url = L"https://cbilling.live/";
}

void uri_cbilling::parse_uri(const std::wstring& url)
{
	// http://s01.iptvx.tv:8090/s/82s4fb5785dcf28dgd6ga681a94ba78f/pervyj.m3u8
	static std::wregex re_url_hls(LR"(^https?:\/\/(.+)\/s\/(.+)\/(.+)\.m3u8$)");

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

std::wstring uri_cbilling::get_templated_stream(StreamSubType subType, const TemplateParams& params) const
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

		switch (subType)
		{
			case StreamSubType::enHLS:
				url = URI_TEMPLATE_HLS;
				if (params.shift_back)
				{
					append_archive(url);
				}
				break;
			case StreamSubType::enMPEGTS:
				url = URI_TEMPLATE_MPEG;
				new_params.domain = std::move(no_port);
				if (params.shift_back)
				{
					utils::string_replace_inplace(url, L"mpegts", L"archive-{START}-10800.ts");
				}
				break;
		}
	}
	else
	{
		url = get_uri();
	}

	replace_vars(url, params);
	return url;
}

std::wstring uri_cbilling::get_access_info_header() const
{
	return ACCOUNT_HEADER_TEMPLATE;
}

bool uri_cbilling::parse_access_info(const PlaylistTemplateParams& params, std::list<AccountInfo>& info_list) const
{
	auto& header = fmt::format(ACCOUNT_HEADER_TEMPLATE, params.password);
	std::vector<BYTE> data;
	if (!utils::DownloadFile(ACCOUNT_TEMPLATE, data, false, &header) || data.empty())
	{
		return false;
	}

	JSON_ALL_TRY
	{
		nlohmann::json parsed_json = nlohmann::json::parse(data);
		nlohmann::json js_data = parsed_json["data"];

		put_account_info("package", js_data, info_list);
		put_account_info("end_date", js_data, info_list);
		put_account_info("devices_num", js_data, info_list);
		put_account_info("server", js_data, info_list);
		put_account_info("vod", js_data, info_list);

		return true;
	}
	JSON_ALL_CATCH;

	return false;
}

std::wstring uri_cbilling::get_playlist_url(const PlaylistTemplateParams& params) const
{
	return fmt::format(PLAYLIST_TEMPLATE, params.password, params.device);
}
