/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2025): sharky72 (https://github.com/KocourKuba)

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

#include "UtilsLib/json_wrapper.h"

enum class epg_id_sources
{
	enEpgId = 0,
	enChannelId = 1,
	enChannelName = 2,
};

/// <summary>
/// Parameters to parse EPG
/// </summary>
struct EpgParameters
{
	std::string epg_param; // not changed! hardcoded
	std::string epg_domain;
	std::string epg_url;
	std::string epg_root;
	std::string epg_name;
	std::string epg_desc;
	std::string epg_start;
	std::string epg_end;
	std::string epg_date_format;
	std::string epg_time_format;
	std::string epg_auth;
	int epg_id_source = (int)epg_id_sources::enEpgId;
	size_t epg_timezone = 0;
	bool epg_use_duration = false;

	bool compare_preset(const EpgParameters& src) const
	{
		return (epg_root == src.epg_root
				&& epg_name == src.epg_name
				&& epg_desc == src.epg_desc
				&& epg_start == src.epg_start
				&& epg_end == src.epg_end
				&& epg_time_format == src.epg_time_format
				&& epg_timezone == src.epg_timezone
				&& epg_use_duration == src.epg_use_duration
				&& epg_auth == src.epg_auth
				&& epg_id_source == src.epg_id_source
				);
	}

	std::wstring get_epg_domain() const { return utils::utf8_to_utf16(epg_domain); }
	void set_epg_domain(const std::wstring& val) { epg_domain = utils::utf16_to_utf8(val); }

	std::wstring get_epg_url() const { return utils::utf8_to_utf16(epg_url); }
	void set_epg_url(const std::wstring& val) { epg_url = utils::utf16_to_utf8(val); }

	std::wstring get_epg_root() const { return utils::utf8_to_utf16(epg_root); }
	void set_epg_root(const std::wstring& val) { epg_root = utils::utf16_to_utf8(val); }

	std::wstring get_epg_name() const { return utils::utf8_to_utf16(epg_name); }
	void set_epg_name(const std::wstring& val) { epg_name = utils::utf16_to_utf8(val); }

	std::wstring get_epg_desc() const { return utils::utf8_to_utf16(epg_desc); }
	void set_epg_desc(const std::wstring& val) { epg_desc = utils::utf16_to_utf8(val); }

	std::wstring get_epg_start() const { return utils::utf8_to_utf16(epg_start); }
	void set_epg_start(const std::wstring& val) { epg_start = utils::utf16_to_utf8(val); }

	std::wstring get_epg_end() const { return utils::utf8_to_utf16(epg_end); }
	void set_epg_end(const std::wstring& val) { epg_end = utils::utf16_to_utf8(val); }

	std::wstring get_epg_date_format() const { return utils::utf8_to_utf16(epg_date_format); }
	void set_epg_date_format(const std::wstring& val) { epg_date_format = utils::utf16_to_utf8(val); }

	std::wstring get_epg_time_format() const { return utils::utf8_to_utf16(epg_time_format); }
	void set_epg_time_format(const std::wstring& val) { epg_time_format = utils::utf16_to_utf8(val); }

	std::wstring get_epg_auth() const { return utils::utf8_to_utf16(epg_auth); }
	void set_epg_auth(const std::wstring& val) { epg_auth = utils::utf16_to_utf8(val); }

	int get_epg_id_source() const { return epg_id_source; }
	void set_epg_id_source(const int val) { epg_id_source = val; }

	static void to_json_wrapper(nlohmann::json& j, const EpgParameters& c)
	{
		to_json(j, c);
	}

	friend void to_json(nlohmann::json& j, const EpgParameters& c)
	{
		SERIALIZE_STRUCT(j, c, epg_param);
		SERIALIZE_STRUCT(j, c, epg_domain);
		SERIALIZE_STRUCT(j, c, epg_url);
		SERIALIZE_STRUCT(j, c, epg_root);
		SERIALIZE_STRUCT(j, c, epg_name);
		SERIALIZE_STRUCT(j, c, epg_desc);
		SERIALIZE_STRUCT(j, c, epg_start);
		SERIALIZE_STRUCT(j, c, epg_end);
		SERIALIZE_STRUCT(j, c, epg_date_format);
		SERIALIZE_STRUCT(j, c, epg_time_format);
		SERIALIZE_STRUCT(j, c, epg_auth);
		SERIALIZE_STRUCT(j, c, epg_timezone);
		SERIALIZE_STRUCT(j, c, epg_id_source);
		SERIALIZE_STRUCT(j, c, epg_use_duration); //-V601
	}

	static void from_json_wrapper(const nlohmann::json& j, EpgParameters& c)
	{
		from_json(j, c);
	}

	friend void from_json(const nlohmann::json& j, EpgParameters& c)
	{
		DESERIALIZE_STRUCT(j, c, epg_param);
		DESERIALIZE_STRUCT(j, c, epg_domain);
		DESERIALIZE_STRUCT(j, c, epg_url);
		DESERIALIZE_STRUCT(j, c, epg_root);
		DESERIALIZE_STRUCT(j, c, epg_name);
		DESERIALIZE_STRUCT(j, c, epg_desc);
		DESERIALIZE_STRUCT(j, c, epg_start);
		DESERIALIZE_STRUCT(j, c, epg_end);
		DESERIALIZE_STRUCT(j, c, epg_date_format);
		DESERIALIZE_STRUCT(j, c, epg_time_format);
		DESERIALIZE_STRUCT(j, c, epg_auth);
		DESERIALIZE_STRUCT(j, c, epg_timezone);
		DESERIALIZE_STRUCT(j, c, epg_id_source);
		DESERIALIZE_STRUCT(j, c, epg_use_duration);
	}
};
