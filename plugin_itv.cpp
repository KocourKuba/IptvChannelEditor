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
#include "plugin_itv.h"

#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr auto API_COMMAND_AUTH = L"{{API_URL}}/data/{{PASSWORD}}";

void plugin_itv::parse_account_info(TemplateParams& params)
{
	if (account_info.empty())
	{
		CWaitCursor cur;
		std::stringstream data;
		if (download_url(replace_params_vars(params, API_COMMAND_AUTH), data))
		{
			JSON_ALL_TRY
			{
				const auto & parsed_json = nlohmann::json::parse(data.str());
				if (parsed_json.contains("user_info"))
				{
					const auto& js_data = parsed_json["user_info"];

					set_json_info("login", js_data, account_info);
					set_json_info("pay_system", js_data, account_info);
					set_json_info("cash", js_data, account_info);
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

				account_info.emplace(L"package_info", subscription);
			}
			JSON_ALL_CATCH;
		}
		else
		{
			LogProtocol(fmt::format(L"plugin_itv: Failed to get account info: {:s}", m_dl.GetLastErrorMessage()));
		}
	}
}
