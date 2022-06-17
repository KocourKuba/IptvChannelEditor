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

#pragma once
#include <algorithm>
#include <locale>

namespace utils
{

static constexpr auto DUNE_PLUGIN_NAME = L"dune_plugin_{:s}_{:s}.zip";
static constexpr auto ICON_TEMPLATE = L"plugin_file://icons/channel_unset.png";
static constexpr auto PLUGIN_PATH = R"(plugin_file://)";

static constexpr auto VERSION_INFO = "version_info";
static constexpr auto LIST_VERSION = "list_version";
static constexpr auto CHANNELS_SETUP = "channels_setup";
static constexpr auto PORTAL_SETUP = "portal_setup";
static constexpr auto ACCESS_TOKEN = "access_key";
static constexpr auto ACCESS_DOMAIN = "access_domain";
static constexpr auto ACCESS_LOGIN = "access_login";
static constexpr auto ACCESS_PASSWORD = "access_password";
static constexpr auto PORTAL_KEY = "portal_key";
static constexpr auto HAS_SECONDARY_EPG = "has_secondary_epg";

static constexpr auto TV_INFO = "tv_info";
static constexpr auto TV_CATEGORIES = "tv_categories";
static constexpr auto TV_CATEGORY = "tv_category";
static constexpr auto TV_CHANNELS = "tv_channels";
static constexpr auto TV_CHANNEL = "tv_channel";
static constexpr auto CHANNEL_ID = "channel_id";
static constexpr auto EPG1_ID = "epg_id";
static constexpr auto EPG2_ID = "tvg_id";
static constexpr auto CAPTION = "caption";
static constexpr auto TIME_SHIFT_HOURS = "timeshift_hours";
static constexpr auto NUM_PAST_EPG_DAYS = "num_past_epg_days";
static constexpr auto NUM_FUTURE_EPG_DAYS = "num_future_epg_days";
static constexpr auto TV_CATEGORY_ID = "tv_category_id";
static constexpr auto INT_ID = "int_id";
static constexpr auto STREAMING_URL = "streaming_url";
static constexpr auto ARCHIVE = "archive";
static constexpr auto PROTECTED = "protected";
static constexpr auto DISABLED = "disabled";
static constexpr auto FAVORITE = "favorite";

static constexpr auto CHANNELS_LOGO_URL = L"icons/channels/";
static constexpr auto CATEGORIES_LOGO_URL = L"icons/";

static constexpr auto PLUGIN_ROOT = L"dune_plugin\\";
static constexpr auto CHANNELS_LOGO_PATH = L"dune_plugin\\icons\\channels\\";
static constexpr auto CATEGORIES_LOGO_PATH = L"dune_plugin\\icons\\";


uint64_t ChronoGetTickCount();
uint64_t GetTimeDiff(uint64_t dwStartTime);
bool CheckForTimeOut(uint64_t dwStartTime, uint32_t dwTimeOut);

template <typename T> class EMSLiterals;

template <> class EMSLiterals<char>
{
public:
	static constexpr char* crlf = "\r\n";
	static constexpr char* whitespace = " \t";
	static constexpr char* empty = "";
	static constexpr char colon = ':';
	static constexpr char cr = '\r';
	static constexpr char lf = '\n';
	static constexpr char space = ' ';
	static constexpr char tab = '\t';
	static constexpr char quote = '\"';
	static constexpr char nil = '\0';
};

template <> class EMSLiterals<wchar_t>
{
public:
	static constexpr wchar_t* crlf = L"\r\n";
	static constexpr wchar_t* whitespace = L" \t";
	static constexpr wchar_t* empty = L"";
	static constexpr wchar_t colon = ':';
	static constexpr wchar_t cr = L'\r';
	static constexpr wchar_t lf = L'\n';
	static constexpr wchar_t space = L' ';
	static constexpr wchar_t tab = '\t';
	static constexpr wchar_t quote = '\"';
	static constexpr wchar_t nil = '\0';
};

inline std::string& string_tolower(std::string& s)
{
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c)
				   {
					   return (char)std::tolower(c);
				   });

	return s;
}

inline std::wstring& wstring_tolower(std::wstring& s)
{
	std::transform(s.begin(), s.end(), s.begin(), [](wchar_t c)
				   {
					   return (wchar_t)std::tolower(c);
				   });

	return s;
}

template<typename T>
int char_to_int(const std::basic_string<T>& str, int base = 10)
{
	int value = 0;
	try
	{
		value = std::stoul(str, nullptr, base);
	}
	catch (...)
	{
	}
	return value;
}

