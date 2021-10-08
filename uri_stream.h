#pragma once
#include "uri_base.h"
#include "utils.h"
#include "Crc32.h"

enum class StreamType
{
	enBase = 0,
	enChannels,
	enEdem,
	enSharavoz,
	enSharaclub,
	enGlanz,
	enAntifriz,
	enFox,
	enOneUsd,
};

typedef struct
{
	std::string domain;
	std::string token;
	std::string login;
	std::string password;
	std::string host;
	int shift_back;
}TemplateParams;

/// <summary>
/// Interface for stream
/// </summary>
class uri_stream : public uri_base
{
public:
	uri_stream() = default;
	uri_stream(const uri_stream& src)
	{
		*this = src;
	}

	void clear() override
	{
		uri_base::clear();
		id.clear();
		clear_hash();
	}

	/// <summary>
	/// parse uri to get id
	/// </summary>
	virtual void parse_uri(const std::string& url)
	{
		// http://rtmp.api.rt.com/hls/rtdru.m3u8
		clear();
		uri_base::set_uri(url);
	}

	/// <summary>
	/// getter channel id
	/// </summary>
	/// <returns>string</returns>
	const std::string& get_id() const { return templated ? id : str_hash; }

	/// <summary>
	/// setter channel id
	/// </summary>
	/// <param name="val"></param>
	void set_id(const std::string& val) { id = val; }

	/// <summary>
	/// getter channel hash
	/// </summary>
	/// <returns>int</returns>
	const int get_hash() const
	{
		if (!hash)
		{
			const auto& uri = is_template() ? id : get_uri();
			hash = crc32_bitwise(uri.c_str(), uri.size());
			str_hash = utils::int_to_char(hash);
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
	const std::string& get_domain() const { return domain; };

	/// <summary>
	/// setter domain
	/// </summary>
	void set_domain(const std::string& val) { domain = val; };

	/// <summary>
	/// getter login
	/// </summary>
	/// <returns>string</returns>
	const std::string& get_login() const { return login; };

	/// <summary>
	/// setter login
	/// </summary>
	void set_login(const std::string& val) { login = val; };

	/// <summary>
	/// setter password
	/// </summary>
	void set_password(const std::string& val) { password = val; };

	/// <summary>
	/// getter password
	/// </summary>
	/// <returns>string</returns>
	const std::string& get_password() const { return password; };

	/// <summary>
	/// setter int_id
	/// </summary>
	void set_int_id(const std::string& val) { int_id = val; };

	/// <summary>
	/// getter int_id
	/// </summary>
	/// <returns>string</returns>
	const std::string& get_int_id() const { return int_id; };

	/// <summary>
	/// setter host
	/// </summary>
	void set_host(const std::string& val) { host = val; };

	/// <summary>
	/// getter host
	/// </summary>
	/// <returns>string</returns>
	const std::string& get_host() const { return host; };

	/// <summary>
	/// getter token
	/// </summary>
	/// <returns>string</returns>
	const std::string& get_token() const { return token; };

	/// <summary>
	/// setter token
	/// </summary>
	void set_token(const std::string& val) { token = val; };

	/// <summary>
	/// get templated url
	/// </summary>
	/// <returns>string</returns>
	virtual std::string get_templated(StreamSubType /*subType*/, const TemplateParams& /*params*/) const { return ""; };

	/// <summary>
	/// get epg url
	/// </summary>
	/// <returns>string</returns>
	virtual std::string get_epg1_uri(const std::string& id) const { return ""; };

	/// <summary>
	/// get epg url
	/// </summary>
	/// <returns>string</returns>
	virtual std::string get_epg2_uri(const std::string& id) const { return ""; };

	/// <summary>
	/// get epg json
	/// </summary>
	/// <returns>string</returns>
	virtual std::string get_epg1_uri_json(const std::string& id) const { return ""; };

	/// <summary>
	/// get epg json
	/// </summary>
	/// <returns>string</returns>
	virtual std::string get_epg2_uri_json(const std::string& id) const { return ""; };

	/// <summary>
	/// get cabinet access url
	/// </summary>
	/// <returns>string</returns>
	virtual std::string get_access_url(const std::string& login, const std::string& password) const { return ""; }

	/// <summary>
	/// is has access info
	/// </summary>
	/// <returns>bool</returns>
	virtual bool isHasAccessInfo() const { return false; }

	/// <summary>
	/// get url template to obtain account playlist
	/// </summary>
	virtual std::string get_playlist_template(bool first = true) const { return ""; };

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

protected:
	std::string id;
	std::string domain;
	std::string login;
	std::string password;
	std::string token;
	std::string int_id;
	std::string host;
	std::string uri_template;
	mutable std::string str_hash;
	mutable int hash = 0;
};
