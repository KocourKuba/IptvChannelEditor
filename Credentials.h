/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2024): sharky72 (https://github.com/KocourKuba)

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

#include "PluginEnums.h"

#include "UtilsLib\json_wrapper.h"

class Credentials
{
public:
	Credentials() = default;
	void Clear();

	std::wstring get_login() const { return utils::utf8_to_utf16(login); }
	void set_login(const std::wstring& value) { login = utils::utf16_to_utf8(value); }

	std::wstring get_password() const { return utils::utf8_to_utf16(password); }
	void set_password(const std::wstring& value) { password = utils::utf16_to_utf8(value); }

	std::wstring get_ott_key() const { return utils::utf8_to_utf16(ott_key); }
	void set_ott_key(const std::wstring& value) { ott_key = utils::utf16_to_utf8(value); }

	std::wstring get_subdomain() const { return utils::utf8_to_utf16(subdomain); }
	void set_subdomain(const std::wstring& value) { subdomain = utils::utf16_to_utf8(value); }

	std::wstring get_portal() const { return utils::utf8_to_utf16(portal); }
	void set_portal(const std::wstring& value) { portal = utils::utf16_to_utf8(value); }

	std::wstring get_token() const { return utils::utf8_to_utf16(token); }
	void set_token(const std::wstring& value) { token = utils::utf16_to_utf8(value); }

	std::wstring get_s_token() const { return utils::utf8_to_utf16(s_token); }
	void set_s_token(const std::wstring& value) { s_token = utils::utf16_to_utf8(value); }

	std::wstring get_comment() const { return utils::utf8_to_utf16(comment); }
	void set_comment(const std::wstring& value) { comment = utils::utf16_to_utf8(value); }

	std::wstring get_config() const { return utils::utf8_to_utf16(config); }
	void set_config(const std::wstring& value) { config = utils::utf16_to_utf8(value); }

	std::wstring get_caption() const { return utils::utf8_to_utf16(caption); }
	void set_caption(const std::wstring& value) { caption = utils::utf16_to_utf8(value); }

	std::wstring get_logo() const { return utils::utf8_to_utf16(logo); }
	void set_logo(const std::wstring& value) { logo = utils::utf16_to_utf8(value); }

	std::wstring get_background() const { return utils::utf8_to_utf16(background); }
	void set_background(const std::wstring& value) { background = utils::utf16_to_utf8(value); }

	std::wstring get_update_url() const { return utils::utf8_to_utf16(update_url); }
	void set_update_url(const std::wstring& value) { update_url = utils::utf16_to_utf8(value); }

	std::wstring get_update_package_url() const { return utils::utf8_to_utf16(update_package_url); }
	void set_update_package_url(const std::wstring& value) { update_package_url = utils::utf16_to_utf8(value); }

	std::wstring get_version_id() const { return utils::utf8_to_utf16(version_id); }
	void set_version_id(const std::wstring& value) { version_id = utils::utf16_to_utf8(value); }

	std::wstring get_plugin_name() const { return utils::utf8_to_utf16(plugin_name); }
	void set_plugin_name(const std::wstring& value) { plugin_name = utils::utf16_to_utf8(value); }

	std::wstring get_update_name() const { return utils::utf8_to_utf16(update_name); }
	void set_update_name(const std::wstring& value) { update_name = utils::utf16_to_utf8(value); }

	std::wstring get_ch_web_path() const { return utils::utf8_to_utf16(ch_web_path); }
	void set_ch_web_path(const std::wstring& value) { ch_web_path = utils::utf16_to_utf8(value); }

	friend void to_json(nlohmann::json& j, const Credentials& c)
	{
		SERIALIZE_STRUCT(j, c, login);
		SERIALIZE_STRUCT(j, c, password);
		SERIALIZE_STRUCT(j, c, ott_key);
		SERIALIZE_STRUCT(j, c, subdomain);
		SERIALIZE_STRUCT(j, c, domain);
		SERIALIZE_STRUCT(j, c, portal);
		SERIALIZE_STRUCT(j, c, token);
		SERIALIZE_STRUCT(j, c, s_token);
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
		SERIALIZE_STRUCT(j, c, domain_id);
		SERIALIZE_STRUCT(j, c, embed);
		SERIALIZE_STRUCT(j, c, ch_list);
		SERIALIZE_STRUCT(j, c, m_direct_links);
		SERIALIZE_STRUCT(j, c, use_dropbox); //-V601
	}

	friend void from_json(const nlohmann::json& j, Credentials& c)
	{
		DESERIALIZE_STRUCT(j, c, login);
		DESERIALIZE_STRUCT(j, c, password);
		DESERIALIZE_STRUCT(j, c, ott_key);
		DESERIALIZE_STRUCT(j, c, subdomain);
		DESERIALIZE_STRUCT(j, c, domain);
		DESERIALIZE_STRUCT(j, c, portal);
		DESERIALIZE_STRUCT(j, c, token);
		DESERIALIZE_STRUCT(j, c, s_token);
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
		DESERIALIZE_STRUCT(j, c, domain_id);
		DESERIALIZE_STRUCT(j, c, embed);
		DESERIALIZE_STRUCT(j, c, ch_list);
		DESERIALIZE_STRUCT(j, c, m_direct_links);
		DESERIALIZE_STRUCT(j, c, use_dropbox);
	}

public:
	std::string login;
	std::string password;
	std::string token;
	std::string ott_key;
	std::string subdomain;
	std::string domain;
	std::string portal;
	std::string s_token;
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
	int domain_id = 0; // zero based index
	int embed = 0;

	bool not_valid = false;
	bool use_dropbox = false;

	std::vector<std::string> ch_list;
	std::map<std::string, std::string> m_direct_links;

	bool operator!=(const Credentials& that)
	{
		return login != that.login
			|| password != that.password
			|| token != that.token
			|| s_token != that.s_token
			|| ott_key != that.ott_key
			|| subdomain != that.subdomain;
	}
};

struct TemplateParams
{
	Credentials creds;
	std::wstring error_string;
	StreamType streamSubtype = StreamType::enHLS;
	int shift_back = 0;
	int playlist_idx = 0;
};
