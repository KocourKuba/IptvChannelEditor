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
#include "plugin_config.h"

class Credentials;

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
/// Parsed variable groups for generate playing stream
/// </summary>
class ParsingGroups
{
	friend class base_plugin;

public:
	const std::wstring& get_id() const { return is_template ? id : str_hash; }
	void set_id(const std::wstring& val) { id = val; }

	const std::wstring& get_domain() const { return domain; }
	void set_domain(const std::wstring& val) { domain = val; }

	const std::wstring& get_port() const { return port; }
	void set_port(const std::wstring& val) { port = val; }

	const std::wstring& get_login() const { return login; }
	void set_login(const std::wstring& val) { login = val; }

	const std::wstring& get_password() const { return password; }
	void set_password(const std::wstring& val) { password = val; }

	const std::wstring& get_subdomain() const { return subdomain; }
	void set_subdomain(const std::wstring& val) { subdomain = val; }

	const std::wstring& get_token() const { return token; }
	void set_token(const std::wstring& val) { token = val; }

	const std::wstring& get_int_id() const { return int_id; }
	void set_int_id(const std::wstring& val) { int_id = val; }

	const std::wstring& get_quality() const { return quality; }
	void set_quality(const std::wstring& val) { quality = val; }

	const std::wstring& get_host() const { return host; }
	void set_host(const std::wstring& val) { host = val; }

	/// <summary>
	/// is uri template
	/// </summary>
	void set_is_template(bool val) { is_template = val; }
	bool get_is_template() const { return is_template; }

	/// <summary>
	/// clear hash variables
	/// </summary>
	void clear_hash() { hash = 0; str_hash.clear(); }

	/// <summary>
	/// uri hash. crc32 of id for template uri or crc32 of uri
	/// </summary>
	int get_hash() const { return hash; }
	void set_hash(const int val) { hash = val; str_hash = hash ? std::to_wstring(hash) : L""; }

	/// <summary>
	/// string representation of the hash
	/// </summary>
	const std::wstring& get_str_hash() const { return str_hash; }
	void set_str_hash(const std::wstring& val) { str_hash = val; }

protected:
	bool is_template = false;

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

	// extracted named groups from uri parse template
	std::vector<std::wstring> regex_named_groups;

	int hash = 0;
	std::wstring str_hash;
};

/// <summary>
/// Interface for stream
/// </summary>
class base_plugin : public uri_base, public plugin_config
{
public:
	static constexpr auto REPL_DOMAIN     = L"{DOMAIN}";     // stream url domain (set from playlist)
	static constexpr auto REPL_PORT       = L"{PORT}";       // stream url port (set from playlist)
	static constexpr auto REPL_ID         = L"{ID}";         // id (set from playlist)
	static constexpr auto REPL_SUBDOMAIN  = L"{SUBDOMAIN}";  // domain (set from settings or set by provider)
	static constexpr auto REPL_TOKEN      = L"{TOKEN}";      // token (set from playlist or set by provider)
	static constexpr auto REPL_LOGIN      = L"{LOGIN}";      // login (set from settings)
	static constexpr auto REPL_PASSWORD   = L"{PASSWORD}";   // password (set from settings)
	static constexpr auto REPL_INT_ID     = L"{INT_ID}";     // internal id (reads from playlist)
	static constexpr auto REPL_HOST       = L"{HOST}";       // host (reads from playlist)
	static constexpr auto REPL_SERVER     = L"{SERVER}";     // server name (read from settings)
	static constexpr auto REPL_SERVER_ID  = L"{SERVER_ID}";  // server id (read from settings)
	static constexpr auto REPL_DEVICE_ID  = L"{DEVICE_ID}";  // device id (read from settings)
	static constexpr auto REPL_QUALITY_ID = L"{QUALITY_ID}"; // quality id (set from settings)
	static constexpr auto REPL_PROFILE_ID = L"{PROFILE_ID}"; // profile id (read from settings)

	static constexpr auto REPL_EPG_ID     = L"{EPG_ID}";     // epg id (set from playlist)
	static constexpr auto REPL_START      = L"{START}";      // EPG archive start time (unix timestamp)
	static constexpr auto REPL_NOW        = L"{NOW}";        // EPG archive current time (unix timestamp)
	static constexpr auto REPL_DATE       = L"{DATE}";       // EPG date (set by format)
	static constexpr auto REPL_TIMESTAMP  = L"{TIMESTAMP}";  // EPG time, unix timestamp (set by format)
	static constexpr auto REPL_OFFSET     = L"{OFFSET}";        // EPG archive current time (unix timestamp)

	static constexpr auto REPL_DURATION   = L"{DURATION}";   // archive duration (in second) in flussonic archive
	static constexpr auto REPL_SHIFT      = L"{CU_SUBST}";   // archive/utc word used in shift/append/flussonic type archive template

	static constexpr auto REPL_YEAR       = L"{YEAR}";       // Year subst template, uset in epg_date_format, epg_time_format
	static constexpr auto REPL_MONTH      = L"{MONTH}";      // Month subst template, uset in epg_date_format, epg_time_format
	static constexpr auto REPL_DAY        = L"{DAY}";        // Day subst template, uset in epg_date_format, epg_time_format
	static constexpr auto REPL_HOUR       = L"{HOUR}";       // Hour subst template, uset in epg_time_format
	static constexpr auto REPL_MIN        = L"{MIN}";        // Minute subst template, uset in epg_time_format

