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
#include "plugin_itv.h"
#include "IPTVChannelEditor.h"

#include "UtilsLib\utils.h"
#include "UtilsLib\inet_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto ACCOUNT_TEMPLATE = L"http://api.itv.live/data/{:s}";

plugin_itv::plugin_itv()
{
	short_name = "itv";
}

void plugin_itv::load_default()
{
	title = "ITV Live TV";
	name = "itv-live.tv";
	access_type = AccountAccessType::enPin;

	provider_url = "https://itv.live/";

	PlaylistTemplateInfo info;
	info.set_name(load_string_resource(IDS_STRING_EDEM_STANDARD));
	info.pl_template = "http://itv.ooo/p/{PASSWORD}/hls.m3u8";
	playlist_templates.emplace_back(info);

	uri_parse_pattern = R"(^https?:\/\/(?<domain>.+)\/(?<id>.+)\/[^\?]+\?token=(?<token>.+)$)";

	square_icons = true;
	per_channel_token = true;

	streams_config[0].cu_type = CatchupType::cu_flussonic;
	streams_config[0].cu_subst = "archive";
	streams_config[0].uri_template = "http://{DOMAIN}/{ID}/video.m3u8?token={TOKEN}";
	streams_config[0].uri_arc_template = "http://{DOMAIN}/{ID}/{CU_SUBST}-{START}-{DURATION}.m3u8?token={TOKEN}";

	streams_config[1].uri_template = "http://{DOMAIN}/{ID}/mpegts?token={TOKEN}";
	streams_config[1].uri_arc_template = "http://{DOMAIN}/{ID}/{CU_SUBST}-{START}-{DURATION}.ts?token={TOKEN}";

	auto& params = epg_params[0];
	params.epg_url = "http://api.itv.live/epg/{EPG_ID}";
	params.epg_root = "res";
	params.epg_name = "title";
	params.epg_desc = "desc";
	params.epg_start = "startTime";
	params.epg_end = "stopTime";
}

bool plugin_itv::parse_access_info(TemplateParams& params, std::list<AccountInfo>& info_list)
{
	std::stringstream data;
	if (!utils::CurlDownload(fmt::format(ACCOUNT_TEMPLATE, params.password), data))
	{
		return false;
	}

	JSON_ALL_TRY
	{
		const auto& parsed_json = nlohmann::json::parse(data.str());
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
