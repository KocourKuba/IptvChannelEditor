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

#include "pch.h"
#include "inet_utils.h"

#include <winhttp.h>
#include <atltrace.h>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include "xxhash.hpp"
#include "utils.h"
#include <coroutine>
#include <future>

#pragma comment(lib, "Winhttp.lib")

#define MAX_URL_LENGTH 210

namespace utils
{

std::string encodeURIComponent(const std::string& value)
{
	std::ostringstream escaped;
	escaped.fill('0');
	escaped << std::hex;

	for (char c : value) {
		// Keep alphanumeric and other accepted characters intact
		if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
			escaped << c;
			continue;
		}

		// Any other characters are percent-encoded
		escaped << std::uppercase;
		escaped << '%' << std::setw(2) << static_cast<int>((static_cast<unsigned char>(c)));
		escaped << std::nouppercase;
	}

	return escaped.str();
}

bool CrackUrl(const std::wstring& url, CrackedUrl* st /*= nullptr*/)
{
	URL_COMPONENTS urlComp{};
	SecureZeroMemory(&urlComp, sizeof(urlComp));
	urlComp.dwStructSize = sizeof(urlComp);

	// Set required component lengths to non-zero so that they are cracked.
	urlComp.dwSchemeLength = (DWORD)-1;
	urlComp.dwUserNameLength = (DWORD)-1;
	urlComp.dwPasswordLength = (DWORD)-1;
	urlComp.dwHostNameLength = (DWORD)-1;
	urlComp.dwUrlPathLength = (DWORD)-1;
	urlComp.dwExtraInfoLength = (DWORD)-1;

	if (!WinHttpCrackUrl(url.c_str(), static_cast<DWORD>(url.size()), 0, &urlComp))
	{
		return false;
	}

	if (st != nullptr)
	{
		if (urlComp.dwSchemeLength)
			st->scheme.assign(urlComp.lpszScheme, urlComp.dwSchemeLength);
		if (urlComp.dwUserNameLength)
			st->user.assign(urlComp.lpszUserName, urlComp.dwUserNameLength);
		if (urlComp.dwPasswordLength)
			st->password.assign(urlComp.lpszPassword, urlComp.dwPasswordLength);
		if (urlComp.dwHostNameLength)
			st->host.assign(urlComp.lpszHostName, urlComp.dwHostNameLength);
		if (urlComp.dwUrlPathLength)
			st->path.assign(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);
		if (urlComp.dwExtraInfoLength)
			st->extra_info.assign(urlComp.lpszExtraInfo, urlComp.dwExtraInfoLength);

		st->port = urlComp.nPort;
		st->nScheme = urlComp.nScheme;
	}

	return true;
}


#ifndef defer

template <typename T>
struct deferrer
{
	T f;
	deferrer(T f) : f(f) {};
	deferrer(const deferrer&) = delete;
	~deferrer() { f(); }
};

#define TOKEN_CONCAT_NX(a, b) a ## b
#define TOKEN_CONCAT(a, b) TOKEN_CONCAT_NX(a, b)
#define defer deferrer TOKEN_CONCAT(__deferred, __COUNTER__) =

#endif

