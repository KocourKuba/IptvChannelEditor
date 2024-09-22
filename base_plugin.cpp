/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2024): sharky72 (https://github.com/KocourKuba)

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

static constexpr auto FILE_COOKIE = L"{:s}/{:s}_{:s}";

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

	if (!get_provider_api_url().empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_API_URL, get_provider_api_url());

	if (!params.creds.login.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_LOGIN, params.creds.get_login());

	if (!params.creds.password.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_PASSWORD, params.creds.get_password());

	if (!params.creds.token.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_TOKEN, params.creds.get_token());

	if (!params.creds.s_token.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_S_TOKEN, params.creds.get_s_token());

	if (!params.creds.subdomain.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_SUBDOMAIN, params.creds.get_subdomain());

	if (!params.creds.ott_key.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_OTT_KEY, params.creds.get_ott_key());

	if (!domains_list.empty())
	{
		size_t domain = ((params.creds.domain_id >= (int)domains_list.size()) ? domains_list.size() - 1 : params.creds.domain_id);
		utils::string_replace_inplace<wchar_t>(url, REPL_PL_DOMAIN, domains_list[domain].get_name());
	}

	if (!servers_list.empty())
	{
		size_t server = (params.creds.server_id >= (int)servers_list.size()) ? servers_list.size() - 1 : params.creds.server_id;
		utils::string_replace_inplace<wchar_t>(url, REPL_SERVER, utils::wstring_tolower(servers_list[server].get_name()));
		utils::string_replace_inplace<wchar_t>(url, REPL_SERVER_ID, utils::string_trim(servers_list[server].get_id()));
	}

	if (!devices_list.empty())
	{
		size_t device = (params.creds.device_id >= (int)devices_list.size()) ? devices_list.size() - 1 : params.creds.device_id;
		utils::string_replace_inplace<wchar_t>(url, REPL_DEVICE_ID, devices_list[device].get_id());
	}

	if (!qualities_list.empty())
	{
		size_t quality = (params.creds.quality_id >= (int)qualities_list.size()) ? qualities_list.size() - 1 : params.creds.quality_id;
		utils::string_replace_inplace<wchar_t>(url, REPL_QUALITY_ID, qualities_list[quality].get_id());
	}

	if (!profiles_list.empty())
	{
		size_t profile = (params.creds.profile_id >= (int)profiles_list.size()) ? profiles_list.size() - 1 : params.creds.profile_id;
		utils::string_replace_inplace<wchar_t>(url, REPL_PROFILE_ID, profiles_list[profile].get_id());
	}

	return url;
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
//		{ CU_DURMIN, REPL_DURATION },
// 		{ CU_YEAR, REPL_YEAR},
// 		{ CU_MONTH, REPL_MONTH},
// 		{ CU_DAY, REPL_DAY },
// 		{ CU_HOUR, REPL_HOUR },
// 		{ CU_MIN, REPL_MIN },
// 		{ CU_SEC, REPL_SEC },
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
			for (const auto& pair : template_mapper)
			{
				utils::string_replace_inplace<wchar_t>(url, pair.first, pair.second);
			}
		}
	}

	utils::string_replace_inplace<wchar_t>(url, REPL_CGI_BIN, fmt::format(L"http://127.0.0.1/cgi-bin/plugins/{:s}/", utils::utf8_to_utf16(get_name())));
	utils::string_replace_inplace<wchar_t>(url, REPL_SCHEME, info->get_scheme());
	utils::string_replace_inplace<wchar_t>(url, REPL_DOMAIN, info->get_domain());
	utils::string_replace_inplace<wchar_t>(url, REPL_PORT, info->get_port());
	utils::string_replace_inplace<wchar_t>(url, REPL_ID, info->get_id());
	utils::string_replace_inplace<wchar_t>(url, REPL_TOKEN, info->get_token());
	utils::string_replace_inplace<wchar_t>(url, REPL_INT_ID, info->get_int_id());
	utils::string_replace_inplace<wchar_t>(url, REPL_HOST, info->get_host());
	utils::string_replace_inplace<wchar_t>(url, REPL_VAR1, info->get_var1());
	utils::string_replace_inplace<wchar_t>(url, REPL_VAR2, info->get_var2());
	utils::string_replace_inplace<wchar_t>(url, REPL_VAR3, info->get_var2());

	if (!params.creds.subdomain.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_SUBDOMAIN, params.creds.get_subdomain());

	if (!params.creds.ott_key.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_OTT_KEY, params.creds.get_ott_key());

	if (!params.creds.login.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_LOGIN, params.creds.get_login());

	if (!params.creds.password.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_PASSWORD, params.creds.get_password());

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
		utils::string_replace_inplace<wchar_t>(url, REPL_SERVER_ID, utils::string_trim(servers_list[params.creds.server_id].get_id()));

	if (!profiles_list.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_PROFILE_ID, profiles_list[params.creds.profile_id].get_id());

	if (!qualities_list.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_QUALITY_ID, qualities_list[params.creds.quality_id].get_id());

	if (!devices_list.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_DEVICE_ID, devices_list[params.creds.device_id].get_id());


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

