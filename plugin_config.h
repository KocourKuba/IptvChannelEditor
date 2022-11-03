#pragma once
#include "PluginDefines.h"
#include "IPTVChannelEditor.h"
#include "UtilsLib\json_wrapper.h"

namespace s_enum
{
enum class StreamType
{
	enHLS = 0,
	enMPEGTS,
};

NLOHMANN_JSON_SERIALIZE_ENUM(StreamType,
{
	{ StreamType::enHLS,    "hls"  },
	{ StreamType::enMPEGTS, "mpeg" }
})

enum class CatchupType {
	cu_shift,
	cu_append,
	cu_flussonic,
};

NLOHMANN_JSON_SERIALIZE_ENUM(CatchupType,
{
	{ CatchupType::cu_shift,     "shift"     },
	{ CatchupType::cu_append,    "append"    },
	{ CatchupType::cu_flussonic, "flussonic" }
})

enum class AccountAccessType
{
	enUnknown = -1,
	enOtt,
	enPin,
	enLoginPass,
	enNone,
};

NLOHMANN_JSON_SERIALIZE_ENUM(AccountAccessType,
{
	{ AccountAccessType::enUnknown,   "unknown" },
	{ AccountAccessType::enOtt,       "ottkey"  },
	{ AccountAccessType::enPin,       "pin"     },
	{ AccountAccessType::enLoginPass, "login"   },
	{ AccountAccessType::enNone,      "none"    }
})

}
using namespace s_enum;

struct TemplateParams
{
	StreamType streamSubtype = StreamType::enHLS;
	std::wstring subdomain;
	std::wstring port;
	std::wstring token;
	std::wstring login;
	std::wstring password;
	std::wstring host;
	std::wstring catchup_source;
	std::wstring catchup_template;
	std::wstring server_id;
	std::wstring device_id;
	std::wstring profile_id;
	std::wstring command;
	std::wstring command_param;
	int shift_back = 0;
	int playlist_idx = 0;
	int server_idx = 0;
	int device_idx = 0;
	int profile_idx = 0;
	int quality_idx = 0;
};

struct DynamicParamsInfo
{
public:
	DynamicParamsInfo() = default;
	DynamicParamsInfo(const std::string& _id, const std::string& _name) : id(_id), name(_name) {}

	std::wstring get_id() const { return utils::utf8_to_utf16(id); }
	void set_id(const std::wstring& val) { id = utils::utf16_to_utf8(val); }

	std::wstring get_name() const { return utils::utf8_to_utf16(name); }
	void set_name(const std::wstring& val) { name = utils::utf16_to_utf8(val); }

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(DynamicParamsInfo, id, name);

	std::string id;
	std::string name;
};

/// <summary>
/// Playlist template parameter
/// </summary>
class PlaylistTemplateInfo
{
public:
	PlaylistTemplateInfo() = default;
	PlaylistTemplateInfo(UINT ID)
	{
		set_name(load_string_resource(ID));
	}

	PlaylistTemplateInfo(UINT ID, const std::string& _pl_template) : pl_template(_pl_template)
	{
		set_name(load_string_resource(ID));
	}

	PlaylistTemplateInfo(const std::string& _name, const std::string& _pl_template) : name(_name), pl_template(_pl_template) {}

	std::wstring get_name() const { return utils::utf8_to_utf16(name); }
	void set_name(const std::wstring& val) { name = utils::utf16_to_utf8(val); }
	void set_name(UINT ID) { name = utils::utf16_to_utf8(load_string_resource(ID)); }

	std::wstring get_template() const { return utils::utf8_to_utf16(pl_template); }
	void set_template(const std::wstring& val) { pl_template = utils::utf16_to_utf8(val); }

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(PlaylistTemplateInfo, name, pl_template);

	std::string name;
	std::string pl_template;
	bool is_custom = false;
};

/// <summary>
/// Parameters to parse EPG
/// </summary>
struct EpgParameters
{
	std::string epg_param; // not changed! hardcoded
	std::string epg_url;
	std::string epg_root;
	std::string epg_name;
	std::string epg_desc;
	std::string epg_start;
	std::string epg_end;
	std::string epg_date_format;
	std::string epg_time_format;
	size_t epg_timezone = 0;
	bool epg_use_duration = false;

