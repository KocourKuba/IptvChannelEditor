#include "pch.h"
#include "inet_utils.h"

#include <wtypes.h>
#include <winhttp.h>
#include <atltrace.h>
#include <unordered_map>

#pragma comment(lib, "Winhttp.lib")

namespace utils
{

bool CrackUrl(const std::wstring& url, std::wstring& host, std::wstring& path, unsigned short& port)
{
	URL_COMPONENTS urlComp{};
	SecureZeroMemory(&urlComp, sizeof(urlComp));
	urlComp.dwStructSize = sizeof(urlComp);

	// Set required component lengths to non-zero so that they are cracked.
	urlComp.dwHostNameLength = (DWORD)-1;
	urlComp.dwUrlPathLength = (DWORD)-1;

	if (WinHttpCrackUrl(url.c_str(), (DWORD)url.size(), 0, &urlComp))
	{
		host = std::wstring(urlComp.lpszHostName, urlComp.dwHostNameLength);
		path = std::wstring(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);
		port = urlComp.nPort;
		return true;
	}

	return false;
}

bool DownloadFile(const std::wstring& url, std::vector<unsigned char>& vData, std::wstring* pHeaders /*= nullptr*/)
{
	ATLTRACE(L"\ndownload url: %s\n", url.c_str());
	std::wstring host;
	std::wstring path;
	WORD port = INTERNET_DEFAULT_HTTP_PORT;
	if (!CrackUrl(url, host, path, port))
		return false;

	// Use WinHttpOpen to obtain a session handle.
	HINTERNET hSession = WinHttpOpen(L"WinHTTP wget/1.0",
									 WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
									 WINHTTP_NO_PROXY_NAME,
									 WINHTTP_NO_PROXY_BYPASS, 0);

	WinHttpSetTimeouts(hSession, 0, 10000, 10000, 10000);

	// Specify an HTTP server.
	HINTERNET hConnect = nullptr;
	if (hSession)
		hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);


	// Create an HTTP request handle.
	HINTERNET hRequest = nullptr;
	if (hConnect)
		hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(),
									  nullptr,
									  WINHTTP_NO_REFERER,
									  nullptr,
									  NULL);

	if (pHeaders && !pHeaders->empty())
	{
		BOOL result = WinHttpAddRequestHeaders(hRequest, pHeaders->c_str(), pHeaders->size(), WINHTTP_ADDREQ_FLAG_ADD);
		ATLTRACE("\nheader added: %d\n", result);
	}

	// Send a request.
	BOOL bResults = FALSE;
	if (hRequest)
		bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);

	// End the request.
	if (bResults)
		bResults = WinHttpReceiveResponse(hRequest, nullptr);

	DWORD dwStatusCode = 0;
	DWORD dwSize = sizeof(dwStatusCode);

	if (WinHttpQueryHeaders(hRequest,
							WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
							WINHTTP_HEADER_NAME_BY_INDEX,
							&dwStatusCode, &dwSize, WINHTTP_NO_HEADER_INDEX) && dwStatusCode != 200)
	{
		ATLTRACE("\nrequested url %ls not found. error code: %d\n", url.c_str(), dwStatusCode);
		return false;
	}

	dwSize = 0;
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
			chunk.resize(dwSize);
			vData.insert(vData.end(), chunk.begin(), chunk.begin() + dwDownloaded);
		}
		else
		{
			ATLTRACE("\nError %u in WinHttpReadData.\n", GetLastError());
			break;
		}
	}

	// Close any open handles.
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);

	return !vData.empty();
}

bool WriteDataToFile(const std::wstring& path, std::vector<unsigned char>& vData)
{
	HANDLE hFile = ::CreateFileW(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE) return false;

	DWORD dwWritten = 0;
	BOOL res = ::WriteFile(hFile, vData.data(), vData.size(), &dwWritten, nullptr);;
	::CloseHandle(hFile);

	return res;
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
	if (!ATL::Base64Encode(pData, nSize, (char*)m_buf.data(), &m_nSize, dwFlags)) return FALSE;

	//Null terminate the data
	m_buf[m_nSize] = 0;

	return TRUE;
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
	if (!Base64Decode(pData, nSize, m_buf.data(), &m_nSize)) return FALSE;

	//Null terminate the data
	m_buf[m_nSize] = 0;

	return TRUE;
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