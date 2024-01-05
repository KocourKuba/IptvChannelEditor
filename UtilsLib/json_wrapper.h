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
#include "nlohmann\json.hpp"
#include "utils.h"

#define SERIALIZE_STRUCT(j, c, s) j[""#s""] = c.s;
#define SERIALIZE_STRUCT2(j, c, s, f) j[""#f""] = c.s;
#define DESERIALIZE_STRUCT(j, c, s) if (j.contains(""#s"")) j.at(""#s"").get_to(c.s);
#define DESERIALIZE_STRUCT2(j, c, s, f) if (j.contains(""#f"")) j.at(""#f"").get_to(c.s);

#define JSON_ALL_TRY try {
#define JSON_ALL_CATCH } \
		catch (const nlohmann::json::parse_error& ex) \
		{ \
			/* parse errors are ok, because input may be random bytes*/ \
			TRACE("\nparse error: %s\n", ex.what()); \
		} \
		catch (const nlohmann::json::out_of_range& ex) \
		{ \
			/* out of range errors may happen if provided sizes are excessive */ \
			TRACE("\nout of range error: %s\n", ex.what()); \
		} \
		catch (const nlohmann::detail::type_error& ex) \
		{ \
			/* type error */ \
			TRACE("\ntype error: %s\n", ex.what()); \
		} \
		catch (...) \
		{ \
			TRACE("\nunknown exception\n"); \
		}

namespace utils
{
	inline std::wstring get_json_wstring(const std::string& key, const nlohmann::json& node)
	{
		std::wstring ret_value;
		if (node.contains(key))
		{
			const auto& js = node[key];
			if (js.is_number_integer())
			{
				ret_value = std::move(std::to_wstring(js.get<int>()));
			}
			if (js.is_number_float())
			{
				ret_value = std::move(std::to_wstring(js.get<float>()));
			}
			else if (js.is_string())
			{
				ret_value = std::move(utf8_to_utf16(js.get<std::string>()));
			}
		}

		return ret_value;
	}

	inline std::string get_json_string(const std::string& key, const nlohmann::json& node)
	{
		std::string ret_value;
		if (node.contains(key))
		{
			const auto& js = node[key];
			if (js.is_number_integer())
			{
				ret_value = std::move(std::to_string(js.get<int>()));
			}
			if (js.is_number_float())
			{
				ret_value = std::move(std::to_string(js.get<float>()));
			}
			else if (js.is_string())
			{
				ret_value = js.get<std::string>();
			}
		}

		return ret_value;
	}

	inline int get_json_int(const std::string& key, const nlohmann::json& node)
	{
		int ret_value = 0;
		if (node.contains(key))
		{
			const auto& js = node[key];
			if (js.is_number_integer())
			{
				ret_value = js.get<int>();
			}
			else if (js.is_string())
			{
				try
				{
					ret_value = std::stoi(js.get<std::string>());
				}
				catch (...)
				{
				}
			}
		}

		return ret_value;
	}


	inline std::string get_json_string_value(const std::string& key, const nlohmann::json& val)
	{
		return val.contains(key) && val[key].is_string() ? val[key] : "";
	}

	inline time_t get_json_int_value(const std::string& key, const nlohmann::json& val)
	{
		if (val.contains(key))
		{
			if (val[key].is_number())
			{
				return val.value(key, 0);
			}

			if (val[key].is_string())
			{
				return utils::char_to_int(val.value(key, ""));
			}
		}

		return 0;
	}
}
