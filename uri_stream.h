/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2022): sharky72 (https://github.com/KocourKuba)

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
#include "uri_base.h"
#include "Config.h"
#include "UtilsLib\json_wrapper.h"

class Credentials;

enum class CatchupType {
	cu_shift,
	cu_append,
	cu_flussonic,
};

enum class ServerSubstType {
	enNone,
	enStream,
	enPlaylist
};

enum class AccountAccessType
{
	enUnknown = -1,
	enOtt,
	enPin,
	enLoginPass
};

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
	std::wstring profile_id;
	std::wstring command;
	std::wstring command_param;
	int shift_back = 0;
	int number = 0;
	int server = 0;
	int profile = 0;
	int quality = 0;
};

struct PlaylistInfo
{
	std::wstring name;
	std::wstring id;
	bool is_default = false;
	bool is_file = false;
};

struct AccountInfo
{
	std::wstring name;
	std::wstring value;
};

struct ServerParamsInfo
{
	std::wstring id;
	std::wstring name;
};

using ServersInfo = ServerParamsInfo;
using ProfilesInfo = ServerParamsInfo;
using QualityInfo = ServerParamsInfo;

struct EpgInfo
{
	time_t time_start = 0;
	time_t time_end = 0;
	std::string name;
	std::string desc;
#ifdef _DEBUG
	std::wstring start;
	std::wstring end;
#endif // _DEBUG
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
/// Parsed variable groups for generate playing stream
/// </summary>
struct ParsingGroups
{
	std::wstring id;
	std::wstring domain;
	std::wstring port;
	std::wstring login;
	std::wstring password;
	std::wstring subdomain;
	std::wstring token;
	std::wstring int_id;
	std::wstring quality;
	std::wstring host;
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

	friend void to_json(nlohmann::json& j, const StreamParameters& s)
	{
		SERIALIZE_STRUCT(j, s, stream_type);
		SERIALIZE_STRUCT(j, s, uri_template);
		SERIALIZE_STRUCT(j, s, uri_arc_template);
		SERIALIZE_STRUCT(j, s, cu_type);
		SERIALIZE_STRUCT(j, s, cu_subst);
		SERIALIZE_STRUCT(j, s, cu_duration);
	}

	friend void from_json(const nlohmann::json& j, StreamParameters& s)
	{
		DESERIALIZE_STRUCT(j, s, stream_type);
		DESERIALIZE_STRUCT(j, s, uri_template);
		DESERIALIZE_STRUCT(j, s, uri_arc_template);
		DESERIALIZE_STRUCT(j, s, cu_type);
		DESERIALIZE_STRUCT(j, s, cu_subst);
		DESERIALIZE_STRUCT(j, s, cu_duration);
	}
};


/// <summary>
/// Interface for stream
/// </summary>
class uri_stream : public uri_base
{
public:
	static constexpr auto REPL_DOMAIN     = L"{DOMAIN}";     // stream url domain (set from playlist)
	static constexpr auto REPL_PORT       = L"{PORT}";       // stream url port (set from playlist)
	static constexpr auto REPL_ID         = L"{ID}";         // id (set from playlist)
	static constexpr auto REPL_SUBDOMAIN  = L"{SUBDOMAIN}";  // domain (set from settings or set by provider)
	static constexpr auto REPL_TOKEN      = L"{TOKEN}";      // token (set from playlist or set by provider)
	static constexpr auto REPL_QUALITY    = L"{QUALITY}";    // quality (set from settings)
	static constexpr auto REPL_LOGIN      = L"{LOGIN}";      // login (set from settings)
	static constexpr auto REPL_PASSWORD   = L"{PASSWORD}";   // password (set from settings)
	static constexpr auto REPL_INT_ID     = L"{INT_ID}";     // internal id (reads from playlist)
	static constexpr auto REPL_HOST       = L"{HOST}";       // host (reads from playlist)
	static constexpr auto REPL_SERVER     = L"{SERVER}";     // server name (read from settings)
	static constexpr auto REPL_SERVER_ID  = L"{SERVER_ID}";  // server id (read from settings)
	static constexpr auto REPL_PROFILE_ID = L"{PROFILE_ID}"; // profile id (read from settings)

	static constexpr auto REPL_START      = L"{START}";      // EPG archive start time (unix timestamp)
	static constexpr auto REPL_NOW        = L"{NOW}";        // EPG archive current time (unix timestamp)
	static constexpr auto REPL_DATE       = L"{DATE}";       // EPG date (set by format)
	static constexpr auto REPL_TIMESTAMP  = L"{TIMESTAMP}";  // EPG time, unix timestamp (set by format)

	static constexpr auto REPL_DURATION   = L"{DURATION}";   // archive duration (in second) in flussonic archive
	static constexpr auto REPL_SHIFT      = L"{CU_SUBST}";   // archive/utc word used in shift/append/flussonic type archive template

