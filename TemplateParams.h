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
#include "Credentials.h"
#include "PluginEnums.h"

struct TemplateParams
{
	StreamType streamSubtype = StreamType::enHLS;
	Credentials creds;

	std::wstring port;
	std::wstring host;
	std::wstring catchup_source;
	std::wstring catchup_template;
	std::wstring server_id;
	std::wstring device_id;
	std::wstring profile_id;
	std::wstring command;
	std::wstring command_param;
	std::wstring error_string;
	int shift_back = 0;
	int playlist_idx = 0;
	int server_idx = 0;
	int device_idx = 0;
	int profile_idx = 0;
	int quality_idx = 0;
	int domain_idx = 0;

	std::string get_port() const { return utils::utf16_to_utf8(port); }
	std::string get_host() const { return utils::utf16_to_utf8(host); }
	std::string get_catchup_source() const { return utils::utf16_to_utf8(catchup_source); }
	std::string get_catchup_template() const { return utils::utf16_to_utf8(catchup_template); }
	std::string get_server_id() const { return utils::utf16_to_utf8(server_id); }
	std::string get_device_id() const { return utils::utf16_to_utf8(device_id); }
	std::string get_profile_id() const { return utils::utf16_to_utf8(profile_id); }
	std::string get_command() const { return utils::utf16_to_utf8(command); }
	std::string get_command_param() const { return utils::utf16_to_utf8(command_param); }
};
