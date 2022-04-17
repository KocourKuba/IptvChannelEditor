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
#include "epg_technic.h"
#include "UtilsLib/xxhash.hpp"

static constexpr auto EPG_MAP_URL = L"http://technic.cf/epg-{:s}/channels";
static constexpr auto EPG1_TEMPLATE_JSON = L"http://technic.cf/epg-{:s}/epg_day?id={:s}&day={:04d}.{:02d}.{:02d}";
static constexpr auto EPG2_TEMPLATE_JSON = L"http://ott-epg.prog4food.eu.org/{:s}/epg/{:d}.json";

epg_technic::epg_technic(std::array<std::wstring, 2> sources) : m_sources(sources)
{
//	epg2 = true;
	m_use_mapper[0] = true;
	epg_params[0]["epg_root"] = "data";
	epg_params[0]["epg_name"] = "title";
	epg_params[0]["epg_desc"] = "description";
	epg_params[0]["epg_start"] = "begin";
	epg_params[0]["epg_end"] = "end";
}

std::array<std::map<std::wstring, std::wstring>, 2> epg_technic::get_tvg_id_mapper()
{
	std::array<std::map<std::wstring, std::wstring>, 2> mapper;

	for (size_t idx = 0; idx < 2; idx++)
	{
		if (!m_use_mapper[idx]) continue;

		std::vector<BYTE> data;
		const auto& mapper_url = fmt::format(EPG_MAP_URL, m_sources[idx]);
		if (!utils::DownloadFile(mapper_url, data) || data.empty()) continue;

		JSON_ALL_TRY
		{
			nlohmann::json parsed_json = nlohmann::json::parse(data);
			nlohmann::json js_data = parsed_json["data"];
			for (const auto& item : js_data.items())
			{
				std::wstring value;
				const auto& name = utils::utf8_to_utf16(item.key());
				switch (item.value().type())
				{
				case nlohmann::detail::value_t::number_integer:
				case nlohmann::detail::value_t::number_unsigned:
					value.swap(std::to_wstring(item.value().get<int>()));
					break;
				case nlohmann::detail::value_t::string:
					value.swap(utils::utf8_to_utf16(item.value().get<std::string>()));
					break;
				}

				if (name != value)
				{
					mapper[idx].emplace(name, value);
				}
			}
		}
		JSON_ALL_CATCH;
	}

	return mapper;
}

std::wstring epg_technic::get_epg_uri_json(int epg_idx, const std::wstring& id, time_t for_time /*= 0*/) const
{
	std::wstring url;
	switch (epg_idx)
	{
		case 0:
		{
			COleDateTime dt(for_time ? for_time : COleDateTime::GetCurrentTime());
			url = fmt::format(EPG1_TEMPLATE_JSON, m_sources[epg_idx], id, dt.GetYear(), dt.GetMonth(), dt.GetDay());
			break;
		}
		case 1:
		{
			url = fmt::format(EPG2_TEMPLATE_JSON, m_sources[epg_idx], xxh::xxhash<32>(utils::utf16_to_utf8(id)));
			break;
		}
	}
	return url;
}
