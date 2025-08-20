/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2025): sharky72 (https://github.com/KocourKuba)

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
#include "plugin_shuriktv.h"

#include "UtilsLib\inet_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr auto API_COMMAND_AUTH = L"{API_URL}/customers/expired_packet/{PASSWORD}/";

void plugin_shuriktv::parse_account_info(TemplateParams& params)
{
	if (account_info.empty())
	{
		utils::http_request req{ replace_params_vars(params, API_COMMAND_AUTH) };
		if (utils::AsyncDownloadFile(req).get())
		{
			JSON_ALL_TRY
			{
				const auto & parsed_json = nlohmann::json::parse(req.body.str());
				if (!parsed_json.is_array())
				{
					account_info.emplace(L"packet", L"No packages");
				}
				else
				{
					const auto& data = parsed_json[0];
					if (data.contains("packet")) {
						account_info.emplace(L"packet", utils::get_json_wstring("packet", parsed_json[0]));
					}
					if (data.contains("expired"))
					{
						const auto str_time = utils::get_json_string("expired", parsed_json[0]);
						COleDateTime dt((time_t)std::stoi(str_time.substr(0, str_time.length() - 3)));
						account_info.emplace(L"expired", std::format(L"{:d}.{:d}.{:d}", dt.GetDay(), dt.GetMonth(), dt.GetYear()));
					}
				}
			}
			JSON_ALL_CATCH;
		}
		else
		{
			LogProtocol(std::format(L"plugin_shuriktv: Failed to get account info: {:s}", req.error_message));
		}
	}
}
