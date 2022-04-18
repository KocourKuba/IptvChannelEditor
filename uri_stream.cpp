#include "pch.h"
#include "uri_stream.h"

#include "UtilsLib\utils.h"
#include "UtilsLib\Crc32.h"
#include "UtilsLib\inet_utils.h"
#include "UtilsLib\xxhash.hpp"

uri_stream::uri_stream()
{
	EpgParameters params;
	params.epg_date_format = L"{:4d}-{:02d}-{:02d}";
	params.epg_root = "epg_data";
	params.epg_name = "name";
	params.epg_desc = "descr";
	params.epg_start = "time";
	params.epg_end = "time_to";

	epg_params = { params, params };
}

uri_stream::uri_stream(const uri_stream& src) : uri_stream()
{
	*this = src;
}

void uri_stream::clear()
{
	uri_base::clear();
	id.clear();
	clear_hash();
}

void uri_stream::parse_uri(const std::wstring& url)
{
	// http://rtmp.api.rt.com/hls/rtdru.m3u8
	clear();
	uri_base::set_uri(url);
}

const int uri_stream::get_hash() const
{
	if (!hash)
	{
		// convert to utf8
		const auto& uri = utils::utf16_to_utf8(is_template() ? id : get_uri());
		hash = crc32_bitwise(uri.c_str(), uri.size());
		str_hash = std::to_wstring(hash);
	}

	return hash;
}

std::vector<std::tuple<StreamSubType, std::wstring>>& uri_stream::get_supported_stream_type() const
{
	static std::vector<std::tuple<StreamSubType, std::wstring>> streams = { {StreamSubType::enHLS, L"HLS"}, {StreamSubType::enMPEGTS, L"MPEG-TS"} };
	return streams;
}

const std::map<std::wstring, std::wstring>& uri_stream::get_epg_id_mapper(int epg_idx)
{
	auto& params = epg_params[epg_idx];
	if (params.epg_use_mapper && params.epg_mapper.empty())
	{
		std::vector<BYTE> data;
		if (utils::DownloadFile(params.epg_mapper_url, data) && !data.empty())
		{
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
						params.epg_mapper.emplace(name, value);
					}
				}
			}
			JSON_ALL_CATCH;
		}
	}

	return params.epg_mapper;
}

bool uri_stream::parse_epg(int epg_idx, const std::wstring& epg_id, std::map<time_t, EpgInfo>& epg_map, time_t for_time)
{
	std::vector<BYTE> data;
	if (!utils::DownloadFile(compile_epg_url(epg_idx, epg_id, for_time), data) || data.empty())
		return false;

	JSON_ALL_TRY
	{
		nlohmann::json parsed_json = nlohmann::json::parse(data);

		bool added = false;
		const auto& root = get_epg_root(epg_idx, parsed_json);
		const auto& params = epg_params[epg_idx];
		std::map<time_t, EpgInfo>::iterator last = epg_map.end();
		for (const auto& item : root.items())
		{
			const auto& val = item.value();

			EpgInfo epg_info;
			epg_info.name = std::move(utils::make_text_rtf_safe(utils::entityDecrypt(get_json_value(params.epg_name, val))));
			epg_info.desc = std::move(utils::make_text_rtf_safe(utils::entityDecrypt(get_json_value(params.epg_desc, val))));

			if (params.epg_time_format.empty())
			{
				epg_info.time_start = get_json_int_value(params.epg_start, val);
			}
			else
			{
				std::tm tm = {};
				std::stringstream ss(get_json_value(params.epg_start, val));
				ss >> std::get_time(&tm, params.epg_time_format.c_str());
				epg_info.time_start = std::mktime(&tm);
			}

			if (params.epg_end.empty())
			{
				epg_info.time_end = epg_info.time_start;
				if (last != epg_map.end())
				{
					last->second.time_end = epg_info.time_start;
				}
			}
			else
			{
				epg_info.time_end = get_json_int_value(params.epg_end, val);
			}

			if (params.epg_use_duration)
			{
				epg_info.time_end += epg_info.time_start;
			}

#ifdef _DEBUG
			COleDateTime ts(epg_info.time_start);
			COleDateTime te(epg_info.time_end);
			epg_info.start = fmt::format(L"{:04d}-{:02d}-{:02d}", ts.GetYear(), ts.GetMonth(), ts.GetDay());
			epg_info.end = fmt::format(L"{:04d}-{:02d}-{:02d}", te.GetYear(), te.GetMonth(), te.GetDay());
#endif // _DEBUG

			if (epg_info.time_start != 0)
			{
				auto it = epg_map.emplace(epg_info.time_start, epg_info);
				added |= it.second;
				last = it.first;
			}
		}

		return added;
	}
	JSON_ALL_CATCH;

	return false;
}

