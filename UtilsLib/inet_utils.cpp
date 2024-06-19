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

#include "pch.h"
#include "inet_utils.h"

#include <winhttp.h>
#include <atltrace.h>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include "xxhash.hpp"
#include "utils.h"
#include "SmartHandle.h"

#pragma comment(lib, "Winhttp.lib")

#define MAX_URL_LENGTH 210

namespace utils
{

template <typename T>
class CCloseHInternet
{
public:
	~CCloseHInternet() = default;

	static bool Close(T handle)
	{
		return FALSE != ::WinHttpCloseHandle(handle);
	}
};

using CAutoHinternet = CSmartHandle<HINTERNET, CCloseHInternet>;

template <typename T>
std::time_t to_time_t(T tp)
{
	auto systp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(tp - T::clock::now() + std::chrono::system_clock::now());
	return std::chrono::system_clock::to_time_t(systp);
}

bool CrackedUrl::CrackUrl(const std::wstring& url)
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

	if (WinHttpCrackUrl(url.c_str(), (DWORD)url.size(), 0, &urlComp))
	{
		if (urlComp.dwSchemeLength)
			scheme.assign(urlComp.lpszScheme, urlComp.dwSchemeLength);
		if (urlComp.dwUserNameLength)
			user.assign(urlComp.lpszUserName, urlComp.dwUserNameLength);
		if (urlComp.dwPasswordLength)
			password.assign(urlComp.lpszPassword, urlComp.dwPasswordLength);
		if (urlComp.dwHostNameLength)
			host.assign(urlComp.lpszHostName, urlComp.dwHostNameLength);
		if (urlComp.dwUrlPathLength)
			path.assign(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);
		if (urlComp.dwExtraInfoLength)
			extra_info.assign(urlComp.lpszExtraInfo, urlComp.dwExtraInfoLength);

		port = urlComp.nPort;
		nScheme = urlComp.nScheme;

		return true;
	}

	return false;
}

