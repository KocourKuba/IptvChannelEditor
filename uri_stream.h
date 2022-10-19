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
#include "base_plugin.h"
#include "IconContainer.h"

#include "UtilsLib\json_wrapper.h"

enum class InfoType { enUndefined, enChannel, enCategory, enPlEntry, enPlCategory };

class uri_stream;

class uri_stream : public uri_base, public IconContainer
{
	friend class base_plugin;

public:
	uri_stream() = delete;
	uri_stream(const uri_stream& src)
	{
		*this = src;
	}

	uri_stream(InfoType type, std::shared_ptr<base_plugin> plugin, std::wstring root_path)
		: IconContainer(root_path)
		, base_type(type)
		, parent_plugin(plugin)
	{};

public:

	/// <summary>
	/// clear uri
	/// </summary>
	void clear() override;

	bool is_valid() const override { return get_is_template() ? true : uri_base::is_valid(); }

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

	int get_adult() const { return adult; }
	void set_adult(int val) { adult = val; }

	bool is_archive() const { return archive_days > 0; }

	int get_archive_days() const { return archive_days; }
	void set_archive_days(int val) { archive_days = val; }

	int get_time_shift_hours() const { return time_shift_hours; }
	void set_time_shift_hours(int val) { time_shift_hours = val; }

	const std::wstring& get_catchup() const { return catchup; }
	void set_catchup(const std::wstring& val) { catchup = val; }

	const std::wstring& get_catchup_template() const { return catchup_template; }
	void set_catchup_template(const std::wstring& val) { catchup_template = val; }

	InfoType get_type() const { return base_type; }

	void copy_data(const uri_stream& src)
	{
		*this = src;
	}

	uri_stream& operator=(const uri_stream& src);

protected:
	std::shared_ptr<base_plugin> parent_plugin;
	InfoType base_type = InfoType::enUndefined;

private:
	bool is_template = false;

	// parsing url variables
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

	// parsed #EXTINF variables
	std::wstring title;
	std::wstring catchup;
	std::wstring catchup_template;
	std::array<std::wstring, 2> epg_id; // epg id
	int time_shift_hours = 0;
	int adult = 0;
	int archive_days = 0;

	int hash = 0;
	std::wstring str_hash;

	std::map<std::wstring, std::wstring*> parser_mapper = {
		{L"id"       , &id},
		{L"domain"   , &domain},
		{L"port"     , &port},
		{L"login"    , &login},
		{L"password" , &password},
		{L"subdomain", &subdomain},
		{L"token"    , &token},
		{L"int_id"   , &int_id},
		{L"quality"  , &quality},
		{L"host"     , &host},
	};
};
