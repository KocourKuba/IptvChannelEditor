#include "pch.h"
#include "uri_stream.h"
#include "IPTVChannelEditor.h"

#include "UtilsLib\utils.h"
#include "UtilsLib\Crc32.h"
#include "UtilsLib\inet_utils.h"
#include "UtilsLib\xxhash.hpp"

uri_stream::uri_stream()
{
	EpgParameters params;
	params.epg_root = "epg_data";
	params.epg_name = "name";
	params.epg_desc = "descr";
	params.epg_start = "time";
	params.epg_end = "time_to";

	epg_params = { params, params };

	PlaylistInfo info;

	streams_config[0].shift_replace = "utc";
	streams_config[1].shift_replace = "archive";

	info.name = load_string_resource(IDS_STRING_PLAYLIST);
	playlists.emplace_back(info);
}

uri_stream::uri_stream(const uri_stream& src)
{
	*this = src;
}

void uri_stream::clear()
{
	uri_base::clear();
	parser.id.clear();
	clear_hash();
}

bool uri_stream::save_plugin_parameters(const std::wstring& filename)
{
	bool res = false;
	try
	{
		nlohmann::json node;
		node["access_type"] = (int)access_type;
		node["title"] = utils::utf16_to_utf8(title);
		node["name"] = name;
		node["short_name"] = short_name;
		node["provider_url"] = utils::utf16_to_utf8(provider_url);
		node["uri_parse_template"] = utils::utf16_to_utf8(uri_parse_template);
		node["per_channel_token"] = per_channel_token ? 1 : 0;

		node["streams_config"] = streams_config;
		node["epg_params"] = epg_params;

		const auto& str = node.dump(2);
		std::ofstream out_file(filename);
		out_file << str << std::endl;
		res = true;
	}
	catch (const nlohmann::json::out_of_range& ex)
	{
		// out of range errors may happen if provided sizes are excessive
		TRACE("out of range error: %s\n", ex.what());
	}
	catch (const nlohmann::detail::type_error& ex)
	{
		// type error
		TRACE("type error: %s\n", ex.what());
	}
	catch (...)
	{
		TRACE("unknown exception\n");
	}

	return res;
}

bool uri_stream::load_plugin_parameters(const std::wstring& filename)
{

	return true;
}

void uri_stream::parse_uri(const std::wstring& url)
{
	std::wstring pre_re(uri_parse_template);

	std::vector<std::wstring> groups;
	std::wregex re_group(L"(\\?<([^>]+)>)");
	std::match_results<std::wstring::const_iterator> ms;
	while (std::regex_search(pre_re, ms, re_group))
	{
		groups.emplace_back(ms[2]);
		pre_re.erase(ms.position(), ms.length());
	}

	std::wregex re_url(pre_re);
	std::wsmatch m;
	if (!std::regex_match(url, m, re_url))
	{
		uri_base::set_uri(url);
		return;
	}

	templated = true;
	size_t pos = 0;
	for (const auto& group : groups)
	{
		pos++;
		if (group == L"subdomain")
		{
			parser.subdomain = std::move(m[pos].str());
		}
		else if (group == L"domain")
		{
			parser.domain = std::move(m[pos].str());
		}
		else if (group == L"id")
		{
			parser.id = std::move(m[pos].str());
		}
		else if (group == L"login")
		{
			parser.login = std::move(m[pos].str());
		}
		else if (group == L"password")
		{
			parser.password = std::move(m[pos].str());
		}
		else if (group == L"token")
		{
			parser.token = std::move(m[pos].str());
		}
		else if (group == L"int_id")
		{
			parser.int_id = std::move(m[pos].str());
		}
		else if (group == L"host")
		{
			parser.host = std::move(m[pos].str());
		}
		else if (group == L"port")
		{
			parser.port = std::move(m[pos].str());
		}
		else if (group == L"quality")
		{
			parser.quality = std::move(m[pos].str());
		}
		else
		{
			// unknown group. fix parser!
			assert(false);
		}
	}
}