	static constexpr auto REPL_YEAR       = "{YEAR}";  // Year subst template, uset in epg_date_format, epg_time_format
	static constexpr auto REPL_MONTH      = "{MONTH}"; // Month subst template, uset in epg_date_format, epg_time_format
	static constexpr auto REPL_DAY        = "{DAY}";   // Day subst template, uset in epg_date_format, epg_time_format
	static constexpr auto REPL_HOUR       = "{HOUR}";  // Hour subst template, uset in epg_time_format
	static constexpr auto REPL_MIN        = "{MIN}";   // Minute subst template, uset in epg_time_format

public:
	uri_stream();

	uri_stream(const uri_stream& src);

	virtual ~uri_stream() = default;

	/// <summary>
	/// clear uri
	/// </summary>
	void clear() override;

	bool save_plugin_parameters(const wchar_t* filename = nullptr);

	/// <summary>
	/// load plugin parameters to file
	/// </summary>
	bool load_plugin_parameters(const wchar_t* filename = nullptr);

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
	/// returns plugin short name
	/// </summary>
	const std::string& get_short_name() const { return short_name; }

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
	std::wstring get_uri_parse_template() const { return utils::utf8_to_utf16(uri_parse_template); }

	/// <summary>
	/// returns regex of uri parse template
	/// </summary>
	/// <returns>wstring</returns>
	const std::wregex& get_uri_regex_parse_template();

	/// <summary>
	/// set uri parse template.
	/// regex string can contain named groups that will be extracted
	/// </summary>
	/// <returns>wstring</returns>
	void set_uri_parse_template(const std::wstring& val);

	/// <summary>
	/// returns parser parameters
	/// </summary>
	const ParsingGroups& get_parser() const noexcept{ return parser; }
	ParsingGroups& get_parser() noexcept { return parser; }

	/// <summary>
	/// set parser parameters
	/// </summary>
	void set_parser(const ParsingGroups& src) { parser = src; }

	/// <summary>
	/// is token used per channel, not the global
	/// </summary>
	/// <returns>bool</returns>
	bool get_per_channel_token() const { return per_channel_token; }

	/// <summary>
	/// getter channel hash
	/// </summary>
	/// <returns>int</returns>
	const int get_hash() const;

	/// <summary>
	/// recalculate has variables
	/// </summary>
	int recalc_hash() { clear_hash(); return get_hash(); }

	/// <summary>
	/// clear hash variables
	/// </summary>
	void clear_hash() { hash = 0; str_hash.clear(); }

	/// <summary>
	/// copy info
	/// </summary>
	void copy(const std::unique_ptr<uri_stream>& src)
	{
		*this = *src;
	}

	/// <summary>
	/// compare uri streams
	/// </summary>
	/// <returns>bool</returns>
	bool compare(const std::unique_ptr<uri_stream>& src)
	{
		return *this == *src;
	}

	/// <summary>
	/// copy info
	/// </summary>
	/// <returns>uri_stream&</returns>
	const uri_stream& operator=(const uri_stream& src)
	{
		if (&src != this)
		{
			set_schema(src.get_schema());
			set_path(src.get_path());
			set_template(src.is_template());
			set_parser(src.get_parser());
			str_hash = src.str_hash;
			hash = src.hash;
		}

		return *this;
	}

	/// <summary>
	/// returns link to provider api url
	/// </summary>
	/// <returns>wstring</returns>
	const std::wstring& get_provider_api_url() const { return provider_api_url; }

	/// <summary>
	/// returns array of playlists
	/// </summary>
	/// <returns>vector<PlaylistInfo>&</returns>
	const std::vector<PlaylistInfo>& get_playlists() const { return playlists; };

	/// <summary>
	/// clear profile list
	/// </summary>
	void clear_profiles_list() { profiles_list.clear(); }

	/// <summary>
	/// returns vod url template
	/// </summary>
	/// <returns>wstring</returns>
	const std::wstring& get_vod_template() const { return provider_vod_url; };

	/// <summary>
	/// returns link to vod download
	/// </summary>
	/// <param name="params">parameters for generating url</param>
	/// <returns>wstring</returns>
	std::wstring get_vod_url(TemplateParams& params) const;

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
	/// returns epg mapper
	/// </summary>
	/// <param name="epg_idx">index of epg, primary/secondary</param>
	/// <returns>map&</returns>
	const std::map<std::wstring, std::wstring>& get_epg_id_mapper(int epg_idx);

	/// <summary>
	/// return epg parameters
	/// </summary>
	/// <returns>EpgParameters</returns>
	EpgParameters& get_epg_parameters(int idx) { return epg_params[idx]; };
	const EpgParameters& get_epg_parameters(int idx) const { return epg_params[idx]; };