template<typename T>
__int64 char_to_int64(const std::basic_string<T>& str, int base = 10)
{
	__int64 value = 0;
	try
	{
		value = std::stoull(str, nullptr, base);
	}
	catch (...)
	{
	}
	return value;
}


//////////////////////////////////////////////////////////////////////////
/// <summary>left trim string</summary>
/// <typeparam name="T">std::base_string or std::base_string_view class</typeparam>
/// <typeparam name="_Elem">char or wchar_t type</typeparam>
/// <param name="str">string to be trimmed by left side</param>
/// <param name="chars">chars that will be removed</param>
/// <returns>trimmed string</returns>
template<class T, typename _Elem>
T& string_ltrim(T& str, const _Elem chr = EMSLiterals<_Elem>::whitespace)
{
	if constexpr (std::is_pointer_v<_Elem>)
	{
		using __Elem = std::remove_cv_t<std::remove_pointer_t<_Elem>>;
		if constexpr (std::is_same_v<T::const_iterator, std::basic_string<__Elem>::const_iterator>)
		{
			return str.erase(0, str.find_first_not_of(chr));
		}
		else if constexpr (std::is_same_v<T::const_iterator, std::basic_string_view<__Elem>::const_iterator>)
		{
			if (!str.empty())
				str.remove_prefix(str.find_first_not_of(chr));

			return str;
		}
		else
		{
			static_assert(false, "std::basic_string or std::basic_string_view");
		}
	}
	else
	{
		if constexpr (std::is_same_v<T::const_iterator, std::basic_string<_Elem>::const_iterator>)
		{
			return str.erase(0, str.find_first_not_of(chr));
		}
		else if constexpr (std::is_same_v<T::const_iterator, std::basic_string_view<_Elem>::const_iterator>)
		{
			if (!str.empty())
				str.remove_prefix(str.find_first_not_of(chr));

			return str;
		}
		else
		{
			static_assert(false, "std::basic_string or std::basic_string_view");
		}
	}
}

//////////////////////////////////////////////////////////////////////////
/// <summary>right trim string</summary>
/// <typeparam name="T">std::base_string or std::base_string_view class</typeparam>
/// <typeparam name="_Elem">char or wchar_t type</typeparam>
/// <param name="str">string to be trimmed by right side</param>
/// <param name="chars">chars that will be removed</param>
/// <returns>trimmed string</returns>
template<class T, typename _Elem>
T& string_rtrim(T& str, const _Elem chr = EMSLiterals<_Elem>::whitespace)
{
	using __Elem = std::remove_cv_t<std::remove_pointer_t<_Elem>>;
	if constexpr (std::is_pointer_v<_Elem>)
	{
		if constexpr (std::is_same_v<T::const_iterator, std::basic_string<__Elem>::const_iterator>)
		{
			return str.erase(str.find_last_not_of(chr) + 1);
		}
		else if constexpr (std::is_same_v<T::const_iterator, std::basic_string_view<__Elem>::const_iterator>)
		{
			str.remove_suffix(str.size() - str.find_last_not_of(chr) - 1);
			return str;
		}
		else
			static_assert(false, "std::basic_string or std::basic_string_view");
	}
	else
	{
		if constexpr (std::is_same_v<T::const_iterator, std::basic_string<_Elem>::const_iterator>)
		{
			return str.erase(str.find_last_not_of(chr) + 1);
		}
		else if constexpr (std::is_same_v<T::const_iterator, std::basic_string_view<_Elem>::const_iterator>)
		{
			str.remove_suffix(str.size() - str.find_last_not_of(chr) - 1);
			return str;
		}
		else
			static_assert(false, "std::basic_string or std::basic_string_view");
	}
}

//////////////////////////////////////////////////////////////////////////
/// <summary>trim string from both sides</summary>
/// <typeparam name="T">std::base_string or std::base_string_view class</typeparam>
/// <typeparam name="_Elem">char or wchar_t type</typeparam>
/// <param name="s">string to be trimmed by right and left side</param>
/// <param name="chars">chars that will be removed</param>
/// <returns>trimmed string</returns>
template<class T, typename _Elem>
T& string_trim(T& s, const _Elem chars = EMSLiterals<_Elem>::whitespace)
{
	return string_ltrim(string_rtrim(s, chars), chars);
}