bool DownloadFile(http_request& request)
{
	request.error_message.clear();
	if (request.url.empty())
	{
		return false;
	}

	std::wstring hash_str = request.url;
	if (!request.post_data.empty())
	{
		hash_str += utf8_to_utf16(std::string(request.post_data));
	}

	ATLTRACE(L"\ndownload url: %s\n", request.url.c_str());

	const auto& cache_file = GetCachedPath(hash_str);
	if (!CheckIsCacheExpired(cache_file, request.cache_ttl))
	{
		std::ifstream in_file(cache_file.c_str());
		if (in_file.good())
		{
			request.body << in_file.rdbuf();
			in_file.close();
			size_t data_size = request.body.rdbuf()->_Get_buffer_view()._Size;
			ATLTRACE(L"\nloaded: %d bytes from cache %s\n", data_size, cache_file.c_str());
			if (data_size != 0)
			{
				request.body.seekg(0);
				return true;
			}
		}
	}
	else
	{
		ATLTRACE("\nCache expired. Download again.\n");
	}

	do
	{
		CrackedUrl cracked;
		if (!utils::CrackUrl(request.url, &cracked))
		{
			request.error_message += L"Error: Failed to parse url";
			break;
		}

		// Use WinHttpOpen to obtain a session handle.
		ATLTRACE(L"\nUserAgent: %s\n", request.user_agent.c_str());
		auto hSession = WinHttpOpen(request.user_agent.c_str(),
									 WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
									 WINHTTP_NO_PROXY_NAME,
									 WINHTTP_NO_PROXY_BYPASS, 0);
		if (!hSession)
		{
			request.error_message = L"Error: Failed to open connection";
			break;
		}
		defer[&]{ WinHttpCloseHandle(hSession); };

		WinHttpSetTimeouts(hSession, 0, 10000, 10000, 60000);

		// Specify an HTTP server.
		auto hConnect = WinHttpConnect(hSession, cracked.host.c_str(), cracked.port, 0);
		if (!hConnect)
		{
			request.error_message = L"Error: Failed to connect";
			break;
		}
		defer[&]{ WinHttpCloseHandle(hConnect); };

		DWORD dwFlags = WINHTTP_FLAG_BYPASS_PROXY_CACHE;
		if (cracked.nScheme == 2) /*INTERNET_SCHEME_HTTPS*/
			dwFlags |= 0x00800000; // INTERNET_FLAG_SECURE
		// Create an HTTP request handle.
		auto hRequest = WinHttpOpenRequest(hConnect,
										   request.verb_post ? L"POST" : L"GET",
										   (cracked.path + cracked.extra_info).c_str(),
										   nullptr,
										   WINHTTP_NO_REFERER,
										   WINHTTP_DEFAULT_ACCEPT_TYPES,
										   dwFlags);

		if (!hRequest)
		{
			request.error_message = L"Error: Failed to open request";
			break;
		}
		defer[&]{ WinHttpCloseHandle(hRequest); };

		if (!request.headers.empty())
		{
			std::wstring all_headers;
			for (const auto& str : request.headers)
			{
				all_headers += utils::utf8_to_utf16(str) + L"\n";
			}

			BOOL result = WinHttpAddRequestHeaders(hRequest, all_headers.c_str(), static_cast<DWORD>(all_headers.size()), WINHTTP_ADDREQ_FLAG_ADD);
			ATLTRACE("\nheader added: %d\n", result);
		}

		DWORD options = SECURITY_FLAG_IGNORE_ALL_CERT_ERRORS;
		WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &options, sizeof(DWORD));

		// Send a request.
		bool bResults = false;
		bool bRepeat = false;
		bool bSaveBadCache = false;
		for (;;)
		{
			DWORD post_size = request.post_data.empty() ? 0 : static_cast<DWORD>(request.post_data.length());
			if (!WinHttpSendRequest(hRequest,
								   WINHTTP_NO_ADDITIONAL_HEADERS,
								   0,
									request.post_data.empty() ? WINHTTP_NO_REQUEST_DATA : reinterpret_cast<void*>(request.post_data.data()),
									post_size,
								   post_size,
								   0))
			{
				request.error_message = L"Error: Failed to send request";
				break;
			}


			// End the request.
			if (!WinHttpReceiveResponse(hRequest, nullptr))
			{
				request.error_message = std::format(L"Error: Failed to receive response: error code {:d}", GetLastError());
				break;
			}

			DWORD dwStatusCode = 0;
			DWORD dwSize = sizeof(dwStatusCode);

			if (!WinHttpQueryHeaders(hRequest,
									 WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
									 WINHTTP_HEADER_NAME_BY_INDEX,
									 &dwStatusCode, &dwSize, WINHTTP_NO_HEADER_INDEX))
			{
				request.error_message = std::format(L"Error: Failed to query headers: error code {:d}", GetLastError());
				break;
			}

			switch (dwStatusCode)
			{
				case 401:
				{
					DWORD dwSupportedSchemes = 0;
					DWORD dwFirstScheme = 0;
					DWORD dwAuthTarget = 0;

					if (!WinHttpQueryAuthSchemes(hRequest, &dwSupportedSchemes, &dwFirstScheme, &dwAuthTarget))
					{
						request.error_message = L"Error: Failed to query authentication schemes";
						break;
					}

					//This default implementation will allow any authentication scheme support
					//and will pick in order of "decreasing strength"
					DWORD dwAuthScheme = 0;

					if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_NEGOTIATE)
						dwAuthScheme = WINHTTP_AUTH_SCHEME_NEGOTIATE;
					else if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_NTLM)
						dwAuthScheme = WINHTTP_AUTH_SCHEME_NTLM;
					else if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_PASSPORT)
						dwAuthScheme = WINHTTP_AUTH_SCHEME_PASSPORT;
					else if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_DIGEST)
						dwAuthScheme = WINHTTP_AUTH_SCHEME_DIGEST;
					else if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_BASIC)
						dwAuthScheme = WINHTTP_AUTH_SCHEME_BASIC;

					if (!WinHttpSetCredentials(hRequest,
											   WINHTTP_AUTH_TARGET_SERVER,
											   dwAuthScheme,
											   cracked.user.empty() ? nullptr : cracked.user.c_str(),
											   cracked.password.c_str(),
											   nullptr))
					{
						request.error_message = std::format(L"Error: Failed to set credentials: error code {:d}", GetLastError());
						break;
					}

					bResults = true;
					bRepeat = true;
					break;
				}

				case 200:
					request.max_redirect = 5;
					bResults = true;
					bRepeat = false;
					break;

				case 301:
				case 302:
				{
					--request.max_redirect;
					if (request.max_redirect <= 0)
					{
						return false;
					}

					DWORD dwBuffer = 0;
					WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, nullptr, nullptr, &dwBuffer, nullptr);
					std::wstring chData;
					chData.resize(dwBuffer / 2);
					wchar_t* chBuf = &chData[0];
					WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, nullptr, chBuf, &dwBuffer, nullptr);
					const auto& lines = utils::regex_split(chData, L"\r\n");
					boost::wregex re(LR"(^Location:\s(.+)$)");
					boost::wsmatch m;
					for (const auto& line : lines)
					{
						if (boost::regex_match(line, m, re))
						{
							request.url = m[1].str();
							return DownloadFile(request);
						}
					}
					break;
				}

				case 304:
					bResults = false;
					bRepeat = false;
					bSaveBadCache = true;
					break;

				default:
					request.error_message = std::format(L"Response returns error code: {:d}", dwStatusCode);
					break;
			}

			if (!bResults || !bRepeat) break;
		}


		DWORD dwSize = 0;
		for (;;)
		{
			if (!bResults)
			{
				request.error_message += std::format(L"\nError code: {:d}", GetLastError());
				break;
			}
			// Check for available data.
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
			{
				request.error_message += std::format(L"\nError: WinHttpQueryDataAvailable error code {:d}", GetLastError());
				break;
			}

			// Allocate space for the buffer.
			if (!dwSize) break;
			std::vector<unsigned char> chunk(dwSize);

			DWORD dwDownloaded = 0;
			if (WinHttpReadData(hRequest, chunk.data(), dwSize, &dwDownloaded))
			{
				request.body.write(reinterpret_cast<const char*>(chunk.data()), dwDownloaded);
			}
			else
			{
				request.error_message = std::format(L"\nError: WinHttpReadData error code {:d}", GetLastError());
				break;
			}
		}

		if (!bResults && !bSaveBadCache) break;

		if (!request.body.fail() || bSaveBadCache)
		{
			std::ofstream out_stream(cache_file, std::ofstream::binary);
			request.body.seekg(0);
			out_stream << request.body.rdbuf();
			ATLTRACE("\nSave to cache for %d seconds\n", request.cache_ttl);
		}

		bool res = request.body.tellp() != std::streampos(0);
		if (!res)
		{
			if (request.error_message.empty())
			{
				request.error_message = L"Error: Empty response";
			}
			break;
		}

		request.body.seekg(0);
		return true;
	} while (false);

