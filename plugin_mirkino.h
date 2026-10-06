/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2026): sharky72 (https://github.com/KocourKuba)

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
#include "vod_movie.h"

/// <summary>
/// Mir Kino VOD provider. Jellyfin server API
/// </summary>
class plugin_mirkino : public base_plugin
{
public:
	bool get_vod_api_token(TemplateParams& params, std::string& api_token) override;
	void clear_account_info() override;
	void parse_vod(const ThreadConfig& config) override;
	void fetch_movie_info(const TemplateParams& params, vod_movie_def& movie) override;
	std::wstring get_movie_url(const std::shared_ptr<Credentials>& creds, const movie_request& request, const vod_movie_def& movie) override;

private:
	bool login(const TemplateParams& params, bool force = false);
	nlohmann::json jellyfin_get(const TemplateParams& params, const std::wstring& path, bool use_cache = true);
	nlohmann::json jellyfin_request(utils::http_request& req) const;
	std::vector<std::string> build_headers(bool auth) const;
	std::wstring get_image_url(const std::wstring& item_id) const;
	std::wstring get_stream_url(const std::wstring& item_id, const std::wstring& source_id, const std::wstring& container, bool direct, const std::wstring& session_id = L"") const;
	void fill_qualities(const std::wstring& item_id, const nlohmann::json& media_sources, vod_episode_def& episode) const;

private:
	std::wstring base_url;
	std::wstring device_id;
	std::wstring session_file;
	std::string access_token;
	std::string user_id;
};