bool CUrlDownload::DownloadFile(const std::wstring& url,
								std::stringstream& vData,
								std::vector<std::string>* pHeaders /*= nullptr*/,
								bool verb_post /*= false*/,
								const char* post_data /*= nullptr*/) const
{
	m_error_message.clear();
	if (url.empty())
		return false;

	std::wstring hash_str = url;
	if (post_data)
		hash_str += utf8_to_utf16(std::string(post_data));

	ATLTRACE(L"\ndownload url: %s\n", url.c_str());

	const auto& cache_file = GetCachedPath(hash_str);
	if (!CheckIsCacheExpired(cache_file))
	{
		std::ifstream in_file(cache_file.c_str());
		if (in_file.good())
		{
			vData << in_file.rdbuf();
			in_file.close();
			size_t data_size = vData.rdbuf()->_Get_buffer_view()._Size;
			ATLTRACE(L"\nloaded: %d bytes from cache %s\n", data_size, cache_file.c_str());
			if (data_size != 0)
			{
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
		if (!cracked.CrackUrl(url))
		{
			m_error_message += L"Error: Failed to parse url";
			break;
		}

		// Use WinHttpOpen to obtain a session handle.
		ATLTRACE(L"\nUserAgent: %s\n", m_user_agent.c_str());
		CAutoHinternet hSession = WinHttpOpen(m_user_agent.c_str(),
											  WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
											  WINHTTP_NO_PROXY_NAME,
											  WINHTTP_NO_PROXY_BYPASS, 0);

		if (hSession.IsNotValid())
		{
			m_error_message = L"Error: Failed to open connection";
			break;
		}

		WinHttpSetTimeouts(hSession, 0, 10000, 10000, 60000);

		// Specify an HTTP server.
		CAutoHinternet hConnect = WinHttpConnect(hSession, cracked.host.c_str(), cracked.port, 0);
		if (hConnect.IsNotValid())
		{
			m_error_message = L"Error: Failed to connect";
			break;
		}

		DWORD dwFlags = WINHTTP_FLAG_BYPASS_PROXY_CACHE;
		if (cracked.nScheme == 2 /*INTERNET_SCHEME_HTTPS*/)
			dwFlags |= 0x00800000; // INTERNET_FLAG_SECURE
		// Create an HTTP request handle.
		CAutoHinternet hRequest = WinHttpOpenRequest(hConnect,
													 verb_post ? L"POST" : L"GET",
													 (cracked.path + cracked.extra_info).c_str(),
													 nullptr,
													 WINHTTP_NO_REFERER,
													 WINHTTP_DEFAULT_ACCEPT_TYPES,
													 dwFlags);

		if (hRequest.IsNotValid())
		{
			m_error_message = L"Error: Failed to open request";
			break;
		}

		if (pHeaders && !pHeaders->empty())
		{
			std::wstring all_headers;
			for (const auto& str : *pHeaders)
			{
				all_headers += utils::utf8_to_utf16(str) + L"\n";
			}

			BOOL result = WinHttpAddRequestHeaders(hRequest, all_headers.c_str(), (DWORD)all_headers.size(), WINHTTP_ADDREQ_FLAG_ADD);
			ATLTRACE("\nheader added: %d\n", result);
		}

		DWORD options = SECURITY_FLAG_IGNORE_ALL_CERT_ERRORS;
		WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, (LPVOID)&options, sizeof(DWORD));

		// Send a request.
		bool bResults = false;
		bool bRepeat = false;
		bool bSaveBadCache = false;
		for (;;)
		{
			DWORD post_size = post_data ? (DWORD)strlen(post_data) : 0;
			if (!WinHttpSendRequest(hRequest,
								   WINHTTP_NO_ADDITIONAL_HEADERS,
								   0,
								   post_data ? (LPVOID)post_data : WINHTTP_NO_REQUEST_DATA,
								   post_size,
								   post_size,
								   0))
			{
				m_error_message = L"Error: Failed to send request";
				break;
			}


			// End the request.
			if (!WinHttpReceiveResponse(hRequest, nullptr))
			{
				m_error_message = fmt::format(L"Error: Failed to receive response: error code {:d}", GetLastError());
				break;
			}

			DWORD dwStatusCode = 0;
			DWORD dwSize = sizeof(dwStatusCode);

			if (!WinHttpQueryHeaders(hRequest,
									 WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
									 WINHTTP_HEADER_NAME_BY_INDEX,
									 &dwStatusCode, &dwSize, WINHTTP_NO_HEADER_INDEX))
			{
				m_error_message = fmt::format(L"Error: Failed to query headers: error code {:d}", GetLastError());
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
						m_error_message = L"Error: Failed to query authentication schemes";
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
						m_error_message = fmt::format(L"Error: Failed to set credentials: error code {:d}", GetLastError());
						break;
					}

					bResults = true;
					bRepeat = true;
					break;
				}

				case 200:
					bResults = true;
					bRepeat = false;
					break;

				case 304:
					bResults = false;
					bRepeat = false;
					bSaveBadCache = true;
					break;

				default:
					m_error_message = fmt::format(L"Response returns error code: {:d}", dwStatusCode);
					break;
			}

			if (!bResults || !bRepeat) break;
		}


		DWORD dwSize = 0;
		for (;;)
		{
			if (!bResults)
			{
				m_error_message += fmt::format(L"\nError code: {:d}", GetLastError());
				break;
			}
			// Check for available data.
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
			{
				m_error_message += fmt::format(L"\nError: WinHttpQueryDataAvailable error code {:d}", GetLastError());
				break;
			}

			// Allocate space for the buffer.
			if (!dwSize) break;
			std::vector<BYTE> chunk(dwSize);

			DWORD dwDownloaded = 0;
			if (WinHttpReadData(hRequest, (LPVOID)chunk.data(), dwSize, &dwDownloaded))
			{
				vData.write((const char*)chunk.data(), dwDownloaded);
			}
			else
			{
				m_error_message = fmt::format(L"\nError: WinHttpReadData error code {:d}", GetLastError());
				break;
			}
		}

		if (!bResults && !bSaveBadCache) break;

		if (vData.good() || bSaveBadCache)
		{
			std::ofstream out_stream(cache_file, std::ofstream::binary);
			out_stream << vData.rdbuf();
			ATLTRACE("\nSave to cache for %d seconds\n", m_cache_ttl_sec);
		}

		bool res = vData.tellp() != std::streampos(0);
		if (!res)
		{
			if (m_error_message.empty())
			{
				m_error_message = L"Error: Empty response";
			}
			break;
		}

		return true;
	} while (false);

#ifdef _DEBUG
	if (!m_error_message.empty())
	{
		ATLTRACE(L"\n%s\n", m_error_message.c_str());
	}
#endif // _DEBUG

	return false;
}

std::filesystem::path CUrlDownload::GetCachedPath(const std::wstring& url)
{
	return GetCacheDir() / fmt::format(L"{:08x}", xxh::xxhash<32>(url));
}