std::wstring base_plugin::get_vod_url(const TemplateParams& params)
{
	return get_vod_url(get_vod_info_idx(), params);
}

std::wstring base_plugin::get_vod_url(size_t idx, const TemplateParams& params)
{
	const auto& info = get_vod_info(idx);
	std::wstring url = info.get_pl_template();
	set_regex_parse_stream(L"");

	if (!domains_list.empty())
	{
		size_t domain = (params.creds.domain_id >= (int)domains_list.size()) ? domains_list.size() - 1 : params.creds.domain_id;
		utils::string_replace_inplace<wchar_t>(url, REPL_PL_DOMAIN, domains_list[domain].get_name());
	}

	if (!servers_list.empty())
	{
		size_t server = (params.creds.server_id >= (int)servers_list.size()) ? servers_list.size() - 1 : params.creds.server_id;
		utils::string_replace_inplace<wchar_t>(url, REPL_SERVER, utils::wstring_tolower(servers_list[server].get_name()));
		utils::string_replace_inplace<wchar_t>(url, REPL_SERVER_ID, utils::string_trim(servers_list[server].get_id()));
	}

	if (!get_provider_api_url().empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_API_URL, get_provider_api_url());

	if (!params.creds.s_token.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_S_TOKEN, params.creds.get_s_token());


	if (!params.creds.login.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_LOGIN, params.creds.get_login());

	if (!params.creds.password.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_PASSWORD, params.creds.get_password());

	if (!params.creds.ott_key.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_OTT_KEY, params.creds.get_ott_key());

	if (!params.creds.subdomain.empty())
		utils::string_replace_inplace<wchar_t>(url, REPL_SUBDOMAIN, params.creds.get_subdomain());

	return url;
}

