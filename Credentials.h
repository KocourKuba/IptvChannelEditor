#pragma once

#include "UtilsLib\json_wrapper.h"

class Credentials
{
public:
	Credentials() = default;
	void Clear();

	std::wstring get_login() const { return utils::utf8_to_utf16(login); }
	std::wstring get_password() const { return utils::utf8_to_utf16(password); }
	std::wstring get_token() const { return utils::utf8_to_utf16(token); }
	std::wstring get_subdomain() const { return utils::utf8_to_utf16(subdomain); }
	std::wstring get_portal() const { return utils::utf8_to_utf16(portal); }
	std::wstring get_comment() const { return utils::utf8_to_utf16(comment); }
	std::wstring get_config() const { return utils::utf8_to_utf16(config); }
	std::wstring get_suffix() const { return utils::utf8_to_utf16(suffix); }
	std::wstring get_caption() const { return utils::utf8_to_utf16(caption); }
	std::wstring get_logo() const { return utils::utf8_to_utf16(logo); }
	std::wstring get_background() const { return utils::utf8_to_utf16(background); }
	std::wstring get_ch_web_path() const { return utils::utf8_to_utf16(ch_web_path); }

	void set_login(const std::wstring& value) { login = utils::utf16_to_utf8(value); }
	void set_password(const std::wstring& value) { password = utils::utf16_to_utf8(value); }
	void set_token(const std::wstring& value) { token = utils::utf16_to_utf8(value); }
	void set_subdomain(const std::wstring& value) { subdomain = utils::utf16_to_utf8(value); }
	void set_portal(const std::wstring& value) { portal = utils::utf16_to_utf8(value); }
	void set_comment(const std::wstring& value) { comment = utils::utf16_to_utf8(value); }
	void set_config(const std::wstring& value) { config = utils::utf16_to_utf8(value); }
	void set_suffix(const std::wstring& value) { suffix = utils::utf16_to_utf8(value); }
	void set_caption(const std::wstring& value) { caption = utils::utf16_to_utf8(value); }
	void set_logo(const std::wstring& value) { logo = utils::utf16_to_utf8(value); }
	void set_background(const std::wstring& value) { background = utils::utf16_to_utf8(value); }
	void set_ch_web_path(const std::wstring& value) { ch_web_path = utils::utf16_to_utf8(value); }

	friend void to_json(nlohmann::json& j, const Credentials& c)
	{
		SERIALIZE_STRUCT(j, c, login);
		SERIALIZE_STRUCT(j, c, password);
		SERIALIZE_STRUCT(j, c, token);
		SERIALIZE_STRUCT2(j, c, subdomain, domain);
		SERIALIZE_STRUCT(j, c, portal);
		SERIALIZE_STRUCT(j, c, comment);
		SERIALIZE_STRUCT(j, c, config);
		SERIALIZE_STRUCT(j, c, suffix);
		SERIALIZE_STRUCT(j, c, caption);
		SERIALIZE_STRUCT(j, c, logo);
		SERIALIZE_STRUCT(j, c, background);
		SERIALIZE_STRUCT(j, c, update_url);
		SERIALIZE_STRUCT(j, c, update_package_url);
		SERIALIZE_STRUCT(j, c, version_id);
		SERIALIZE_STRUCT(j, c, update_name);
		SERIALIZE_STRUCT(j, c, package_name);
		SERIALIZE_STRUCT(j, c, ch_web_path);
		SERIALIZE_STRUCT(j, c, custom_increment);
		SERIALIZE_STRUCT(j, c, custom_update_name);
		SERIALIZE_STRUCT(j, c, custom_package_name);
		SERIALIZE_STRUCT(j, c, server_id);
		SERIALIZE_STRUCT(j, c, device_id);
		SERIALIZE_STRUCT(j, c, profile_id);
		SERIALIZE_STRUCT(j, c, quality_id);
		SERIALIZE_STRUCT(j, c, embed);
		SERIALIZE_STRUCT(j, c, ch_list);
		SERIALIZE_STRUCT(j, c, m_direct_links);
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
		DESERIALIZE_STRUCT(j, c, suffix);
		DESERIALIZE_STRUCT(j, c, caption);
		DESERIALIZE_STRUCT(j, c, logo);
		DESERIALIZE_STRUCT(j, c, background);
		DESERIALIZE_STRUCT(j, c, update_url);
		DESERIALIZE_STRUCT(j, c, update_package_url);
		DESERIALIZE_STRUCT(j, c, version_id);
		DESERIALIZE_STRUCT(j, c, update_name);
		DESERIALIZE_STRUCT(j, c, package_name);
		DESERIALIZE_STRUCT(j, c, ch_web_path);
		DESERIALIZE_STRUCT(j, c, custom_increment);
		DESERIALIZE_STRUCT(j, c, custom_update_name);
		DESERIALIZE_STRUCT(j, c, custom_package_name);
		DESERIALIZE_STRUCT(j, c, server_id);
		DESERIALIZE_STRUCT(j, c, device_id);
		DESERIALIZE_STRUCT(j, c, profile_id);
		DESERIALIZE_STRUCT(j, c, quality_id);
		DESERIALIZE_STRUCT(j, c, embed);
		DESERIALIZE_STRUCT(j, c, ch_list);
		DESERIALIZE_STRUCT(j, c, m_direct_links);
	}

public:
	std::string login;
	std::string password;
	std::string token;
	std::string subdomain;
	std::string portal;
	std::string comment;
	std::string config;
	std::string suffix;
	std::string caption;
	std::string logo;
	std::string background;
	std::string update_url;
	std::string update_package_url;
	std::string version_id;
	std::string update_name;
	std::string package_name;
	std::string ch_web_path;
	int custom_increment = 0;
	int custom_update_name = 0;
	int custom_package_name = 0;
	int server_id = 0; // zero based index
	int device_id = 0; // zero based index
	int profile_id = 0; // zero based index
	int quality_id = 0; // zero based index
	int embed = 0;
	std::vector<std::string> ch_list;
	std::map<std::string, std::string> m_direct_links;
	bool not_valid = false;
};