std::filesystem::path CUrlDownload::GetCacheDir()
{
	std::filesystem::path dir = std::filesystem::temp_directory_path().append(L"iptv_cache");
	std::error_code err;
	std::filesystem::create_directories(dir, err);
	return dir;
}

void CUrlDownload::ClearCache()
{
	std::error_code err;
	std::filesystem::remove_all(GetCacheDir(), err);
}

void CUrlDownload::ClearCachedUrl(const std::wstring& url)
{
	const auto& cache_file = GetCachedPath(url);
	if (std::filesystem::exists(cache_file))
	{
		std::error_code err;
		std::filesystem::remove(cache_file, err);
	}
}

bool CUrlDownload::CheckIsCacheExpired(const std::wstring& cache_file) const
{
	if (m_cache_ttl_sec && std::filesystem::exists(cache_file) && std::filesystem::file_size(cache_file) != 0)
	{
		std::time_t file_time = to_time_t(std::filesystem::last_write_time(cache_file));
		std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		int diff = int(now - file_time);
		ATLTRACE(L"\nttl: %d hours %d minutes %d seconds\n", diff / 3600, (diff - (diff / 3600 * 3600)) / 60, diff - diff / 60 * 60);
		return (diff > m_cache_ttl_sec);
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
		for (const auto& it : convert)
		{
			const auto& key = it.first;
			const auto& value = it.second;
			if (i + key.size() - 1 < text.size()
				&& text.substr(i, key.size()) == key)
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
	if (!ATL::Base64Encode(pData, nSize, (char*)m_buf.data(), &m_nSize, dwFlags)) return false;

	//Null terminate the data
	m_buf[m_nSize] = 0;

	return true;
}

bool CBase64Coder::Encode(const char* szMessage, int nSize /*= 0*/, unsigned long dwFlags /*= ATL_BASE64_FLAG_NOCRLF*/)
{
	if (!nSize)
		nSize = (int)strlen(szMessage);

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
	return Decode(szMessage, static_cast<int>(strlen(szMessage)));
}

//////////////////////////////////////////////////////////////////////////

CRC4Coder::CRC4Coder(const unsigned char* pKey, size_t nKeyLen)
{
	SetKey(pKey, nKeyLen);
}

CRC4Coder::CRC4Coder(const char* szKey, size_t nKeyLen)
{
	SetKey(szKey, nKeyLen);
}

void CRC4Coder::Init()
{
	SecureZeroMemory(m_state, 256);
	m_nSize = 0;
	m_x = 0;
	m_y = 0;
	m_bKeySet = false;
}

void CRC4Coder::SetKey(const char* szKey, size_t nKeyLen)
{
	SetKey((const BYTE*)szKey, nKeyLen);
}

void CRC4Coder::SetKey(const unsigned char* pKey, size_t nKeyLen)
{
	m_x = 0;
	m_y = 0;
	for (int i = 0; i < sizeof(m_state); i++)
		m_state[i] = LOBYTE(i);

	unsigned char idx1 = 0;
	unsigned char idx2 = 0;
	unsigned char nDataLen = LOBYTE(nKeyLen);
	for (unsigned char& i : m_state)
	{
		idx2 = (pKey[idx1] + i + idx2) % 256;

		unsigned char t = i;
		i = m_state[idx2];
		m_state[idx2] = t;

		idx1 = (idx1 + 1) % nDataLen;
	}

	m_bKeySet = true;
}

bool CRC4Coder::Encode(const char* szBuf, size_t nBufLen)
{
	return Encode((const unsigned char*)szBuf, nBufLen);
}

bool CRC4Coder::Encode(const unsigned char* pBuf, size_t nBufLen)
{
	ATLASSERT(m_bKeySet);
	if (!m_bKeySet)
		return false;

	m_nSize = nBufLen;
	m_buf.assign(pBuf, pBuf + nBufLen);

	unsigned char x = m_x;
	unsigned char y = m_y;
	for (size_t i = 0; i < m_nSize; i++)
	{
		x = (x + 1) % 256;
		y = (m_state[x] + y) % 256; //-V537

		unsigned char t = m_state[x];
		m_state[x] = m_state[y];
		m_state[y] = t;

		unsigned char xorIndex = (m_state[x] + m_state[y]) % 256;
		m_buf[i] ^= m_state[xorIndex];
	}

	m_x = x;
	m_y = y;

	return true;
}

}