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
#include "base_plugin.h"

class plugin_sharaclub : public base_plugin
{
public:
	void configure_provider_plugin() override;
	std::wstring get_playlist_url(TemplateParams& params, std::wstring url = L"") override;
	std::map<std::wstring, std::wstring, std::less<>> parse_access_info(TemplateParams& params) override;
	void fill_servers_list(TemplateParams* params = nullptr) override;
	bool set_server(TemplateParams& params) override;
	void fill_profiles_list(TemplateParams* params = nullptr) override;
	bool set_profile(TemplateParams& params) override;
};
