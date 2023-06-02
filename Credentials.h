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

#pragma once

#include "UtilsLib\json_wrapper.h"

class Credentials
{
public:
	Credentials() = default;
	void Clear();

	MAKE_ACCESSORS(login);
	MAKE_ACCESSORS(password);
	MAKE_ACCESSORS(token);
	MAKE_ACCESSORS(subdomain);
	MAKE_ACCESSORS(portal);
	MAKE_ACCESSORS(comment);
	MAKE_ACCESSORS(config);
	MAKE_ACCESSORS(caption);
	MAKE_ACCESSORS(logo);
	MAKE_ACCESSORS(background);
	MAKE_ACCESSORS(update_url);
	MAKE_ACCESSORS(update_package_url);
	MAKE_ACCESSORS(version_id);
	MAKE_ACCESSORS(plugin_name);
	MAKE_ACCESSORS(update_name);
	MAKE_ACCESSORS(ch_web_path);

	friend void to_json(nlohmann::json& j, const Credentials& c)
	{
		SERIALIZE_STRUCT(j, c, login);
		SERIALIZE_STRUCT(j, c, password);
		SERIALIZE_STRUCT(j, c, token);
		SERIALIZE_STRUCT2(j, c, subdomain, domain);
		SERIALIZE_STRUCT(j, c, portal);
		SERIALIZE_STRUCT(j, c, comment);
		SERIALIZE_STRUCT(j, c, config);
		SERIALIZE_STRUCT(j, c, caption);
		SERIALIZE_STRUCT(j, c, logo);
		SERIALIZE_STRUCT(j, c, background);
		SERIALIZE_STRUCT(j, c, update_url);
		SERIALIZE_STRUCT(j, c, update_package_url);
		SERIALIZE_STRUCT(j, c, version_id);
		SERIALIZE_STRUCT(j, c, plugin_name);
		SERIALIZE_STRUCT(j, c, update_name);
		SERIALIZE_STRUCT(j, c, package_name);
		SERIALIZE_STRUCT(j, c, ch_web_path);
		SERIALIZE_STRUCT(j, c, custom_caption);
		SERIALIZE_STRUCT(j, c, custom_logo);
		SERIALIZE_STRUCT(j, c, custom_background);
		SERIALIZE_STRUCT(j, c, custom_plugin_name);
		SERIALIZE_STRUCT(j, c, custom_increment);
		SERIALIZE_STRUCT(j, c, custom_update_name);
		SERIALIZE_STRUCT(j, c, server_id);
		SERIALIZE_STRUCT(j, c, device_id);
		SERIALIZE_STRUCT(j, c, profile_id);
		SERIALIZE_STRUCT(j, c, quality_id);
		SERIALIZE_STRUCT(j, c, embed);
		SERIALIZE_STRUCT(j, c, ch_list);
		SERIALIZE_STRUCT(j, c, m_direct_links);
		SERIALIZE_STRUCT(j, c, use_dropbox); //-V601
	}

	friend void from_json(const nlohmann::json& j, Credentials& c)
	{
		DESERIALIZE_STRUCT(j, c, login);
		DESERIALIZE_STRUCT(j, c, password);
		DESERIALIZE_STRUCT(j, c, token);
		DESERIALIZE_STRUCT2(j, c, subdomain, domain);
		DESERIALIZE_STRUCT(j, c, portal);
		DESERIALIZE_STRUCT(j, c, comment);
		DESERIALIZE_STRUCT(j, c, config);
		DESERIALIZE_STRUCT(j, c, caption);
		DESERIALIZE_STRUCT(j, c, logo);
		DESERIALIZE_STRUCT(j, c, background);
		DESERIALIZE_STRUCT(j, c, update_url);
		DESERIALIZE_STRUCT(j, c, update_package_url);
		DESERIALIZE_STRUCT(j, c, version_id);
		DESERIALIZE_STRUCT(j, c, plugin_name);
		DESERIALIZE_STRUCT(j, c, update_name);
		DESERIALIZE_STRUCT(j, c, package_name);
		DESERIALIZE_STRUCT(j, c, ch_web_path);
		DESERIALIZE_STRUCT(j, c, custom_caption);
		DESERIALIZE_STRUCT(j, c, custom_logo);
		DESERIALIZE_STRUCT(j, c, custom_background);
		DESERIALIZE_STRUCT(j, c, custom_plugin_name);
		DESERIALIZE_STRUCT(j, c, custom_increment);
		DESERIALIZE_STRUCT(j, c, custom_update_name);
		DESERIALIZE_STRUCT(j, c, server_id);
		DESERIALIZE_STRUCT(j, c, device_id);
		DESERIALIZE_STRUCT(j, c, profile_id);
		DESERIALIZE_STRUCT(j, c, quality_id);
		DESERIALIZE_STRUCT(j, c, embed);
		DESERIALIZE_STRUCT(j, c, ch_list);
		DESERIALIZE_STRUCT(j, c, m_direct_links);
		DESERIALIZE_STRUCT(j, c, use_dropbox);
	}

public:
	std::string login;
	std::string password;
	std::string token;
	std::string subdomain;
	std::string portal;
	std::string comment;
	std::string config;
	std::string caption;
	std::string logo;
	std::string background;
	std::string update_url;
	std::string update_package_url;
	std::string version_id;
	std::string plugin_name;
	std::string update_name;
	std::string package_name;
	std::string ch_web_path;

	int custom_caption = 0;
	int custom_logo = 0;
	int custom_background = 0;
	int custom_plugin_name = 0;
	int custom_increment = 0;
	int custom_update_name = 0;
	int server_id = 0; // zero based index
	int device_id = 0; // zero based index
	int profile_id = 0; // zero based index
	int quality_id = 0; // zero based index
	int embed = 0;

	bool not_valid = false;
	bool use_dropbox = false;

	std::vector<std::string> ch_list;
	std::map<std::string, std::string> m_direct_links;
};