void uri_stream::get_playlist_url(std::wstring& url, TemplateParams& params)
{
	if (url.empty())
		url = playlist_template;

	if (!params.token.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_TOKEN, params.token);

	if (!params.login.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_LOGIN, params.login);

	if (!params.password.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_PASSWORD, params.password);

	if (!params.subdomain.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_SUBDOMAIN, params.subdomain);

	if (!servers_list.empty())
	{
		int server = (params.server >= (int)servers_list.size()) ? servers_list.size() - 1 : params.server;
		utils::string_replace_inplace<wchar_t>(url, REPL_SERVER_ID, servers_list[server].id);
	}

	if (!quality_list.empty())
	{
		int quality = (params.quality >= (int)quality_list.size()) ? quality_list.size() - 1 : params.quality;
		utils::string_replace_inplace<wchar_t>(url, REPL_QUALITY, quality_list[quality].id);
	}
}

std::wstring uri_stream::get_templated_stream(TemplateParams& params) const
{
	std::wstring url;
	size_t subtype = (size_t)params.streamSubtype;
	switch (streams_config[subtype].catchup_type)
	{
		case CatchupType::cu_shift:
		case CatchupType::cu_append:
		case CatchupType::cu_flussonic:
			if (is_template())
				url = params.shift_back ? streams_config[subtype].get_uri_arc_template() : streams_config[subtype].get_uri_template();
			else
				url = get_uri();
			break;

		case CatchupType::cu_none:
			break;

		default:
			ASSERT(false);
			break;
	}

	replace_vars(url, params);

	return url;
}

const int uri_stream::get_hash() const
{
	if (!hash)
	{
		// convert to utf8
		const auto& uri = utils::utf16_to_utf8(is_template() ? parser.id : get_uri());
		hash = crc32_bitwise(uri.c_str(), uri.size());
		str_hash = std::to_wstring(hash);
	}

	return hash;
}

std::wstring uri_stream::get_vod_url(TemplateParams& params) const
{
	std::wstring url(provider_vod_url);
	if (!params.login.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_LOGIN, params.login);

	if (!params.password.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_PASSWORD, params.password);

	if (!params.subdomain.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_SUBDOMAIN, params.subdomain);

	return url;
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
				nlohmann::json parsed_json = nlohmann::json::parse(data.begin(), data.end());
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
	const auto& url = compile_epg_url(epg_idx, epg_id, for_time);
	if (!utils::DownloadFile(url, data, true) || data.empty())
		return false;

	JSON_ALL_TRY;
	{
		const auto& parsed_json = nlohmann::json::parse(data.begin(), data.end());

		bool added = false;
		const auto& root = get_epg_root(epg_idx, parsed_json);
		const auto& params = epg_params[epg_idx];
		time_t prev_start = 0;
		for (const auto& item : root.items())
		{
			const auto& val = item.value();

			EpgInfo epg_info;

			if (params.epg_time_format.empty())
			{
				epg_info.time_start = utils::get_json_int_value(params.epg_start, val);
			}
			else
			{
				std::tm tm = {};
				std::stringstream ss(utils::get_json_string_value(params.epg_start, val));
				ss >> std::get_time(&tm, params.epg_time_format.c_str());
				epg_info.time_start = _mkgmtime(&tm); // parsed time assumed as UTC+00
			}

			epg_info.time_start -= 3600 * params.epg_tz; // subtract real EPG timezone offset

			// Not valid time start or already added. Skip processing
			if (epg_info.time_start == 0 || epg_map.find(epg_info.time_start) != epg_map.end()) continue;

			if (params.epg_end.empty())
			{
				if (prev_start != 0)
				{
					epg_map[prev_start].time_end = epg_info.time_start;
#ifdef _DEBUG
					COleDateTime te(epg_info.time_start);
					epg_map[prev_start].end = fmt::format(L"{:04d}-{:02d}-{:02d} {:02d}:{:02d}", te.GetYear(), te.GetMonth(), te.GetDay(), te.GetHour(), te.GetMinute());
#endif // _DEBUG
				}
				prev_start = epg_info.time_start;
			}
			else
			{
				epg_info.time_end = utils::get_json_int_value(params.epg_end, val);
			}

			if (params.epg_use_duration)
			{
				epg_info.time_end += epg_info.time_start;
			}

#ifdef _DEBUG
			COleDateTime ts(epg_info.time_start);
			COleDateTime te(epg_info.time_end);
			epg_info.start = fmt::format(L"{:04d}-{:02d}-{:02d} {:02d}:{:02d}", ts.GetYear(), ts.GetMonth(), ts.GetDay(), ts.GetHour(), ts.GetMinute());
			epg_info.end = fmt::format(L"{:04d}-{:02d}-{:02d} {:02d}:{:02d}", te.GetYear(), te.GetMonth(), te.GetDay(), te.GetHour(), te.GetMinute());
#endif // _DEBUG

			epg_info.name = std::move(utils::make_text_rtf_safe(utils::entityDecrypt(utils::get_json_string_value(params.epg_name, val))));
			epg_info.desc = std::move(utils::make_text_rtf_safe(utils::entityDecrypt(utils::get_json_string_value(params.epg_desc, val))));

			epg_map.emplace(epg_info.time_start, epg_info);
			added = true;
		}

		if (params.epg_end.empty() && prev_start != 0)
		{
			epg_map[prev_start].time_end = epg_map[prev_start].time_start + 3600; // fake end
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
			const auto& pair = mapper.find(epg_id);
			subst_id = (pair != mapper.end()) ? pair->second : epg_id;
		}
	}
	else
	{
		subst_id = epg_id;
	}

	auto epg_template = utils::string_replace<wchar_t>(epg_params[epg_idx].get_epg_url(), REPL_ID, subst_id);

	COleDateTime dt(for_time ? for_time : COleDateTime::GetCurrentTime());

	utils::string_replace_inplace<wchar_t>(epg_template, REPL_DATE, dt.Format(params.get_epg_date_format().c_str()));

	// set to begin of the day
	std::tm lt =  fmt::localtime(for_time);
	lt.tm_hour = 0;
	lt.tm_min = 0;
	lt.tm_sec = 0;
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_TIMESTAMP, fmt::format(L"{:d}", std::mktime(&lt)));

	utils::string_replace_inplace<wchar_t>(epg_template, REPL_TOKEN, parser.token);

	return epg_template;
}

