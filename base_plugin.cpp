/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2025): sharky72 (https://github.com/KocourKuba)

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

constexpr auto FILE_COOKIE = L"{:s}/{:s}_{:s}";

void base_plugin::parse_stream_uri(const std::wstring& url, uri_stream& info)
{
	info.set_uri(url);

	boost::wsmatch m;
	if (regex_uri_template.empty() || !boost::regex_match(url, m, regex_uri_template))
	{
		info.set_is_template(false);
		return;
	}

	info.set_is_template(true);

	// map groups to parser members
	int pos = 1;
	for (const auto& group : regex_named_groups)
	{
		auto setter = info.parser_mapper[group];
		(info.*setter)(m[pos++].str());
	}
}

std::wstring base_plugin::get_playlist_url(const TemplateParams& params, std::wstring url /*= L""*/)
{
	const auto& info = get_playlist_info(params.playlist_idx);
	set_regex_parse_stream(info.get_parse_regex());

	if (url.empty())
	{
		url = info.get_pl_template();
	}

	return replace_params_vars(params, url);
}

std::wstring base_plugin::get_play_stream(const TemplateParams& params, uri_stream* info) const
{
	static const std::map<std::wstring, std::wstring> template_mapper = {
		{ CU_START, REPL_START },
		{ CU_UTC, REPL_START },
		{ CU_CURRENT_UTC, REPL_NOW },
		{ CU_TIMESTAMP, REPL_NOW },
		{ CU_END, REPL_NOW },
		{ CU_UTCEND, REPL_NOW },
		{ CU_OFFSET, REPL_OFFSET },
		{ CU_DURATION, REPL_DURATION },
	};

	size_t subtype = (size_t)params.streamSubtype;

	const auto& live_url = get_live_template(subtype, info);
	std::wstring url = params.shift_back ? get_archive_template(subtype, info) : live_url;

	if (params.shift_back)
	{
		utils::string_replace_inplace<wchar_t>(url, REPL_LIVE_URL, live_url);
		utils::string_replace_inplace<wchar_t>(url, REPL_CH_CATCHUP, utils::utf8_to_utf16(info->get_catchup_source()));
		if (url.find(L"${") != std::string::npos)
		{
			for (const auto& [key, value] : template_mapper)
			{
				utils::string_replace_inplace<wchar_t>(url, key, value);
			}
		}

		size_t subtype = (size_t)params.streamSubtype;
		utils::string_replace_inplace<wchar_t>(url, REPL_START, std::to_wstring(params.shift_back));
		utils::string_replace_inplace<wchar_t>(url, REPL_NOW, std::to_wstring(_time32(nullptr)));
		utils::string_replace_inplace<wchar_t>(url, REPL_DURATION, std::to_wstring(streams_config[subtype].cu_duration));
		utils::string_replace_inplace<wchar_t>(url, REPL_OFFSET, std::to_wstring(_time32(nullptr) - params.shift_back));
		utils::string_replace_inplace<wchar_t>(url, REPL_STOP, std::to_wstring(params.shift_back + streams_config[subtype].cu_duration));
	}

	utils::string_replace_inplace<wchar_t>(url, REPL_CGI_BIN, std::format(L"http://127.0.0.1/cgi-bin/plugins/{:s}/", utils::utf8_to_utf16(get_name())));

	return replace_params_vars(params, replace_uri_stream_vars(info, url));
}

std::wstring base_plugin::replace_params_vars(const TemplateParams& params, const std::wstring& url) const
{
	std::wstring replaced(url);

	if (!get_provider_api_url().empty())
	{
		replaced = utils::string_replace<wchar_t>(replaced, REPL_API_URL, get_provider_api_url());
	}

	if (!params.creds.subdomain.empty())
	{
		replaced = utils::string_replace<wchar_t>(replaced, REPL_SUBDOMAIN, params.creds.get_subdomain());
	}

	if (!params.creds.ott_key.empty())
	{
		replaced = utils::string_replace<wchar_t>(replaced, REPL_OTT_KEY, params.creds.get_ott_key());
	}

	if (!params.creds.login.empty())
	{
		replaced = utils::string_replace<wchar_t>(replaced, REPL_LOGIN, params.creds.get_login());
	}

	if (!params.creds.password.empty())
	{
		replaced = utils::string_replace<wchar_t>(replaced, REPL_PASSWORD, params.creds.get_password());
	}

	if (!params.creds.token.empty())
	{
		replaced = utils::string_replace<wchar_t>(replaced, REPL_TOKEN, params.creds.get_token());
	}

	if (!params.creds.s_token.empty())
	{
		replaced = utils::string_replace<wchar_t>(replaced, REPL_S_TOKEN, params.creds.get_s_token());
	}


	if (!domains_list.empty())
	{
		size_t domain = ((params.creds.domain_id >= (int)domains_list.size()) ? domains_list.size() - 1 : params.creds.domain_id);
		replaced = utils::string_replace<wchar_t>(replaced, REPL_DOMAIN_ID, domains_list[domain].get_name());
	}

	if (!servers_list.empty())
	{
		size_t server = (params.creds.server_id >= (int)servers_list.size()) ? servers_list.size() - 1 : params.creds.server_id;
		auto srv_name = servers_list[server].get_name();
		replaced = utils::string_replace<wchar_t>(replaced, REPL_SERVER, utils::wstring_tolower(srv_name));
		auto id = servers_list[server].get_id();
		replaced = utils::string_replace<wchar_t>(replaced, REPL_SERVER_ID, utils::string_trim(id));
	}

	if (!devices_list.empty())
	{
		size_t device = (params.creds.device_id >= (int)devices_list.size()) ? devices_list.size() - 1 : params.creds.device_id;
		replaced = utils::string_replace<wchar_t>(replaced, REPL_DEVICE_ID, devices_list[device].get_id());
	}

	if (!qualities_list.empty())
	{
		size_t quality = (params.creds.quality_id >= (int)qualities_list.size()) ? qualities_list.size() - 1 : params.creds.quality_id;
		replaced = utils::string_replace<wchar_t>(replaced, REPL_QUALITY_ID, qualities_list[quality].get_id());
	}

	if (!profiles_list.empty())
	{
		size_t profile = (params.creds.profile_id >= (int)profiles_list.size()) ? profiles_list.size() - 1 : params.creds.profile_id;
		replaced = utils::string_replace<wchar_t>(replaced, REPL_PROFILE_ID, profiles_list[profile].get_id());
	}


	return replaced;
}

std::wstring base_plugin::replace_uri_stream_vars(const uri_stream* info, const std::wstring& url) const
{
	std::wstring replaced(url);

	if (!info->get_scheme().empty())
	{
		replaced = utils::string_replace<wchar_t>(replaced, REPL_SCHEME, info->get_scheme());
	}

	if (!info->get_domain().empty())
	{
		replaced = utils::string_replace<wchar_t>(replaced, REPL_DOMAIN, info->get_domain());
	}

	if (!info->get_port().empty())
	{
		replaced = utils::string_replace<wchar_t>(replaced, REPL_PORT, info->get_port());
	}

	if (!info->get_id().empty())
	{
		replaced = utils::string_replace<wchar_t>(replaced, REPL_ID, info->get_id());
	}

	if (!info->get_token().empty())
	{
		replaced = utils::string_replace<wchar_t>(replaced, REPL_TOKEN, info->get_token());
	}

	if (!info->get_int_id().empty())
	{
		replaced = utils::string_replace<wchar_t>(replaced, REPL_INT_ID, info->get_int_id());
	}

	if (!info->get_host().empty())
	{
		replaced = utils::string_replace<wchar_t>(replaced, REPL_HOST, info->get_host());
	}

	if (!info->get_var1().empty())
	{
		replaced = utils::string_replace<wchar_t>(replaced, REPL_VAR1, info->get_var1());
	}

	if (!info->get_var2().empty())
	{
		replaced = utils::string_replace<wchar_t>(replaced, REPL_VAR2, info->get_var2());
	}

	if (!info->get_var3().empty())
	{
		replaced = utils::string_replace<wchar_t>(replaced, REPL_VAR3, info->get_var3());
	}


	return replaced;
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

std::wstring base_plugin::get_vod_url(const TemplateParams& params)
{
	return get_vod_url(get_vod_info_idx(), params);
}

std::wstring base_plugin::get_vod_url(size_t idx, const TemplateParams& params)
{
	const auto& info = get_vod_info(idx);
	std::wstring url = info.get_pl_template();
	set_regex_parse_stream(L"");

	return replace_params_vars(params, url);
}

std::wstring base_plugin::compile_epg_url(int epg_idx, const std::wstring& epg_id, time_t for_time, const uri_stream* info, const TemplateParams& params)
{
	const auto& epg_param = epg_params[epg_idx];

	COleDateTime dt(for_time ? for_time : static_cast<time_t>(COleDateTime::GetCurrentTime()));
	// set to begin of the day
	std::tm lt{};
	localtime_s(&lt, &for_time);
	lt.tm_hour = 0;
	lt.tm_min = 0;
	lt.tm_sec = 0;

	auto epg_template = epg_param.get_epg_url();
	if (!epg_param.get_epg_domain().empty())
	{
		utils::string_replace_inplace<wchar_t>(epg_template, REPL_EPG_DOMAIN, epg_param.get_epg_domain());
	}

	if (!info->get_domain().empty())
	{
		utils::string_replace_inplace<wchar_t>(epg_template, REPL_DOMAIN, info->get_domain());
	}

	if (!epg_id.empty())
	{
		utils::string_replace_inplace<wchar_t>(epg_template, REPL_EPG_ID, epg_id);
	}

	if (!info->get_id().empty())
	{
		utils::string_replace_inplace<wchar_t>(epg_template, REPL_ID, info->get_id());
	}

	if (!info->get_token().empty())
	{
		utils::string_replace_inplace<wchar_t>(epg_template, REPL_TOKEN, info->get_token());
		utils::string_replace_inplace<wchar_t>(epg_template, REPL_S_TOKEN, info->get_token());
	}


	if (!epg_param.get_epg_date_format().empty())
	{
		utils::string_replace_inplace<wchar_t>(epg_template, REPL_DATE, epg_param.get_epg_date_format());
	}

	utils::string_replace_inplace<wchar_t>(epg_template, REPL_YEAR, std::to_wstring(dt.GetYear()));
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_MONTH, std::to_wstring(dt.GetMonth()));
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_DAY, std::to_wstring(dt.GetDay()));
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_TIMESTAMP, std::format(L"{:d}", std::mktime(&lt)));
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_DUNE_IP, GetConfig().get_string(true, REG_DUNE_IP).c_str());

	return replace_params_vars(params, epg_template);
}

