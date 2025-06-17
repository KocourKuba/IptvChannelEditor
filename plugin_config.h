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

#pragma once
#include "Credentials.h"
#include "DynamicParamsInfo.h"
#include "PlaylistTemplateInfo.h"
#include "StreamParameters.h"
#include "EpgParameters.h"

#include "UtilsLib\inet_utils.h"
#include "UtilsLib\json_wrapper.h"

class plugin_config
{
public:
	plugin_config();
	virtual ~plugin_config() {}

public:
	/// <summary>
	/// load default settings
	/// </summary>
	void clear();

	/// <summary>
	/// copy info
	/// </summary>
	void copy_config(const plugin_config& src)
	{
		if (this != &src)
		{
			clear();
			*this = src;
		}
	}

	/// <summary>
	/// save plugin parameters to file
	/// </summary>
	bool save_plugin_parameters(const std::wstring& filename, const std::wstring& parent_name, bool use_full_path = false);

	/// <summary>
	/// load plugin parameters to file
	/// </summary>
	void load_plugin_parameters(const std::wstring& filename, const std::wstring& parent_name);

	/// <summary>
	/// set playlist epg url
	/// </summary>
	void set_custom_epg_urls(const std::vector<DynamicParamsInfo>& urls) { custom_epg_urls = urls; }

	/// <summary>
	/// get playlist epg url
	/// </summary>
	const std::vector<DynamicParamsInfo>& get_custom_epg_urls() { return custom_epg_urls; }

	/// <summary>
	/// set playlist epg url
	/// </summary>
	void set_internal_epg_urls(const std::vector<DynamicParamsInfo>& urls) { internal_epg_urls = urls; }

	/// <summary>
	/// get playlist epg url
	/// </summary>
	const std::vector<DynamicParamsInfo>& get_internal_epg_urls() { return internal_epg_urls; }

	/// <summary>
	/// get playlist epg url
	/// </summary>
	std::wstring get_internal_epg_url(const std::wstring& source_id);

	/// <summary>
	/// set prefilled EPG parsing preset for selected epg type
	/// </summary>
	void set_epg_preset(size_t epg_idx, const std::string& preset_name);

	/// <summary>
	/// get EPG parsing preset index for selected epg type
	/// </summary>
	size_t get_epg_preset_idx(size_t epg_idx) const;

	/// <summary>
	/// set path used to send logs to developer
	/// </summary>
	void set_dev_path(const std::string& path) { dev_code = path; }

	/// <summary>
	/// configure internal plugin settings
	/// </summary>
	virtual void configure_plugin();

	/// <summary>
	/// configure provider plugin settings
	/// </summary>
	virtual void configure_provider_plugin() {}

	/// <summary>
	/// is plugin enabled
	/// </summary>
	bool get_enabled() const { return enabled; }
	void set_enabled(bool val) { enabled = val; }

	/// <summary>
	/// is plugin enabled
	/// </summary>
	bool get_custom() const { return custom; }
	void set_custom(bool val) { custom = val; }

	/// <summary>
	/// plugin class name
	/// </summary>
	const std::string& get_class_name() const { return class_name; }
	void set_class_name(const std::wstring& val) { class_name = utils::utf16_to_utf8(val); }

	/// <summary>
	/// returns link to provider api url
	/// </summary>
	/// <returns>wstring</returns>
	std::wstring get_provider_api_url() const { return utils::utf8_to_utf16(provider_api_url); }
	void set_provider_api_url(const std::wstring& val) { provider_api_url = utils::utf16_to_utf8(val); }

	/// <summary>
	/// property plugin title
	/// </summary>
	std::wstring get_title() const { return utils::utf8_to_utf16(title); }
	void set_title(const std::wstring& val) { title = utils::utf16_to_utf8(val); }

	/// <summary>
	/// property user-agent
	/// </summary>
	std::wstring get_user_agent() const { return utils::utf8_to_utf16(user_agent); }
	void set_user_agent(const std::wstring& val) { user_agent = utils::utf16_to_utf8(val); }

	/// <summary>
	/// property plugin name
	/// </summary>
	const std::string& get_name() const { return name; }
	void set_name(const std::wstring& val) { name = utils::utf16_to_utf8(val); }

	/// <summary>
	/// property plugin access type
	/// </summary>
	const AccountAccessType get_access_type() const { return access_type; }
	void set_access_type(AccountAccessType val) { access_type = val; }

