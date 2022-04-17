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
#include "UtilsLib\utils.h"
#include "UtilsLib\Crc32.h"
#include "UtilsLib\json_wrapper.h"
#include "UtilsLib\inet_utils.h"

struct TemplateParams
{
	std::wstring domain;
	std::wstring token;
	std::wstring login;
	std::wstring password;
	std::wstring host;
	int shift_back = 0;
};

struct PlaylistTemplateParams
{
	int number = 0;
	int device = 1;
	std::wstring domain;
	std::wstring token;
	std::wstring login;
	std::wstring password;
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
/// Interface for stream
/// </summary>
class uri_stream : public uri_base
{
protected:
	static constexpr auto REPL_SUBDOMAIN = L"{SUBDOMAIN}";
	static constexpr auto REPL_ID = L"{ID}";
	static constexpr auto REPL_TOKEN = L"{TOKEN}";
	static constexpr auto REPL_START = L"{START}";
	static constexpr auto REPL_NOW = L"{NOW}";

public:
	uri_stream()
	{
		std::map<std::string, std::string> params = {
			{"epg_root",  "epg_data"},
			{"epg_name",  "name"},
			{"epg_desc",  "descr"},
			{"epg_start", "time"},
			{"epg_end",   "time_to"},
		};

		epg_params = { params, params };
	}

	uri_stream(const uri_stream& src) : uri_stream()
	{
		*this = src;
	}

	virtual ~uri_stream() = default;

	/// <summary>
	/// clear uri
	/// </summary>
	void clear() override
	{
		uri_base::clear();
		id.clear();
		clear_hash();
	}

	/// <summary>
	/// parse uri to get id
	/// </summary>
	/// <param name="url"></param>
	virtual void parse_uri(const std::wstring& url)
	{
		// http://rtmp.api.rt.com/hls/rtdru.m3u8
		clear();
		uri_base::set_uri(url);
	}

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
	const int get_hash() const
	{
		if (!hash)
		{
			// convert to utf8
			const auto& uri = utils::utf16_to_utf8(is_template() ? id : get_uri());
			hash = crc32_bitwise(uri.c_str(), uri.size());
			str_hash = std::to_wstring(hash);
		}

		return hash;
	}

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
	const std::wstring& get_int_id() const { return int_id; };

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
	/// get templated url
	/// </summary>
	/// <param name="subType">stream subtype HLS/MPEG_TS</param>
	/// <param name="params">parameters for generating url</param>
	/// <returns>string url</returns>
	virtual std::wstring get_templated_stream(StreamSubType subType, const TemplateParams& params) const { return L""; };

	/// <summary>
	/// get epg1 json url
	/// </summary>
	/// <returns>string</returns>
	virtual std::wstring get_epg_uri_json(int epg_idx, const std::wstring& id, time_t for_time = 0) const { return L""; };

	/// <summary>
	/// get additional get headers
	/// </summary>
	/// <returns>std::wstring</returns>
	virtual std::wstring get_access_info_header() const { return L""; }

	/// <summary>
	/// parse access info
	/// </summary>
	/// <returns>bool</returns>
	virtual bool parse_access_info(const PlaylistTemplateParams& params, std::list<AccountInfo>& info_list) const { return false; }

	/// <summary>
	/// get url template to obtain account playlist
	/// </summary>
	/// <param name="first">number of playlist url</param>
	/// <returns></returns>
	virtual std::wstring get_playlist_template(const PlaylistTemplateParams& params) const { ASSERT(false); return L""; };

	/// <summary>
	/// get url template to obtain account playlist
	/// </summary>
	/// <param name="first">number of playlist url</param>
	/// <returns></returns>
	virtual std::wstring get_api_token(const std::wstring& login, const std::wstring& password) const { return L""; };

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

	virtual std::vector<std::tuple<StreamSubType, std::wstring>>& get_supported_stream_type() const
	{
		static std::vector<std::tuple<StreamSubType, std::wstring>> streams = { {StreamSubType::enHLS, L"HLS"}, {StreamSubType::enMPEGTS, L"MPEG-TS"}};
		return streams;
	}

	virtual const std::wstring get_tvg_id_mapper_url() const
	{
		return L"";
	}

	virtual std::array<std::map<std::wstring, std::wstring>, 2> get_tvg_id_mapper()
	{
		return {};
	}

	bool has_epg2() const { return epg2; };