bool base_plugin::parse_xml_epg(const std::wstring& internal_epg_url, EpgStorage& epg_map, EpgAliases& alias_map, CProgressCtrl* pCtrl /*= nullptr*/)
{
	if (internal_epg_url.empty())
		return false;

	CWaitCursor cur;
	std::stringstream data;
	if (!download_url(internal_epg_url, data, GetConfig().get_int(true, REG_MAX_CACHE_TTL) * 3600))
		return false;

	std::vector<char> buf(8);
	data.seekg(0);
	data.read(buf.data(), 8);
	data.clear();

	auto file_fmt = SevenZip::CompressionFormat::Unknown;

	const char* str = buf.data();

	if (memcmp(str, "\x1F\x8B\x08", 3) == 0)
	{
		file_fmt = SevenZip::CompressionFormat::GZip;
	}
	else if (memcmp(str, "\x50\x4B\x03\x04", 4) == 0)
	{
		file_fmt = SevenZip::CompressionFormat::Zip;
	}
	else
	{
		if (memcmp(str, "\xEF\xBB\xBF", 3) == 0)
		{
			str += 3; // Skip utf-8 bom
		}

		if (memcmp(buf.data(), "<?xml", 5) == 0)
		{
			file_fmt = SevenZip::CompressionFormat::XZ;
		}
	}

	if (file_fmt == SevenZip::CompressionFormat::Unknown)
		return false;

	const auto& real_url = m_dl.GetUrl();
	auto cache_file = utils::CUrlDownload::GetCachedPath(real_url);
	if (file_fmt == SevenZip::CompressionFormat::GZip || file_fmt == SevenZip::CompressionFormat::Zip)
	{
		std::vector<char> buffer;
		const auto& unpacked_path = utils::CUrlDownload::GetCachedPath(real_url.substr(0, real_url.length() - 3));
		if (m_dl.CheckIsCacheExpired(unpacked_path))
		{
			auto& extractor = theApp.m_archiver.GetExtractor();
			extractor.SetArchivePath(cache_file);
			extractor.SetCompressionFormat(file_fmt);

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
	bool added = false;
	try
	{
		DWORD dwStart = GetTickCount();
		int ch_cnt = 0;
		auto node_doc = std::make_unique<rapidxml::xml_document<>>();
		rapidxml::file<> xmlFileTmp(utils::utf16_to_utf8(cache_file).c_str());
		node_doc->parse<rapidxml::parse_fastest>(xmlFileTmp.data());
		auto ch_node = node_doc->first_node("tv")->first_node("channel");
		while (ch_node)
		{
			++ch_cnt;
			ch_node = ch_node->next_sibling();
		}

		int prg_cnt = 0;
		auto prog_node = node_doc->first_node("tv")->first_node("programme");
		while (prog_node)
		{
			++prg_cnt;
			prog_node = prog_node->next_sibling();
		}
		node_doc.release();

		DWORD dwEnd = GetTickCount();
		TRACE("\nCount nodes time %f, Total channel nodes %d, programme nodes %d\n", (double)(dwEnd - dwStart) / 1000., ch_cnt, prg_cnt);

		//////////////////////////////////////////////////////////////////////////

		auto docParse = std::make_unique<rapidxml::xml_document<>>();
		rapidxml::file<> xmlFile(utils::utf16_to_utf8(cache_file).c_str());
		docParse->parse<rapidxml::parse_default>(xmlFile.data());

		//////////////////////////////////////////////////////////////////////////
		// begin parsing channels nodes
		int i = 0;

		if (pCtrl)
		{
			pCtrl->SetRange32(0, ch_cnt);
			pCtrl->ShowWindow(SW_SHOW);
		}

		auto channel_node = docParse->first_node("tv")->first_node("channel");
		while (channel_node)
		{
			const auto channel_id = rapidxml::get_value_wstring(channel_node->first_attribute("id"));
			auto display_name_node = channel_node->first_node("display-name");
			while (display_name_node)
			{
				std::wstring channel_name = utils::wstring_tolower_l(rapidxml::get_value_wstring(display_name_node));
				if (!channel_name.empty())
				{
					alias_map.emplace(channel_name, channel_id);
				}
				display_name_node = display_name_node->next_sibling();
			}

			channel_node = channel_node->next_sibling();
			if (pCtrl && (++i % 100) == 0)
			{
				pCtrl->SetPos(i);
			}
		}
		dwEnd = GetTickCount();
		TRACE("\nParse channels time %f\n", (double)(dwEnd - dwStart) / 1000.);

		//////////////////////////////////////////////////////////////////////////
		// begin parsing programme nodes
		dwStart = dwEnd;
		i = 0;

		if (pCtrl)
		{
			pCtrl->SetRange32(0, prg_cnt);
			pCtrl->ShowWindow(SW_SHOW);
		}

		// Iterate <tv_category> nodes
		prog_node = docParse->first_node("tv")->first_node("programme");
		while (prog_node)
		{
			auto epg_info = std::make_shared<EpgInfo>();

			const auto& channel_id = rapidxml::get_value_wstring(prog_node->first_attribute("channel"));

			const auto& attr_start = prog_node->first_attribute("start");
			epg_info->time_start = utils::parse_xmltv_date(attr_start->value(), attr_start->value_size());

			const auto& attr_stop = prog_node->first_attribute("stop");
			epg_info->time_end = utils::parse_xmltv_date(attr_stop->value(), attr_stop->value_size());

			epg_info->name = utils::make_text_rtf_safe(rapidxml::get_value_string(prog_node->first_node("title")));
			epg_info->desc = utils::make_text_rtf_safe(rapidxml::get_value_string(prog_node->first_node("desc")));

			epg_map[channel_id].emplace(epg_info->time_start, epg_info);
			prog_node = prog_node->next_sibling();
			added = true;
			if (pCtrl && (++i % 100) == 0)
			{
				pCtrl->SetPos(i);
			}
		}
		dwEnd = GetTickCount();
		TRACE("\nParse programme time %f\n", (double)(dwEnd - dwStart) / 1000.);
	}
	catch (rapidxml::parse_error& ex)
	{
		ex;
		return false;
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

			auto epg_info = std::make_shared<EpgInfo>();

			if (time_format.empty())
			{
				epg_info->time_start = utils::get_json_number_value(params.epg_start, val);
			}
			else
			{
				std::tm tm = {};
				std::stringstream ss(utils::get_json_string_value(params.epg_start, val));
				ss >> std::get_time(&tm, utils::utf16_to_utf8(time_format).c_str());
				epg_info->time_start = _mkgmtime(&tm); // parsed time assumed as UTC+00
			}

			epg_info->time_start -= 3600 * params.epg_timezone; // subtract real EPG timezone offset

			// Not valid time start or already added. Skip processing
			if (epg_info->time_start == 0 || epg_map.find(epg_info->time_start) != epg_map.end()) continue;

			if (params.epg_end.empty())
			{
				if (prev_start != 0)
				{
					epg_map[prev_start]->time_end = epg_info->time_start;
#ifdef _DEBUG
					COleDateTime te(epg_info->time_start);
					epg_map[prev_start]->end = fmt::format(L"{:04d}-{:02d}-{:02d} {:02d}:{:02d}", te.GetYear(), te.GetMonth(), te.GetDay(), te.GetHour(), te.GetMinute());
#endif // _DEBUG
				}
				prev_start = epg_info->time_start;
			}
			else
			{
				epg_info->time_end = utils::get_json_number_value(params.epg_end, val);
			}

			if (params.epg_use_duration)
			{
				epg_info->time_end += epg_info->time_start;
			}

#ifdef _DEBUG
			COleDateTime ts(epg_info->time_start);
			COleDateTime te(epg_info->time_end);
			epg_info->start = fmt::format(L"{:04d}-{:02d}-{:02d} {:02d}:{:02d}", ts.GetYear(), ts.GetMonth(), ts.GetDay(), ts.GetHour(), ts.GetMinute());
			epg_info->end = fmt::format(L"{:04d}-{:02d}-{:02d} {:02d}:{:02d}", te.GetYear(), te.GetMonth(), te.GetDay(), te.GetHour(), te.GetMinute());
#endif // _DEBUG

			epg_info->name = std::move(utils::make_text_rtf_safe(utils::entityDecrypt(utils::get_json_string_value(params.epg_name, val))));
			epg_info->desc = std::move(utils::make_text_rtf_safe(utils::entityDecrypt(utils::get_json_string_value(params.epg_desc, val))));

			epg_map.emplace(epg_info->time_start, epg_info);
			added = true;
		}

		if (params.epg_end.empty() && prev_start != 0)
		{
			epg_map[prev_start]->time_end = epg_map[prev_start]->time_start + 3600; // fake end
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
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_DOMAIN, info->get_domain());
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_ID, info->get_id());
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_EPG_ID, epg_id);
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_TOKEN, info->get_token());
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_S_TOKEN, info->get_token());
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_DATE, params.get_epg_date_format());
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_YEAR, std::to_wstring(dt.GetYear()));
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_MONTH, std::to_wstring(dt.GetMonth()));
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_DAY, std::to_wstring(dt.GetDay()));
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_TIMESTAMP, fmt::format(L"{:d}", std::mktime(&lt)));
	utils::string_replace_inplace<wchar_t>(epg_template, REPL_DUNE_IP, GetConfig().get_string(true, REG_DUNE_IP).c_str());

	return epg_template;
}

std::string base_plugin::get_file_cookie(const std::wstring& name) const
{
	const auto& cookie_name = fmt::format(FILE_COOKIE,
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
	const auto& cookie_name = fmt::format(FILE_COOKIE,
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
