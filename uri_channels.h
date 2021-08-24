#pragma once
#include "uri_stream.h"

class uri_channels : public uri_stream
{
	static constexpr auto URI_TEMPLATE = "http://ts://{SUBDOMAIN}/iptv/{UID}/{ID}/index.m3u8";

public:
	/// <summary>
	/// getter for id translated uri
	/// return url with substituted channel id
	/// </summary>
	/// <returns></returns>
	std::string get_id_translated_url() const override;

	/// <summary>
	/// getter for playable translated uri
	/// return url with substituted access_key and access_domain
	/// </summary>
	/// <returns></returns>
	std::string get_playable_url(const std::string& access_domain, const std::string& access_key) const override;

	void parse_uri(const std::string& url) override;
};

