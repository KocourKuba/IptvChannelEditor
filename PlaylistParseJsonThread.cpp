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
#include "PlaylistParseJsonThread.h"
#include "PlayListEntry.h"
#include "vod_category.h"

#include "UtilsLib\utils.h"
#include "UtilsLib\vectormap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CPlaylistParseJsonThread, CWinThread)

BOOL CPlaylistParseJsonThread::InitInstance()
{
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	auto categories = std::make_unique<utils::vectormap<std::wstring, std::shared_ptr<vod_category>>>();
	if (m_config.m_data)
	{
		nlohmann::json parsed_json;
		JSON_ALL_TRY
		{
			parsed_json = nlohmann::json::parse(m_config.m_data->begin(), m_config.m_data->end());
		}
		JSON_ALL_CATCH;
		m_config.NotifyParent(WM_INIT_PROGRESS, (int)parsed_json.size(), TRUE);

		int cnt = 0;
		for (const auto& item : parsed_json.items())
		{
			std::shared_ptr<vod_category> category;
			std::wstring category_name;
			auto movie = std::make_shared<vod_movie>();
			JSON_ALL_TRY
			{
				nlohmann::json val = item.value();
				if (val.empty()) continue;

				category_name = utils::utf8_to_utf16(val.value("category", ""));

				if (!categories->tryGet(category_name, category))
				{
					category = std::make_shared<vod_category>(category_name);
					categories->set(category_name, category);
				}

				movie->title = utils::utf8_to_utf16(val.value("name", ""));
				movie->id = utils::utf8_to_utf16(val.value("id", ""));
				movie->url = utils::utf8_to_utf16(val.value("video", ""));

				const auto& info = val["info"];
				if (!info.empty())
				{
					movie->poster_url.set_uri(utils::utf8_to_utf16(info.value("poster", "")));
					movie->poster_url.set_schema(L"http://");
					movie->description = utils::utf8_to_utf16(info.value("plot", ""));
					movie->rating = utils::utf8_to_utf16(info.value("rating", ""));
					movie->year = std::to_wstring(info.value("year", 0));
					movie->director = utils::utf8_to_utf16(info.value("director", ""));
					movie->casting = utils::utf8_to_utf16(info.value("cast", ""));
					movie->age = utils::utf8_to_utf16(info.value("adult", ""));
					movie->length = info.value("duration_secs", 0);

					std::string genre;
					for (const auto& genre_item : info["genre"].items())
					{
						movie->genres.emplace(utils::utf8_to_utf16(genre_item.value().get<std::string>()));
					}

					std::string country;
					for (const auto& country_item : info["country"].items())
					{
						if (!country.empty())
							country += ", ";
						country += country_item.value().get<std::string>();
					}
					movie->country = utils::utf8_to_utf16(country);
				}

				category->movies.set(movie->id, movie);
			}
			JSON_ALL_CATCH;
			cnt++;
			if (cnt % 50 == 0)
			{
				m_config.NotifyParent(WM_UPDATE_PROGRESS, cnt, cnt);
				if (::WaitForSingleObject(m_config.m_hStop, 0) == WAIT_OBJECT_0)
				{
					categories.reset();
					break;
				}
			}
		}
	}

	m_config.NotifyParent(WM_END_LOAD_JSON_PLAYLIST, (WPARAM)categories.release());

	CoUninitialize();

	return FALSE;
}
