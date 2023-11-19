/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2023): sharky72 (https://github.com/KocourKuba)

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
#include "plugin_cbilling.h"
#include "PlayListEntry.h"
#include "IPTVChannelEditor.h"

#include "UtilsLib\inet_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

plugin_cbilling::plugin_cbilling()
{
	type_name = "cbilling";
	class_name = "cbilling_config";
}

void plugin_cbilling::load_default()
{
	base_plugin::load_default();

	title = "Cbilling TV";
	name = "cbillingtv";
	access_type = AccountAccessType::enPin;

	provider_url = "https://cbilling.eu/";
	provider_api_url = "http://protected-api.com";
	balance_support = true;

	PlaylistTemplateInfo vod_info;
	vod_info.set_name(load_string_resource(0, IDS_STRING_EDEM_STANDARD));
	vod_info.pl_template = "{API_URL}";
	vod_templates.emplace_back(vod_info);
	vod_support = true;

	PlaylistTemplateInfo info(IDS_STRING_EDEM_STANDARD);
	info.pl_domain = "http://248on.com";
	info.pl_template = "{PL_DOMAIN}/playlist/{PASSWORD}_otp_dev{DEVICE_ID}.m3u8";
	info.pl_parse_regex = R"(^https?:\/\/.*\/playlist\/(?<token>.+)_otp_dev.*$)";
	info.parse_regex = R"(^(?<scheme>https?:\/\/)(?<domain>.+):(?<port>.+)\/s\/(?<token>.+)\/.+\.m3u8$)";
	info.tag_id_match = "tvg-id";
	playlist_templates.emplace_back(info);

	streams_config[0].uri_template = "{SCHEME}{DOMAIN}:{PORT}/s/{TOKEN}/{ID}.m3u8";
	streams_config[0].uri_arc_template = "{LIVE_URL}?utc={START}&lutc={NOW}";

	streams_config[1].uri_template = "{SCHEME}{DOMAIN}/{ID}/mpegts?token={TOKEN}";
	streams_config[1].uri_arc_template = "{SCHEME}{DOMAIN}/{ID}/archive-{START}-{DURATION}.ts?token={TOKEN}";

	set_epg_preset(0, EpgPresets::enCbilling);
	epg_params[0].epg_url = "{API_URL}/epg/{EPG_ID}/?date=";

	epg_params[1].epg_url = "{EPG_DOMAIN}/cbilling%2Fepg%2F{EPG_ID}.json";

	static_devices = true;
	fill_devices_list();
}

void plugin_cbilling::fill_devices_list(TemplateParams* params /*= nullptr*/)
{
	if (!get_devices_list().empty())
		return;

	std::vector<DynamicParamsInfo> devices;
	for (int i = 0; i <= IDS_STRING_CBILLING_TV_P3 - IDS_STRING_CBILLING_TV_P1; i++)
	{
		DynamicParamsInfo info;
		info.set_id(fmt::format(L"{:d}", i + 1));
		info.set_name(load_string_resource(1049, IDS_STRING_CBILLING_TV_P1 + i));
		devices.emplace_back(info);
	}

	set_devices_list(devices);
}

std::map<std::wstring, std::wstring> plugin_cbilling::parse_access_info(TemplateParams& params)
{
	/*
	{
		"data": {
			"public_token": "2f5787bd53fcaeeae27ba3ed3669babc",
			"private_token": "5acf87d0206da05b73f8923a703gf666",
			"end_time": 1706129968,
			"end_date": "2024-01-24 23:59:28",
			"devices_num": 1,
			"server": "s01.wsbof.com",
			"vod": true,
			"ssl": false,
			"disable_adult": false
	}
}	*/

	static constexpr auto ACCOUNT_HEADER_TEMPLATE = "x-public-key: {:s}";
	static constexpr auto ACCOUNT_TEMPLATE = L"/auth/info";

	std::map<std::wstring, std::wstring> info;

	CWaitCursor cur;
	std::vector<std::string> headers;
	headers.emplace_back("accept: */*");
	headers.emplace_back(fmt::format(ACCOUNT_HEADER_TEMPLATE, utils::utf16_to_utf8(params.password)));
	std::stringstream data;
	if (download_url(get_provider_api_url() + ACCOUNT_TEMPLATE, data, 0, &headers))
	{
		JSON_ALL_TRY
		{
			const auto& parsed_json = nlohmann::json::parse(data.str());
			if (parsed_json.contains("data"))
			{
				const auto& js_data = parsed_json["data"];

				set_json_info("package", js_data, info);
				set_json_info("end_date", js_data, info);
				set_json_info("devices_num", js_data, info);
				set_json_info("server", js_data, info);
				set_json_info("vod", js_data, info);
				set_json_info("ssl", js_data, info);
				set_json_info("public_token", js_data, info);
				set_json_info("private_token", js_data, info);
			}
		}
		JSON_ALL_CATCH;
	}

	return info;
}
