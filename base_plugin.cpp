#include "pch.h"
#include "base_plugin.h"
#include "IPTVChannelEditor.h"
#include "AccountSettings.h"
#include "Constants.h"

#include "UtilsLib\utils.h"
#include "UtilsLib\inet_utils.h"

base_plugin::base_plugin()
{
	PlaylistInfo info;
	info.name = load_string_resource(IDS_STRING_PLAYLIST);
	playlists.emplace_back(info);
}

base_plugin::base_plugin(const base_plugin& src)
{
	*this = src;
}

bool base_plugin::save_plugin_parameters(const std::wstring& filename, bool use_full_path/* = false*/)
{
	std::filesystem::path full_path;
	if (use_full_path)
	{
		full_path = filename;
	}
	else
	{
		std::filesystem::path config_dir(GetConfig().get_string(true, REG_SAVE_SETTINGS_PATH) + get_short_name_w());
		std::filesystem::create_directory(config_dir);
		full_path = config_dir.append(filename);
	}

	bool res = false;
	try
	{
		nlohmann::json node = *this;

		const auto& str = node.dump(2);
		std::ofstream out_stream(full_path);
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

void base_plugin::load_plugin_parameters(const std::wstring& filename)
{
	clear();
	if (filename.empty())
	{
		load_default();
		return;
	}

	std::filesystem::path config_dir(GetConfig().get_string(true, REG_SAVE_SETTINGS_PATH) + get_short_name_w());
	const auto& full_path = config_dir.append(filename);

	bool res = false;
	try
	{
		nlohmann::json node;
		std::ifstream in_stream(full_path);
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

	if (!res)
		load_default();
}

void base_plugin::parse_stream_uri(const std::wstring& url, uri_stream* info)
{
	info->set_uri(url);

	std::wsmatch m;
	if (!std::regex_match(url, m, get_stream_regex_parse_template()))
	{
		return;
	}

	info->set_is_template(true);

	// map groups to parser members
	size_t pos = 1;
	for (const auto& group : regex_named_groups)
	{
		*info->parser_mapper[group] = std::move(m[pos++].str());
	}
}

std::wstring base_plugin::get_playlist_url(TemplateParams& params, std::wstring url /*= L""*/)
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

std::wstring base_plugin::get_templated_stream(const TemplateParams& params, uri_stream* info) const
{
	std::wstring url;
	size_t subtype = (size_t)params.streamSubtype;
	if (!info->get_is_template())
	{
		url = info->get_uri();
		if (params.shift_back)
		{
			url += streams_config[subtype].get_stream_arc_template();
		}
	}
	else
	{
		switch (streams_config[subtype].cu_type)
		{
			case CatchupType::cu_shift:
			case CatchupType::cu_append:
				url = streams_config[subtype].get_stream_template();
				if (params.shift_back)
				{
					url += (url.rfind('?') != std::wstring::npos) ? '&' : '?';
					url += streams_config[subtype].get_stream_arc_template();
				}
				break;

			case CatchupType::cu_flussonic:
				url = params.shift_back ? streams_config[subtype].get_stream_arc_template() : streams_config[subtype].get_stream_template();
				break;
		}
	}

	replace_vars(url, params, info);

	return url;
}

std::wstring base_plugin::get_archive_template(const TemplateParams& params, const uri_stream* info) const
{
	std::wstring url;
	size_t subtype = (size_t)params.streamSubtype;
	if (info->get_is_template())
	{
		url = streams_config[subtype].get_stream_arc_template();
	}
	else
	{
		url = streams_config[subtype].get_custom_stream_arc_template();
	}

	return url;
}

const std::wregex& base_plugin::get_stream_regex_parse_template()
{
	// to speed up parsing process and avoid multiple conversions
	if (uri_parse_regex_template._Empty())
	{
		set_stream_regex_parse_template(utils::utf8_to_utf16(uri_parse_pattern));
	}

	return uri_parse_regex_template;
}

void base_plugin::set_stream_regex_parse_template(const std::wstring& val)
{
	static std::set<std::wstring> groups_mapper = { L"id", L"domain", L"port", L"login", L"password", L"subdomain", L"token", L"int_id", L"quality", L"host", };

	// store original template
	set_uri_parse_pattern(val);

	// clear named group
	regex_named_groups.clear();

	try
	{
		std::wstring ecmascript_re(val);
		std::wregex re_group(L"(\\?<([^>]+)>)");
		std::match_results<std::wstring::const_iterator> ms;
		while (std::regex_search(ecmascript_re, ms, re_group))
		{
			if (groups_mapper.find(ms[2]) != groups_mapper.end())
			{
				// add only known group!
				regex_named_groups.emplace_back(ms[2]);
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

std::wstring base_plugin::get_vod_url(TemplateParams& params) const
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

const std::map<std::wstring, std::wstring>& base_plugin::get_epg_id_mapper(int epg_idx)
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

bool base_plugin::parse_epg(int epg_idx, const std::wstring& epg_id, std::map<time_t, EpgInfo>& epg_map, time_t for_time, const uri_stream* info)
{
	if (epg_id.empty())
		return false;

	std::vector<BYTE> data;
	const auto& url = compile_epg_url(epg_idx, epg_id, for_time, info);
	if (!utils::DownloadFile(url, data, true) || data.empty())
		return false;

	const auto& params = epg_params[epg_idx];
	std::string time_format = params.epg_time_format;
	// replace to "%d-%m-%Y %H:%M"
	if (!time_format.empty())
	{
		utils::string_replace_inplace<char>(time_format, REPL_YEAR_N, "%Y");
		utils::string_replace_inplace<char>(time_format, REPL_MONTH_N, "%m");
		utils::string_replace_inplace<char>(time_format, REPL_DAY_N, "%d");
		utils::string_replace_inplace<char>(time_format, REPL_HOUR_N, "%H");
		utils::string_replace_inplace<char>(time_format, REPL_MIN_N, "%M");
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

std::wstring base_plugin::compile_epg_url(int epg_idx, const std::wstring& epg_id, time_t for_time, const uri_stream* info)
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


	COleDateTime dt(for_time ? for_time : COleDateTime::GetCurrentTime());
	// set to begin of the day
	std::tm lt =  fmt::localtime(for_time);
	lt.tm_hour = 0;
	lt.tm_min = 0;
	lt.tm_sec = 0;

	auto epg_template = epg_params[epg_idx].get_epg_url();
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_EPG_ID, subst_id);
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_TOKEN, info->token);
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_DATE, params.get_epg_date_format());
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_YEAR, std::to_wstring(dt.GetYear()));
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_MONTH, std::to_wstring(dt.GetMonth()));
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_DAY, std::to_wstring(dt.GetDay()));
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_TIMESTAMP, fmt::format(L"{:d}", std::mktime(&lt)));


	return epg_template;
}

void base_plugin::put_account_info(const std::string& name, const nlohmann::json& js_data, std::list<AccountInfo>& params) const
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

void base_plugin::replace_vars(std::wstring& url, const TemplateParams& params, const uri_stream* info) const
{
	if (!info->domain.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_DOMAIN, info->domain);

	if (!info->port.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_PORT, info->port);

	if (!info->get_id().empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_ID, info->get_id());

	if (!info->token.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_TOKEN, info->token);

	if (!info->int_id.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_INT_ID, info->int_id);

	if (!info->host.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_HOST, info->host);

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
		utils::string_replace_inplace<wchar_t>(url, REPL_OFFSET, std::to_wstring(_time32(nullptr) - params.shift_back));
	}

	if (!servers_list.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_SERVER_ID, servers_list[params.server_idx].get_id());

	if (!profiles_list.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_PROFILE_ID, profiles_list[params.profile_idx].get_id());

	if (!qualities_list.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_QUALITY_ID, qualities_list[params.quality_idx].get_id());
}
