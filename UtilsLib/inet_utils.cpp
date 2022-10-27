/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2022): sharky72 (https://github.com/KocourKuba)

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

// Модифицированный алгоритм unescape поддерживающий utf8.
HRESULT SHUrlUnescapeW(const std::wstring& src, std::wstring& result, DWORD dwFlags)
{
	if (src.empty())
		return E_INVALIDARG;

	result = src;
	wchar_t* pchSrc = result.data();
	wchar_t* pchDst = result.data();

	wchar_t ch;
	int more = -1;
	int symb = 0;
	while (0 != (ch = *pchSrc++))
	{
		if ((ch == '#' || ch == '?') && (dwFlags & URL_DONT_ESCAPE_EXTRA_INFO))
		{
			StrCpyW(pchDst, --pchSrc);
			pchDst += wcslen(pchDst);
			break;
		}

		/* Get next byte b from URL segment wszUrl */
		int b;
		switch (ch)
		{
			case '%':
			{
				ch = *pchSrc++;
				if (ch == '\0')
				{
					pchSrc--;
					continue;
				}
				int hb = (iswdigit(ch) ? ch - '0' : 10 + towlower(ch) - 'a') & 0xF;
				ch = *pchSrc++;
				if (ch == '\0')
				{
					pchSrc--;
					continue;
				}
				int lb = (iswdigit(ch) ? ch - '0' : 10 + towlower(ch) - 'a') & 0xF;
				b = (hb << 4) | lb;
			}
			break;
			case '+':
				*pchDst++ = L' ';
				continue;
			default:
				*pchDst++ = ch;
				continue;
		}

		/* Decode byte b as UTF-8, symb collects incomplete chars */
		if ((b & 0xC0) == 0x80)
		{			// 10xxxxxx (continuation byte)
			symb = (symb << 6) | (b & 0x3f);	// Add 6 bits to symb
			if (--more == 0)
			{
				*pchDst++ = (wchar_t)symb;
			}
		}
		else if ((b & 0x80) == 0x00)
		{		// 0xxxxxxx (yields 7 bits)
			*pchDst++ = (wchar_t)b;
		}
		else if ((b & 0xE0) == 0xC0)
		{	// 110xxxxx (yields 5 bits)
			symb = b & 0x1F;
			more = 1;				// Expect 1 more byte
		}
		else if ((b & 0xF0) == 0xE0)
		{	// 1110xxxx (yields 4 bits)
			symb = b & 0x0F;
			more = 2;				// Expect 2 more bytes
		}
		else if ((b & 0xF8) == 0xF0)
		{	// 11110xxx (yields 3 bits)
			symb = b & 0x07;
			more = 3;				// Expect 3 more bytes
		}
		else if ((b & 0xFC) == 0xF8)
		{	// 111110xx (yields 2 bits)
			symb = b & 0x03;
			more = 4;				// Expect 4 more bytes
		}
		else /*if((b & 0xFE) == 0xFC)*/
		{	// 1111110x (yields 1 bit)
			symb = b & 0x01;
			more = 5;				// Expect 5 more bytes
		}
		/* No need to test if the UTF-8 encoding is well-formed */
	}

	*pchDst = '\0';
	int sz = pchDst - result.c_str();
	if (sz >= 0)
	{
		result.resize(sz);
		return S_OK;
	}

	return E_FAIL;
}

bool CrackUrl(const std::wstring& url, CrackedUrl& cracked)
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
			cracked.scheme.assign(urlComp.lpszScheme, urlComp.dwSchemeLength);
		if (urlComp.dwUserNameLength)
			cracked.user.assign(urlComp.lpszUserName, urlComp.dwUserNameLength);
		if (urlComp.dwPasswordLength)
			cracked.password.assign(urlComp.lpszPassword, urlComp.dwPasswordLength);
		if (urlComp.dwHostNameLength)
			cracked.host.assign(urlComp.lpszHostName, urlComp.dwHostNameLength);
		if (urlComp.dwUrlPathLength)
			cracked.path.assign(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);
		if (urlComp.dwExtraInfoLength)
			cracked.extra_info.assign(urlComp.lpszExtraInfo, urlComp.dwExtraInfoLength);

		cracked.port = urlComp.nPort;

		return true;
	}

	return false;
}

