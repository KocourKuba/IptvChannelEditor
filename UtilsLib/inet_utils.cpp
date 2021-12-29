#include "pch.h"
#include "inet_utils.h"

#include <wtypes.h>
#include <winhttp.h>
#include <atltrace.h>
#include <unordered_map>

#pragma comment(lib, "Winhttp.lib")

namespace utils
{

bool CrackUrl(const std::wstring& url, std::wstring& host, std::wstring& path)
{
	URL_COMPONENTS urlComp;
	SecureZeroMemory(&urlComp, sizeof(urlComp));
	urlComp.dwStructSize = sizeof(urlComp);

	// Set required component lengths to non-zero so that they are cracked.
	urlComp.dwHostNameLength = (DWORD)-1;
	urlComp.dwUrlPathLength = (DWORD)-1;

	if (WinHttpCrackUrl(url.c_str(), (DWORD)url.size(), 0, &urlComp))
	{
		host = std::wstring(urlComp.lpszHostName, urlComp.dwHostNameLength);
		path = std::wstring(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);
		return true;
	}

	return false;
}

bool DownloadFile(const std::wstring& url, std::vector<unsigned char>& vData, std::wstring* pHeaders /*= nullptr*/)
{
	ATLTRACE(L"download url: %s\n", url.c_str());
	std::wstring host;
	std::wstring path;
	if (!CrackUrl(url, host, path))
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
		hConnect = WinHttpConnect(hSession, host.c_str(), INTERNET_DEFAULT_HTTP_PORT, 0);


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
		ATLTRACE("header added: %d\n", result);
	}

	// Send a request.
	BOOL bResults = FALSE;
	if (hRequest)
		bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);

	// End the request.
	if (bResults)
		bResults = WinHttpReceiveResponse(hRequest, nullptr);

	DWORD dwSize = 0;
	for (;;)
	{
		if (!bResults)
		{
			ATLTRACE("Error %d has occurred.\n", GetLastError());
			break;
		}
		// Check for available data.
		if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
		{
			ATLTRACE("Error %u in WinHttpQueryDataAvailable.\n", GetLastError());
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
			ATLTRACE("Error %u in WinHttpReadData.\n", GetLastError());
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

}