	// not saved to the config!
	bool epg_use_mapper = false;
	std::wstring epg_mapper_url;
	std::map<std::wstring, std::wstring> epg_mapper;

	std::wstring get_epg_url() const { return utils::utf8_to_utf16(epg_url); }
	void set_epg_url(const std::wstring& val) { epg_url = utils::utf16_to_utf8(val); }

	std::wstring get_epg_root() const { return utils::utf8_to_utf16(epg_root); }
	void set_epg_root(const std::wstring& val) { epg_root = utils::utf16_to_utf8(val); }

	std::wstring get_epg_name() const { return utils::utf8_to_utf16(epg_name); }
	void set_epg_name(const std::wstring& val) { epg_name = utils::utf16_to_utf8(val); }

	std::wstring get_epg_desc() const { return utils::utf8_to_utf16(epg_desc); }
	void set_epg_desc(const std::wstring& val) { epg_desc = utils::utf16_to_utf8(val); }

	std::wstring get_epg_start() const { return utils::utf8_to_utf16(epg_start); }
	void set_epg_start(const std::wstring& val) { epg_start = utils::utf16_to_utf8(val); }

	std::wstring get_epg_end() const { return utils::utf8_to_utf16(epg_end); }
	void set_epg_end(const std::wstring& val) { epg_end = utils::utf16_to_utf8(val); }

	std::wstring get_epg_date_format() const { return utils::utf8_to_utf16(epg_date_format); }
	void set_epg_date_format(const std::wstring& val) { epg_date_format = utils::utf16_to_utf8(val); }

	std::wstring get_epg_time_format() const { return utils::utf8_to_utf16(epg_time_format); }
	void set_epg_time_format(const std::wstring& val) { epg_time_format = utils::utf16_to_utf8(val); }

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(EpgParameters, epg_param, epg_url, epg_root, epg_name, epg_desc, epg_start, epg_end, epg_date_format, epg_time_format, epg_timezone, epg_use_duration); //-V601
};

/// <summary>
/// Catchup parameters to generate online and archive streams
/// </summary>
struct StreamParameters
{
	StreamType stream_type = StreamType::enHLS;
	CatchupType cu_type = CatchupType::cu_shift;
	int cu_duration = 10800;

	std::string uri_template;
	std::string uri_arc_template;
	std::string uri_custom_arc_template;
	std::string cu_subst;

	std::wstring get_stream_template() const { return utils::utf8_to_utf16(uri_template); }
	std::wstring get_stream_arc_template() const { return utils::utf8_to_utf16(uri_arc_template); }
	std::wstring get_custom_stream_arc_template() const { return utils::utf8_to_utf16(uri_custom_arc_template); }
	std::wstring get_shift_replace() const { return utils::utf8_to_utf16(cu_subst); }

	void set_uri_template(const std::wstring& value) { uri_template = utils::utf16_to_utf8(value); }
	void set_uri_arc_template(const std::wstring& value) { uri_arc_template = utils::utf16_to_utf8(value); }
	void set_uri_custom_arc_template(const std::wstring& value) { uri_custom_arc_template = utils::utf16_to_utf8(value); }
	void set_shift_replace(const std::wstring& value) { cu_subst = utils::utf16_to_utf8(value); }

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(StreamParameters, stream_type, uri_template, uri_arc_template, uri_custom_arc_template, cu_type, cu_subst, cu_duration);
};

class plugin_config
{
public:
	plugin_config() = default;

protected:
	/// <summary>
	/// load default settings
	/// </summary>
	virtual void load_default() {}

public:
	/// <summary>
	/// clear all parameters
	/// </summary>
	virtual void clear();

	/// <summary>
	/// save plugin parameters to file
	/// </summary>
	virtual bool save_plugin_parameters(const std::wstring& filename, bool use_full_path = false);

	/// <summary>
	/// load plugin parameters to file
	/// </summary>
	virtual void load_plugin_parameters(const std::wstring& filename);

