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

/// <summary>
/// Catchup parameters to generate online and archive streams
/// </summary>
struct StreamParameters
{
	StreamType stream_type = StreamType::enLast;
	CatchupType cu_type = CatchupType::cu_shift;
	int cu_duration = 10800;

	std::string uri_template;
	std::string uri_arc_template;
	std::string uri_custom_arc_template;
	std::string dune_params;

	std::wstring get_stream_template() const { return utils::utf8_to_utf16(uri_template); }
	std::wstring get_stream_arc_template() const { return utils::utf8_to_utf16(uri_arc_template); }
	std::wstring get_custom_stream_arc_template() const { return utils::utf8_to_utf16(uri_custom_arc_template); }
	std::wstring get_dune_params() const { return utils::utf8_to_utf16(dune_params); }

	void set_uri_template(const std::wstring& value) { uri_template = utils::utf16_to_utf8(value); }
	void set_uri_arc_template(const std::wstring& value) { uri_arc_template = utils::utf16_to_utf8(value); }
	void set_uri_custom_arc_template(const std::wstring& value) { uri_custom_arc_template = utils::utf16_to_utf8(value); }
	void set_dune_params(const std::wstring& value) { dune_params = utils::utf16_to_utf8(value); }

	friend void to_json(nlohmann::json& j, const StreamParameters& c)
	{
		SERIALIZE_STRUCT(j, c, stream_type);
		SERIALIZE_STRUCT(j, c, uri_template);
		SERIALIZE_STRUCT(j, c, uri_arc_template);
		SERIALIZE_STRUCT(j, c, uri_custom_arc_template);
		SERIALIZE_STRUCT(j, c, cu_type);
		SERIALIZE_STRUCT(j, c, cu_duration);
		SERIALIZE_STRUCT(j, c, dune_params);
	}

	friend void from_json(const nlohmann::json& j, StreamParameters& c)
	{
		DESERIALIZE_STRUCT(j, c, stream_type);
		DESERIALIZE_STRUCT(j, c, uri_template);
		DESERIALIZE_STRUCT(j, c, uri_arc_template);
		DESERIALIZE_STRUCT(j, c, uri_custom_arc_template);
		DESERIALIZE_STRUCT(j, c, cu_type);
		DESERIALIZE_STRUCT(j, c, cu_duration);
		DESERIALIZE_STRUCT(j, c, dune_params);
	}
};