	/// <summary>
	/// property link to provider account
	/// </summary>
	std::wstring get_provider_url() const { return utils::utf8_to_utf16(provider_url); }
	void set_provider_url(const std::wstring& val) { provider_url = utils::utf16_to_utf8(val); }

	/// <summary>
	/// selected playlist template index
	/// </summary>
	size_t get_playlist_idx() const { return playlist_template_index; }
	void set_playlist_idx(size_t idx) { playlist_template_index = idx; }

	/// <summary>
	/// property playlist templates
	/// </summary>
	const std::vector<PlaylistTemplateInfo>& get_playlist_infos() const { return playlist_templates; }
	void set_playlist_infos(const std::vector<PlaylistTemplateInfo>& val) { playlist_templates = val; }

	const PlaylistTemplateInfo& get_playlist_info(size_t idx) const;
	PlaylistTemplateInfo& get_playlist_info(size_t idx);

	const PlaylistTemplateInfo& get_current_playlist_info() const { return get_playlist_info(get_playlist_idx()); };

	/// <summary>
	/// vod engine type
	/// </summary>
	/// <returns>bool</returns>
	VodEngine get_vod_engine() const { return vod_engine; }
	void set_vod_engine(VodEngine val) { vod_engine = val; }

	/// <summary>
	/// property plugin vod support season
	/// </summary>
	bool get_vod_season() const { return vod_season; }
	void set_vod_season(bool val) { vod_season = val; }

	/// <summary>
	/// property vod server filter
	/// </summary>
	bool get_vod_server_filter() const { return vod_server_filter; }
	void set_vod_server_filter(bool val) { vod_server_filter = val; }

	/// <summary>
	/// property vod filter
	/// </summary>
	bool get_vod_filter() const { return vod_filter; }
	void set_vod_filter(bool val) { vod_filter = val; }

	/// <summary>
	/// property vod templates
	/// </summary>
	const std::vector<PlaylistTemplateInfo>& get_vod_infos() const { return vod_templates; }
	void set_vod_infos(const std::vector<PlaylistTemplateInfo>& val) { vod_templates = val; }

	/// <summary>
	/// selected vod template index
	/// </summary>
	size_t get_vod_info_idx() const { return vod_template_index; }
	void set_vod_info_idx(size_t idx) { vod_template_index = idx; }

	const PlaylistTemplateInfo& get_vod_info(size_t idx) const;
	PlaylistTemplateInfo& get_vod_info(size_t idx);

	const PlaylistTemplateInfo& get_current_vod_info() const { return get_vod_info(get_vod_info_idx()); };

	/// <summary>
	/// property enable show balance info
	/// </summary>
	bool get_balance_support() const { return balance_support; }
	void set_balance_support(bool val) { balance_support = val; }

	/// <summary>
	/// property supported streams HLS,MPEGTS etc.
	/// </summary>
	/// <returns>vector&</returns>
	const std::array<StreamParameters, 2>& get_supported_streams() const { return streams_config; }

	/// <summary>
	/// property index of supported stream
	/// </summary>
	const StreamParameters& get_supported_stream(size_t idx) const { return streams_config[idx]; }
	StreamParameters& get_supported_stream(size_t idx) { return streams_config[idx]; }

	void set_supported_stream(size_t idx, const StreamParameters& val) { streams_config[idx] = val; }

	/// <summary>
	/// return supported stream
	/// </summary>
	/// <returns>const StreamParameters&</returns>
	const std::array<EpgParameters, 2>& get_epg_parameters() const { return epg_params; }
	std::array<EpgParameters, 2>& get_epg_parameters() { return epg_params; }

	/// <summary>
	/// property epg parameter by index
	/// </summary>
	EpgParameters& get_epg_parameter(int idx) { return epg_params[idx]; };

	/// <summary>
	/// property epg domain
	/// </summary>
	std::wstring get_epg_domain(size_t idx) { return epg_params[idx].get_epg_domain(); };
	void set_epg_domain(size_t idx, const std::wstring& val) { epg_params[idx].set_epg_domain(val); };

	/// <summary>
	/// returns json root for epg iteration
	/// </summary>
	/// <param name="epg_idx">index of epg, primary/secondary</param>
	/// <param name="epg_data">downloaded json</param>
	/// <returns>json entry pointed to epg list</returns>
	nlohmann::json get_epg_root(int epg_idx, const nlohmann::json& epg_data) const;

	/// <summary>
	/// property servers list static, not loaded from provider settings
	/// </summary>
	bool get_static_servers() const { return static_servers; }
	void set_static_servers(bool val) { static_servers = val; }