	static constexpr auto REPL_YEAR_N     = "{YEAR}";       // Year subst template, uset in epg_date_format, epg_time_format
	static constexpr auto REPL_MONTH_N    = "{MONTH}";      // Month subst template, uset in epg_date_format, epg_time_format
	static constexpr auto REPL_DAY_N      = "{DAY}";        // Day subst template, uset in epg_date_format, epg_time_format
	static constexpr auto REPL_HOUR_N     = "{HOUR}";       // Hour subst template, uset in epg_time_format
	static constexpr auto REPL_MIN_N      = "{MIN}";        // Minute subst template, uset in epg_time_format

public:
	base_plugin();

	base_plugin(const base_plugin& src);

	virtual ~base_plugin() = default;

	/// <summary>
	/// clear uri
	/// </summary>
	void clear() override;

	bool is_valid() const override { return parser.get_is_template() ? true : uri_base::is_valid(); }

	/// <summary>
	/// save plugin parameters to file
	/// </summary>
	bool save_plugin_parameters(const std::wstring& filename, bool use_full_path = false);

	/// <summary>
	/// load plugin parameters to file
	/// </summary>
	void load_plugin_parameters(const std::wstring& filename);

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
	/// regex of uri parse template
	/// regex string can contain named groups that will be extracted
	/// </summary>
	const std::wregex& get_stream_regex_parse_template();
	void set_stream_regex_parse_template(const std::wstring& val);

	/// <summary>
	/// parser parameters
	/// </summary>
	void set_parser(const ParsingGroups& src) { parser = src; }
	const ParsingGroups& get_parser() const noexcept{ return parser; }
	ParsingGroups& get_parser() noexcept { return parser; }

	/// <summary>
	/// getter channel hash
	/// </summary>
	/// <returns>int</returns>
	const int get_hash();

	/// <summary>
	/// recalculate has variables
	/// </summary>
	int recalc_hash() { parser.clear_hash(); return get_hash(); }

	/// <summary>
	/// copy info
	/// </summary>
	void copy(const base_plugin* src)
	{
		*this = *src;
	}

	/// <summary>
	/// compare uri streams
	/// </summary>
	/// <returns>bool</returns>
	bool compare(const base_plugin* src)
	{
		return *this == *src;
	}

	/// <summary>
	/// copy info
	/// </summary>
	/// <returns>uri_stream&</returns>
	const base_plugin& operator=(const base_plugin& src)
	{
		if (&src != this)
		{
			set_schema(src.get_schema());
			set_path(src.get_path());
			set_parser(src.get_parser());
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
	/// returns is vod m3u based
	/// </summary>
	/// <returns>bool</returns>
	bool is_vod_m3u() const { return vod_m3u; }

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
	/// returns epg mapper
	/// </summary>
	/// <param name="epg_idx">index of epg, primary/secondary</param>
	/// <returns>map&</returns>
	const std::map<std::wstring, std::wstring>& get_epg_id_mapper(int epg_idx);

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

	/// <summary>
	/// get templated url
	/// </summary>
	/// <param name="params">parameters for generating url</param>
	/// <returns>string url</returns>
	std::wstring get_archive_template(TemplateParams& params) const;

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
	virtual void parse_stream_uri(const std::wstring& url);

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
	virtual std::wstring get_playlist_url(TemplateParams& params, std::wstring url = L"");

	/// <summary>
	/// returns token from account if exist
	/// </summary>
	/// <param name="login">login</param>
	/// <param name="password">password</param>
	/// <returns>wstring</returns>
	virtual std::wstring get_api_token(const Credentials& creds) const { return L""; };

protected:

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(base_plugin, access_type, short_name, title, name, provider_url, //-V601
								   playlist_template, uri_id_parse_pattern, uri_parse_pattern,
								   square_icons, requested_token,
								   static_servers, static_qualities, static_devices, static_profiles,
								   streams_config, epg_params, servers_list, qualities_list, devices_list, profiles_list);

	void replace_vars(std::wstring& url, const TemplateParams& params) const;

	void put_account_info(const std::string& name, const nlohmann::json& js_data, std::list<AccountInfo>& params) const;

protected:

	// non configurable parameters
	std::string short_name;

	std::vector<PlaylistInfo> playlists;

	std::wstring provider_api_url;
	std::wstring provider_vod_url;
	bool vod_m3u = false;

	ParsingGroups parser;
	std::map<std::wstring, std::wstring ParsingGroups::*> parser_mapper = {
		{L"id"       , &ParsingGroups::id},
		{L"domain"   , &ParsingGroups::domain},
		{L"port"     , &ParsingGroups::port},
		{L"login"    , &ParsingGroups::login},
		{L"password" , &ParsingGroups::password},
		{L"subdomain", &ParsingGroups::subdomain},
		{L"token"    , &ParsingGroups::token},
		{L"int_id"   , &ParsingGroups::int_id},
		{L"quality"  , &ParsingGroups::quality},
		{L"host"     , &ParsingGroups::host},
	};

	// compiled regex for uri parse template
	std::wregex uri_parse_regex_template;
};
