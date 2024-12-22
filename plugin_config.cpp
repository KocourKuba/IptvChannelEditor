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
#include "PluginFactory.h"

plugin_config::plugin_config()
{
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
	static PlaylistTemplateInfo emptyInfo;

	if (idx != -1 && idx >= playlist_templates.size())
	{
		idx = 0;
	}

	return playlist_templates.empty() ? emptyInfo : playlist_templates[idx];
}

PlaylistTemplateInfo& plugin_config::get_playlist_info(size_t idx)
{
	static PlaylistTemplateInfo emptyInfo;

	if (idx != -1 && idx >= playlist_templates.size())
	{
		idx = 0;
	}

	return playlist_templates.empty() ? emptyInfo : playlist_templates[idx];
}

const PlaylistTemplateInfo& plugin_config::get_vod_info(size_t idx) const
{
	static PlaylistTemplateInfo emptyInfo;

	if (idx != -1 && idx >= vod_templates.size())
	{
		idx = 0;
	}

	return vod_templates.empty() ? emptyInfo : vod_templates[idx];
}

PlaylistTemplateInfo& plugin_config::get_vod_info(size_t idx)
{
	static PlaylistTemplateInfo emptyInfo;

	if (idx != -1 && idx >= vod_templates.size())
	{
		idx = 0;
	}

	return vod_templates.empty() ? emptyInfo : vod_templates[idx];
}

void plugin_config::clear()
{
	title.clear();
	name.clear();
	user_agent = "DuneHD/1.0";
	access_type = AccountAccessType::enNone;
	provider_url.clear();

	balance_support = false;

	playlist_template_index = 0;
	playlist_templates.clear();

	servers_list.clear();
	qualities_list.clear();
	devices_list.clear();
	profiles_list.clear();
	domains_list.clear();

	static_servers = false;
	static_domains = false;
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

	vod_template_index = 0;
	vod_templates.clear();
	vod_engine = VodEngine::enNone;
	vod_filter = false;
	PlaylistTemplateInfo vod_info(IDS_STRING_EDEM_STANDARD);
	vod_templates.emplace_back(vod_info);

	streams_config.fill(StreamParameters());

	internal_epg_urls.clear();
	set_epg_preset(0, "drm");
	epg_params[0].epg_param = "first";

	set_epg_preset(1, "drm");
	epg_params[1].epg_param = "second";
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
	m_dl.SetUrl(url);
	return m_dl.DownloadFile(vData, pHeaders, verb_post, post_data);
}

bool plugin_config::save_plugin_parameters(const std::wstring& filename, const std::wstring& parent_name, bool use_full_path/* = false*/)
{
	std::filesystem::path full_path;
	if (use_full_path)
	{
		full_path = filename;
	}
	else
	{
		std::filesystem::path config_dir(GetConfig().get_string(true, REG_SAVE_SETTINGS_PATH) + parent_name);
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

void plugin_config::load_plugin_parameters(const std::wstring& filename, const std::wstring& parent_name)
{
	std::filesystem::path config_dir = GetConfig().get_string(true, REG_SAVE_SETTINGS_PATH) + parent_name;
	config_dir.append(filename);
	std::ifstream in_stream(config_dir.c_str());
	if (in_stream.good())
	{
		clear();
		JSON_ALL_TRY
		{
			nlohmann::json node;
			in_stream >> node;
			from_json(node, *this);
		}
		JSON_ALL_CATCH;
	}
}

std::wstring plugin_config::get_internal_epg_url(const std::wstring& source_id)
{
	const auto& it = std::find_if(internal_epg_urls.begin(), internal_epg_urls.end(), [source_id](const auto& item)
								  {
									  return item.get_id() == source_id;
								  });
	return it != internal_epg_urls.end() ? it->get_name() : L"";
}

void plugin_config::set_epg_preset(size_t epg_idx, const std::string& preset_name)
{
	auto& epg_param = epg_params[epg_idx];
	const auto& preset = GetPluginFactory().get_epg_preset(preset_name);

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
	size_t preset_idx = 0;
	const auto& epg_param = epg_params[epg_idx];
	for (const auto& preset : GetPluginFactory().get_epg_presets())
	{
		if (epg_param.compare_preset(preset.second))
			return preset_idx;

		preset_idx++;
	}

	return preset_idx;
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

void plugin_config::update_provider_params(TemplateParams& params)
{
	fill_servers_list(params);
	fill_domains_list(params);
	fill_devices_list(params);
	fill_profiles_list(params);
	fill_qualities_list(params);
}
