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
#include "plugin_config.h"
#include "uri_stream.h"
#include "Credentials.h"

class uri_stream;

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
/// Interface for stream
/// </summary>
class base_plugin : public plugin_config
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


protected:
	/// <summary>
	/// load default settings
	/// </summary>
	/// <param name="url"></param>
	void load_default() override;

public:
	void clear() override;

	/// <summary>
	/// save plugin parameters to file
	/// </summary>
	bool save_plugin_parameters(const std::wstring& filename, bool use_full_path = false) override;

	/// <summary>
	/// load plugin parameters to file
	/// </summary>
	void load_plugin_parameters(const std::wstring& filename) override;

	/// <summary>
	/// regex of uri parse template
	/// regex string can contain named groups that will be extracted
	/// </summary>

	const std::wregex& get_regex_parse_stream_template() const { return regex_uri_template; }
	void set_regex_parse_stream(const std::wstring& val);

	/// <summary>
	/// copy info
	/// </summary>
	void copy(const base_plugin* src)
	{
		*this = *src;
	}

	/// <summary>
	/// returns array of playlists
	/// </summary>
	/// <returns>vector<PlaylistInfo>&</returns>
	//const std::vector<PlaylistInfo>& get_playlists() const { return playlists; };

	/// <summary>
	/// clear profile list
	/// </summary>
	void clear_profiles_list() { profiles_list.clear(); }

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
	bool parse_epg(int epg_idx, const std::wstring& epg_id, std::map<time_t, EpgInfo>& epg_map, time_t for_time, const uri_stream* info);

	/// <summary>
	/// returns compiled epg url for channel
	/// </summary>
	/// <param name="epg_idx">index of epg, primary/secondary</param>
	/// <param name="epg_id">channel epg id</param>
	/// <param name="for_time">date to request</param>
	/// <returns>wstring</returns>
	std::wstring compile_epg_url(int epg_idx, const std::wstring& epg_id, time_t for_time, const uri_stream* info);

	/// <summary>
	/// get templated url
	/// </summary>
	/// <param name="params">parameters for generating url</param>
	/// <returns>string url</returns>
	std::wstring get_templated_stream(const TemplateParams& params, uri_stream* info) const;

	/// <summary>
	/// get templated url
	/// </summary>
	/// <param name="params">parameters for generating url</param>
	/// <returns>string url</returns>
	std::wstring get_archive_template(const TemplateParams& params, const uri_stream* info) const;

	//////////////////////////////////////////////////////////////////////////
	// virtual methods

	/// <summary>
	/// parse uri to get id
	/// </summary>
	/// <param name="url"></param>
	virtual void parse_stream_uri(const std::wstring& url, uri_stream* info);

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

	void replace_vars(std::wstring& url, const TemplateParams& params, const uri_stream* info) const;
	void put_account_info(const std::string& name, const nlohmann::json& js_data, std::list<AccountInfo>& params) const;

protected:

	// compiled regex for uri parse template
	std::wregex regex_uri_template;

	// extracted named groups from uri parse template
	std::vector<std::wstring> regex_named_groups;
};
