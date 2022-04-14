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

nlohmann::json epg_technic::get_epg_root(int epg_idx, const nlohmann::json& epg_data) const
{
	return epg_idx == 0 ? epg_data["data"] : uri_stream::get_epg_root(epg_idx, epg_data);
}

std::string epg_technic::get_epg_name(int epg_idx, const nlohmann::json& val) const
{
	return epg_idx == 0 ? get_json_value("title", val) : uri_stream::get_epg_name(epg_idx, val);
}

std::string epg_technic::get_epg_desc(int epg_idx, const nlohmann::json& val) const
{
	return epg_idx == 0 ? get_json_value("description", val) : uri_stream::get_epg_desc(epg_idx, val);
}

time_t epg_technic::get_epg_time_start(int epg_idx, const nlohmann::json& val) const
{
	return epg_idx == 0 ? get_json_int_value("begin", val) : uri_stream::get_epg_time_start(epg_idx, val);
}

time_t epg_technic::get_epg_time_end(int epg_idx, const nlohmann::json& val) const
{
	return epg_idx == 0 ? get_json_int_value("end", val) : uri_stream::get_epg_time_end(epg_idx, val);
}