	/// <summary>
	/// load default plugin parameters and set default values for templates
	/// </summary>
	void set_plugin_defaults(PluginType val);

	/// <summary>
	/// plugin type
	/// </summary>
	PluginType get_plugin_type() const { return plugin_type; }

	/// <summary>
	/// plugin short name
	/// </summary>
	const std::string& get_short_name() const { return short_name; }
	void set_short_name(const std::string& val) { short_name = val; }

	/// <summary>
	/// plugin short name wide char
	/// </summary>
	std::wstring get_short_name_w() const { return utils::utf8_to_utf16(short_name); }
	void set_short_name_w(const std::wstring& val) { short_name = utils::utf16_to_utf8(val); }

	/// <summary>
	/// returns link to provider api url
	/// </summary>
	/// <returns>wstring</returns>
	const std::wstring& get_provider_api_url() const { return provider_api_url; }

	/// <summary>
	/// property plugin title
	/// </summary>
	std::wstring get_title() const { return utils::utf8_to_utf16(title); }
	void set_title(const std::wstring& val) { title = utils::utf16_to_utf8(val); }

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
	/// active vod template
	/// </summary>
	void set_current_pl_vod_template();

	/// <summary>
	/// selected playlist template index
	/// </summary>
	size_t get_playlist_template_idx() const { return playlist_template_index; }
	void set_playlist_template_idx(size_t idx) { playlist_template_index = idx; }

	/// <summary>
	/// property playlist template
	/// </summary>
	std::wstring get_playlist_template(int idx) const { return (idx != -1 && idx < (int)playlist_templates.size()) ? playlist_templates[idx].get_template() : L""; }
	void set_playlist_template(int idx, const std::wstring& val) { if ((idx != -1 && idx < (int)playlist_templates.size())) playlist_templates[idx].set_template(val); }

	/// <summary>
	/// property playlist templates
	/// </summary>
	const std::vector <PlaylistTemplateInfo>& get_playlist_templates() const { return playlist_templates; }
	void set_playlist_templates(const std::vector<PlaylistTemplateInfo>& val) { playlist_templates = val; }

	/// <summary>
	/// property uri parse template
	/// </summary>
	std::wstring get_uri_parse_pattern() const { return utils::utf8_to_utf16(uri_parse_pattern); }
	void set_uri_parse_pattern(const std::wstring& val) { uri_parse_pattern = utils::utf16_to_utf8(val); }

	/// <summary>
	/// property uri id parse template
	/// </summary>
	std::wstring get_tag_id_match() const { return utils::utf8_to_utf16(tag_id_match); }
	void set_tag_id_match(const std::wstring& val) { tag_id_match = utils::utf16_to_utf8(val); }

	/// <summary>
	/// plugin supports vod
	/// </summary>
	/// <returns>bool</returns>
	bool get_vod_support() const { return vod_support; }
	void set_vod_support(bool val) { vod_support = val; }

	/// <summary>
	/// vod m3u based
	/// </summary>
	/// <returns>bool</returns>
	bool get_vod_m3u() const { return vod_m3u; }
	void set_vod_m3u(bool val) { vod_m3u = val; }

	/// <summary>
	/// active vod template
	/// </summary>
	std::wstring get_current_vod_template() const { return utils::utf8_to_utf16(provider_vod_url); }

	/// <summary>
	/// property vod templates
	/// </summary>
	const std::vector <PlaylistTemplateInfo>& get_vod_templates() const { return vod_templates; }
	void set_vod_templates(const std::vector<PlaylistTemplateInfo>& val) { vod_templates = val; }

	/// <summary>
	/// selected vod template index
	/// </summary>
	size_t get_vod_template_idx() const { return vod_template_index; }
	void set_vod_template_idx(size_t idx) { vod_template_index = idx; }

	/// <summary>
	/// vod url template
	/// </summary>
	/// <returns>wstring</returns>
	std::wstring get_vod_template(int idx) const { return (idx != -1 && idx < (int)vod_templates.size()) ? vod_templates[idx].get_template() : L""; }
	void set_vod_template(int idx, const std::wstring& val) { if ((idx != -1 && idx < (int)vod_templates.size())) vod_templates[idx].set_template(val); }

