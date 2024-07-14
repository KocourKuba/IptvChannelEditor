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

class plugin_iptvonline : public base_plugin
{
public:
	bool get_api_token(TemplateParams& params) override;
	std::wstring get_playlist_url(const TemplateParams& params, std::wstring url = L"") override;
	void parse_account_info(TemplateParams& params) override;
	void fill_servers_list(TemplateParams& params) override;
	bool set_server(TemplateParams& params) override;
	void parse_vod(const CThreadConfig& config) override;
	void fetch_movie_info(const Credentials& creds, vod_movie& movie) override;
	std::wstring get_movie_url(const Credentials& creds, const movie_request& request, const vod_movie& movie) override;

private:
	void collect_movies(const std::wstring& id,
						const std::wstring& category_name,
						const CThreadConfig & config,
						std::unique_ptr<vod_category_storage>& categories,
						bool is_serial = false);
	nlohmann::json server_request(const std::wstring& url,
								  const Credentials& creds,
								  int cache_ttl,
								  bool web_post = false,
								  const nlohmann::json& post_data = {});
};
