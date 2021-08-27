#pragma once
#include "uri_base.h"
#include "utils.h"
#include "Crc32.h"

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
		hash = 0;
	}

	/// <summary>
	/// parse uri to get id
	/// </summary>
	virtual void parse_uri(const std::string& url)
	{
		// http://rtmp.api.rt.com/hls/rtdru.m3u8
		set_template(false);
		uri_base::set_uri(url);
	}

	/// <summary>
	/// getter channel id
	/// </summary>
	/// <returns>id</returns>
	virtual const std::string& get_id() const { return templated ? id : str_hash; }

	/// <summary>
	/// setter channel id
	/// </summary>
	/// <param name="val"></param>
	virtual void set_id(const std::string& val) { id = val; }

	/// <summary>
	/// getter channel hash
	/// </summary>
	/// <returns></returns>
	virtual const int get_hash() const
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
	/// getter domain
	/// </summary>
	/// <returns></returns>
	virtual const std::string& get_domain() const { return domain; };

	/// <summary>
	/// setter domain
	/// </summary>
	/// <returns></returns>
	virtual void set_domain(const std::string& val) { domain = val; };

	/// <summary>
	/// getter token
	/// </summary>
	/// <returns></returns>
	virtual const std::string& get_token() const { return token; };

	/// <summary>
	/// setter token
	/// </summary>
	/// <returns></returns>
	virtual void set_token(const std::string& val) { token = val; };

	const uri_stream& operator=(const uri_stream& src)
	{
		if (&src != this)
		{
			set_schema(src.get_schema());
			set_path(src.get_path());
			set_template(src.is_template());
			id = src.id;
			domain = src.domain;
			token = src.token;
			str_hash = src.str_hash;
			hash = src.hash;
		}

		return *this;
	}

protected:
	std::string id;
	std::string domain;
	std::string token;
	std::string uri_template;
	mutable std::string str_hash;
	mutable int hash = 0;
};