//////////////////////////////////////////////////////////////////////////
/// <summary>inplace replace for substring case insensitive</summary>
/// <param name="source">string where to search</param>
/// <param name="search">substring to search</param>
/// <param name="replace">substring to replace</param>
/// <param name="pos">position from searching will start</param>
/// <returns>how many changes performed</returns>
template<typename T>
size_t string_replace_inplace(std::basic_string<T>& source, const std::basic_string<T>& search, const std::basic_string<T>& replace, size_t pos = 0)
{
	size_t replaced = 0;
	if (!search.empty() && pos < source.size())
	{
		for (; (pos = source.find(search, pos)) != std::basic_string<T>::npos; pos += replace.size())
		{
			source.replace(pos, search.size(), replace);
			replaced++;
		}
	}

	return replaced;
}

template<typename T>
size_t string_replace_inplace(std::basic_string<T>& source, const T* search, const T* replace, size_t pos = 0)
{
	return string_replace_inplace(source, std::basic_string<T>(search), std::basic_string<T>(replace), pos);
}


//////////////////////////////////////////////////////////////////////////
/// <summary>replace for substring case insensitive</summary>
/// <param name="source">string where to search</param>
/// <param name="search">substring to search</param>
/// <param name="replace">substring to replace</param>
/// <param name="pos">position from searching will start</param>
/// <returns>replaced string</returns>
template<typename T>
std::basic_string<T> string_replace(const std::basic_string<T>& source, const std::basic_string<T>& search, const std::basic_string<T>& replace, size_t pos = 0)
{
	std::basic_string<T> replaced(source);
	string_replace_inplace(replaced, search, replace, pos);
	return replaced;
}

/// <summary>
/// Converts a UTF-16 string to a UTF-8 string.
/// </summary>
/// <param name="w">A two byte character UTF-16 string.</param>
/// <returns>A single byte character UTF-8 string.</returns>
std::string utf16_to_utf8(const wchar_t* srcData, size_t srcSize);

inline std::string utf16_to_utf8(std::wstring_view w)
{
	return utf16_to_utf8(w.data(), w.size());
}

inline std::string utf16_to_utf8(const std::wstring& w)
{
	return utf16_to_utf8(w.c_str(), w.size());
}

/// <summary>
/// Converts a UTF-8 string to a UTF-16
/// </summary>
/// <param name="s">A single byte character UTF-8 string.</param>
/// <returns>A two byte character UTF-16 string.</returns>
std::wstring utf8_to_utf16(const char* srcData, size_t srcSize);

inline std::wstring utf8_to_utf16(std::string_view s)
{
	return utf8_to_utf16(s.data(), s.size());
}

inline std::wstring utf8_to_utf16(const std::string& s)
{
	return utf8_to_utf16(s.c_str(), s.size());
}

std::vector<std::string> regex_split(const std::string& str, const std::string& token = "\\s+");

template<typename T>
std::vector<std::basic_string<T>> string_split(const std::basic_string<T>& str, T delim = ' ')
{
	std::vector<std::basic_string<T>> v;
	std::basic_stringstream<T, std::char_traits<T>, std::allocator<T>> ss(str);
	std::basic_string<T> item;

	while (std::getline(ss, item, delim))
	{
		v.emplace_back(std::move(item));
	}
	return v;
}

template<typename CharT, typename TraitsT = std::char_traits<CharT> >
class vector_to_streambuf : public std::basic_streambuf<CharT, TraitsT>
{
public:
	vector_to_streambuf(std::vector<unsigned char>& vec)
	{
		setg((CharT*)vec.data(), (CharT*)vec.data(), (CharT*)vec.data() + vec.size());
	}
};

template <typename Ch, size_t S>
static constexpr auto any_string(const char(&literal)[S]) -> const std::array<Ch, S>
{
	std::array<Ch, S> r = {};

	for (size_t i = 0; i < S; i++)
		r[i] = literal[i];

	return r;
}

template<typename T>
static std::basic_string<T> make_text_rtf_safe(const std::basic_string<T>& text)
{
	// modify the specified text to make it safe for use in a rich-edit
	// control, by escaping special RTF characters '\', '{' and '}'

	const auto& paragraph = any_string<T>(R"({\par})");

	std::basic_string<T> rtf;
	for (auto& it = text.begin(); it != text.end(); ++it)
	{
		if (*it == '\r') continue;;

		if (*it == '\n')
		{
			rtf.append(paragraph.begin(), paragraph.end());
			++it;
			continue;
		}

		if (*it == '\\' || *it == '{' || *it == '}')
		{
			rtf += '\\';
		}

		rtf += *it;
	}

	return rtf;
}

std::string generateRandomId(size_t length = 0);

}
