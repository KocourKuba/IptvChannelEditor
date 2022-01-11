#pragma once
#include "uri_base.h"
#include "UtilsLib\utils.h"
#include "UtilsLib\Crc32.h"
#include "UtilsLib\json.hpp"

struct TemplateParams
{
	std::wstring domain;
	std::wstring token;
	std::wstring login;
	std::wstring password;
	std::wstring host;
	int shift_back = 0;
};

struct AccountParams
{
	AccountParams(const std::wstring& r_name, const std::wstring& r_value) : name(r_name), value(r_value) {}
	std::wstring name;
	std::wstring value;
};

inline void PutAccountParameter(const std::string& name, nlohmann::json& js_data, std::list<AccountParams>& params)
{
	try
	{
		const auto& js_param = js_data[name];

		if (js_param.is_number_integer())
			params.emplace_back(utils::utf8_to_utf16(name), std::to_wstring(js_param.get<int>()));
		if (js_param.is_number_float())
			params.emplace_back(utils::utf8_to_utf16(name), std::to_wstring(js_param.get<float>()));
		else if (js_param.is_string())
			params.emplace_back(utils::utf8_to_utf16(name), utils::utf8_to_utf16(js_param.get<std::string>()));
	}
	catch (const nlohmann::json::parse_error&)
	{
		// parse errors are ok, because input may be random bytes
	}
	catch (const nlohmann::json::out_of_range&)
	{
		// out of range errors may happen if provided sizes are excessive
	}
	catch (const nlohmann::json::type_error&)
	{
		// type errors may happen if provided sizes are excessive
	}
}


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
	uri_stream() = default;
	uri_stream(const uri_stream& src)
	{
		*this = src;
	}

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
	virtual std::wstring get_templated(StreamSubType subType, TemplateParams& params) const { return L""; };

	/// <summary>
	/// get epg1 url for view
	/// </summary>
	/// <returns>string</returns>
	virtual std::wstring get_epg1_uri(const std::wstring& id) const { return L""; };

	/// <summary>
	/// get epg2 url for view
	/// </summary>
	/// <returns>string</returns>
	virtual std::wstring get_epg2_uri(const std::wstring& id) const { return L""; };

	/// <summary>
	/// get epg1 json url
	/// </summary>
	/// <returns>string</returns>
	virtual std::wstring get_epg1_uri_json(const std::wstring& id) const { return L""; };

	/// <summary>
	/// get epg2 json url
	/// </summary>
	/// <returns>string</returns>
	virtual std::wstring get_epg2_uri_json(const std::wstring& id) const { return L""; };

	/// <summary>
	/// get cabinet access url
	/// </summary>
	/// <returns>string</returns>
	virtual std::wstring get_access_url(const std::wstring& login, const std::wstring& password) const { return L""; }

	/// <summary>
	/// json root for epg iteration
	/// </summary>
	/// <returns>string</returns>
	virtual std::string get_epg_root(bool first = true) const { return "epg_data"; }

	/// <summary>
	/// json epg name node
	/// </summary>
	/// <returns>string</returns>
	virtual std::string get_epg_name(bool first = true) const { return "name"; }

	/// <summary>
	/// json epg description node
	/// </summary>
	/// <returns>string</returns>
	virtual std::string get_epg_desc(bool first = true) const { return "descr"; }

	/// <summary>
	/// json epg start time node
	/// </summary>
	/// <returns>string</returns>
	virtual std::string get_epg_time_start(bool first = true) const { return "time"; }

	/// <summary>
	/// json epg end time node
	/// </summary>
	/// <returns>string</returns>
	virtual std::string get_epg_time_end(bool first = true) const { return "time_to"; }

	/// <summary>
	/// get additional get headers
	/// </summary>
	/// <returns>std::wstring</returns>
	virtual std::wstring get_access_info_header() const { return L""; }

	/// <summary>
	/// parse access info
	/// </summary>
	/// <returns>bool</returns>
	virtual bool parse_access_info(const std::vector<BYTE>& json_data, std::list<AccountParams>& params) const { return false; }

	/// <summary>
	/// get url template to obtain account playlist
	/// </summary>
	/// <param name="first">number of playlist url</param>
	/// <returns></returns>
	virtual std::wstring get_playlist_template(bool first = true) const { return L""; };

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

	virtual std::vector<std::tuple<StreamSubType, std::wstring>>& getSupportedStreamType() const
	{
		static std::vector<std::tuple<StreamSubType, std::wstring>> streams = { {StreamSubType::enHLS, L"HLS"}, {StreamSubType::enMPEGTS, L"MPEG-TS"}};
		return streams;
	};

	virtual bool has_epg2() const
	{
		return false;
	};

protected:
	void ReplaceVars(std::wstring& url, const TemplateParams& params) const
	{
		utils::string_replace_inplace<wchar_t>(url, REPL_SUBDOMAIN, params.domain);
		utils::string_replace_inplace<wchar_t>(url, REPL_ID, get_id());
		utils::string_replace_inplace<wchar_t>(url, REPL_TOKEN, get_token());
		utils::string_replace_inplace<wchar_t>(url, REPL_START, std::to_wstring(params.shift_back));
		utils::string_replace_inplace<wchar_t>(url, REPL_NOW, std::to_wstring(_time32(nullptr)));
	}

	std::wstring& AppendArchive(std::wstring& url) const
	{
		if (url.rfind('?') != std::wstring::npos)
			url += '&';
		else
			url += '?';

		url += L"utc={START}&lutc={NOW}";

		return url;
	}


protected:
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