#ifdef _DEBUG
	if (!request.error_message.empty())
	{
		request.error_message = request.url + L"\n" + request.error_message;
		ATLTRACE(L"%s\n", request.error_message.c_str());
	}
#endif // _DEBUG

	return false;
}

std::filesystem::path GetCachedPath(const std::wstring& url)
{
	return GetCacheDir() / std::format(L"{:08x}", xxh::xxhash<32>(url));
}

std::filesystem::path GetCacheDir()
{
	std::filesystem::path dir = std::filesystem::temp_directory_path().append(L"iptv_cache");
	std::error_code err;
	std::filesystem::create_directories(dir, err);
	return dir;
}

void ClearCache()
{
	std::error_code err;
	std::filesystem::remove_all(GetCacheDir(), err);
}

void ClearCachedUrl(const std::wstring& url)
{
	const auto& cache_file = GetCachedPath(url);
	if (std::filesystem::exists(cache_file))
	{
		std::error_code err;
		std::filesystem::remove(cache_file, err);
	}
}

bool CheckIsCacheExpired(const std::wstring& cache_file, const std::chrono::seconds& cache_ttl)
{
	if (cache_ttl != std::chrono::seconds::zero() && std::filesystem::exists(cache_file) && std::filesystem::file_size(cache_file) != 0)
	{
		auto diff = std::chrono::duration_cast<std::chrono::seconds>(std::filesystem::file_time_type::clock::now() - std::filesystem::last_write_time(cache_file));
		ATLTRACE(L"\nttl: %d hours %d minutes %d seconds\n",
				 std::chrono::duration_cast<std::chrono::hours>(diff),
				 std::chrono::duration_cast<std::chrono::minutes>(diff) % 60,
				 diff % 60);
		return (diff > cache_ttl);
	}

	return true;
}

