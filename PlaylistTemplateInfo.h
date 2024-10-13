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
#include "IPTVChannelEditor.h"
#include "UtilsLib/json_wrapper.h"

/// <summary>
/// Playlist template parameter
/// </summary>
class PlaylistTemplateInfo
{
public:
	PlaylistTemplateInfo() = default;

	PlaylistTemplateInfo(const std::string& _name) : name(_name) {}

	PlaylistTemplateInfo(UINT ID)
	{
		set_name(load_string_resource(ID));
	}

	std::wstring get_id() const { return L""; }

	std::wstring get_name() const { return utils::utf8_to_utf16(name); }
	void set_name(const std::wstring& val) { name = utils::utf16_to_utf8(val); }
	void set_name(UINT ID) { name = utils::utf16_to_utf8(load_string_resource(ID)); }

	std::wstring get_pl_template() const { return utils::utf8_to_utf16(pl_template); }
	void set_pl_template(const std::wstring& val) { pl_template = utils::utf16_to_utf8(val); }

	std::wstring get_pl_parse_regex() const { return utils::utf8_to_utf16(pl_parse_regex); }
	void set_pl_parse_regex(const std::wstring& val) { pl_parse_regex = utils::utf16_to_utf8(val); }

	std::wstring get_parse_regex() const { return utils::utf8_to_utf16(parse_regex); }
	void set_parse_regex(const std::wstring& val) { parse_regex = utils::utf16_to_utf8(val); }

	std::wstring get_parse_regex_title() const { return utils::utf8_to_utf16(parse_regex_title); }
	void set_parse_regex_title(const std::wstring& val) { parse_regex = utils::utf16_to_utf8(val); }

	std::wstring get_url_prefix() const { return utils::utf8_to_utf16(url_prefix); }
	void set_url_prefix(const std::wstring& val) { url_prefix = utils::utf16_to_utf8(val); }

	std::wstring get_url_params() const { return utils::utf8_to_utf16(url_params); }
	void set_url_params(const std::wstring& val) { url_params = utils::utf16_to_utf8(val); }

	std::wstring get_tag_id_match() const { return utils::utf8_to_utf16(tag_id_match); }
	void set_tag_id_match(const std::wstring& val) { tag_id_match = utils::utf16_to_utf8(val); }

	bool get_square_icons() const { return square_icons; }
	void set_square_icons(bool val) { square_icons = val; }

	friend void to_json(nlohmann::json& j, const PlaylistTemplateInfo& c)
	{
		SERIALIZE_STRUCT(j, c, name);
		SERIALIZE_STRUCT(j, c, pl_template);
		SERIALIZE_STRUCT(j, c, pl_parse_regex);
		SERIALIZE_STRUCT(j, c, parse_regex);
		SERIALIZE_STRUCT(j, c, parse_regex_title);
		SERIALIZE_STRUCT(j, c, url_prefix);
		SERIALIZE_STRUCT(j, c, url_params);
		SERIALIZE_STRUCT(j, c, tag_id_match);
		SERIALIZE_STRUCT(j, c, square_icons); //-V601
	}

	friend void from_json(const nlohmann::json& j, PlaylistTemplateInfo& c)
	{
		DESERIALIZE_STRUCT(j, c, name);
		DESERIALIZE_STRUCT(j, c, pl_template);
		DESERIALIZE_STRUCT(j, c, pl_parse_regex);
		DESERIALIZE_STRUCT(j, c, parse_regex);
		DESERIALIZE_STRUCT(j, c, parse_regex_title);
		DESERIALIZE_STRUCT(j, c, url_prefix);
		DESERIALIZE_STRUCT(j, c, url_params);
		DESERIALIZE_STRUCT(j, c, tag_id_match);
		DESERIALIZE_STRUCT(j, c, square_icons);
	}

	std::string name;
	std::string pl_template;
	std::string pl_parse_regex;
	std::string parse_regex;
	std::string parse_regex_title;
	std::string url_prefix;
	std::string url_params;
	std::string tag_id_match;
	bool square_icons = false;
	bool is_custom = false;
};

