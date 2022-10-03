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
#include "uri_config.h"

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

	std::vector<std::wstring> regex_named_groups; // extracted named groups from uri parse template
};

/// <summary>
/// Interface for stream
/// </summary>
class uri_stream : public uri_base, public uri_config
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

	/// <summary>
	/// save plugin parameters to file
	/// </summary>
	bool save_plugin_parameters(const wchar_t* filename = nullptr);

	/// <summary>
	/// load plugin parameters to file
	/// </summary>
	bool load_plugin_parameters(const wchar_t* filename = nullptr);

	/// <summary>
	/// returns plugin short name
	/// </summary>
	const std::string& get_short_name() const { return short_name; }

	/// <summary>
	/// returns regex of uri parse template
	/// </summary>
	/// <returns>wstring</returns>
	const std::wregex& get_uri_regex_parse_template();

	/// <summary>
	/// set regex of uri parse template
	/// regex string can contain named groups that will be extracted
	/// </summary>
	/// <returns>wstring</returns>
	void set_uri_regex_parse_template(const std::wstring& val);

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
	void copy(const uri_stream* src)
	{
		*this = *src;
	}

	/// <summary>
	/// compare uri streams
	/// </summary>
	/// <returns>bool</returns>
	bool compare(const uri_stream* src)
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
	virtual std::wstring get_playlist_url(TemplateParams& params, std::wstring url = L"");

	/// <summary>
	/// returns token from account if exist
	/// </summary>
	/// <param name="login">login</param>
	/// <param name="password">password</param>
	/// <returns>wstring</returns>
	virtual std::wstring get_api_token(const Credentials& creds) const { return L""; };

protected:

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(uri_stream, access_type, short_name, title, name, provider_url, //-V601
								   playlist_template, uri_id_parse_pattern, uri_parse_pattern,
								   square_icons, requested_token, use_token_as_id,
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

	std::wregex uri_parse_regex_template; // compiled regex for uri parse template

	mutable std::wstring str_hash;
	mutable int hash = 0;
};