bool DownloadFile(const std::wstring& url,
				  std::stringstream& vData,
				  bool use_cache /*= false*/,
				  std::vector<std::string>* pHeaders /*= nullptr*/,
				  bool verb_post /*= false*/,
				  const char* post_data /*= nullptr*/)
{
	std::wstring hash_str = url;
	if (post_data)
		hash_str += utf8_to_utf16(std::string(post_data));

	std::filesystem::path cache_file = std::filesystem::temp_directory_path().append(L"iptv_cache");
	std::filesystem::create_directory(cache_file);
	cache_file.append(fmt::format(L"{:08x}", xxh::xxhash<32>(hash_str)));
	ATLTRACE(L"\ndownload url: %s\n", url.c_str());

	if (use_cache && std::filesystem::exists(cache_file) && std::filesystem::file_size(cache_file) != 0)
	{
		ATLTRACE(L"\ncache file: %s\n", cache_file.c_str());
		std::time_t file_time = to_time_t(std::filesystem::last_write_time(cache_file));
		std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		int diff = int(now - file_time);
		ATLTRACE(L"\nttl: %d hours %d minutes %d seconds\n", diff / 3600, (diff - (diff / 3600 * 3600)) / 60, diff - diff / 60 * 60);
		// cache ttl 1 day
		if (diff < 60 * 60 * 24)
		{
			std::ifstream in_file(cache_file.c_str());
			if (in_file.good())
			{
				vData << in_file.rdbuf();
				in_file.close();
				ATLTRACE(L"\nloaded from cache: %d bytes\n", vData.rdbuf()->_Get_buffer_view()._Size);
				return vData.tellp() != std::streampos(0);
			}
		}
	}

	do
	{
		CrackedUrl cracked;
		if (!CrackUrl(url, cracked))
		{
			ATLTRACE(L"\nError: Failed to crack url\n");
			break;
		}

		// Use WinHttpOpen to obtain a session handle.
		CAutoHinternet hSession = WinHttpOpen(L"DuneHD/1.0",
											  /*L"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/105.0.0.0 Safari/537.36 Edg/105.0.1343.53"*/
											  WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
											  WINHTTP_NO_PROXY_NAME,
											  WINHTTP_NO_PROXY_BYPASS, 0);

		if (hSession.IsNotValid())
		{
			ATLTRACE(L"\nError: Failed to open connection\n");
			break;
		}

		WinHttpSetTimeouts(hSession, 0, 10000, 10000, 60000);

		// Specify an HTTP server.
		CAutoHinternet hConnect = WinHttpConnect(hSession, cracked.host.c_str(), cracked.port, 0);
		if (hConnect.IsNotValid())
		{
			ATLTRACE(L"\nError: Failed to connect\n");
			break;
		}

		// Create an HTTP request handle.
		CAutoHinternet hRequest = WinHttpOpenRequest(hConnect, verb_post ? L"POST" : L"GET", (cracked.path + cracked.extra_info).c_str(),
													 nullptr,
													 WINHTTP_NO_REFERER,
													 nullptr,
													 NULL);

		if (hRequest.IsNotValid())
		{
			ATLTRACE(L"\nError: Failed to open request\n");
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


		// Send a request.
		bool bResults = false;
		bool bRepeat = false;
		for (;;)
		{
			DWORD post_size = post_data ? strlen(post_data) : 0;
			if (!WinHttpSendRequest(hRequest,
								   WINHTTP_NO_ADDITIONAL_HEADERS,
								   0,
								   post_data ? (LPVOID)post_data : WINHTTP_NO_REQUEST_DATA,
								   post_size,
								   post_size,
								   0))
			{
				ATLTRACE(L"\nError: Failed to send request\n");
				break;
			}


			// End the request.
			if (!WinHttpReceiveResponse(hRequest, nullptr))
			{
				ATLTRACE(L"\nError: Failed to receive response\n");
				break;
			}

			DWORD dwStatusCode = 0;
			DWORD dwSize = sizeof(dwStatusCode);

			if (!WinHttpQueryHeaders(hRequest,
									 WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
									 WINHTTP_HEADER_NAME_BY_INDEX,
									 &dwStatusCode, &dwSize, WINHTTP_NO_HEADER_INDEX))
			{
				ATLTRACE(L"\nError: Failed to query headers\n");
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
						ATLTRACE(L"\nError: Failed to query auth schemes\n");
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
						ATLTRACE(L"\nError: Failed to set credentials\n");
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

				default:
					ATLTRACE("\nresponse returns error code: %d\n", dwStatusCode);
					break;
			}

			if (!bResults || !bRepeat) break;
		}


		DWORD dwSize = 0;
		for (;;)
		{
			if (!bResults)
			{
				ATLTRACE("\nError %d has occurred.\n", GetLastError());
				break;
			}
			// Check for available data.
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
			{
				ATLTRACE("\nError %u in WinHttpQueryDataAvailable.\n", GetLastError());
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
				ATLTRACE("\nError %u in WinHttpReadData.\n", GetLastError());
				break;
			}
		}

		if (use_cache && vData.good())
		{
			std::ofstream out_stream(cache_file, std::ios::out | std::ios::binary);
			out_stream << vData.rdbuf();
		}

		return vData.tellp() != std::streampos(0);
	} while (false);

	return false;
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