	bool parse_epg(int epg_idx, const std::wstring& epg_id, std::map<time_t, EpgInfo>& epg_map, time_t for_time, bool use_proxy = false)
	{
		std::vector<BYTE> data;
		auto& epg_uri = get_epg_uri_json(epg_idx, epg_id, for_time);
		if (epg_proxy[epg_idx] && use_proxy)
		{
			utils::string_replace_inplace(epg_uri, L"epg.ott-play.com", L"epg.esalecrm.net");
		}

		if (!utils::DownloadFile(epg_uri, data) || data.empty())
			return false;

		JSON_ALL_TRY
		{
			nlohmann::json parsed_json = nlohmann::json::parse(data);

			bool added = false;
			const auto& root = get_epg_root(epg_idx, parsed_json);
			for (const auto& item : root.items())
			{
				const auto& val = item.value();

				EpgInfo epg_info;
				epg_info.name = std::move(utils::make_text_rtf_safe(utils::entityDecrypt(get_json_value(epg_params[epg_idx].at("epg_name"), val))));
				epg_info.desc = std::move(utils::make_text_rtf_safe(utils::entityDecrypt(get_json_value(epg_params[epg_idx].at("epg_desc"), val))));

				epg_info.time_start = get_json_int_value(epg_params[epg_idx].at("epg_start"), val);
				epg_info.time_end = get_json_int_value(epg_params[epg_idx].at("epg_end"), val);
				if (use_duration[epg_idx])
				{
					epg_info.time_end += epg_info.time_start;
				}

#ifdef _DEBUG
				COleDateTime ts(epg_info.time_start);
				COleDateTime te(epg_info.time_end);
				epg_info.start = fmt::format(L"{:04d}-{:02d}-{:02d}", ts.GetYear(), ts.GetMonth(), ts.GetDay());
				epg_info.end = fmt::format(L"{:04d}-{:02d}-{:02d}", te.GetYear(), te.GetMonth(), te.GetDay());
#endif // _DEBUG

				if (epg_info.time_start != 0)
				{
					added |= epg_map.emplace(epg_info.time_start, epg_info).second;
				}
			}

			return added;
		}
		JSON_ALL_CATCH;

		return false;
	}

protected:
	/// <summary>
	/// json root for epg iteration
	/// </summary>
	/// <returns>string</returns>
	virtual nlohmann::json get_epg_root(int epg_idx, const nlohmann::json& epg_data) const
	{
		const auto& root = epg_params[epg_idx].at("epg_root");
		return root.empty() ? epg_data : epg_data[root];
	}

	static void put_account_info(const std::string& name, nlohmann::json& js_data, std::list<AccountInfo>& params)
	{
		JSON_ALL_TRY
		{
			const auto& js_param = js_data[name];

			AccountInfo info;
			info.name = std::move(utils::utf8_to_utf16(name));
			if (js_param.is_number_integer())
			{
				info.value = std::move(std::to_wstring(js_param.get<int>()));
			}
			if (js_param.is_number_float())
			{
				info.value = std::move(std::to_wstring(js_param.get<float>()));
			}
			else if (js_param.is_string())
			{
				info.value = std::move(utils::utf8_to_utf16(js_param.get<std::string>()));
			}

			params.emplace_back(info);
		}
		JSON_ALL_CATCH;
	}

	void replace_vars(std::wstring& url, const TemplateParams& params) const
	{
		utils::string_replace_inplace<wchar_t>(url, REPL_SUBDOMAIN, params.domain);
		utils::string_replace_inplace<wchar_t>(url, REPL_ID, get_id());
		utils::string_replace_inplace<wchar_t>(url, REPL_TOKEN, get_token());
		utils::string_replace_inplace<wchar_t>(url, REPL_START, std::to_wstring(params.shift_back));
		utils::string_replace_inplace<wchar_t>(url, REPL_NOW, std::to_wstring(_time32(nullptr)));
	}

	virtual std::wstring& append_archive(std::wstring& url) const
	{
		if (url.rfind('?') != std::wstring::npos)
			url += '&';
		else
			url += '?';

		url += L"utc={START}&lutc={NOW}";

		return url;
	}

	static std::string get_json_value(const std::string& key, const nlohmann::json& val)
	{
		return val.contains(key) && val[key].is_string() ? val[key] : "";
	}

	static time_t get_json_int_value(const std::string& key, const nlohmann::json& val)
	{
		if (val[key].is_number())
		{
			return val.value(key, 0);
		}

		if (val[key].is_string())
		{
			return utils::char_to_int(val.value(key, ""));
		}

		return 0;
	}

protected:
	bool epg2 = false;
	std::array<bool, 2> epg_proxy = {false, false};
	std::array<bool, 2> use_duration = { false, false };
	std::array <std::map<std::string, std::string>, 2> epg_params;
	std::wstring id;
	std::wstring domain;
	std::wstring login;
	std::wstring password;
	std::wstring token;
	std::wstring int_id;
	std::wstring host;
	std::wstring uri_template;
	mutable std::wstring str_hash;
	mutable int hash = 0;
};
