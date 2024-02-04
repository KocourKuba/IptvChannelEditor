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

#pragma once

#include "base_plugin.h"
#include "BaseThread.h"
#include "vod_movie.h"
#include "UtilsLib\inet_utils.h"

class CPlaylistParseJsonThread : public CBaseThread
{
	DECLARE_DYNCREATE(CPlaylistParseJsonThread)

public:
	BOOL InitInstance() override;

protected:
	void ParseSharaclub();
	void ParseCbilling();
	void ParseEdem();
	void ParseGlanz();
	void ParseSharavoz();

	std::wstring xtream_parse_category(const nlohmann::json& val, std::shared_ptr<vod_category>& category, std::unique_ptr<vod_category_storage>& categories);
	nlohmann::json xtream_request(const std::wstring& action, std::wstring param = L"", std::wstring value = L"");

	utils::CUrlDownload m_dl;
};
