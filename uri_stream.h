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
#include "UtilsLib\json_wrapper.h"

enum class ServerSubstType {
	enNone,
	enStream,
	enPlaylist
};

struct TemplateParams
{
	std::wstring domain;
	std::wstring token;
	std::wstring login;
	std::wstring password;
	std::wstring host;
	int shift_back = 0;
	int number = 0;
	int server = 0;
	int profile = 0;
};

struct PlaylistInfo
{
	std::wstring name;
	std::wstring id;
	bool is_default = false;
	bool is_custom = false;
};

struct ServersInfo
{
	std::wstring name;
	std::wstring id;
};

struct ProfilesInfo
{
	std::wstring name;
	std::wstring id;
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

struct EpgParameters
{
	bool epg_use_mapper = false;
	bool epg_use_id_hash = false;
	bool epg_use_duration = false;
	size_t epg_tz = 0;
	std::wstring epg_url;
	std::wstring epg_mapper_url;
	std::wstring epg_date_format;
	std::string epg_root;
	std::string epg_name;
	std::string epg_desc;
	std::string epg_start;
	std::string epg_end;
	std::string epg_time_format;
	std::map<std::wstring, std::wstring> epg_mapper;
};

/// <summary>
/// Interface for stream
/// </summary>
class uri_stream : public uri_base
{
protected:
	static constexpr auto REPL_DOMAIN = L"{DOMAIN}";
	static constexpr auto REPL_ID = L"{ID}";
	static constexpr auto REPL_TOKEN = L"{TOKEN}";
	static constexpr auto REPL_START = L"{START}";
	static constexpr auto REPL_NOW = L"{NOW}";
	static constexpr auto REPL_DATE = L"{DATE}";
	static constexpr auto REPL_TIME = L"{TIME}";
	static constexpr auto REPL_QUALITY = L"{QUALITY}";

public:
	uri_stream();

	uri_stream(const uri_stream& src);

	virtual ~uri_stream() = default;

	/// <summary>
	/// clear uri
	/// </summary>
	void clear() override;

	/// <summary>
	/// getter channel id
	/// </summary>
	/// <returns>string</returns>
	const std::wstring& get_id() const { return templated ? id : str_hash; }

	/// <summary>
	/// setter channel id
	/// </summary>
	/// <param name="val"></param>
	void set_id(const std::wstring& val) { id = val; }

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
	/// getter domain
	/// </summary>
	/// <returns>string</returns>
	const std::wstring& get_domain() const { return domain; };

	/// <summary>
	/// setter domain
	/// </summary>
	void set_domain(const std::wstring& val) { domain = val; };

	/// <summary>
	/// getter port
	/// </summary>
	/// <returns>string</returns>
	const std::wstring& get_port() const { return port; };

	/// <summary>
	/// setter port
	/// </summary>
	void set_port(const std::wstring& val) { port = val; };

	/// <summary>
	/// getter login
	/// </summary>
	/// <returns>string</returns>
	const std::wstring& get_login() const { return login; };

	/// <summary>
	/// setter login
	/// </summary>
	void set_login(const std::wstring& val) { login = val; };

	/// <summary>
	/// setter password
	/// </summary>
	void set_password(const std::wstring& val) { password = val; };

	/// <summary>
	/// getter password
	/// </summary>
	/// <returns>string</returns>
	const std::wstring& get_password() const { return password; };

	/// <summary>
	/// setter int_id
	/// </summary>
	void set_int_id(const std::wstring& val) { int_id = val; };

	/// <summary>
	/// getter int_id
	/// </summary>
	/// <returns>string</returns>
	const std::wstring& get_internal_id() const { return int_id; };

	/// <summary>
	/// setter host
	/// </summary>
	void set_host(const std::wstring& val) { host = val; };

	/// <summary>
	/// getter host
	/// </summary>
	/// <returns>string</returns>
	const std::wstring& get_host() const { return host; };

	/// <summary>
	/// getter token
	/// </summary>
	/// <returns>string</returns>
	const std::wstring& get_token() const { return token; };

	/// <summary>
	/// setter token
	/// </summary>
	void set_token(const std::wstring& val) { token = val; };

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
			id = src.id;
			domain = src.domain;
			login = src.login;
			password = src.password;
			token = src.token;
			int_id = src.int_id;
			host = src.host;
			str_hash = src.str_hash;
			hash = src.hash;
		}