	/// <summary>
	/// property devices list static, not loaded from provider settings
	/// </summary>
	bool get_static_devices() const { return static_devices; }
	void set_static_devices(bool val) { static_devices = val; }

	/// <summary>
	/// is qualities list static, not loaded from provider settings
	/// </summary>
	bool get_static_qualities() const { return static_qualities; }
	void set_static_qualities(bool val) { static_qualities = val; }

	/// <summary>
	/// property profiles list static, not loaded from provider settings
	/// </summary>
	bool get_static_profiles() const { return static_profiles; }
	void set_static_profiles(bool val) { static_profiles = val; }

	/// <summary>
	/// property domain list static, not loaded from provider settings
	/// </summary>
	bool get_static_domains() const { return static_domains; }
	void set_static_domains(bool val) { static_domains = val; }

	/// <summary>
	/// property list external bin files (puts in /bin folder)
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	virtual const std::vector<DynamicParamsInfo>& get_files_list() { return files_list; }
	virtual void set_files_list(const std::vector<DynamicParamsInfo>& info) { files_list = info; }

	/// <summary>
	/// property list for plugin manifest
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	virtual const std::vector<DynamicParamsInfo>& get_manifest_list() { return manifest_list; }
	virtual void set_manifest_list(const std::vector<DynamicParamsInfo>& info) { manifest_list = info; }

	/// <summary>
	/// update servers/domains/profiles/devices/qualities
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	void update_provider_params(TemplateParams& params);

	/// <summary>
	/// clear servers list
	/// </summary>
	void clear_servers_list() { servers_list.clear(); }

	/// returns list of servers
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	virtual void fill_servers_list(TemplateParams& params) {}

	/// <summary>
	/// set server
	/// </summary>
	/// <param name="params">Template parameters.</param>
	virtual bool set_server(TemplateParams& /*params*/) { return true; }

	/// <summary>
	/// property list of servers
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	virtual const std::vector<DynamicParamsInfo>& get_servers_list() { return servers_list; }
	virtual void set_servers_list(const std::vector<DynamicParamsInfo>& info) { servers_list = info; }

	/// <summary>
	/// clear device list
	/// </summary>
	void clear_device_list() { devices_list.clear(); }

	/// <summary>
	/// returns list of device variants
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	virtual void fill_devices_list(TemplateParams& params) {}

	/// <summary>
	/// set device
	/// </summary>
	/// <param name="params">Template parameters.</param>
	virtual bool set_device(TemplateParams& /*params*/) { return true; }

	/// <summary>
	/// property list of quality variants
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	virtual const std::vector<DynamicParamsInfo>& get_devices_list() { return devices_list; }
	virtual void set_devices_list(const std::vector<DynamicParamsInfo>& info) { devices_list = info; }

	/// <summary>
	/// clear qualities list
	/// </summary>
	void clear_qualities_list() { qualities_list.clear(); }

	/// <summary>
	/// fill list of quality variants
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	virtual void fill_qualities_list(TemplateParams& params) {}

	/// <summary>
	/// set quality
	/// </summary>
	/// <param name="params">Template parameters.</param>
	virtual bool set_quality(TemplateParams& /*params*/) { return true; }

	/// <summary>
	/// property list of quality variants
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	virtual const std::vector<DynamicParamsInfo>& get_qualities_list() { return qualities_list; }
	virtual void set_qualities_list(const std::vector<DynamicParamsInfo>& info) { qualities_list = info; }

	/// <summary>
	/// clear profile list
	/// </summary>
	void clear_profiles_list() { profiles_list.clear(); }

	/// <summary>
	/// fill list of profiles variants
	/// </summary>
	virtual void fill_profiles_list(TemplateParams& params) {}

	/// <summary>
	/// set profile
	/// </summary>
	/// <param name="params">Template parameters.</param>
	virtual bool set_profile(TemplateParams& /*params*/) { return true; }

	/// <summary>
	/// property list of profiles
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	virtual const std::vector<DynamicParamsInfo>& get_profiles_list() { return profiles_list; }
	virtual void set_profiles_list(const std::vector<DynamicParamsInfo>& info) { profiles_list = info; }

	/// <summary>
	/// set profile
	/// </summary>
	/// <param name="params">Template parameters.</param>
	virtual bool set_domain(TemplateParams& /*params*/) { return true; }

	/// <summary>
	/// fill list of domains variants
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	virtual void fill_domains_list(TemplateParams& /*params*/) {}

