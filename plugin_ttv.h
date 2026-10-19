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
/// TorrentTV provider. API v3 https://tapitv.com/v3/api_v3.html
/// Account is set by login/password pair: auth returns the API session,
/// the playlist token (S_TOKEN macro of the playlist url) is taken from the account info
/// </summary>
class plugin_ttv : public base_plugin
{
public:
	bool get_api_token(TemplateParams& params, std::string& api_token) override;
	void clear_account_info() override;
	void parse_account_info(TemplateParams& params) override;
	void parse_vod(const ThreadConfig& config) override;
	void fetch_movie_info(const TemplateParams& params, vod_movie_def& movie) override;
	bool get_genre_movies(const TemplateParams& params, const std::wstring& category_id, const std::wstring& genre_id, std::set<std::wstring>& movie_ids) override;

private:
	bool login(const TemplateParams& params, bool force = false);
	nlohmann::json api_request(const TemplateParams& params, const std::wstring& command, const std::wstring& query = L"", bool use_cache = false);
	nlohmann::json download_json(utils::http_request& req) const;
	static std::string make_guid(const std::string& login);

private:
	std::wstring session_file;
	std::string session;
	std::string playlist_token;
	// movies ids of the genre: category|genre_id
	std::map<std::wstring, std::set<std::wstring>> genre_movies_cache;
};
