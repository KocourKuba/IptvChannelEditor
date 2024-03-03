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
#include "plugin_config.h"
#include "AccountSettings.h"
#include "Constants.h"
#include "IPTVChannelEditor.h"

std::array<EpgParameters, (size_t)EpgPresets::enCustom> plugin_config::known_presets;

plugin_config::plugin_config()
{
	type_name = "custom";
	class_name = "default_config";
}

void plugin_config::configure_plugin()
{
	JSON_ALL_TRY;
	{
		const auto& custom_sources = GetConfig().get_string(false, REG_CUSTOM_XMLTV_SOURCE);
		if (!custom_sources.empty())
		{
			const auto& sources = nlohmann::json::parse(custom_sources);
			set_custom_epg_urls(sources.get<std::vector<DynamicParamsInfo>>());
		}
	}
	JSON_ALL_CATCH;
}

const PlaylistTemplateInfo& plugin_config::get_playlist_info(size_t idx) const
{
	if (idx != -1 && idx >= playlist_templates.size())
	{
		idx = 0;
	}

	return playlist_templates[idx];
}

PlaylistTemplateInfo& plugin_config::get_playlist_info(size_t idx)
{
	if (idx != -1 && idx >= playlist_templates.size())
	{
		idx = 0;
	}

	return playlist_templates[idx];
}

const PlaylistTemplateInfo& plugin_config::get_vod_info(size_t idx) const
{
	if (idx != -1 && idx >= vod_templates.size())
	{
		idx = 0;
	}

	return vod_templates[idx];
}

PlaylistTemplateInfo& plugin_config::get_vod_info(size_t idx)
{
	if (idx != -1 && idx >= vod_templates.size())
	{
		idx = 0;
	}

	return vod_templates[idx];
}

void plugin_config::load_default()
{
	FillEpgPresets();

	title = "Custom";
	name = "custom.iptv";
	user_agent = "DuneHD/1.0";

	provider_url = "http://dune-hd.com/";
	access_type = AccountAccessType::enNone;
	playlist_template_index = 0;
	provider_url.clear();
	playlist_templates.clear();
	vod_templates.clear();
	servers_list.clear();
	qualities_list.clear();
	devices_list.clear();
	profiles_list.clear();
	domains_list.clear();
	vod_engine = VodEngine::enNone;
	requested_token = false;
	static_servers = false;
	static_qualities = false;
	static_devices = false;
	static_profiles = false;
	files_list.clear();

	manifest_list = {
		{"boot", "" },
		{"boot_end", "bin/update_suppliers.sh" },
		{"gui_start", "" },
		{"install", "#install" },
		{"uninstall", "#uninstall" },
		{"update", "#update" },
	};

	PlaylistTemplateInfo vod_info(IDS_STRING_EDEM_STANDARD);
	vod_templates.emplace_back(vod_info);

	StreamParameters hls;
	hls.uri_arc_template = "{LIVE_URL}?utc={START}&lutc={NOW}";
	hls.uri_custom_arc_template = "{LIVE_URL}?utc={START}&lutc={NOW}";

	StreamParameters mpeg;
	mpeg.stream_type = StreamType::enMPEGTS;
	mpeg.cu_type = CatchupType::cu_flussonic;
	mpeg.dune_params = "buffering_ms:{BUFFERING}";

	streams_config = { hls, mpeg };

	set_epg_preset(0, EpgPresets::enDRM);
	epg_params[0].epg_param = "first";
	epg_params[0].epg_domain = "http://epg.drm-play.com";

	set_epg_preset(1, EpgPresets::enDRM);
	epg_params[1].epg_param = "second";
	epg_params[1].epg_domain = "http://epg.drm-play.com";
}

bool plugin_config::download_url(const std::wstring& url,
								 std::stringstream& vData,
								 int cache_ttl /*= 0*/,
								 std::vector<std::string>* pHeaders /*= nullptr*/,
								 bool verb_post /*= false*/,
								 const char* post_data /*= nullptr*/)
{
	m_dl.SetUserAgent(get_user_agent());
	m_dl.SetCacheTtl(cache_ttl);
	return m_dl.DownloadFile(url, vData, pHeaders, verb_post, post_data);
}

