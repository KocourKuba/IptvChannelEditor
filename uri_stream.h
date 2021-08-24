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
	/// <returns></returns>
	virtual void parse_uri(const std::string&) = 0;

	/// <summary>
	/// getter for id translated uri
	/// return url with substituted channel id
	/// </summary>
	/// <returns></returns>
	virtual std::string get_id_translated_url() const = 0;

	/// <summary>
	/// getter for playable translated uri
	/// return url with substituted access_key and access_domain
	/// </summary>
	/// <returns></returns>
	virtual std::string get_playable_url(const std::string& access_domain, const std::string& access_key) const = 0;

	/// <summary>
	/// getter channel id
	/// </summary>
	/// <returns></returns>
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
			const auto& uri = get_id_translated_url();

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
	/// getter uid
	/// </summary>
	/// <returns></returns>
	virtual const std::string& get_uid() const { return uid; };

	/// <summary>
	/// setter uid
	/// </summary>
	/// <returns></returns>
	virtual void set_uid(const std::string& val) { uid = val; };

	const uri_stream& operator=(const uri_stream& src)
	{
		if (&src != this)
		{
			set_schema(src.get_schema());
			set_path(src.get_path());
			set_template(src.is_template());
			id = src.id;
			domain = src.domain;
			uid = src.uid;
			str_hash = src.str_hash;
			hash = src.hash;
		}

		return *this;
	}

protected:
	std::string id;
	std::string domain;
	std::string uid;
	mutable std::string str_hash;
	mutable int hash = 0;
};