	/// <summary>
	/// regex for parsing title
	/// </summary>
	/// <returns>wstring</returns>
	std::wstring get_vod_parse_pattern() const { return utils::utf8_to_utf16(vod_parse_pattern); };
	void set_vod_parse_regex(const std::wstring& val) { vod_parse_pattern = utils::utf16_to_utf8(val); };

	/// <summary>
	/// property square icons, php GUI setting
	/// </summary>
	bool get_square_icons() const { return square_icons; }
	void set_square_icons(bool val) { square_icons = val; }

	/// <summary>
	/// property token used per channel, not the global
	/// </summary>
	bool get_per_channel_token() const { return per_channel_token; }
	void set_per_channel_token(bool val) { per_channel_token = val; }

	/// <summary>
	/// property token requested from provider
	/// </summary>
	bool get_requested_token() const { return requested_token; }
	void set_requested_token(bool val) { requested_token = val; }

	/// <summary>
	/// property supported streams HLS,MPEGTS etc.
	/// </summary>
	/// <returns>vector&</returns>
	const std::array<StreamParameters, 2>& get_supported_streams() const { return streams_config; }

	/// <summary>
	/// property index of supported stream
	/// </summary>
	const StreamParameters& get_supported_stream(size_t idx) const { return streams_config[idx]; }
	void set_supported_stream(size_t idx, const StreamParameters& val) { streams_config[idx] = val; }

	/// <summary>
	/// return supported stream
	/// </summary>
	/// <returns>const StreamParameters&</returns>
	const std::array<EpgParameters, 2>& get_epg_parameters() const { return epg_params; }

	/// <summary>
	/// property index pf epg parameters
	/// </summary>
	const EpgParameters& get_epg_parameter(int idx) const { return epg_params[idx]; };
	void set_epg_parameter(int idx, const EpgParameters& val) { epg_params[idx] = val; };

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
	/// returns list of servers
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	virtual void fill_servers_list(TemplateParams& /*params*/) {}

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
	/// returns list of device variants
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	/// <returns>vector<QualityInfo></returns>
	virtual void fill_devices_list(TemplateParams& /*params*/) {}

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
	/// returns list of quality variants
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	/// <returns>vector<QualityInfo></returns>
	virtual void fill_qualities_list(TemplateParams& /*params*/) {}

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
	/// returns list of profiles variants
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	/// <returns>vector<QualityInfo></returns>
	virtual void fill_profiles_list(TemplateParams& /*params*/) {}

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

	friend void to_json(nlohmann::json& j, const plugin_config& c)
	{
		SERIALIZE_STRUCT(j, c, access_type);
		SERIALIZE_STRUCT(j, c, short_name);
		SERIALIZE_STRUCT(j, c, title);
		SERIALIZE_STRUCT(j, c, name);
		SERIALIZE_STRUCT(j, c, provider_url);
		SERIALIZE_STRUCT(j, c, playlist_templates);
		SERIALIZE_STRUCT(j, c, playlist_template_index);
		SERIALIZE_STRUCT(j, c, playlist_template);
		SERIALIZE_STRUCT(j, c, tag_id_match);
		SERIALIZE_STRUCT(j, c, uri_parse_pattern);
		SERIALIZE_STRUCT(j, c, vod_support); //-V601
		SERIALIZE_STRUCT(j, c, vod_m3u); //-V601
		SERIALIZE_STRUCT(j, c, vod_templates);
		SERIALIZE_STRUCT(j, c, vod_template_index);
		SERIALIZE_STRUCT(j, c, provider_vod_url);
		SERIALIZE_STRUCT(j, c, vod_parse_pattern);
		SERIALIZE_STRUCT(j, c, square_icons); //-V601
		SERIALIZE_STRUCT(j, c, per_channel_token); //-V601
		SERIALIZE_STRUCT(j, c, requested_token); //-V601
		SERIALIZE_STRUCT(j, c, static_servers); //-V601
		SERIALIZE_STRUCT(j, c, static_qualities); //-V601
		SERIALIZE_STRUCT(j, c, static_devices); //-V601
		SERIALIZE_STRUCT(j, c, static_profiles); //-V601
		SERIALIZE_STRUCT(j, c, streams_config);
		SERIALIZE_STRUCT(j, c, epg_params);
		SERIALIZE_STRUCT(j, c, servers_list);
		SERIALIZE_STRUCT(j, c, qualities_list);
		SERIALIZE_STRUCT(j, c, devices_list);
		SERIALIZE_STRUCT(j, c, profiles_list);
	}

