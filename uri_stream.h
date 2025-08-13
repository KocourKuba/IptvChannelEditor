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
#include "uri_base.h"
#include "PluginEnums.h"

enum class InfoType { enUndefined, enChannel, enCategory, enPlEntry, enPlCategory };

class uri_stream;

using pointer_to_getter = const std::wstring& (uri_stream::*)();
using pointer_to_setter = void (uri_stream::*)(const std::wstring&);

class uri_stream : public uri_base
{
public:
	uri_stream() = default;
	uri_stream(const uri_stream& src)
	{
		copy_data(src);
	}

	uri_stream(InfoType type) : base_type(type) {};

public:

	/// <summary>
	/// clear uri
	/// </summary>
	void clear() override;

	bool is_valid() const override { return get_is_template() ? true : uri_base::is_valid(); }

	const std::wstring& get_id() const { return is_template ? id : (id.empty() ? str_hash : id); }
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

	const std::wstring& get_ott_key() const { return ott_key; }
	void set_ott_key(const std::wstring& val) { ott_key = val; }

	const std::wstring& get_token() const { return token; }
	void set_token(const std::wstring& val) { token = val; }

	const std::wstring& get_int_id() const { return int_id; }
	void set_int_id(const std::wstring& val) { int_id = val; }

	const std::wstring& get_quality() const { return quality; }
	void set_quality(const std::wstring& val) { quality = val; }

	const std::wstring& get_host() const { return host; }
	void set_host(const std::wstring& val) { host = val; }

	const std::wstring& get_var1() const { return var1; }
	void set_var1(const std::wstring& val) { var1 = val; }

	const std::wstring& get_var2() const { return var2; }
	void set_var2(const std::wstring& val) { var2 = val; }

	const std::wstring& get_var3() const { return var3; }
	void set_var3(const std::wstring& val) { var3 = val; }

	/// <summary>
	/// is uri template
	/// </summary>
	void set_is_template(bool val) { is_template = val; }
	bool get_is_template() const { return is_template; }

	/// <summary>
	/// is custom archive uri template
	/// </summary>
	void set_is_custom_archive(bool val) { is_custom_archive = val; }
	bool get_is_custom_archive() const { return is_custom_archive; }

	/// <summary>
	/// clear hash variables
	/// </summary>
	void clear_hash() { hash = 0; str_hash.clear(); }

	/// <summary>
	/// uri hash. crc32 of id for template uri or crc32 of uri
	/// </summary>
	int get_hash();

	void set_hash(const int val) { hash = val; str_hash = hash ? std::to_wstring(hash) : L""; }

	/// <summary>
	/// recalculate has variables
	/// </summary>
	int recalc_hash() { clear_hash(); return get_hash(); }

	/// <summary>
	/// string representation of the hash
	/// </summary>
	const std::wstring& get_str_hash() const { return str_hash; }
	void set_str_hash(const std::wstring& val) { str_hash = val; }

	const std::wstring& get_title() const { return title; }
	void set_title(const std::wstring& val) { title = val; }

	std::wstring get_epg_id(int idx = 0) const { return epg_id[idx]; }
	void set_epg_id(int idx, const std::wstring& val) { epg_id[idx] = val; }

	std::array<std::wstring, 2> get_epg_ids() const { return epg_id; }

	int get_adult() const { return adult; }
	void set_adult(int val) { adult = val; }

	bool is_archive() const { return archive_days > 0; }

	int get_archive_days() const { return archive_days; }
	void set_archive_days(int val) { archive_days = val; }

	int get_time_shift_hours() const { return time_shift_hours; }
	void set_time_shift_hours(int val) { time_shift_hours = val; }

	CatchupType get_catchup() const { return catchup; }
	void set_catchup(const CatchupType& val) { catchup = val; }

	const std::string& get_catchup_source() const { return catchup_source; }
	void set_catchup_source(const std::string& val) { catchup_source = val; }

	const std::wstring& get_custom_archive_url() const { return custom_archive_url; }
	void set_custom_archive_url(const std::wstring& val) { custom_archive_url = val; }

	int get_custom_url_type() const { return custom_url_type; }
	void set_custom_url_type(int val) { custom_url_type = val; }

	std::string get_custom_url_type_str() const { return stream_type_to_str(custom_url_type); }
	void set_custom_url_type_str(const std::string& val) { custom_url_type = str_to_stream_type(val); }

	int get_custom_archive_url_type() const { return custom_arc_url_type; }
	void set_custom_archive_url_type(int val) { custom_arc_url_type = val; }

	std::string get_custom_archive_url_type_str() const { return stream_type_to_str(custom_arc_url_type); }
	void set_custom_archive_url_type_str(const std::string& val) { custom_arc_url_type = str_to_stream_type(val); }

	InfoType get_type() const { return base_type; }

	void copy_data(const uri_stream& src)
	{
		if (this != &src)
		{
			*this = src;
		}
	}

	uri_stream& operator=(const uri_stream& src);

	std::map<std::wstring, pointer_to_setter, std::less<>> parser_mapper = {
		{L"scheme"   , &uri_stream::set_scheme},
		{L"domain"   , &uri_stream::set_domain},
		{L"port"     , &uri_stream::set_port},
		{L"id"       , &uri_stream::set_id},
		{L"login"    , &uri_stream::set_login},
		{L"password" , &uri_stream::set_password},
		{L"subdomain", &uri_stream::set_subdomain},
		{L"ott_key"  , &uri_stream::set_ott_key},
		{L"token"    , &uri_stream::set_token},
		{L"int_id"   , &uri_stream::set_int_id},
		{L"quality"  , &uri_stream::set_quality},
		{L"host"     , &uri_stream::set_host},
		{L"var1"     , &uri_stream::set_var1},
		{L"var2"     , &uri_stream::set_var2},
		{L"var3"     , &uri_stream::set_var3},
	};

protected:
	static std::string stream_type_to_str(int type);
	static int str_to_stream_type(const std::string& str_type);

	InfoType base_type = InfoType::enUndefined;

	bool is_template = false;
	bool is_custom_archive = false;
	int custom_url_type = 0;
	int custom_arc_url_type = 0;
	std::wstring custom_archive_url;

	// parsing url variables
	std::wstring id;
	std::wstring domain;
	std::wstring port;
	std::wstring login;
	std::wstring password;
	std::wstring subdomain;
	std::wstring ott_key;
	std::wstring token;
	std::wstring int_id;
	std::wstring quality;
	std::wstring host;
	std::wstring var1;
	std::wstring var2;
	std::wstring var3;

	// parsed #EXTINF variables
	std::wstring title;
	CatchupType catchup = CatchupType::cu_not_set;
	std::string catchup_source;
	std::array<std::wstring, 2> epg_id; // epg id
	int time_shift_hours = 0;
	int adult = 0;
	int archive_days = 0;

	int hash = 0;
	std::wstring str_hash;
};
