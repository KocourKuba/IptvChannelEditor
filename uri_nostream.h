#pragma once
#include "uri_stream.h"

class uri_nostream : public uri_stream
{
public:
	uri_nostream() = default;

	void parse_uri(const std::string&) override {}

	/// <summary>
	/// getter for id translated uri
	/// return url with substituted channel id
	/// </summary>
	/// <returns></returns>
	std::string get_id_translated_url() const override { return ""; }

	/// <summary>
	/// getter for playable translated uri
	/// return url with substituted access_key and access_domain
	/// </summary>
	/// <returns></returns>
	std::string get_playable_url(const std::string&, const std::string&) const override { return ""; }
};
