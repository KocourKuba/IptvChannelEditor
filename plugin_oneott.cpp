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
#include "plugin_oneott.h"
#include "IPTVChannelEditor.h"

#include "UtilsLib\utils.h"
#include "UtilsLib\inet_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

plugin_oneott::plugin_oneott()
{
	type_name = "oneott";
	class_name = "oneott_config";
}

void plugin_oneott::load_default()
{
	base_plugin::load_default();

	title = "1OTT TV";
	name = "oneott.tv";
	access_type = AccountAccessType::enLoginPass;

	provider_url = "http://1ott.net/";

	PlaylistTemplateInfo info(IDS_STRING_EDEM_STANDARD);
	info.pl_domain = "http://list.1ott.net";
	info.pl_template = "{PL_DOMAIN}/api/{TOKEN}/high/ottplay.m3u8";
	info.parse_regex = R"(^https?:\/\/(?<domain>.+)\/~(?<token>.+)\/(?<id>.+)\/hls\/.+\.m3u8$)";
	info.epg_id_from_id = true;
	playlist_templates.emplace_back(info);

	streams_config[0].uri_template = "http://{DOMAIN}/~{TOKEN}/{ID}/hls/pl.m3u8";
	streams_config[0].uri_arc_template = "{LIVE_URL}?utc={START}&lutc={NOW}";

	streams_config[1].cu_type = CatchupType::cu_shift;
	streams_config[1].uri_template = "http://{DOMAIN}/~{TOKEN}/{ID}";
	streams_config[1].uri_arc_template = "{LIVE_URL}?utc={START}&lutc={NOW}";

	epg_params[0].epg_domain = "http://epg.propg.net";
	epg_params[0].epg_url = "{EPG_DOMAIN}/{EPG_ID}/epg2/{DATE}";
	epg_params[0].epg_date_format = "{YEAR}-{MONTH}-{DAY}";
	epg_params[0].epg_root = "";
	epg_params[0].epg_name = "epg";
	epg_params[0].epg_desc = "desc";
	epg_params[0].epg_start = "start";
	epg_params[0].epg_end = "stop";

	epg_params[1].epg_domain = "http://epg.drm-play.com";
	epg_params[1].epg_url = "{EPG_DOMAIN}/1ott%2Fepg%2F{EPG_ID}.json";
}

bool plugin_oneott::parse_access_info(TemplateParams& params, std::list<AccountInfo>& info_list)
{
	static constexpr auto ACCOUNT_TEMPLATE = L"http://list.1ott.net/PinApi/{:s}/{:s}";

	CWaitCursor cur;
	std::stringstream data;
	if (!download_url(fmt::format(ACCOUNT_TEMPLATE, params.login, params.password), data))
	{
		return false;
	}

	JSON_ALL_TRY
	{
		const auto& parsed_json = nlohmann::json::parse(data.str());
		if (parsed_json.contains("token"))
		{
			const auto& token = utils::utf8_to_utf16(parsed_json.value("token", ""));
			AccountInfo info_token{ L"token", token };
			info_list.emplace_back(info_token);

			TemplateParams param;
			param.token = token;

			AccountInfo info_url{ L"url", get_playlist_url(param) };
			info_list.emplace_back(info_url);
		}

		return true;
	}
	JSON_ALL_CATCH;

	return false;
}
