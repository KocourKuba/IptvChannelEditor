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
	short_name = "cbilling";
}

void plugin_cbilling::load_default()
{
	title = "Cbilling TV";
	name = "cbillingtv";
	access_type = AccountAccessType::enPin;

	provider_url = "https://cbilling.eu/";

	PlaylistTemplateInfo vod_info;
	vod_info.set_name(load_string_resource(0, IDS_STRING_EDEM_STANDARD));
	vod_info.pl_template = "http://protected-api.com";
	vod_templates.emplace_back(vod_info);
	vod_support = true;

	PlaylistTemplateInfo info(IDS_STRING_EDEM_STANDARD);
	info.pl_template = "http://247on.cc/playlist/{PASSWORD}_otp_dev{DEVICE_ID}.m3u8";
	info.parse_regex = R"(^https?:\/\/(?<domain>.+):(?<port>.+)\/s\/(?<token>.+)\/(?<id>.+)\.m3u8$)";
	playlist_templates.emplace_back(info);

	streams_config[0].uri_template = "http://{DOMAIN}:{PORT}/s/{TOKEN}/{ID}.m3u8";
	streams_config[0].uri_arc_template = "{LIVE_URL}?utc={START}&lutc={NOW}";

	streams_config[1].uri_template = "http://{DOMAIN}/{ID}/mpegts?token={TOKEN}";
	streams_config[1].uri_arc_template = "http://{DOMAIN}/{ID}/archive-{START}-{DURATION}.ts?token={TOKEN}";

	auto& params1 = epg_params[0];
	params1.epg_url = "http://protected-api.com/epg/{EPG_ID}/?date=";
	params1.epg_root = "";

	static_devices = true;
	fill_devices_list(TemplateParams());
}

void plugin_cbilling::fill_devices_list(TemplateParams& /*params*/)
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

bool plugin_cbilling::parse_access_info(TemplateParams& params, std::list<AccountInfo>& info_list)
{
	static constexpr auto ACCOUNT_HEADER_TEMPLATE = "x-public-key: {:s}";
	static constexpr auto ACCOUNT_TEMPLATE = L"http://protected-api.com/auth/info";

	CWaitCursor cur;
	std::vector<std::string> headers;
	headers.emplace_back("accept: */*");
	headers.emplace_back(fmt::format(ACCOUNT_HEADER_TEMPLATE, utils::utf16_to_utf8(params.password)));
	std::stringstream data;
	if (!utils::DownloadFile(ACCOUNT_TEMPLATE, data, 0, &headers))
	{
		return false;
	}

	JSON_ALL_TRY
	{
		const auto& parsed_json = nlohmann::json::parse(data.str());
		if (parsed_json.contains("data"))
		{
			const auto& js_data = parsed_json["data"];

			put_account_info("package", js_data, info_list);
			put_account_info("end_date", js_data, info_list);
			put_account_info("devices_num", js_data, info_list);
			put_account_info("server", js_data, info_list);
			put_account_info("vod", js_data, info_list);
			put_account_info("ssl", js_data, info_list);

			return true;
		}
	}
	JSON_ALL_CATCH;

	return false;
}
