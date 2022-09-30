#pragma once
#include "Config.h"
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
	{ StreamType::enHLS, "hls" },
	{ StreamType::enMPEGTS, "mpeg" }
})

enum class CatchupType {
	cu_shift,
	cu_append,
	cu_flussonic,
};

NLOHMANN_JSON_SERIALIZE_ENUM(CatchupType,
{
	{ CatchupType::cu_shift, "shift" },
	{ CatchupType::cu_append, "append" },
	{ CatchupType::cu_flussonic, "flussonic" }
})

enum class AccountAccessType
{
	enUnknown = -1,
	enOtt,
	enPin,
	enLoginPass
};

NLOHMANN_JSON_SERIALIZE_ENUM(AccountAccessType,
{
	{ AccountAccessType::enUnknown, "unknown" },
	{ AccountAccessType::enOtt, "ottkey" },
	{ AccountAccessType::enPin, "pin" },
	{ AccountAccessType::enLoginPass, "login" }
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
	int number = 0;
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

protected:
	std::string id;
	std::string name;
};

/// <summary>
/// Parameters to parse EPG
/// </summary>
struct EpgParameters
{
	std::string epg_param;
	std::string epg_url;
	std::string epg_root;
	std::string epg_name;
	std::string epg_desc;
	std::string epg_start;
	std::string epg_end;
	std::string epg_date_format;
	std::string epg_time_format;
	size_t epg_timezone = 0;

	// not saved to the config!
	bool epg_use_mapper = false;
	bool epg_use_duration = false;
	std::wstring epg_mapper_url;
	std::map<std::wstring, std::wstring> epg_mapper;

	std::wstring get_epg_url() const { return utils::utf8_to_utf16(epg_url); }
	std::wstring get_epg_date_format() const { return utils::utf8_to_utf16(epg_date_format); }

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(EpgParameters, epg_param, epg_url, epg_date_format, epg_root, epg_name, epg_desc, epg_start, epg_end, epg_time_format, epg_timezone);
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
	std::string cu_subst;

	std::wstring get_uri_template() const { return utils::utf8_to_utf16(uri_template); }
	std::wstring get_uri_arc_template() const { return utils::utf8_to_utf16(uri_arc_template); }
	std::wstring get_shift_replace() const { return utils::utf8_to_utf16(cu_subst); }

	void set_uri_template(const std::wstring& value) { uri_template = utils::utf16_to_utf8(value); }
	void set_uri_arc_template(const std::wstring& value) { uri_arc_template = utils::utf16_to_utf8(value); }
	void set_shift_replace(const std::wstring& value) { cu_subst = utils::utf16_to_utf8(value); }

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(StreamParameters, stream_type, uri_template, uri_arc_template, cu_type, cu_subst, cu_duration);
};

class uri_config
{
public:
	uri_config();

public:
	/// <summary>
	/// returns plugin title
	/// </summary>
	std::wstring get_title() const { return utils::utf8_to_utf16(title); }

	/// <summary>
	/// set plugin title
	/// </summary>
	void set_title(const std::wstring& val) { title = utils::utf16_to_utf8(val); }

	/// <summary>
	/// returns plugin name
	/// </summary>
	const std::string& get_name() const { return name; }

	/// <summary>
	/// set plugin name
	/// </summary>
	void set_name(const std::wstring& val) { name = utils::utf16_to_utf8(val); }

	/// <summary>
	/// returns plugin access type
	/// </summary>
	const AccountAccessType get_access_type() const { return access_type; }

	/// <summary>
	/// returns link to provider account
	/// </summary>
	/// <returns>wstring</returns>
	std::wstring get_provider_url() const { return utils::utf8_to_utf16(provider_url); }

	/// <summary>
	/// set link to provider account
	/// </summary>
	/// <returns>wstring</returns>
	void set_provider_url(const std::wstring& val) { provider_url = utils::utf16_to_utf8(val); }

	/// <summary>
	/// get playlist template
	/// </summary>
	/// <returns>wstring</returns>
	std::wstring get_playlist_template() const { return utils::utf8_to_utf16(playlist_template); }

	/// <summary>
	/// set playlist template
	/// </summary>
	/// <returns>wstring</returns>
	void set_playlist_template(const std::wstring& val) { playlist_template = utils::utf16_to_utf8(val); }

	/// <summary>
	/// returns uri parse template
	/// </summary>
	/// <returns>wstring</returns>
	std::wstring get_uri_parse_pattern() const { return utils::utf8_to_utf16(uri_parse_pattern); }

	/// <summary>
	/// set uri parse template.
	/// returns uri parse template
	/// </summary>
	/// <returns>wstring</returns>
	void set_uri_parse_pattern(const std::wstring& val) { uri_parse_pattern = utils::utf16_to_utf8(val); }

	/// <summary>
	/// returns uri id parse template
	/// </summary>
	/// <returns>wstring</returns>
	std::wstring get_uri_id_parse_pattern() const { return utils::utf8_to_utf16(uri_id_parse_pattern); }

	/// <summary>
	/// set uri parse template.
	/// returns uri id parse template
	/// </summary>
	/// <returns>wstring</returns>
	void set_uri_id_parse_pattern(const std::wstring& val) { uri_id_parse_pattern = utils::utf16_to_utf8(val); }

	/// <summary>
	/// is icons square, php GUI setting
	/// </summary>
	/// <returns>bool</returns>
	bool is_square_icons() const { return square_icons; }

	/// <summary>
	/// is token used per channel, not the global
	/// </summary>
	/// <returns>bool</returns>
	bool is_per_channel_token() const { return per_channel_token; }

	/// <summary>
	/// is token used per channel, not the global
	/// </summary>
	/// <returns>bool</returns>
	bool is_requested_token() const { return per_channel_token; }

	/// <summary>
	/// supported streams HLS,MPEGTS etc.
	/// </summary>
	/// <returns>vector&</returns>
	const std::array<StreamParameters, 2>& get_supported_streams() const { return streams_config; }

	/// <summary>
	/// return supported stream
	/// </summary>
	/// <returns>const StreamParameters&</returns>
	const StreamParameters& get_supported_stream(size_t idx) const { return streams_config[idx]; }

	/// <summary>
	/// return epg parameters
	/// </summary>
	/// <returns>EpgParameters</returns>
	EpgParameters& get_epg_parameters(int idx) { return epg_params[idx]; };
	const EpgParameters& get_epg_parameters(int idx) const { return epg_params[idx]; };

	/// <summary>
	/// returns json root for epg iteration
	/// </summary>
	/// <param name="epg_idx">index of epg, primary/secondary</param>
	/// <param name="epg_data">downloaded json</param>
	/// <returns>json entry pointed to epg list</returns>
	nlohmann::json get_epg_root(int epg_idx, const nlohmann::json& epg_data) const;

	/// <summary>
	/// is servers list static, not loaded from provider settings
	/// </summary>
	/// <returns>bool</returns>
	bool is_static_servers() const { return static_servers; }

	/// <summary>
	/// is devices list static, not loaded from provider settings
	/// </summary>
	/// <returns>bool</returns>
	bool is_static_devices() const { return static_devices; }

	/// <summary>
	/// is qualities list static, not loaded from provider settings
	/// </summary>
	/// <returns>bool</returns>
	bool is_static_qualities() const { return static_qualities; }

	/// <summary>
	/// is profiles list static, not loaded from provider settings
	/// </summary>
	/// <returns>bool</returns>
	bool is_static_profiles() const { return static_profiles; }

	/// <summary>
	/// returns list of servers
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	/// <returns>vector<ServersInfo></returns>
	virtual void fill_servers_list(TemplateParams& /*params*/) {}

	/// <summary>
	/// returns list of servers
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	/// <returns>vector<ServersInfo></returns>
	virtual const std::vector<DynamicParamsInfo>& get_servers_list() { return servers_list; }

	/// <summary>
	/// set list of servers
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	/// <returns>vector<ServersInfo></returns>
	virtual void set_servers_list(const std::vector<DynamicParamsInfo>& info) { servers_list = info; }

	/// <summary>
	/// set server
	/// </summary>
	/// <param name="params">Template parameters.</param>
	virtual bool set_server(TemplateParams& /*params*/) { return true; }

	/// <summary>
	/// returns list of device variants
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	/// <returns>vector<QualityInfo></returns>
	virtual void fill_devices_list(TemplateParams& /*params*/) {}

	/// <summary>
	/// returns list of quality variants
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	/// <returns>vector<ServersInfo></returns>
	virtual const std::vector<DynamicParamsInfo>& get_devices_list() { return devices_list; }

	/// <summary>
	/// set list of quality variants
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	/// <returns>vector<QualityInfo></returns>
	virtual void set_devices_list(const std::vector<DynamicParamsInfo>& info) { devices_list = info; }

	/// <summary>
	/// returns list of quality variants
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	/// <returns>vector<QualityInfo></returns>
	virtual void fill_qualities_list(TemplateParams& /*params*/) {}

	/// <summary>
	/// returns list of quality variants
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	/// <returns>vector<ServersInfo></returns>
	virtual const std::vector<DynamicParamsInfo>& get_qualities_list() { return qualities_list; }

	/// <summary>
	/// set list of quality variants
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	/// <returns>vector<QualityInfo></returns>
	virtual void set_qualities_list(const std::vector<DynamicParamsInfo>& info) { qualities_list = info; }

	/// <summary>
	/// set quality
	/// </summary>
	/// <param name="params">Template parameters.</param>
	virtual bool set_quality(TemplateParams& /*params*/) { return true; }

	/// <summary>
	/// returns list of profiles variants
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	/// <returns>vector<QualityInfo></returns>
	virtual void fill_profiles_list(TemplateParams& /*params*/) {}

	/// <summary>
	/// returns list of profiles
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	/// <returns>vector<ProfilesInfo></returns>
	virtual const std::vector<DynamicParamsInfo>& get_profiles_list() { return profiles_list; }

	/// <summary>
	/// set list of profiles variants
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	/// <returns>vector<QualityInfo></returns>
	virtual void set_profiles_list(const std::vector<DynamicParamsInfo>& info) { profiles_list = info; }

	/// <summary>
	/// set profile
	/// </summary>
	/// <param name="params">Template parameters.</param>
	virtual bool set_profile(TemplateParams& /*params*/) { return true; }

protected:

	// configurable parameters

	// plugin access type
	AccountAccessType access_type = AccountAccessType::enOtt;
	// plugin title
	std::string title;
	// plugin internal name (used by Dune)
	std::string name;
	// url to provider account
	std::string provider_url;
	// template url to load playlist
	std::string playlist_template;
	// original uri id parse template
	std::string uri_id_parse_pattern;
	// original uri parse template
	std::string uri_parse_pattern;

	// use channels logo are squared, plugin UI settings
	bool square_icons = false;
	// use token from uri instead of account settings
	bool per_channel_token = false;
	// use token generated or received from provider
	bool requested_token = false;
	// flag for php plugin if uri does not contains parsed 'id' for channel
	bool use_token_as_id = false;
	bool static_servers = false;
	bool static_qualities = false;
	bool static_devices = false;
	bool static_profiles = false;
	// setting for parsing uri streams
	std::array<StreamParameters, 2> streams_config;
	// setting for parsing json EPG
	std::array<EpgParameters, 2> epg_params;
	std::vector<DynamicParamsInfo> servers_list;
	std::vector<DynamicParamsInfo> qualities_list;
	std::vector<DynamicParamsInfo> devices_list;
	std::vector<DynamicParamsInfo> profiles_list;
};
