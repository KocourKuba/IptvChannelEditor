#pragma once
#include <string>
#include <vector>

namespace utils
{
bool CrackUrl(const std::wstring& url, std::wstring& host, std::wstring& path);

bool DownloadFile(const std::wstring& url, std::vector<unsigned char>& vData, std::wstring* pHeaders = nullptr);

bool WriteDataToFile(const std::wstring& path, std::vector<unsigned char>& vData);

std::string entityDecrypt(const std::string& text);
}
