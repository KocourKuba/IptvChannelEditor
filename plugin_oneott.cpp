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
#include "plugin_oneott.h"

#include "UtilsLib\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void plugin_oneott::parse_account_info(Credentials& creds)
{
	static constexpr auto ACCOUNT_TEMPLATE = L"{:s}/PinApi/{:s}/{:s}";

	if (account_info.empty())
	{
		CWaitCursor cur;
		const auto& url = fmt::format(ACCOUNT_TEMPLATE, get_provider_api_url(), creds.get_login(), creds.get_password());
		std::stringstream data;
		if (download_url(url, data))
		{
			JSON_ALL_TRY
			{
				const auto & parsed_json = nlohmann::json::parse(data.str());
				if (parsed_json.contains("token"))
				{
					creds.s_token = parsed_json.value("token", "");
					account_info.emplace(L"token", creds.get_s_token());
				}
			}
			JSON_ALL_CATCH;
		}
	}
}