	/// <summary>
	/// parse epg for channel.
	/// </summary>
	/// <param name="epg_idx">index of epg, primary/secondary</param>
	/// <param name="epg_id">channel epg id</param>
	/// <param name="epg_map">map of downloaded epg entries, used for cache</param>
	/// <param name="for_time">date to request</param>
	/// <returns>bool</returns>
	bool parse_epg(int epg_idx, const std::wstring& epg_id, std::map<time_t, EpgInfo>& epg_map, time_t for_time);

	/// <summary>
	/// returns compiled epg url for channel
	/// </summary>
	/// <param name="epg_idx">index of epg, primary/secondary</param>
	/// <param name="epg_id">channel epg id</param>
	/// <param name="for_time">date to request</param>
	/// <returns>wstring</returns>
	std::wstring compile_epg_url(int epg_idx, const std::wstring& epg_id, time_t for_time);

	/// <summary>
	/// get templated url
	/// </summary>
	/// <param name="params">parameters for generating url</param>
	/// <returns>string url</returns>
	std::wstring get_templated_stream(TemplateParams& params) const;

	//////////////////////////////////////////////////////////////////////////
	// virtual methods

	/// <summary>
	/// load default settings
	/// </summary>
	/// <param name="url"></param>
	virtual void load_default() {}

	/// <summary>
	/// parse uri to get id
	/// </summary>
	/// <param name="url"></param>
	virtual void parse_uri(const std::wstring& url);

	/// <summary>
	/// parse access info
	/// </summary>
	/// <param name="params">parameters used to download access info</param>
	/// <param name="info_list">parsed parameters list</param>
	/// <returns>bool</returns>
	virtual bool parse_access_info(TemplateParams& params, std::list<AccountInfo>& info_list) { return false; }

	/// <summary>
	/// get url to obtain account playlist
	/// </summary>
	/// <param name="params">parameters used to download access info</param>
	/// <returns>wstring</returns>
	virtual void get_playlist_url(std::wstring& url, TemplateParams& params);

	/// <summary>
	/// returns token from account if exist
	/// </summary>
	/// <param name="login">login</param>
	/// <param name="password">password</param>
	/// <returns>wstring</returns>
	virtual std::wstring get_api_token(const Credentials& creds) const { return L""; };

	/// <summary>
	/// returns json root for epg iteration
	/// </summary>
	/// <param name="epg_idx">index of epg, primary/secondary</param>
	/// <param name="epg_data">downloaded json</param>
	/// <returns>json entry pointed to epg list</returns>
	virtual nlohmann::json get_epg_root(int epg_idx, const nlohmann::json& epg_data) const;

	/// <summary>
	/// returns list of servers
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	/// <returns>vector<ServersInfo></returns>
	virtual const std::vector<ServersInfo>& get_servers_list(TemplateParams& /*params*/) { return servers_list; }

	/// <summary>
	/// returns list of profiles
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	/// <returns>vector<ProfilesInfo></returns>
	virtual const std::vector<ProfilesInfo>& get_profiles_list(TemplateParams& /*params*/) { return profiles_list; }

	/// <summary>
	/// returns list of quality variants
	/// </summary>
	/// <param name="params">Template parameters. Can be changed</param>
	/// <returns>vector<ServersInfo></returns>
	virtual const std::vector<QualityInfo>& get_quality_list(TemplateParams& /*params*/) { return quality_list; }

	/// <summary>
	/// set server
	/// </summary>
	/// <param name="params">Template parameters.</param>
	virtual bool set_server(TemplateParams& /*params*/) { return true; }

	/// <summary>
	/// set profile
	/// </summary>
	/// <param name="params">Template parameters.</param>
	virtual bool set_profile(TemplateParams& /*params*/) { return true; }

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(uri_stream, access_type, title, name, provider_url, playlist_template, uri_parse_template, use_token_as_id, streams_config, epg_params);

protected:

	virtual void replace_vars(std::wstring& url, const TemplateParams& params) const;

	void put_account_info(const std::string& name, const nlohmann::json& js_data, std::list<AccountInfo>& params) const;

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
	// original uri parse template
	std::string uri_parse_template;

	// use token from uri instead of account settings
	bool per_channel_token = false;
	// flag for php plugin if uri does not contains parsed 'id' for channel
	bool use_token_as_id = false;
	// setting for parsing uri streams
	std::array<StreamParameters, 2> streams_config;
	// setting for parsing json EPG
	std::array<EpgParameters, 2> epg_params;

	// non configurable parameters
	std::string short_name;

	std::vector<ServersInfo> servers_list;
	std::vector<ProfilesInfo> profiles_list;
	std::vector<QualityInfo> quality_list;
	std::vector<PlaylistInfo> playlists;

	std::wstring provider_api_url;
	std::wstring provider_vod_url;

	ParsingGroups parser;
	std::wregex uri_parse_regex_template; // compiled regex for uri parse template
	std::vector<std::wstring> regex_named_groups; // extracted named groups from uri parse template

	mutable std::wstring str_hash;
	mutable int hash = 0;
};
