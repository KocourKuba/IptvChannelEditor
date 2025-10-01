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
#include <string>
#include <vector>
#include <atlenc.h>
#include <sstream>
#include <future>
#include <variant>

namespace utils
{

constexpr auto pc_user_agent = L"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/112.0.0.0 Safari/537.36 Edg/112.0.1722.48";

struct CrackedUrl
{
	std::wstring scheme;
	std::wstring user;
	std::wstring password;
	std::wstring host;
	std::wstring path;
	std::wstring extra_info;
	unsigned short port = 80;
	unsigned short nScheme = 1; // INTERNET_SCHEME_HTTP
};

enum class ProgressType
{
	Initializing,
	Finalizing,
	Progress

};

struct progress_info
{
	ProgressType type = ProgressType::Initializing;
	int maxPos = 0;
	int curPos = 0;
	int value = 0;
	void* extraData = nullptr;
};

struct http_request
{
	std::wstring url;
	std::chrono::seconds cache_ttl = std::chrono::seconds::zero();
	int max_redirect = 5;
	std::vector<std::string> headers;
	std::wstring user_agent{pc_user_agent};
	std::string post_data;
	std::wstring error_message;
	bool verb_post = false;
	std::stringstream body;
	std::stop_token stop_token;
	std::function<void(const progress_info&)> progress_callback = nullptr;
};

std::future<bool> AsyncDownloadFile(http_request& request);
bool DownloadFile(http_request& request);
bool CheckIsCacheExpired(const std::wstring& cache_file, const std::chrono::seconds& cache_ttl);
void ClearCache();
void ClearCachedUrl(const std::wstring& url);
std::filesystem::path GetCacheDir();
std::filesystem::path GetCachedPath(const std::wstring& url);
bool CrackUrl(const std::wstring& url, CrackedUrl* st = nullptr);
std::string encodeURIComponent(const std::string& url);
std::string entityDecrypt(const std::string& text);

class CBase64Coder
{
	//Methods
public:
	// Encode data to internal buffer to BASE64
	bool Encode(const unsigned char* pData, int nSize, unsigned long dwFlags = ATL_BASE64_FLAG_NOCRLF);
	bool Encode(const char* szMessage, int nSize = 0, unsigned long dwFlags = ATL_BASE64_FLAG_NOCRLF);

	// Decode data to internal buffer from BASE64
	bool Decode(const char* pData, int nSize);
	bool Decode(const char* szMessage);

	// Reinterpret internal buffer to type
	std::string GetResultAsString() const { return { m_buf.begin(), m_buf.end() }; };
	const unsigned char* GetResultBytes() const { return m_buf.data(); };

	// size of encoded/decoded buffer
	int	GetResultSize() const { return m_nSize; };

	const std::vector<unsigned char>& GetBuffer() const { return m_buf; }

protected:
	//Member variables
	std::vector<unsigned char> m_buf{};
	int m_nSize = 0;
};
}
