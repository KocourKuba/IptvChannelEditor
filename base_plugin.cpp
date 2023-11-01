/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2023): sharky72 (https://github.com/KocourKuba)

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
#include "base_plugin.h"
#include "IPTVChannelEditor.h"
#include "AccountSettings.h"
#include "Constants.h"

#include "UtilsLib\utils.h"
#include "UtilsLib\inet_utils.h"

#include "7zip\SevenZipWrapper.h"

base_plugin::base_plugin()
{
	type_name = "custom";
	class_name = "default_config";
}

base_plugin::base_plugin(const base_plugin& src)
{
	*this = src;
}

void base_plugin::load_default()
{
	plugin_config::load_default();

	title = "Custom";
	name = "custom.iptv";
	user_agent = "DuneHD/1.0";

	regex_uri_template.empty();

	provider_url = "http://dune-hd.com/";
	access_type = AccountAccessType::enNone;
}

bool base_plugin::save_plugin_parameters(const std::wstring& filename, bool use_full_path /*= false*/)
{
	return plugin_config::save_plugin_parameters(filename, use_full_path);
}

void base_plugin::load_plugin_parameters(const std::wstring& filename)
{
	plugin_config::load_plugin_parameters(filename);
}

void base_plugin::parse_stream_uri(const std::wstring& url, uri_stream* info)
{
	info->set_uri(url);

	boost::wsmatch m;
	if (regex_uri_template.empty() || !boost::regex_match(url, m, regex_uri_template))
	{
		info->set_is_template(false);
		return;
	}

	info->set_is_template(true);

	// map groups to parser members
	size_t pos = 1;
	for (const auto& group : regex_named_groups)
	{
		auto setter = info->parser_mapper[group];
		(info->*setter)(m[pos++].str());
	}
}

std::wstring base_plugin::get_playlist_url(TemplateParams& params, std::wstring url /*= L""*/)
{
	set_regex_parse_stream(get_uri_parse_pattern(params.playlist_idx));

	if (url.empty())
	{
		url = get_playlist_template(params.playlist_idx);
	}

	if (!get_provider_api_url().empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_API_URL, get_provider_api_url());

	if (!get_playlist_domain(params.playlist_idx).empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_PL_DOMAIN, get_playlist_domain(params.playlist_idx));

	if (!params.s_token.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_S_TOKEN, params.s_token);

	if (!params.login.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_LOGIN, params.login);

	if (!params.password.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_PASSWORD, params.password);

	if (!params.subdomain.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_SUBDOMAIN, params.subdomain);

	if (!params.ott_key.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_OTT_KEY, params.ott_key);

	fill_servers_list(&params);
	if (!servers_list.empty())
	{
		int server = (params.server_idx >= (int)servers_list.size()) ? servers_list.size() - 1 : params.server_idx;
		utils::string_replace_inplace<wchar_t>(url, REPL_SERVER, utils::wstring_tolower(servers_list[server].get_name()));
		utils::string_replace_inplace<wchar_t>(url, REPL_SERVER_ID, utils::string_trim(servers_list[server].get_id()));
	}

	fill_devices_list(&params);
	if (!devices_list.empty())
	{
		int device = (params.device_idx >= (int)devices_list.size()) ? devices_list.size() - 1 : params.device_idx;
		utils::string_replace_inplace<wchar_t>(url, REPL_DEVICE_ID, devices_list[device].get_id());
	}

	fill_qualities_list(&params);
	if (!qualities_list.empty())
	{
		int quality = (params.quality_idx >= (int)qualities_list.size()) ? qualities_list.size() - 1 : params.quality_idx;
		utils::string_replace_inplace<wchar_t>(url, REPL_QUALITY_ID, qualities_list[quality].get_id());
	}

	fill_profiles_list(&params);
	if (!profiles_list.empty())
	{
		int profile = (params.profile_idx >= (int)profiles_list.size()) ? profiles_list.size() - 1 : params.profile_idx;
		utils::string_replace_inplace<wchar_t>(url, REPL_PROFILE_ID, profiles_list[profile].get_id());
	}

	return url;
}

std::wstring base_plugin::get_play_stream(const TemplateParams& params, uri_stream* info) const
{
	size_t subtype = (size_t)params.streamSubtype;

	const auto& live_url = get_live_template(subtype, info);
	std::wstring url = params.shift_back ? get_archive_template(subtype, info) : live_url;

	if (params.shift_back)
		utils::string_replace_inplace<wchar_t>(url, REPL_LIVE_URL, live_url);

	utils::string_replace_inplace<wchar_t>(url, REPL_CGI_BIN, fmt::format(L"http://127.0.0.1/cgi-bin/plugins/{:s}/", utils::utf8_to_utf16(get_name())));
	utils::string_replace_inplace<wchar_t>(url, REPL_SCHEME, info->scheme);
	utils::string_replace_inplace<wchar_t>(url, REPL_DOMAIN, info->domain);
	utils::string_replace_inplace<wchar_t>(url, REPL_PORT, info->port);
	utils::string_replace_inplace<wchar_t>(url, REPL_ID, info->get_id());
	utils::string_replace_inplace<wchar_t>(url, REPL_TOKEN, info->token);
	utils::string_replace_inplace<wchar_t>(url, REPL_INT_ID, info->int_id);
	utils::string_replace_inplace<wchar_t>(url, REPL_HOST, info->host);
	utils::string_replace_inplace<wchar_t>(url, REPL_VAR1, info->var1);
	utils::string_replace_inplace<wchar_t>(url, REPL_VAR2, info->var2);
	utils::string_replace_inplace<wchar_t>(url, REPL_VAR3, info->var2);

	if (!params.subdomain.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_SUBDOMAIN, params.subdomain);

	if (!params.ott_key.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_OTT_KEY, params.ott_key);

	if (!params.login.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_LOGIN, params.login);

	if (!params.password.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_PASSWORD, params.password);

	if (params.shift_back)
	{
		size_t subtype = (size_t)params.streamSubtype;
		utils::string_replace_inplace<wchar_t>(url, REPL_START, std::to_wstring(params.shift_back));
		utils::string_replace_inplace<wchar_t>(url, REPL_NOW, std::to_wstring(_time32(nullptr)));
		utils::string_replace_inplace<wchar_t>(url, REPL_DURATION, std::to_wstring(streams_config[subtype].cu_duration));
		utils::string_replace_inplace<wchar_t>(url, REPL_OFFSET, std::to_wstring(_time32(nullptr) - params.shift_back));
		utils::string_replace_inplace<wchar_t>(url, REPL_STOP, std::to_wstring(params.shift_back + streams_config[subtype].cu_duration));
	}

	if (!servers_list.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_SERVER_ID, utils::string_trim(servers_list[params.server_idx].get_id()));

	if (!profiles_list.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_PROFILE_ID, profiles_list[params.profile_idx].get_id());

	if (!qualities_list.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_QUALITY_ID, qualities_list[params.quality_idx].get_id());

	if (!devices_list.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_DEVICE_ID, devices_list[params.device_idx].get_id());


	return url;
}

std::wstring base_plugin::get_live_template(size_t stream_idx, const uri_stream* info) const
{
	return (info->get_is_template()) ? streams_config[stream_idx].get_stream_template() : info->get_uri();
}

std::wstring base_plugin::get_archive_template(size_t stream_idx, const uri_stream* info) const
{
	std::wstring url;
	if (info->get_is_custom_archive())
	{
		url = info->get_custom_archive_url();
	}
	else
	{
		url = streams_config[stream_idx].get_stream_arc_template();
	}

	return url;
}

bool base_plugin::is_custom_archive_template(bool is_template, size_t stream_idx, const std::wstring& url) const
{
	bool custom = false;

	if (url.empty())
		return custom;

	const auto& stream_info = streams_config[stream_idx];
	if (is_template)
	{
		custom = (url != stream_info.get_stream_arc_template());
	}
	else
	{
		custom = (url != stream_info.get_custom_stream_arc_template());
	}

	return custom;
}

void base_plugin::set_regex_parse_stream(const std::wstring& val)
{
	static::uri_stream uri;

	// clear named group
	regex_named_groups.clear();

	try
	{
		regex_uri_template = val;
		boost::wregex re_group(L"(\\?<([^>]+)>)");
		boost::match_results<std::wstring::const_iterator> what;
		std::wstring::const_iterator start = val.begin();
		std::wstring::const_iterator end = val.end();
		auto flags = boost::match_default;

		while (boost::regex_search(start, end, what, re_group, flags))
		{
			if (uri.parser_mapper.find(what[2]) != uri.parser_mapper.end())
			{
				// add only known group!
				regex_named_groups.emplace_back(what[2]);
			}
			start = what[0].second;
			flags |= boost::match_prev_avail;
			flags |= boost::match_not_bob;
		}
	}
	catch (...)
	{

	}
}

std::wstring base_plugin::get_vod_url(TemplateParams& params)
{
	return get_vod_url(get_vod_template_idx(), params);
}

std::wstring base_plugin::get_vod_url(size_t idx, TemplateParams& params)
{
	std::wstring url = get_vod_template(idx);

	fill_servers_list(&params);
	if (!servers_list.empty())
	{
		int server = (params.server_idx >= (int)servers_list.size()) ? servers_list.size() - 1 : params.server_idx;
		utils::string_replace_inplace<wchar_t>(url, REPL_SERVER, utils::wstring_tolower(servers_list[server].get_name()));
		utils::string_replace_inplace<wchar_t>(url, REPL_SERVER_ID, utils::string_trim(servers_list[server].get_id()));
	}

	if (!get_vod_domain(idx).empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_VOD_DOMAIN, get_vod_domain(idx));

	if (!get_provider_api_url().empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_API_URL, get_provider_api_url());

	if (!params.login.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_S_TOKEN, params.s_token);


	if (!params.login.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_LOGIN, params.login);

	if (!params.password.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_PASSWORD, params.password);

	if (!params.ott_key.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_OTT_KEY, params.ott_key);

	if (!params.subdomain.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_SUBDOMAIN, params.subdomain);

	return url;
}

bool base_plugin::parse_xml_epg(const std::wstring& internal_epg_url, EpgStorage& epg_map, CProgressCtrl* pCtrl /*= nullptr*/)
{
	if (internal_epg_url.empty())
		return false;

	CWaitCursor cur;
	std::stringstream data;
	if (!download_url(internal_epg_url, data, GetConfig().get_int(true, REG_MAX_CACHE_TTL) * 3600))
		return false;

	data.clear();

	auto cache_file = m_dl.GetCachePath(internal_epg_url);
	boost::wregex re(LR"(^.*\.(gz|zip)$)");
	boost::wsmatch m;
	if (boost::regex_match(internal_epg_url, m, re))
	{
		std::vector<char> buffer;
		const auto& unpacked_path = m_dl.GetCachePath(internal_epg_url.substr(0, internal_epg_url.length() - 3));
		if (m_dl.CheckIsCacheExpired(unpacked_path))
		{
			const auto& pack_dll = GetAppPath((AccountSettings::PACK_DLL_PATH).c_str()) + PACK_DLL;
			if (!std::filesystem::exists(pack_dll))
			{
				AfxMessageBox(IDS_STRING_ERR_DLL_MISSING, MB_OK | MB_ICONSTOP);
				return false;
			}

			SevenZip::SevenZipWrapper archiver(pack_dll);
			auto& extractor = archiver.GetExtractor();
			extractor.SetArchivePath(cache_file);
			if (m[3].str() == L"zip")
				extractor.SetCompressionFormat(SevenZip::CompressionFormat::Zip);
			else
				extractor.SetCompressionFormat(SevenZip::CompressionFormat::GZip);

			const auto& names = extractor.GetItemsNames();
			if (names.empty())
				return false;

			const auto& sizes = extractor.GetOrigSizes();
			buffer.reserve(sizes[0]);
			if (!extractor.ExtractFileToMemory(0, (std::vector<BYTE>&)buffer))
				return false;

			std::ofstream unpacked_file(unpacked_path, std::ofstream::binary);
			unpacked_file.write((char*)buffer.data(), buffer.size());
			unpacked_file.close();
		}

		cache_file = unpacked_path;
	}

	// Parse the buffer using the xml file parsing library into doc
	auto doc = std::make_unique<rapidxml::xml_document<>>();
	rapidxml::file<> xmlFile(utils::utf16_to_utf8(cache_file).c_str());

	DWORD dwStart = GetTickCount();
	try
	{
		doc->parse<rapidxml::parse_default>(xmlFile.data());
	}
	catch (rapidxml::parse_error& ex)
	{
		ex;
		return false;
	}

	auto prog_node = doc->first_node("tv")->first_node("programme");
	int cnt = 0;
	while (prog_node)
	{
		cnt++;
		prog_node = prog_node->next_sibling();
	}

	TRACE("\nParse time %d, Total nodes %d\n", GetTickCount() - dwStart, cnt);

	if (pCtrl)
	{
		pCtrl->SetRange32(0, cnt);
		pCtrl->ShowWindow(SW_SHOW);
	}
	// Iterate <tv_category> nodes
	bool added = false;
	int i = 0;
	prog_node = doc->first_node("tv")->first_node("programme");
	while (prog_node)
	{
		EpgInfo epg_info;
		const auto& channel = rapidxml::get_value_wstring(prog_node->first_attribute("channel"));
		const auto& attr_start = prog_node->first_attribute("start");
		epg_info.time_start = utils::parse_xmltv_date(attr_start->value(), attr_start->value_size());
		const auto& attr_stop = prog_node->first_attribute("stop");
		epg_info.time_end = utils::parse_xmltv_date(attr_stop->value(), attr_stop->value_size());
		epg_info.name = rapidxml::get_value_string(prog_node->first_node("title"));
		epg_info.desc = rapidxml::get_value_string(prog_node->first_node("desc"));

		epg_map[channel].emplace(epg_info.time_start, epg_info);

		prog_node = prog_node->next_sibling();
		added = true;
		if (pCtrl && (++i % 10) == 0)
		{
			pCtrl->SetPos(i);
		}
	}

	if (pCtrl)
	{
		pCtrl->ShowWindow(SW_HIDE);
	}
	return added;
}

bool base_plugin::parse_json_epg(int epg_idx, const std::wstring& epg_id, std::array<EpgStorage, 3>& all_epg_map, time_t for_time, const uri_stream* info)
{
	if (epg_id.empty())
		return false;

	bool added = false;

	CWaitCursor cur;
	std::stringstream data;

	const auto& url = compile_epg_url(epg_idx, epg_id, for_time, info);
	if (!download_url(url, data, GetConfig().get_int(true, REG_MAX_CACHE_TTL) * 3600))
		return false;

	const auto& params = epg_params[epg_idx];
	std::wstring time_format = utils::utf8_to_utf16(params.epg_time_format);
	// replace to "%d-%m-%Y %H:%M"
	if (!time_format.empty())
	{
		utils::string_replace_inplace<wchar_t>(time_format, REPL_YEAR, L"%Y");
		utils::string_replace_inplace<wchar_t>(time_format, REPL_MONTH, L"%m");
		utils::string_replace_inplace<wchar_t>(time_format, REPL_DAY, L"%d");
		utils::string_replace_inplace<wchar_t>(time_format, REPL_HOUR, L"%H");
		utils::string_replace_inplace<wchar_t>(time_format, REPL_MIN, L"%M");
	}

	JSON_ALL_TRY;
	{
		const auto& parsed_json = nlohmann::json::parse(data.str());

		auto& epg_map = all_epg_map[epg_idx][epg_id];

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
				ss >> std::get_time(&tm, utils::utf16_to_utf8(time_format).c_str());
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

	COleDateTime dt(for_time ? for_time : COleDateTime::GetCurrentTime());
	// set to begin of the day
	std::tm lt =  fmt::localtime(for_time);
	lt.tm_hour = 0;
	lt.tm_min = 0;
	lt.tm_sec = 0;

	auto epg_template = params.get_epg_url();
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_API_URL, get_provider_api_url());
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_EPG_DOMAIN, params.get_epg_domain());
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_DOMAIN, info->domain);
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_ID, info->id);
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_EPG_ID, epg_id);
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_TOKEN, info->token);
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_DATE, params.get_epg_date_format());
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_YEAR, std::to_wstring(dt.GetYear()));
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_MONTH, std::to_wstring(dt.GetMonth()));
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_DAY, std::to_wstring(dt.GetDay()));
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_TIMESTAMP, fmt::format(L"{:d}", std::mktime(&lt)));
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_DUNE_IP, GetConfig().get_string(true, REG_DUNE_IP).c_str());

	return epg_template;
}