	friend void from_json(const nlohmann::json& j, plugin_config& c)
	{
		DESERIALIZE_STRUCT(j, c, access_type);
		DESERIALIZE_STRUCT(j, c, short_name);
		DESERIALIZE_STRUCT(j, c, title);
		DESERIALIZE_STRUCT(j, c, name);
		DESERIALIZE_STRUCT(j, c, provider_url);
		DESERIALIZE_STRUCT(j, c, playlist_templates);
		DESERIALIZE_STRUCT(j, c, playlist_template_index);
		DESERIALIZE_STRUCT(j, c, playlist_template);
		DESERIALIZE_STRUCT(j, c, tag_id_match);
		DESERIALIZE_STRUCT(j, c, uri_parse_pattern);
		DESERIALIZE_STRUCT(j, c, vod_support);
		DESERIALIZE_STRUCT(j, c, vod_m3u);
		DESERIALIZE_STRUCT(j, c, vod_templates);
		DESERIALIZE_STRUCT(j, c, vod_template_index);
		DESERIALIZE_STRUCT(j, c, provider_vod_url);
		DESERIALIZE_STRUCT(j, c, vod_parse_pattern);
		DESERIALIZE_STRUCT(j, c, square_icons);
		DESERIALIZE_STRUCT(j, c, per_channel_token);
		DESERIALIZE_STRUCT(j, c, requested_token);
		DESERIALIZE_STRUCT(j, c, static_servers);
		DESERIALIZE_STRUCT(j, c, static_qualities);
		DESERIALIZE_STRUCT(j, c, static_devices);
		DESERIALIZE_STRUCT(j, c, static_profiles);
		DESERIALIZE_STRUCT(j, c, streams_config);
		DESERIALIZE_STRUCT(j, c, epg_params);
		DESERIALIZE_STRUCT(j, c, servers_list);
		DESERIALIZE_STRUCT(j, c, qualities_list);
		DESERIALIZE_STRUCT(j, c, devices_list);
		DESERIALIZE_STRUCT(j, c, profiles_list);
	}

protected:

	// non configurable parameters
	PluginType plugin_type = PluginType::enCustom;
	std::string short_name;

	std::wstring provider_api_url;

	// configurable parameters

	// plugin access type
	AccountAccessType access_type = AccountAccessType::enNone;
	// plugin title
	std::string title;
	// plugin internal name (used by Dune)
	std::string name;
	// url to provider account
	std::string provider_url;
	// template url to load playlist
	std::string playlist_template;
	// tag to match to detect id
	std::string tag_id_match;
	// original uri parse template
	std::string uri_parse_pattern;

	// enable vod
	bool vod_support = false;
	// vod based on m3u8 playlist
	bool vod_m3u = false;
	// url to vod playlist
	std::string provider_vod_url;
	// vod title parse regex
	std::string vod_parse_pattern;

	// use channels logo are squared, plugin UI settings
	bool square_icons = false;
	// use token from uri instead of account settings
	bool per_channel_token = false;
	// use token generated or received from provider
	bool requested_token = false;
	// flag for php plugin if uri does not contains parsed 'id' for channel
	bool static_servers = false;
	bool static_qualities = false;
	bool static_devices = false;
	bool static_profiles = false;
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
	std::vector<DynamicParamsInfo> servers_list;
	std::vector<DynamicParamsInfo> qualities_list;
	std::vector<DynamicParamsInfo> devices_list;
	std::vector<DynamicParamsInfo> profiles_list;
};
