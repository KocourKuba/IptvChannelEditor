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
#include "plugin_config.h"
#include "AccountSettings.h"
#include "Constants.h"
#include "IPTVChannelEditor.h"

void plugin_config::set_plugin_defaults(PluginType val)
{
	plugin_type = val;
	load_default();
}

const PlaylistTemplateInfo& plugin_config::get_playlist_info(int idx) const
{
	if (idx != -1 && idx >= (int)playlist_templates.size())
	{
		idx = 0;
	}

	return playlist_templates[idx];
}

void plugin_config::load_default()
{
	title = "Custom";
	name = "custom";
	playlist_template_index = 0;
	provider_url.clear();
	playlist_templates.clear();
	vod_templates.clear();
	servers_list.clear();
	qualities_list.clear();
	devices_list.clear();
	profiles_list.clear();
	square_icons = false;
	vod_support = false;
	vod_m3u = false;
	requested_token = false;
	static_servers = false;
	static_qualities = false;
	static_devices = false;
	static_profiles = false;
	files_list = {
		{"0", "bin\\update_suppliers.sh" },
		{"1", "www\\cgi-bin\\https_proxy.sh" },
	};
	manifest_list = {
		{"boot", "" },
		{"boot_end", "bin/update_suppliers.sh" },
		{"gui_start", "" },
		{"install", "" },
		{"uninstall", "" },
		{"update", "" },
	};

	StreamParameters hls;
	hls.uri_arc_template = "{LIVE_URL}?utc={START}&lutc={NOW}";
	hls.uri_custom_arc_template = "{LIVE_URL}?utc={START}&lutc={NOW}";

	StreamParameters mpeg;
	mpeg.stream_type = StreamType::enMPEGTS;
	mpeg.cu_type = CatchupType::cu_flussonic;
	mpeg.dune_params = "buffering_ms:{BUFFERING}";

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
		nlohmann::json node = *this;

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

void plugin_config::load_plugin_parameters(const std::wstring& filename)
{
	load_default();
	if (filename.empty())
	{
		return;
	}

	std::filesystem::path config_dir(GetConfig().get_string(true, REG_SAVE_SETTINGS_PATH) + get_type_name());
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
	{
		load_default();
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
