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

#pragma once
#include "plugin_config.h"
#include "uri_stream.h"
#include "Credentials.h"

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
	std::wstring get_vod_url(const TemplateParams& params) const;
	std::wstring get_vod_url(const size_t idx, const TemplateParams& params) const;

	/// <summary>
	/// parse entire epg
	/// </summary>
	/// <param name="internal_epg_url">url to xmltv epg</param>
	/// <param name="epg_idx">index of epg, primary/secondary</param>
	/// <param name="epg_id">channel epg id</param>
	/// <param name="epg_map">map of downloaded epg entries, used for cache</param>
	/// <returns>bool</returns>
	bool parse_xml_epg(const std::wstring& internal_epg_url, EpgStorage& epg_map, CProgressCtrl* pCtrl = nullptr);

	/// <summary>
	/// parse epg for channel.
	/// </summary>
	/// <param name="epg_idx">index of epg, primary/secondary</param>
	/// <param name="epg_id">channel epg id</param>
	/// <param name="epg_map">map of downloaded epg entries, used for cache</param>
	/// <param name="for_time">date to request</param>
	/// <returns>bool</returns>
	bool parse_json_epg(int epg_idx, const std::wstring& epg_id, std::array<EpgStorage, 3>& epg_map, time_t for_time, const uri_stream* info);

	/// <summary>
	/// returns compiled epg url for channel
	/// </summary>
	/// <param name="epg_idx">index of epg, primary/secondary</param>
	/// <param name="epg_id">channel epg id</param>
	/// <param name="for_time">date to request</param>
	/// <returns>wstring</returns>
	std::wstring compile_epg_url(int epg_idx, const std::wstring& epg_id, time_t for_time, const uri_stream* info);

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
	/// parse access info
	/// </summary>
	/// <param name="params">parameters used to download access info</param>
	/// <param name="info_list">parsed parameters list</param>
	/// <returns>std::map<std::wstring, std::wstring, std::less<>></returns>
	virtual std::map<std::wstring, std::wstring, std::less<>> parse_access_info(const TemplateParams& params) { return {}; }

	/// <summary>
	/// get url to obtain account playlist
	/// </summary>
	/// <param name="params">parameters used to download access info</param>
	/// <returns>wstring</returns>
	virtual std::wstring get_playlist_url(const TemplateParams& params, std::wstring url = L"");

	/// <summary>
	/// returns token from account if exist
	/// </summary>
	/// <param name="login">login</param>
	/// <param name="password">password</param>
	/// <returns>wstring</returns>
	virtual std::wstring get_api_token(const Credentials& creds) const { return L""; };

protected:

	void set_json_info(const std::string& name, const nlohmann::json& js_data, std::map<std::wstring, std::wstring, std::less<>>& info) const;

protected:
	PluginType plugin_type = PluginType::enCustom;
	std::string internal_name;

	// compiled regex for uri parse template
	boost::wregex regex_uri_template;

	// extracted named groups from uri parse template
	std::vector<std::wstring> regex_named_groups;
};
