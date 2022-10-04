#include "pch.h"
#include "uri_stream.h"
#include "IPTVChannelEditor.h"

#include "UtilsLib\utils.h"
#include "UtilsLib\Crc32.h"
#include "UtilsLib\inet_utils.h"

uri_stream::uri_stream()
{
	PlaylistInfo info;
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

bool uri_stream::save_plugin_parameters(const wchar_t* filename /*= nullptr*/)
{
	std::wstring out_file;
	if (filename == nullptr)
	{
		std::filesystem::create_directory(L"settings");
		out_file = fmt::format(LR"(settings\{:s}_config.json)", utils::utf8_to_utf16(short_name));
	}
	else
	{
		out_file = filename;
	}

	bool res = false;
	try
	{
		nlohmann::json node = *this;

		const auto& str = node.dump(2);
		std::ofstream out_stream(out_file);
		out_stream << str << std::endl;
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

bool uri_stream::load_plugin_parameters(const wchar_t* filename /*= nullptr*/)
{
	std::wstring in_file;
	if (filename == nullptr)
	{
		in_file = fmt::format(LR"({:s}\{:s})", GetAppPath(utils::PLUGIN_SETTINGS), utils::utf8_to_utf16(short_name + "_config.json"));
	}
	else
	{
		in_file = filename;
	}

	bool res = false;
	try
	{
		nlohmann::json node;
		std::ifstream in_stream(in_file);
		if (in_stream.good())
		{
			in_stream >> node;
			from_json(node, *this);
			res = true;
		}
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

void uri_stream::parse_uri(const std::wstring& url)
{
	uri_base::set_uri(url);

	std::wsmatch m;
	if (!std::regex_match(url, m, get_uri_regex_parse_template()))
	{
		return;
	}

	templated = true;

	// map groups to parser members
	size_t pos = 1;
	for (const auto& group : parser.regex_named_groups)
	{
		parser.*parser_mapper[group] = std::move(m[pos++].str());
	}
}

std::wstring uri_stream::get_playlist_url(TemplateParams& params, std::wstring url /*= L""*/)
{
	if (url.empty())
		url = get_playlist_template();

	if (!params.token.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_TOKEN, params.token);

	if (!params.login.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_LOGIN, params.login);

	if (!params.password.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_PASSWORD, params.password);

	if (!params.subdomain.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_SUBDOMAIN, params.subdomain);

	fill_servers_list(params);
	if (!servers_list.empty())
	{
		int server = (params.server_idx >= (int)servers_list.size()) ? servers_list.size() - 1 : params.server_idx;
		utils::string_replace_inplace<wchar_t>(url, REPL_SERVER, utils::wstring_tolower(servers_list[server].get_name()));
		utils::string_replace_inplace<wchar_t>(url, REPL_SERVER_ID, servers_list[server].get_id());
	}

	fill_devices_list(params);
	if (!devices_list.empty())
	{
		int device = (params.device_idx >= (int)devices_list.size()) ? devices_list.size() - 1 : params.device_idx;
		utils::string_replace_inplace<wchar_t>(url, REPL_DEVICE_ID, devices_list[device].get_id());
	}

	fill_qualities_list(params);
	if (!qualities_list.empty())
	{
		int quality = (params.quality_idx >= (int)qualities_list.size()) ? qualities_list.size() - 1 : params.quality_idx;
		utils::string_replace_inplace<wchar_t>(url, REPL_QUALITY_ID, qualities_list[quality].get_id());
	}

	fill_profiles_list(params);
	if (!profiles_list.empty())
	{
		int profile = (params.profile_idx >= (int)profiles_list.size()) ? profiles_list.size() - 1 : params.profile_idx;
		utils::string_replace_inplace<wchar_t>(url, REPL_PROFILE_ID, profiles_list[profile].get_id());
	}

	return url;
}

std::wstring uri_stream::get_templated_stream(TemplateParams& params) const
{
	std::wstring url;
	if (!is_template())
	{
		url = get_uri();
	}
	else
	{
		size_t subtype = (size_t)params.streamSubtype;
		switch (streams_config[subtype].cu_type)
		{
			case CatchupType::cu_shift:
			case CatchupType::cu_append:
				url = streams_config[subtype].get_uri_template();
				if (params.shift_back)
				{
					url += (url.rfind('?') != std::wstring::npos) ? '&' : '?';
					url += streams_config[subtype].get_uri_arc_template();
				}
				break;

			case CatchupType::cu_flussonic:
				url = params.shift_back ? streams_config[subtype].get_uri_arc_template() : streams_config[subtype].get_uri_template();
				break;
		}
	}

	replace_vars(url, params);

	return url;
}

const std::wregex& uri_stream::get_uri_regex_parse_template()
{
	// to speed up parsing process and avoid multiple conversions
	if (uri_parse_regex_template._Empty())
	{
		set_uri_regex_parse_template(utils::utf8_to_utf16(uri_parse_pattern));
	}

	return uri_parse_regex_template;
}

void uri_stream::set_uri_regex_parse_template(const std::wstring& val)
{
	// store original template
	set_uri_parse_pattern(val);

	// clear named group
	parser.regex_named_groups.clear();

	try
	{
		std::wstring ecmascript_re(val);
		std::wregex re_group(L"(\\?<([^>]+)>)");
		std::match_results<std::wstring::const_iterator> ms;
		while (std::regex_search(ecmascript_re, ms, re_group))
		{
			if (parser_mapper.find(ms[2]) != parser_mapper.end())
			{
				// add only known group!
				parser.regex_named_groups.emplace_back(ms[2]);
			}
			ecmascript_re.erase(ms.position(), ms.length());
		}

		// store regex without named groups
		uri_parse_regex_template = ecmascript_re;
	}
	catch (...)
	{

	}
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

	const auto& params = epg_params[epg_idx];
	std::string time_format = params.epg_time_format;
	// replace to "%d-%m-%Y %H:%M"
	if (!time_format.empty())
	{
		utils::string_replace_inplace<char>(time_format, REPL_YEAR, "%Y");
		utils::string_replace_inplace<char>(time_format, REPL_MONTH, "%m");
		utils::string_replace_inplace<char>(time_format, REPL_DAY, "%d");
		utils::string_replace_inplace<char>(time_format, REPL_HOUR, "%H");
		utils::string_replace_inplace<char>(time_format, REPL_MIN, "%M");
	}

	JSON_ALL_TRY;
	{
		const auto& parsed_json = nlohmann::json::parse(data.begin(), data.end());

		bool added = false;
		time_t prev_start = 0;
		const auto& root = get_epg_root(epg_idx, parsed_json);
		for (const auto& item : root.items())
		{
			const auto& val = item.value();

			EpgInfo epg_info;

			if (time_format.empty())
			{
				epg_info.time_start = utils::get_json_int_value(params.epg_start, val);
			}
			else
			{
				std::tm tm = {};
				std::stringstream ss(utils::get_json_string_value(params.epg_start, val));
				ss >> std::get_time(&tm, time_format.c_str());
				epg_info.time_start = _mkgmtime(&tm); // parsed time assumed as UTC+00
			}

			epg_info.time_start -= 3600 * params.epg_timezone; // subtract real EPG timezone offset

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

	auto epg_template = utils::string_replace<wchar_t>(epg_params[epg_idx].get_epg_url(), REPL_EPG_ID, subst_id);

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

	if (!params.login.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_LOGIN, params.login);

	if (!params.password.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_PASSWORD, params.password);

	if (params.shift_back)
	{
		size_t subtype = (size_t)params.streamSubtype;
		utils::string_replace_inplace<wchar_t>(url, REPL_START, std::to_wstring(params.shift_back));
		utils::string_replace_inplace<wchar_t>(url, REPL_NOW, std::to_wstring(_time32(nullptr)));
		utils::string_replace_inplace<wchar_t>(url, REPL_SHIFT, streams_config[subtype].get_shift_replace());
		utils::string_replace_inplace<wchar_t>(url, REPL_DURATION, std::to_wstring(streams_config[subtype].cu_duration));
	}

	if (!servers_list.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_SERVER_ID, servers_list[params.server_idx].get_id());

	if (!profiles_list.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_PROFILE_ID, profiles_list[params.profile_idx].get_id());

	if (!qualities_list.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_QUALITY_ID, qualities_list[params.quality_idx].get_id());
}
