#include "pch.h"
#include "epg_technic.h"

static constexpr auto EPG_MAP_URL = L"http://technic.cf/epg-{:s}/channels";
static constexpr auto EPG_TEMPLATE_JSON = L"http://technic.cf/epg-{:s}/epg_day?id={:s}&day={:04d}.{:02d}.{:02d}";

std::map<std::wstring, std::wstring> epg_technic::get_tvg_id_mapper() const
{
	std::map<std::wstring, std::wstring> mapper;
	if (m_use_mapper)
	{
		std::vector<BYTE> data;
		const auto& mapper_url = fmt::format(EPG_MAP_URL, m_source);
		if (utils::DownloadFile(mapper_url, data) || data.empty())
		{
			JSON_ALL_TRY
			{
				nlohmann::json parsed_json = nlohmann::json::parse(data);
				nlohmann::json js_data = parsed_json["data"];
				for (const auto& item : js_data.items())
				{
					const auto& name = utils::utf8_to_utf16(item.key());
					switch (item.value().type())
					{
					case nlohmann::detail::value_t::number_integer:
					case nlohmann::detail::value_t::number_unsigned:
						mapper.emplace(name, std::to_wstring(item.value().get<int>()));
						break;
					case nlohmann::detail::value_t::string:
						mapper.emplace(name, utils::utf8_to_utf16(item.value().get<std::string>()));
						break;
					}
				}
			}
			JSON_ALL_CATCH;
		}
	}

	return mapper;
}

std::wstring epg_technic::get_epg_uri_json(bool first, const std::wstring& id, time_t for_time /*= 0*/) const
{
	COleDateTime dt(for_time ? for_time : COleDateTime::GetCurrentTime());
	return fmt::format(EPG_TEMPLATE_JSON, m_source, id, dt.GetYear(), dt.GetMonth(), dt.GetDay());
}

const nlohmann::json& epg_technic::get_epg_root(bool first, const nlohmann::json& epg_data) const
{
	return epg_data["data"];
}

std::string epg_technic::get_epg_name(bool first, const nlohmann::json& val) const
{
	return get_json_value("title", val);
}

std::string epg_technic::get_epg_desc(bool first, const nlohmann::json& val) const
{
	return get_json_value("description", val);
}

time_t epg_technic::get_epg_time_start(bool first, const nlohmann::json& val) const
{
	return get_json_int_value("begin", val);
}

time_t epg_technic::get_epg_time_end(bool first, const nlohmann::json& val) const
{
	return get_json_int_value("end", val);
}
