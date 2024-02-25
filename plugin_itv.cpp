/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2024): sharky72 (https://github.com/KocourKuba)

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
	type_name = "itv";
	class_name = "itv_config";
}

void plugin_itv::load_default()
{
	base_plugin::load_default();

	title = "ITV Live TV";
	name = "itv-live.tv";
	access_type = AccountAccessType::enPin;

	provider_url = "https://itv.live/";
	provider_api_url = "http://api.itv.live";

	PlaylistTemplateInfo info(IDS_STRING_EDEM_STANDARD);
	info.pl_template = "{PL_DOMAIN}/p/{PASSWORD}/hls.m3u8";
	info.pl_parse_regex = R"(^https?:\/\/.*\/p\/(?<password>.+)\/.+$)";
	info.parse_regex = R"(^(?<scheme>https?:\/\/)(?<domain>.+)\/.+\/video\.m3u8\?token=(?<token>.+)$)";
	info.tag_id_match = "tvg-id";
	info.square_icons = true;
	playlist_templates.emplace_back(info);

	balance_support = true;

	streams_config[0].cu_type = CatchupType::cu_flussonic;
	streams_config[0].uri_template = "{SCHEME}{DOMAIN}/{ID}/video.m3u8?token={TOKEN}";
	streams_config[0].uri_arc_template = "{SCHEME}{DOMAIN}/{ID}/archive-{START}-{DURATION}.m3u8?token={TOKEN}";

	streams_config[1].uri_template = "{SCHEME}{DOMAIN}/{ID}/mpegts?token={TOKEN}";
	streams_config[1].uri_arc_template = "{SCHEME}{DOMAIN}/{ID}/archive-{START}-{DURATION}.ts?token={TOKEN}";

	set_epg_preset(0, EpgPresets::enItvLive);
	epg_params[0].epg_domain = "";
	epg_params[0].epg_url = "{API_URL}/epg/{EPG_ID}";
}

std::map<std::wstring, std::wstring, std::less<>> plugin_itv::parse_access_info(TemplateParams& params)
{
	std::map<std::wstring, std::wstring, std::less<>> info;

	CWaitCursor cur;
	std::stringstream data;
	if (download_url(fmt::format(ACCOUNT_TEMPLATE, params.password), data))
	{
		JSON_ALL_TRY
		{
			const auto& parsed_json = nlohmann::json::parse(data.str());
			if (parsed_json.contains("user_info"))
			{
				const auto& js_data = parsed_json["user_info"];

				set_json_info("login", js_data, info);
				set_json_info("pay_system", js_data, info);
				set_json_info("cash", js_data, info);
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

			info.emplace(L"package_info", subscription);
		}
		JSON_ALL_CATCH;
	}


	return info;
}

void plugin_itv::fill_domains_list(TemplateParams* params /*= nullptr*/)
{
	if (!get_domains_list().empty())
		return;

	DynamicParamsInfo info;
	info.set_id(L"0");
	info.set_name(L"https://itv.ooo");

	std::vector<DynamicParamsInfo> domains;
	domains.emplace_back(info);

	set_domains_list(domains);
}
