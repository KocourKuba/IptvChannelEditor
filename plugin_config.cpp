#include "pch.h"
#include "plugin_config.h"
#include "AccountSettings.h"
#include "Constants.h"
#include "IPTVChannelEditor.h"

void plugin_config::clear()
{
	title = "Custom";
	name = "custom";
	playlist_template_index = 0;
	provider_url.clear();
	playlist_template.clear();
	playlist_templates.clear();
	uri_id_parse_pattern.clear();
	uri_parse_pattern.clear();
	provider_vod_url.clear();
	vod_templates.clear();
	vod_parse_pattern.clear();
	servers_list.clear();
	qualities_list.clear();
	devices_list.clear();
	profiles_list.clear();
	square_icons = false;
	vod_support = false;
	vod_m3u = false;
	per_channel_token = false;
	requested_token = false;
	static_servers = false;
	static_qualities = false;
	static_devices = false;
	static_profiles = false;

	StreamParameters hls;
	hls.cu_subst = "utc";
	hls.uri_arc_template = "{CU_SUBST}={START}&lutc={NOW}";
	hls.uri_custom_arc_template = "{CU_SUBST}={START}&lutc={NOW}";

	StreamParameters mpeg;
	mpeg.stream_type = StreamType::enMPEGTS;
	mpeg.cu_type = CatchupType::cu_flussonic;
	mpeg.cu_subst = "archive";

	streams_config = { hls, mpeg };

	EpgParameters params;
	params.epg_root = "epg_data";
	params.epg_name = "name";
	params.epg_desc = "descr";
	params.epg_start = "time";
	params.epg_end = "time_to";

	epg_params = { params, params };
	epg_params[0].epg_param = "first";
	epg_params[1].epg_param = "second";
}

void plugin_config::set_plugin_defaults(PluginType val)
{
	plugin_type = val;
	clear();
	load_default();
	set_current_playlist_template(get_playlist_template(get_playlist_template_idx()));
	set_current_vod_template(get_vod_template(get_vod_template_idx()));
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
		std::filesystem::path config_dir(GetConfig().get_string(true, REG_SAVE_SETTINGS_PATH) + get_short_name_w());
		std::filesystem::create_directories(config_dir);
		full_path = config_dir.append(filename);
	}

	bool res = false;
	try
	{
		set_current_playlist_template(get_playlist_template(get_playlist_template_idx()));
		set_current_vod_template(get_vod_template(get_vod_template_idx()));

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

void plugin_config::load_plugin_parameters(const std::wstring& filename)
{
	clear();
	if (filename.empty())
	{
		load_default();
		set_current_playlist_template(get_playlist_template(get_playlist_template_idx()));
		set_current_vod_template(get_vod_template(get_vod_template_idx()));
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

			set_current_playlist_template(get_playlist_template(get_playlist_template_idx()));
			set_current_vod_template(get_vod_template(get_vod_template_idx()));
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
	{
		load_default();
		set_current_playlist_template(get_playlist_template(get_playlist_template_idx()));
		set_current_vod_template(get_vod_template(get_vod_template_idx()));
	}
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
