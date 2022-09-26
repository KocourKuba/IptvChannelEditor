#include "pch.h"
#include "uri_config.h"

uri_config::uri_config()
{
	streams_config[0].stream_type = StreamType::enHLS;
	streams_config[0].cu_type = CatchupType::cu_shift;
	streams_config[0].cu_subst = "utc";
	streams_config[0].uri_arc_template = "{CU_SUBST}={START}&lutc={NOW}";

	streams_config[1].stream_type = StreamType::enMPEGTS;
	streams_config[1].cu_type = CatchupType::cu_flussonic;
	streams_config[1].cu_subst = "archive";

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

nlohmann::json uri_config::get_epg_root(int epg_idx, const nlohmann::json& epg_data) const
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