std::wstring uri_stream::compile_epg_url(int epg_idx, const std::wstring& epg_id, time_t for_time)
{
	const auto& params = epg_params[epg_idx];
	std::wstring subst_id;
	if (params.epg_use_mapper)
	{
		const auto& mapper = get_epg_id_mapper(epg_idx);
		if (!mapper.empty())
		{
			if (const auto& pair = mapper.find(epg_id); pair != mapper.end())
			{
				subst_id = pair->second;
			}
		}
	}
	else if (params.epg_use_id_hash)
	{
		subst_id = fmt::format(L"{:d}", xxh::xxhash<32>(utils::utf16_to_utf8(id)));
	}
	else
	{
		subst_id = epg_id;
	}

	auto epg_template = utils::string_replace<wchar_t>(epg_params[epg_idx].epg_url, REPL_ID, subst_id);

	COleDateTime dt(for_time ? for_time : COleDateTime::GetCurrentTime());
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_DATE, fmt::format(params.epg_date_format, dt.GetYear(), dt.GetMonth(), dt.GetDay()));

	// set to begin of the day
	std::tm lt =  fmt::localtime(for_time);
	lt.tm_hour = 0;
	lt.tm_min = 0;
	lt.tm_sec = 0;
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_TIME, fmt::format(L"{:d}", std::mktime(&lt)));

	utils::string_replace_inplace<wchar_t>(epg_template, REPL_TOKEN, token);

	return epg_template;
}

nlohmann::json uri_stream::get_epg_root(int epg_idx, const nlohmann::json& epg_data) const
{
	const auto& root = epg_params[epg_idx].epg_root;
	return root.empty() ? epg_data : epg_data[root];
}

void uri_stream::put_account_info(const std::string& name, nlohmann::json& js_data, std::list<AccountInfo>& params)
{
	JSON_ALL_TRY
	{
		const auto & js_param = js_data[name];

		AccountInfo info;
		info.name = std::move(utils::utf8_to_utf16(name));
		if (js_param.is_number_integer())
		{
			info.value = std::move(std::to_wstring(js_param.get<int>()));
		}
		if (js_param.is_number_float())
		{
			info.value = std::move(std::to_wstring(js_param.get<float>()));
		}
		else if (js_param.is_string())
		{
			info.value = std::move(utils::utf8_to_utf16(js_param.get<std::string>()));
		}

		params.emplace_back(info);
	}
	JSON_ALL_CATCH;
}

void uri_stream::replace_vars(std::wstring& url, const TemplateParams& params) const
{
	utils::string_replace_inplace<wchar_t>(url, REPL_SUBDOMAIN, params.domain);
	utils::string_replace_inplace<wchar_t>(url, REPL_ID, get_id());
	utils::string_replace_inplace<wchar_t>(url, REPL_TOKEN, get_token());
	utils::string_replace_inplace<wchar_t>(url, REPL_START, std::to_wstring(params.shift_back));
	utils::string_replace_inplace<wchar_t>(url, REPL_NOW, std::to_wstring(_time32(nullptr)));
}

std::wstring& uri_stream::append_archive(std::wstring& url) const
{
	if (url.rfind('?') != std::wstring::npos)
		url += '&';
	else
		url += '?';

	url += L"utc={START}&lutc={NOW}";

	return url;
}

std::string uri_stream::get_json_value(const std::string& key, const nlohmann::json& val)
{
	return val.contains(key) && val[key].is_string() ? val[key] : "";
}

time_t uri_stream::get_json_int_value(const std::string& key, const nlohmann::json& val)
{
	if (val[key].is_number())
	{
		return val.value(key, 0);
	}

	if (val[key].is_string())
	{
		return utils::char_to_int(val.value(key, ""));
	}

	return 0;
}