std::string entityDecrypt(const std::string& text)
{
	std::unordered_map<std::string, std::string> convert(
		{
			{ "&quot;",  "\"" },
			{ "&apos;",  "'"  },
			{ "&#39;",   "'"  },
			{ "&amp;",   "&"  },
			{ "&gt;",    ">"  },
			{ "&lt;",    "<"  },
			{ "&frasl;", "/"  },
			{ "<br>",   "\n" },
		});

	std::string res;
	for (size_t i = 0; i < text.size(); ++i)
	{
		bool flag = false;
		for (const auto& [key, value] : convert)
		{
			if ((i + key.size() - 1 < text.size()) && (text.substr(i, key.size()) == key))
			{
				res += value;
				i += key.size() - 1;
				flag = true;
				break;
			}
		}

		if (!flag)
		{
			res += text[i];
		}
	}
	return res;
}

//////////////////////////////////////////////////////////////////////////

bool CBase64Coder::Encode(const unsigned char* pData, int nSize, unsigned long dwFlags /*= ATL_BASE64_FLAG_NOCRLF*/)
{
	//Tidy up any heap memory we have been using
	m_buf.clear();

	//Calculate and allocate the buffer to store the encoded data
	m_nSize = Base64EncodeGetRequiredLength(nSize, dwFlags) + 1; //We allocate an extra byte so that we can null terminate the result
	m_buf.resize(m_nSize);

	//Finally do the encoding
	if (!ATL::Base64Encode(pData, nSize, reinterpret_cast<char*>(m_buf.data()), &m_nSize, dwFlags)) return false;

	//Null terminate the data
	m_buf[m_nSize] = 0;

	return true;
}

bool CBase64Coder::Encode(const char* szMessage, int nSize /*= 0*/, unsigned long dwFlags /*= ATL_BASE64_FLAG_NOCRLF*/)
{
	if (!nSize)
		nSize = static_cast<int>(std::strlen(szMessage));

	return Encode(reinterpret_cast<const BYTE*>(szMessage), nSize, dwFlags);
}

bool CBase64Coder::Decode(const char* pData, int nSize)
{
	//Tidy up any heap memory we have been using
	m_buf.clear();

	//Calculate and allocate the buffer to store the encoded data
	m_nSize = Base64DecodeGetRequiredLength(nSize) + 1;
	m_buf.resize(m_nSize);

	//Finally do the encoding
	if (!Base64Decode(pData, nSize, m_buf.data(), &m_nSize)) return false;

	//Null terminate the data
	m_buf[m_nSize] = 0;

	return true;
}

bool CBase64Coder::Decode(const char* szMessage)
{
	return Decode(szMessage, static_cast<int>(std::strlen(szMessage)));
}

}
