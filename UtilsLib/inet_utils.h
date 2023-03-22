/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2023): sharky72 (https://github.com/KocourKuba)

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

#ifdef _USE_CURL
# define DownloadFile CurlDownload
#else
# define DownloadFile WinHttpDownload

#endif

namespace utils
{
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

bool CrackUrl(const std::wstring& url, CrackedUrl& cracked);

#ifdef _USE_CURL
bool CurlDownload(const std::wstring& url,
				  std::stringstream& vData,
				  LPCWSTR user_agent = nullptr,
				  int cache_ttl_sec = 0,
				  std::vector<std::string>* pHeaders = nullptr,
				  bool verb_post = false,
				  const char* post_data = nullptr);
#endif // _USE_CURL

bool WinHttpDownload(const std::wstring& url,
				  std::stringstream& vData,
				  LPCWSTR user_agent = nullptr,
				  int cache_ttl_sec = 0, // in seconds
				  std::vector<std::string>* pHeaders = nullptr,
				  bool verb_post = false,
				  const char* post_data = nullptr);

std::string entityDecrypt(const std::string& text);

class CBase64Coder
{
public:
	//Constructors / Destructors
	CBase64Coder() = default;
	~CBase64Coder() = default;

	//Methods

public:
	// Encode data to internal buffer to BASE64
	bool Encode(const unsigned char* pData, int nSize, unsigned long dwFlags = ATL_BASE64_FLAG_NOCRLF);
	bool Encode(const char* szMessage, int nSize = 0, unsigned long dwFlags = ATL_BASE64_FLAG_NOCRLF);

	// Decode data to internal buffer from BASE64
	bool Decode(const char* pData, int nSize);
	bool Decode(const char* szMessage);

	// Reinterpret internal buffer to type
	std::string GetResultString() const { return std::string((char*)m_buf.data(), m_nSize); };
	const unsigned char* GetResultBytes() const { return m_buf.data(); };

	// size of encoded/decoded buffer
	int	GetResultSize() const { return m_nSize; };

	const std::vector<unsigned char>& GetBuffer() const { return m_buf; }

protected:
	//Member variables
	std::vector<unsigned char> m_buf;
	int m_nSize = 0;
};

class CRC4Coder
{
public:
	//Constructors / Destructors
	CRC4Coder() = default;
	CRC4Coder(const unsigned char* pKey, size_t nKeyLen);
	CRC4Coder(const char* pKey, size_t nKeyLen);

	~CRC4Coder() = default;

	//Methods

public:
	// Set key and init state
	void SetKey(const unsigned char* pKey, size_t nKeyLen);
	void SetKey(const char* szKey, size_t nKeyLen);
	// Perform RC4 coding
	bool Encode(const unsigned char* pBuf, size_t nBufLen);
	bool Encode(const char* szBuf, size_t nBufLen);

	// Reinterpret internal buffer to type
	std::string GetResultString() const { return std::string(m_buf.begin(), m_buf.end()); };
	const unsigned char* GetResultBytes() const { return m_buf.data(); };

	// size of encoded/decoded buffer
	size_t GetResultSize() const { return m_nSize; };

protected:
	void Init();

protected:
	//Member variables
	std::vector<unsigned char> m_buf;
	size_t m_nSize = 0;

	bool m_bKeySet = false;
	unsigned char m_state[256] = { 0 };
	unsigned char m_x = 0;
	unsigned char m_y = 0;
};
}