std::string base_plugin::get_file_cookie(const std::wstring& name) const
{
	const auto& cookie_name = std::format(FILE_COOKIE,
										  std::filesystem::temp_directory_path().append(L"iptv_cache").wstring(),
										  get_internal_name(),
										  name);

	if (std::filesystem::exists(cookie_name))
	{
		CFileStatus fs;
		if (CFile::GetStatus(cookie_name.c_str(), fs) && CTime::GetCurrentTime() < fs.m_mtime)
		{
			std::ifstream ssd(cookie_name);
			std::stringstream buffer;
			buffer << ssd.rdbuf();
			ssd.close();
			return buffer.str();
		}

		std::error_code ec;
		std::filesystem::remove(cookie_name, ec);
	}

	return {};
}

void base_plugin::set_file_cookie(const std::wstring& name, const std::string& session, time_t expire_time) const
{
	const auto& cookie_name = std::format(FILE_COOKIE,
											  std::filesystem::temp_directory_path().append(L"iptv_cache").wstring(),
											  get_internal_name(),
											  name);
	std::ofstream ssd(cookie_name);
	ssd << session;
	ssd.close();

	CFileStatus fs;
	if (CFile::GetStatus(cookie_name.c_str(), fs))
	{
		fs.m_mtime = CTime(expire_time);
		CFile::SetStatus(cookie_name.c_str(), fs);
	}
}

void base_plugin::delete_file_cookie(const std::wstring& name) const
{
	const auto& cookie_name = std::format(FILE_COOKIE,
										  std::filesystem::temp_directory_path().append(L"iptv_cache").wstring(),
										  get_internal_name(),
										  name);

	std::error_code ec;
	std::filesystem::remove(cookie_name, ec);
}

void base_plugin::set_json_info(const std::string& name, const nlohmann::json& js_data, std::map<std::wstring, std::wstring, std::less<>>& info) const
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
	JSON_ALL_CATCH
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
		version_index = std::format(L"{:d}{:02d}{:02d}{:02d}", cur_dt.GetYear(), cur_dt.GetMonth(), cur_dt.GetDay(), cur_dt.GetHour());
	}

	CTime st(cur_dt.GetYear(), cur_dt.GetMonth(), cur_dt.GetDay(), cur_dt.GetHour(), cur_dt.GetMinute(), cur_dt.GetSecond());
	time_t time = st.GetTime();
	std::tm lt{};
	localtime_s(&lt, &time);

	utils::string_replace_inplace<wchar_t>(packed_name, REPL_TYPE, get_internal_name());
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
