/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2023): sharky72 (https://github.com/KocourKuba)

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
		catchup = src.catchup;
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
