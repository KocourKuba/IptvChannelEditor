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
#include "uri_itv.h"

#include "UtilsLib\utils.h"
#include "UtilsLib\inet_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto ACCOUNT_TEMPLATE = L"http://api.itv.live/data/{:s}";
static constexpr auto PLAYLIST_TEMPLATE = L"http://itv.ooo/p/{:s}/hls.m3u8";
static constexpr auto URI_TEMPLATE_HLS = L"http://{DOMAIN}/{ID}/video.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_MPEG = L"http://{DOMAIN}/{ID}/mpegts?token={TOKEN}";
static constexpr auto URI_TEMPLATE_ARCH_HLS = L"http://{DOMAIN}/{ID}/archive-{START}-10800.m3u8?token={TOKEN}";
static constexpr auto URI_TEMPLATE_ARCH_MPEG = L"http://{DOMAIN}/{ID}/archive-{START}-10800.ts?token={TOKEN}";

uri_itv::uri_itv()
{
	auto& params = epg_params[0];
	params.epg_url = L"http://api.itv.live/epg/{ID}";
	params.epg_root = "res";
	params.epg_name = "title";
	params.epg_desc = "desc";
	params.epg_start = "startTime";
	params.epg_end = "stopTime";
	provider_url = L"https://itv.live/";
}

void uri_itv::parse_uri(const std::wstring& url)
{
	// http://cloud15.05cdn.wf/ch378/video.m3u8?token=5bdbc7125f6ed805a5fd238b9f885d1a3c67a6594
	// http://cloud15.05cdn.wf/ch378/mpegts?token=5bdbc7125f6ed805a5fd238b9f885d1a3c67a6594
	static std::wregex re_url_hls(LR"(^https?:\/\/(.+)\/(.+)\/[^\?]+\?token=(.+)$)");
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

std::wstring uri_itv::get_templated_stream(TemplateParams& params) const
{
	std::wstring url;

	if (is_template())
	{
		switch (params.streamSubtype)
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

std::wstring uri_itv::get_playlist_url(TemplateParams& params)
{
	return fmt::format(PLAYLIST_TEMPLATE, params.password);
}

bool uri_itv::parse_access_info(TemplateParams& params, std::list<AccountInfo>& info_list)
{
	std::vector<BYTE> data;
	if (!utils::DownloadFile(fmt::format(ACCOUNT_TEMPLATE, params.password), data) || data.empty())
	{
		return false;
	}

	JSON_ALL_TRY
	{
		nlohmann::json parsed_json = nlohmann::json::parse(data);
		if (parsed_json.contains("user_info"))
		{
			const auto& js_data = parsed_json["user_info"];

			put_account_info("login", js_data, info_list);
			put_account_info("pay_system", js_data, info_list);
			put_account_info("cash", js_data, info_list);
		}

		std::wstring subscription;
		if (!parsed_json.contains("package_info"))
		{
			subscription = L"No packages";
		}
		else
		{
			const auto& pkg_data = parsed_json["package_info"];
			for (const auto& item : pkg_data)
			{
				if (!subscription.empty())
					subscription += L", ";

				subscription += fmt::format(L"{:s}", utils::utf8_to_utf16(item.value("name", "")));
			}
		}

		AccountInfo info{ L"package_info", subscription };
		info_list.emplace_back(info);

		return true;
	}
	JSON_ALL_CATCH;

	return false;
}