bool plugin_config::save_plugin_parameters(const std::wstring& filename, bool use_full_path/* = false*/)
{
	std::filesystem::path full_path;
	if (use_full_path)
	{
		full_path = filename;
	}
	else
	{
		std::filesystem::path config_dir(GetConfig().get_string(true, REG_SAVE_SETTINGS_PATH) + get_type_name());
		std::filesystem::create_directories(config_dir);
		full_path = config_dir.append(filename);
	}

	bool res = false;
	try
	{
		nlohmann::json node;
		to_json(node, *this);

		const auto& str = node.dump(2);
		std::ofstream out_stream(full_path, std::ofstream::binary);
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

void plugin_config::load_plugin_parameters(const std::wstring& filename /*= L""*/)
{
	load_default();

	std::filesystem::path config_dir = GetConfig().get_string(true, REG_SAVE_SETTINGS_PATH) + get_type_name();
	config_dir.append(filename);
	std::ifstream in_stream(config_dir.c_str());
	if (in_stream.good())
	{
		JSON_ALL_TRY
		{
			nlohmann::json node;
			in_stream >> node;
			from_json(node, *this);
		}
		JSON_ALL_CATCH;
	}
}

EpgParameters plugin_config::get_epg_preset(EpgPresets idx) const
{
	if (idx < EpgPresets::enCustom)
		return known_presets[(size_t)idx];

	return {};
}

void plugin_config::FillEpgPresets() const
{
	if (!known_presets.empty())
		return;

	{ // 0
		EpgParameters params;
		params.epg_root = "epg_data";
		params.epg_name = "name";
		params.epg_desc = "descr";
		params.epg_start = "time";
		params.epg_end = "time_to";
		known_presets[(size_t)EpgPresets::enDRM] = std::move(params);
	}

	{ // 1
		EpgParameters params;
		params.epg_root = "ch_programme";
		params.epg_name = "title";
		params.epg_desc = "description";
		params.epg_start = "start";
		params.epg_end = "";
		params.epg_time_format = "{DAY}-{MONTH}-{YEAR} {HOUR}:{MIN}"; // "%d-%m-%Y %H:%M";
		params.epg_timezone = 3; // iptvx.one uses moscow time (UTC+3)
		known_presets[(size_t)EpgPresets::enIptvxOne] = std::move(params);
	}

	{ // 2
		EpgParameters params;
		params.epg_root = "";
		params.epg_name = "name";
		params.epg_desc = "descr";
		params.epg_start = "time";
		params.epg_end = "time_to";
		known_presets[(size_t)EpgPresets::enCbilling] = std::move(params);
	}

	{ // 3
		EpgParameters params;
		params.epg_root = "res";
		params.epg_name = "title";
		params.epg_desc = "desc";
		params.epg_start = "startTime";
		params.epg_end = "stopTime";
		known_presets[(size_t)EpgPresets::enItvLive] = std::move(params);
	}

	{ // 4
		EpgParameters params;
		params.epg_root = "";
		params.epg_name = "epg";
		params.epg_desc = "desc";
		params.epg_start = "start";
		params.epg_end = "stop";
		known_presets[(size_t)EpgPresets::enPropgNet] = std::move(params);
	}

	{ // 5
		EpgParameters params;
		params.epg_root = "epg|channels|[0]|epg";
		params.epg_name = "text";
		params.epg_desc = "description";
		params.epg_start = "start";
		params.epg_end = "end";
		known_presets[(size_t)EpgPresets::enTVClub] = std::move(params);
	}

	{ // 6
		EpgParameters params;
		params.epg_root = "epg";
		params.epg_name = "title";
		params.epg_desc = "description";
		params.epg_start = "start";
		params.epg_end = "end";
		known_presets[(size_t)EpgPresets::enVidok] = std::move(params);
	}

	{ // 7
		EpgParameters params;
		params.epg_root = "";
		params.epg_name = "title";
		params.epg_desc = "desc";
		params.epg_start = "start";
		params.epg_end = "stop";
		known_presets[(size_t)EpgPresets::enMyEPGServer] = std::move(params);
	}
}

void plugin_config::set_epg_preset(size_t epg_idx, EpgPresets idx)
{
	auto& epg_param = epg_params[epg_idx];
	const auto& preset = known_presets[(size_t)idx];

	epg_param.epg_root = preset.epg_root;
	epg_param.epg_name = preset.epg_name;
	epg_param.epg_desc = preset.epg_desc;
	epg_param.epg_start = preset.epg_start;
	epg_param.epg_end = preset.epg_end;
	epg_param.epg_time_format = preset.epg_time_format;
	epg_param.epg_timezone = preset.epg_timezone;
	epg_param.epg_use_duration = preset.epg_use_duration;
}

size_t plugin_config::get_epg_preset_idx(size_t epg_idx) const
{
	size_t preset_idx = (size_t)EpgPresets::enDRM;
	const auto& epg_param = epg_params[epg_idx];
	for (const auto& preset : known_presets)
	{
		if (epg_param.compare_preset(preset))
			return preset_idx;

		preset_idx++;
	}

	return (size_t)EpgPresets::enCustom;
}

nlohmann::json plugin_config::get_epg_root(int epg_idx, const nlohmann::json& epg_data) const
{
	std::string epg_root = epg_params[epg_idx].epg_root;
	nlohmann::json ch_data = epg_data;
	if (epg_root.empty())
	{
		return ch_data;
	}

	if (epg_root.find('|') == std::string::npos)
	{
		return ch_data.contains(epg_root) ? ch_data[epg_root] : nlohmann::json();
	}

	JSON_ALL_TRY;
	{
		auto& tokens = utils::regex_split(epg_root, "\\|");
		for (auto& token : tokens)
		{
			if (token.front() != '[')
			{
				if (ch_data.contains(token))
				{
					epg_root = token;
					ch_data = ch_data[epg_root];
				}
			}
			else if (ch_data.is_array())
			{
				int idx = utils::char_to_int(utils::string_trim(token, "[]"));
				if (idx < (int)ch_data.size() && !ch_data[idx].empty())
				{
					epg_root = token;
					ch_data = ch_data[idx];
				}
			}
		}
		return ch_data;
	}
	JSON_ALL_CATCH;

	return {};
}

void plugin_config::to_json(nlohmann::json& j, const plugin_config& c)
{
	SERIALIZE_STRUCT(j, c, access_type);
	SERIALIZE_STRUCT(j, c, class_name);
	SERIALIZE_STRUCT(j, c, name);
	SERIALIZE_STRUCT(j, c, title);
	SERIALIZE_STRUCT(j, c, dev_code);
	SERIALIZE_STRUCT(j, c, user_agent);
	SERIALIZE_STRUCT(j, c, provider_url);
	SERIALIZE_STRUCT(j, c, provider_api_url);
	SERIALIZE_STRUCT(j, c, playlist_templates);
	SERIALIZE_STRUCT(j, c, playlist_template_index);
	SERIALIZE_STRUCT(j, c, vod_engine); //-V601
	SERIALIZE_STRUCT(j, c, vod_filter); //-V601
	SERIALIZE_STRUCT(j, c, vod_templates);
	SERIALIZE_STRUCT(j, c, vod_template_index);
	SERIALIZE_STRUCT(j, c, balance_support); //-V601
	SERIALIZE_STRUCT(j, c, requested_token); //-V601
	SERIALIZE_STRUCT(j, c, static_servers); //-V601
	SERIALIZE_STRUCT(j, c, static_qualities); //-V601
	SERIALIZE_STRUCT(j, c, static_devices); //-V601
	SERIALIZE_STRUCT(j, c, static_profiles); //-V601
	SERIALIZE_STRUCT(j, c, static_domains); //-V601
	SERIALIZE_STRUCT(j, c, streams_config);
	SERIALIZE_STRUCT(j, c, epg_params);
	SERIALIZE_STRUCT(j, c, files_list);
	SERIALIZE_STRUCT(j, c, manifest_list);
	SERIALIZE_STRUCT(j, c, servers_list);
	SERIALIZE_STRUCT(j, c, qualities_list);
	SERIALIZE_STRUCT(j, c, devices_list);
	SERIALIZE_STRUCT(j, c, profiles_list);
	SERIALIZE_STRUCT(j, c, domains_list);
	SERIALIZE_STRUCT(j, c, custom_epg_urls);
}

void plugin_config::from_json(const nlohmann::json& j, plugin_config& c)
{
	DESERIALIZE_STRUCT(j, c, access_type);
	DESERIALIZE_STRUCT(j, c, class_name);
	DESERIALIZE_STRUCT(j, c, name);
	DESERIALIZE_STRUCT(j, c, title);
	DESERIALIZE_STRUCT(j, c, user_agent);
	DESERIALIZE_STRUCT(j, c, provider_url);
	DESERIALIZE_STRUCT(j, c, provider_api_url);
	DESERIALIZE_STRUCT(j, c, playlist_templates);
	DESERIALIZE_STRUCT(j, c, playlist_template_index);
	DESERIALIZE_STRUCT(j, c, vod_engine);
	DESERIALIZE_STRUCT(j, c, vod_filter);
	DESERIALIZE_STRUCT(j, c, vod_templates);
	DESERIALIZE_STRUCT(j, c, vod_template_index);
	DESERIALIZE_STRUCT(j, c, balance_support);
	DESERIALIZE_STRUCT(j, c, requested_token);
	DESERIALIZE_STRUCT(j, c, static_servers);
	DESERIALIZE_STRUCT(j, c, static_qualities);
	DESERIALIZE_STRUCT(j, c, static_devices);
	DESERIALIZE_STRUCT(j, c, static_profiles);
	DESERIALIZE_STRUCT(j, c, static_domains);
	DESERIALIZE_STRUCT(j, c, streams_config);
	DESERIALIZE_STRUCT(j, c, epg_params);
	DESERIALIZE_STRUCT(j, c, files_list);
	DESERIALIZE_STRUCT(j, c, manifest_list);
	DESERIALIZE_STRUCT(j, c, servers_list);
	DESERIALIZE_STRUCT(j, c, qualities_list);
	DESERIALIZE_STRUCT(j, c, devices_list);
	DESERIALIZE_STRUCT(j, c, profiles_list);
	DESERIALIZE_STRUCT(j, c, domains_list);
	DESERIALIZE_STRUCT(j, c, custom_epg_urls);
}