	/// <summary>
	/// property list of domains
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	virtual const std::vector<DynamicParamsInfo>& get_domains_list() { return domains_list; }
	virtual void set_domains_list(const std::vector<DynamicParamsInfo>& info) { domains_list = info; }

	static void to_json_wrapper(nlohmann::json& j, const plugin_config& c)
	{
		to_json(j, c);
	}

	friend void to_json(nlohmann::json& j, const plugin_config& c)
	{
		SERIALIZE_STRUCT(j, c, enabled); //-V601
		SERIALIZE_STRUCT(j, c, custom); //-V601
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
		SERIALIZE_STRUCT(j, c, vod_season); //-V601
		SERIALIZE_STRUCT(j, c, vod_server_filter); //-V601
		SERIALIZE_STRUCT(j, c, vod_filter); //-V601
		SERIALIZE_STRUCT(j, c, vod_filters);
		SERIALIZE_STRUCT(j, c, vod_quality); //-V601
		SERIALIZE_STRUCT(j, c, vod_audio); //-V601
		SERIALIZE_STRUCT(j, c, vod_templates);
		SERIALIZE_STRUCT(j, c, vod_template_index);
		SERIALIZE_STRUCT(j, c, balance_support); //-V601
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

	static void from_json_wrapper(const nlohmann::json& j, plugin_config& c)
	{
		from_json(j, c);
	}

	friend void from_json(const nlohmann::json& j, plugin_config& c)
	{
		DESERIALIZE_STRUCT(j, c, enabled);
		DESERIALIZE_STRUCT(j, c, custom);
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
		DESERIALIZE_STRUCT(j, c, vod_season);
		DESERIALIZE_STRUCT(j, c, vod_server_filter);
		DESERIALIZE_STRUCT(j, c, vod_filter);
		DESERIALIZE_STRUCT(j, c, vod_filters);
		DESERIALIZE_STRUCT(j, c, vod_quality);
		DESERIALIZE_STRUCT(j, c, vod_audio);
		DESERIALIZE_STRUCT(j, c, vod_templates);
		DESERIALIZE_STRUCT(j, c, vod_template_index);
		DESERIALIZE_STRUCT(j, c, balance_support);
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

protected:

	bool download_url(const std::wstring& url,
					  std::stringstream& vData,
					  int cache_ttl = 0,
					  std::vector<std::string>* pHeaders = nullptr,
					  bool verb_post = false,
					  const char* post_data = nullptr);

	utils::CUrlDownload m_dl;

	// non configurable parameters
	std::vector<DynamicParamsInfo> internal_epg_urls;

	// configurable parameters

	// name of specialized php class for plugin
	std::string class_name;

	// plugin access type
	AccountAccessType access_type = AccountAccessType::enNone;
	// plugin title
	std::string title;
	// plugin user-agent
	std::string user_agent;
	// plugin internal name (used by Dune)
	std::string name;
	// url to provider account
	std::string provider_url;
	// developer url
	std::string dev_code;

	// provider api url
	std::string provider_api_url;

	// vod engine
	VodEngine vod_engine = VodEngine::enNone;

	bool enabled = true;
	bool custom = false;
	// show balance info in plugin
	bool balance_support = false;
	bool vod_season = false;
	bool vod_server_filter = false;
	bool vod_filter = false;
	bool vod_quality = false;
	bool vod_audio = false;
	// flag for php plugin if uri does not contains parsed 'id' for channel
	bool static_servers = false;
	bool static_qualities = false;
	bool static_devices = false;
	bool static_profiles = false;
	bool static_domains = false;

	std::vector<std::string> vod_filters;

	// selected playlist template
	size_t playlist_template_index = 0;
	// available playlist templates
	std::vector<PlaylistTemplateInfo> playlist_templates;
	// selected vod template
	size_t vod_template_index = 0;
	// available vod templates
	std::vector<PlaylistTemplateInfo> vod_templates;
	// setting for parsing uri streams
	std::array<StreamParameters, 2> streams_config;
	// setting for parsing json EPG
	std::array<EpgParameters, 2> epg_params;
	std::vector<DynamicParamsInfo> files_list;
	std::vector<DynamicParamsInfo> manifest_list;
	std::vector<DynamicParamsInfo> servers_list;
	std::vector<DynamicParamsInfo> qualities_list;
	std::vector<DynamicParamsInfo> devices_list;
	std::vector<DynamicParamsInfo> profiles_list;
	std::vector<DynamicParamsInfo> domains_list;
	std::vector<DynamicParamsInfo> custom_epg_urls;
};
