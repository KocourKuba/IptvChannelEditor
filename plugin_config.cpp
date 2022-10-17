#include "pch.h"
#include "plugin_config.h"

plugin_config::plugin_config()
{
	clear();
}

void plugin_config::clear()
{
	title = "Custom";
	name = "custom";
	provider_url.clear();
	playlist_template.clear();
	uri_id_parse_pattern.clear();
	uri_parse_pattern.clear();
	servers_list.clear();
	qualities_list.clear();
	devices_list.clear();
	profiles_list.clear();
	square_icons = false;
	per_channel_token = false;
	requested_token = false;
	static_servers = false;
	static_qualities = false;
	static_devices = false;
	static_profiles = false;

	StreamParameters hls;
	hls.stream_type = StreamType::enHLS;
	hls.cu_type = CatchupType::cu_shift;
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