		return *this;
	}

	/// <summary>
	/// returns link to provider account
	/// </summary>
	/// <returns>wstring</returns>
	bool is_vod_supported() const { return vod_supported; }

	/// <summary>
	/// returns link to provider account
	/// </summary>
	/// <returns>wstring</returns>
	const std::wstring& get_provider_url() const { return provider_url; }

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
	/// returns server substitution type
	/// </summary>
	/// <returns>ServerSubstType</returns>
	const ServerSubstType get_server_subst_type() const { return server_subst_type; }

	/// <summary>
	/// returns link to vod download
	/// </summary>
	/// <returns>wstring</returns>
	const std::wstring& get_vod_url() const { return provider_vod_url; }

	/// <summary>
	/// supported streams HLS,MPEGTS etc.
	/// </summary>
	/// <returns>vector&</returns>
	const std::vector<std::tuple<StreamSubType, std::wstring>>& get_supported_stream_type() const { return streams; }

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

	/// <summary>
	/// is stream has secondary epg
	/// </summary>
	/// <returns>bool</returns>
	bool has_epg2() const { return secondary_epg; };

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

	//////////////////////////////////////////////////////////////////////////
	// virtual methods

	/// <summary>
	/// parse uri to get id
	/// </summary>
	/// <param name="url"></param>
	virtual void parse_uri(const std::wstring& url);

	/// <summary>
	/// get templated url
	/// </summary>
	/// <param name="subType">stream subtype HLS/MPEG_TS</param>
	/// <param name="params">parameters for generating url</param>
	/// <returns>string url</returns>
	virtual std::wstring get_templated_stream(const StreamSubType subType, TemplateParams& params) const { return L""; };

	/// <summary>
	/// get additional get headers
	/// </summary>
	/// <returns>wstring</returns>
	virtual std::wstring get_access_info_header() const { return L""; }

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
	virtual std::wstring get_playlist_url(TemplateParams& params) { ASSERT(false); return L""; };

	/// <summary>
	/// returns token from account if exist
	/// </summary>
	/// <param name="login">login</param>
	/// <param name="password">password</param>
	/// <returns>wstring</returns>
	virtual std::wstring get_api_token(const std::wstring& login, const std::wstring& password) const { return L""; };

	/// <summary>
	/// returns json root for epg iteration
	/// </summary>
	/// <param name="epg_idx">index of epg, primary/secondary</param>
	/// <param name="epg_data">downloaded json</param>
	/// <returns>json entry pointed to epg list</returns>
	virtual nlohmann::json get_epg_root(int epg_idx, const nlohmann::json& epg_data) const;

	/// <summary>
	/// add archive parameters to url
	/// </summary>
	/// <param name="url">url/secondary</param>
	/// <returns>wstring&</returns>
	virtual std::wstring& append_archive(std::wstring& url) const;

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
	/// set server
	/// </summary>
	/// <param name="params">Template parameters.</param>
	virtual bool set_server(TemplateParams& /*params*/) { return true; }

	/// <summary>
	/// set profile
	/// </summary>
	/// <param name="params">Template parameters.</param>
	virtual bool set_profile(TemplateParams& /*params*/) { return true; }

protected:

	void replace_vars(std::wstring& url, const TemplateParams& params) const;

	void put_account_info(const std::string& name, const nlohmann::json& js_data, std::list<AccountInfo>& params) const;

	std::string get_json_string_value(const std::string& key, const nlohmann::json& val) const;

	time_t get_json_int_value(const std::string& key, const nlohmann::json& val) const;

protected:
	std::array <EpgParameters, 2> epg_params;
	std::vector<std::tuple<StreamSubType, std::wstring>> streams = { {StreamSubType::enHLS, L"HLS"}, {StreamSubType::enMPEGTS, L"MPEG-TS"} };

	ServerSubstType server_subst_type = ServerSubstType::enNone;
	std::vector<ServersInfo> servers_list;
	std::vector<ProfilesInfo> profiles_list;
	std::vector<PlaylistInfo> playlists;
	std::wstring provider_url;
	std::wstring provider_api_url;
	std::wstring provider_vod_url;
	std::wstring id;
	std::wstring domain;
	std::wstring port;
	std::wstring login;
	std::wstring password;
	std::wstring token;
	std::wstring int_id;
	std::wstring host;
	bool vod_supported = false;
	bool secondary_epg = false;
	mutable std::wstring str_hash;
	mutable int hash = 0;
};
