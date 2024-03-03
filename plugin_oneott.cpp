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
#include "IPTVChannelEditor.h"

#include "UtilsLib\utils.h"
#include "UtilsLib\inet_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

std::map<std::wstring, std::wstring, std::less<>> plugin_oneott::parse_access_info(TemplateParams& params)
{
	static constexpr auto ACCOUNT_TEMPLATE = L"{:s}/PinApi/{:s}/{:s}";

	std::map<std::wstring, std::wstring, std::less<>> info;

	CWaitCursor cur;
	std::stringstream data;
	const auto& url = fmt::format(ACCOUNT_TEMPLATE, get_provider_api_url(), params.login, params.password);
	if (download_url(url, data))
	{
		JSON_ALL_TRY
		{
			const auto& parsed_json = nlohmann::json::parse(data.str());
			if (parsed_json.contains("token"))
			{
				const auto& token = utils::utf8_to_utf16(parsed_json.value("token", ""));
				info.emplace(L"token", token);

				TemplateParams param;
				param.s_token = token;

				info.emplace(L"url", get_playlist_url(param));
			}
		}
		JSON_ALL_CATCH;
	}


	return info;
}
