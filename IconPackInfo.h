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

#include "PluginEnums.h"

#include "UtilsLib\json_wrapper.h"

/// <summary>
/// Parameters to parse ImageLibs
/// </summary>
class IconPackInfo
{
public:
	ImageLibType get_type() const { return type; }
	void set_type(const ImageLibType val) { type = val; }

	std::wstring get_name() const { return utils::utf8_to_utf16(name); }
	void set_name(const std::wstring& val) { name = utils::utf16_to_utf8(val); }

	std::wstring get_package_name() const { return utils::utf8_to_utf16(package_name); }
	void set_package_name(const std::wstring& val) { package_name = utils::utf16_to_utf8(val); }

	std::wstring get_url() const { return utils::utf8_to_utf16(url); }
	void set_url(const std::wstring& val) { url = utils::utf16_to_utf8(val); }

	bool get_square() const { return square; }
	void set_square(const bool val) { square = val; }

	static void to_json_wrapper(nlohmann::json& j, const IconPackInfo& c)
	{
		to_json(j, c);
	}

	static void from_json_wrapper(const nlohmann::json& j, IconPackInfo& c)
	{
		from_json(j, c);
	}

	friend void to_json(nlohmann::json& j, const IconPackInfo& c)
	{
		SERIALIZE_STRUCT(j, c, type);
		SERIALIZE_STRUCT(j, c, name);
		SERIALIZE_STRUCT(j, c, package_name);
		SERIALIZE_STRUCT(j, c, url);
		SERIALIZE_STRUCT(j, c, square); //-V601
	}

	friend void from_json(const nlohmann::json& j, IconPackInfo& c)
	{
		DESERIALIZE_STRUCT(j, c, type);
		DESERIALIZE_STRUCT(j, c, name);
		DESERIALIZE_STRUCT(j, c, package_name);
		DESERIALIZE_STRUCT(j, c, url);
		DESERIALIZE_STRUCT(j, c, square);
	}

	ImageLibType type = ImageLibType::enNone;
	std::string name;
	std::string package_name;
	std::string url;
	bool square = false;
};