void base_plugin::set_json_info(const std::string& name, const nlohmann::json& js_data, std::map<std::wstring, std::wstring>& info) const
{
	JSON_ALL_TRY
	{
		if (js_data.contains(name))
		{
			const auto& w_name = utils::utf8_to_utf16(name);
			const auto& js_param = js_data[name];

			if (js_param.is_boolean())
			{
				info.emplace(w_name, load_string_resource(js_param.get<bool>() ? IDS_STRING_YES : IDS_STRING_NO));
			}
			else if (js_param.is_number_integer())
			{
				info.emplace(w_name, std::to_wstring(js_param.get<int>()));
			}
			else if (js_param.is_number_float())
			{
				info.emplace(w_name, std::to_wstring(js_param.get<float>()));
			}
			else if (js_param.is_string())
			{
				info.emplace(w_name, utils::utf8_to_utf16(js_param.get<std::string>()));
			}
		}
	}
	JSON_ALL_CATCH;
}

std::wstring base_plugin::compile_name_template(std::wstring packed_name, const Credentials& cred) const
{
	COleDateTime cur_dt = COleDateTime::GetCurrentTime();
	std::wstring version_index;
	if (cred.custom_increment && !cred.version_id.empty())
	{
		version_index = utils::utf8_to_utf16(cred.version_id);
	}
	else
	{
		version_index = fmt::format(L"{:d}{:02d}{:02d}{:02d}", cur_dt.GetYear(), cur_dt.GetMonth(), cur_dt.GetDay(), cur_dt.GetHour());
	}

	CTime st(cur_dt.GetYear(), cur_dt.GetMonth(), cur_dt.GetDay(), cur_dt.GetHour(), cur_dt.GetMinute(), cur_dt.GetSecond());
	std::tm lt = fmt::localtime(st.GetTime());

	utils::string_replace_inplace<wchar_t>(packed_name, REPL_TYPE, get_type_name());
	utils::string_replace_inplace<wchar_t>(packed_name, REPL_NAME, utils::utf8_to_utf16(name));
	utils::string_replace_inplace<wchar_t>(packed_name, REPL_YEAR, std::to_wstring(cur_dt.GetYear()));
	utils::string_replace_inplace<wchar_t>(packed_name, REPL_MONTH, std::to_wstring(cur_dt.GetMonth()));
	utils::string_replace_inplace<wchar_t>(packed_name, REPL_DAY, std::to_wstring(cur_dt.GetDay()));
	utils::string_replace_inplace<wchar_t>(packed_name, REPL_HOUR, std::to_wstring(cur_dt.GetHour()));
	utils::string_replace_inplace<wchar_t>(packed_name, REPL_MIN, std::to_wstring(cur_dt.GetMinute()));
	utils::string_replace_inplace<wchar_t>(packed_name, REPL_TIMESTAMP, std::to_wstring(std::mktime(&lt)));
	utils::string_replace_inplace<wchar_t>(packed_name, REPL_COMMENT, cred.get_comment());
	utils::string_replace_inplace<wchar_t>(packed_name, REPL_VERSION, utils::utf8_to_utf16(std::string_view(STRPRODUCTVER)));
	utils::string_replace_inplace<wchar_t>(packed_name, REPL_VERSION_INDEX, version_index);

	return packed_name;
}
