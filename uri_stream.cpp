#include "pch.h"
#include "uri_stream.h"

#include "UtilsLib/Crc32.h"

void uri_stream::clear()
{
	uri_base::clear();

	is_template = false;
	id.clear();
	domain.clear();
	port.clear();
	login.clear();
	password.clear();
	subdomain.clear();
	token.clear();
	int_id.clear();
	quality.clear();
	host.clear();

	clear_hash();
}

int uri_stream::get_hash()
{
	if (!hash)
	{
		// convert to utf8
		const auto& uri = utils::utf16_to_utf8(get_id().empty() ? get_uri() : get_id());
		set_hash(crc32_bitwise(uri.c_str(), uri.size()));
	}

	return hash;
}

std::string uri_stream::stream_type_to_str(int type)
{
	switch (type)
	{
		case 1:
			return "hls";
		case 2:
			return "mpeg";
		default:
			break;
	}

	return "";
}

int uri_stream::str_to_stream_type(const std::string& str_type)
{
	if (str_type == "hls")
		return 1;

	if (str_type == "mpeg")
		return 2;

	return 0;
}

uri_stream& uri_stream::operator=(const uri_stream& src)
{
	if (this != &src)
	{
		IconContainer::operator=(src);
		base_type = src.base_type;
		parent_plugin = src.parent_plugin;

		is_template = src.is_template;
		id = src.id;
		domain = src.domain;
		port = src.port;
		login = src.login;
		password = src.password;
		subdomain = src.subdomain;
		token = src.token;
		int_id = src.int_id;
		quality = src.quality;
		host = src.host;

		title = src.title;
		catchup_id = src.catchup_id;
		custom_archive_url = src.custom_archive_url;
		epg_id = src.epg_id;
		time_shift_hours = src.time_shift_hours;
		adult = src.adult;
		archive_days = src.archive_days;
		hash = src.hash;
		str_hash = src.str_hash;
	}

	return *this;
}