nlohmann::json uri_stream::get_epg_root(int epg_idx, const nlohmann::json& epg_data) const
{
	const auto& root = epg_params[epg_idx].epg_root;
	return root.empty() ? epg_data : (epg_data.contains(root) ? epg_data[root] : nlohmann::json());
}

void uri_stream::put_account_info(const std::string& name, const nlohmann::json& js_data, std::list<AccountInfo>& params) const
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
	size_t subtype = (size_t)params.streamSubtype;

	if (!parser.domain.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_DOMAIN, parser.domain);

	if (!parser.port.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_PORT, parser.port);

	if (!parser.id.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_ID, parser.id);

	if (!parser.token.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_TOKEN, parser.token);

	if (!parser.int_id.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_INT_ID, parser.int_id);

	if (!parser.host.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_HOST, parser.host);

	if (!params.subdomain.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_SUBDOMAIN, params.subdomain);

	if (params.shift_back)
	{
		utils::string_replace_inplace<wchar_t>(url, REPL_NOW, std::to_wstring(_time32(nullptr)));
		utils::string_replace_inplace<wchar_t>(url, REPL_SHIFT, streams_config[subtype].get_shift_replace());
		utils::string_replace_inplace<wchar_t>(url, REPL_DURATION, std::to_wstring(streams_config[subtype].catchup_duration));
	}

	if (params.shift_back)
		utils::string_replace_inplace<wchar_t>(url, REPL_START, std::to_wstring(params.shift_back));

	if (!params.login.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_LOGIN, params.login);

	if (!params.password.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_PASSWORD, params.password);

	if (!servers_list.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_SERVER_ID, servers_list[params.server].id);

	if (!profiles_list.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_PROFILE_ID, profiles_list[params.profile].id);

	if (!quality_list.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_QUALITY, quality_list[params.quality].id);
}
