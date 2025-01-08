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
#include "PlayListEntry.h"
#include "Credentials.h"
#include "ThreadConfig.h"
#include "vod_movie.h"

class uri_stream;

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

using EpgStorage = std::unordered_map<std::wstring, std::map<time_t, std::shared_ptr<EpgInfo>>>;
using EpgAliases = std::unordered_map<std::wstring, std::wstring>;

/// <summary>
/// Interface for stream
/// </summary>
class base_plugin : public plugin_config
{
public:
	base_plugin() = default;
	explicit base_plugin(const base_plugin& src) = delete;
	virtual ~base_plugin() = default;

public:
	struct movie_request
	{
		int season_idx = -1;
		int episode_idx = -1;
		int quality_idx = -1;
		int audio_idx = -1;
	};

	/// <summary>
	/// regex of uri parse template
	/// regex string can contain named groups that will be extracted
	/// </summary>
	const boost::wregex& get_regex_parse_stream() const { return regex_uri_template; }
	void set_regex_parse_stream(const std::wstring& val);

	/// <summary>
	/// plugin type
	/// </summary>
	PluginType get_plugin_type() const { return plugin_type; }
	void set_plugin_type(const PluginType type) { plugin_type = type; }

	/// <summary>
	/// plugin internal name
	/// </summary>
	void set_internal_name(const std::string& val) { internal_name = val; }
	std::wstring get_internal_name() const { return utils::utf8_to_utf16(internal_name); }
	const std::string& get_internal_name_a() const { return internal_name; }

	/// <summary>
	/// returns link to vod download
	/// </summary>
	/// <param name="params">parameters for generating url</param>
	/// <returns>wstring</returns>
	std::wstring get_vod_url(const TemplateParams& params);
	std::wstring get_vod_url(const size_t idx, const TemplateParams& params);

	/// <summary>
	/// returns compiled epg url for channel
	/// </summary>
	/// <param name="epg_idx">index of epg, primary/secondary</param>
	/// <param name="epg_id">channel epg id</param>
	/// <param name="for_time">date to request</param>
	/// <returns>wstring</returns>
	std::wstring compile_epg_url(int epg_idx, const std::wstring& epg_id, time_t for_time, const uri_stream* info, const TemplateParams& params);

	/// <summary>
	/// get playable url
	/// </summary>
	/// <param name="params">parameters for generating url</param>
	/// <returns>string url</returns>
	std::wstring get_play_stream(const TemplateParams& params, uri_stream* info) const;

	/// <summary>
	/// get template live url
	/// </summary>
	/// <param name="params">parameters for generating url</param>
	/// <returns>string url</returns>
	std::wstring get_live_template(size_t stream_idx, const uri_stream* info) const;

	/// <summary>
	/// get archive template url
	/// </summary>
	/// <param name="params">parameters for generating url</param>
	/// <returns>string url</returns>
	std::wstring get_archive_template(size_t stream_idx, const uri_stream* info) const;

	/// <summary>
	/// is custom archive template url
	/// </summary>
	/// <param name="params">parameters for generating url</param>
	/// <returns>bool url</returns>
	bool is_custom_archive_template(bool is_template, size_t stream_idx, const std::wstring& url) const;

	/// <summary>
	/// compile name from template
	/// </summary>
	/// <param name="packed_name">template for generating url</param>
	/// <param name="cred">parameters for generating url</param>
	/// <param name="defaultTemplate">default template for generating url</param>
	/// <returns>string url</returns>
	std::wstring compile_name_template(std::wstring packed_name, const Credentials& cred) const;

	//////////////////////////////////////////////////////////////////////////
	// virtual methods

	/// <summary>
	/// parse uri to get id
	/// </summary>
	/// <param name="url"></param>
	void parse_stream_uri(const std::wstring& url, uri_stream& info);

	/// <summary>
	/// returns parsed account info
	/// </summary>
	const std::map<std::wstring, std::wstring, std::less<>>& get_account_info() { return account_info; }

	/// <summary>
	/// clear account info
	/// </summary>
	virtual void clear_account_info() { return account_info.clear(); }

	/// <summary>
	/// parse access info
	/// </summary>
	/// <param name="params">parameters used to download access info</param>
	virtual void parse_account_info(TemplateParams& params) {}

	/// <summary>
	/// get url to obtain account playlist
	/// </summary>
	/// <param name="params">parameters used to download url</param>
	/// <returns>wstring</returns>
	virtual std::wstring get_playlist_url(const TemplateParams& params, std::wstring url = L"");

	/// <summary>
	/// returns s_token from account if exist
	/// </summary>
	/// <param name="creds">credentials</param>
	virtual void update_entry(PlaylistEntry& entry) {};

	/// <summary>
	/// returns s_token from account if exist
	/// </summary>
	/// <param name="params">parameters used to download url</param>
	virtual std::string get_api_token(TemplateParams& params) { return {}; };

	/// <summary>
	/// parse vod
	/// </summary>
	virtual void parse_vod(const CThreadConfig& config) {}

	/// <summary>
	/// parse movie
	/// </summary>
	virtual void fetch_movie_info(const Credentials& creds, vod_movie& movie) {}

	/// <summary>
	/// get movie url
	/// </summary>
	virtual std::wstring get_movie_url(const Credentials& creds, const movie_request& request, const vod_movie& movie) { return movie.url; }

protected:

	std::string get_file_cookie(const std::wstring& name) const;
	void set_file_cookie(const std::wstring& name, const std::string& session, time_t expire_time) const;
	void delete_file_cookie(const std::wstring& name) const;

	void set_json_info(const std::string& name, const nlohmann::json& js_data, std::map<std::wstring, std::wstring, std::less<>>& info) const;
	std::wstring replace_params_vars(const TemplateParams& params, std::wstring& url) const;

protected:
	PluginType plugin_type = PluginType::enCustom;
	std::string internal_name;

	// compiled regex for uri parse template
	boost::wregex regex_uri_template;

	// extracted named groups from uri parse template
	std::vector<std::wstring> regex_named_groups;
	std::map<std::wstring, std::wstring, std::less<>> account_info;
};
