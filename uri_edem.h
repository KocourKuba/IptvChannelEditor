#pragma once
#include "uri_stream.h"

class uri_edem : public uri_stream
{
	static constexpr auto URI_TEMPLATE = "http://{SUBDOMAIN}/iptv/{UID}/{ID}/index.m3u8";

public:
	std::string get_id_translated_url() const override;
	std::string get_playable_url(const std::string& access_domain, const std::string& access_key) const override;
	void parse_uri(const std::string& url) override;
};

