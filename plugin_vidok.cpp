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
#include "plugin_vidok.h"
#include "IPTVChannelEditor.h"

#include "UtilsLib\md5.h"
#include "UtilsLib\utils.h"
#include "UtilsLib\inet_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static constexpr auto API_COMMAND_GET_URL = L"http://sapi.ott.st/v2.4/json/{:s}?token={:s}";
static constexpr auto API_COMMAND_SET_URL = L"http://sapi.ott.st/v2.4/json/{:s}?token={:s}&{:s}={:s}";

plugin_vidok::plugin_vidok()
{
	short_name = "vidok";
	requested_token = true;
}

void plugin_vidok::load_default()
{
	title = "Vidok TV";
	name = "vidok.tv";
	access_type = AccountAccessType::enLoginPass;

	provider_url = "https://vidok.tv/";

	PlaylistTemplateInfo info;
	info.set_name(load_string_resource(IDS_STRING_EDEM_STANDARD));
	info.pl_template = "http://vidok.tv/p/{TOKEN}";
	playlist_templates.emplace_back(info);

	uri_parse_pattern = R"(^https?:\/\/(?<domain>.+)\/p\/(?<token>.+)\/(?<id>.+)$)";

	streams_config[0].cu_type = CatchupType::cu_append;
	streams_config[0].uri_template = "http://{DOMAIN}/p/{TOKEN}/{ID}";
	streams_config[0].uri_arc_template = "{CU_SUBST}={START}";

	auto& params = epg_params[0];
	params.epg_url = "http://sapi.ott.st/v2.4/json/epg2?cid={EPG_ID}&token={TOKEN}";
	params.epg_root = "epg";
	params.epg_name = "title";
	params.epg_desc = "description";
	params.epg_start = "start";
	params.epg_end = "end";
}

std::wstring plugin_vidok::get_api_token(const Credentials& creds) const
{
	std::string login_a = utils::string_tolower(utils::utf16_to_utf8(creds.get_login()));
	std::string password_a = utils::utf16_to_utf8(creds.get_password());
	return utils::utf8_to_utf16(utils::md5_hash_hex(login_a + utils::md5_hash_hex(password_a)));
}

bool plugin_vidok::parse_access_info(TemplateParams& params, std::list<AccountInfo>& info_list)
{
	Credentials creds;
	creds.set_login(params.login);
	creds.set_password(params.password);

	std::stringstream data;
	if (!utils::CurlDownload(fmt::format(API_COMMAND_GET_URL, L"account", get_api_token(creds)), data))
	{
		return false;
	}

	JSON_ALL_TRY
	{
		const auto& parsed_json = nlohmann::json::parse(data.str());
		if (parsed_json.contains("account"))
		{
			const auto& js_account = parsed_json["account"];

			put_account_info("login", js_account, info_list);
			put_account_info("balance", js_account, info_list);

			if (js_account.contains("packages"))
			{
				for (auto& item : js_account["packages"].items())
				{
					const auto& val = item.value();
					COleDateTime dt(utils::char_to_int64(val.value("expire", "")));
					const auto& value = fmt::format("expired {:d}.{:d}.{:d}", dt.GetDay(), dt.GetMonth(), dt.GetYear());
					AccountInfo info{ utils::utf8_to_utf16(val.value("name", "")), utils::utf8_to_utf16(value) };
					info_list.emplace_back(info);
				}
			}

			return true;
		}
	}
	JSON_ALL_CATCH;

	return false;
}

void plugin_vidok::fill_servers_list(TemplateParams& params)
{
	if (params.login.empty() || params.password.empty() || !get_servers_list().empty())
		return;

	std::vector<DynamicParamsInfo> servers;

	Credentials creds;
	creds.set_login(params.login);
	creds.set_password(params.password);

	const auto& url = fmt::format(API_COMMAND_GET_URL, L"settings", get_api_token(creds));
	std::stringstream data;
	if (utils::CurlDownload(url, data))
	{
		JSON_ALL_TRY;
		{
			const auto& parsed_json = nlohmann::json::parse(data.str());
			if (parsed_json.contains("settings"))
			{
				const auto& settings = parsed_json["settings"];

				const auto& current = utils::get_json_wstring("id", settings["current"]["server"]);

				for (const auto& item : settings["lists"]["servers"].items())
				{
					const auto& server = item.value();
					DynamicParamsInfo info{ utils::get_json_string("id", server), utils::get_json_string("name", server) };
					if (info.get_id() == current)
						params.server_idx = servers.size();

					servers.emplace_back(info);
				}
			}
		}
		JSON_ALL_CATCH;
	}

	set_servers_list(servers);
}

bool plugin_vidok::set_server(TemplateParams& params)
{
	if (servers_list.empty())
	{
		fill_servers_list(params);
	}

	if (!servers_list.empty())
	{
		Credentials creds;
		creds.set_login(params.login);
		creds.set_password(params.password);

		const auto& url = fmt::format(API_COMMAND_SET_URL,
									  L"settings_set",
									  get_api_token(creds),
									  L"server",
									  servers_list[params.server_idx].get_id());

		std::stringstream data;
		if (utils::CurlDownload(url, data))
		{
			JSON_ALL_TRY;
			{
				const auto& parsed_json = nlohmann::json::parse(data.str());
				for (const auto& item : parsed_json["settings"].items())
				{
					const auto& server = item.value();
					return (utils::get_json_wstring("value", server) == servers_list[params.server_idx].get_id()); //-V612
				}
			}
			JSON_ALL_CATCH;
		}
	}

	return false;
}
