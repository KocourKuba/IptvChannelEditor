#pragma once
#include "rapidxml.hpp"

namespace utils
{
	static constexpr auto DUNE_PLUGIN_NAME = L"dune_plugin_{:s}_mod.zip";
	static constexpr auto ICON_TEMPLATE = L"plugin_file://icons/channel_unset.png";

	static constexpr auto VERSION_INFO = "version_info";
	static constexpr auto LIST_VERSION = "list_version";
	static constexpr auto CHANNELS_SETUP = "channels_setup";
	static constexpr auto ACCESS_TOKEN = "access_key";
	static constexpr auto ACCESS_DOMAIN = "access_domain";
	static constexpr auto ACCESS_LOGIN = "access_login";
	static constexpr auto ACCESS_PASSWORD = "access_password";

	static constexpr auto TV_INFO = "tv_info";
	static constexpr auto TV_CATEGORIES = "tv_categories";
	static constexpr auto TV_CHANNELS = "tv_channels";
	static constexpr auto CHANNELS_LOGO_URL = L"icons/channels/";
	static constexpr auto CATEGORIES_LOGO_URL = L"icons/";

	static constexpr auto PLUGIN_ROOT = L"dune_plugin\\";
	static constexpr auto PLAYLISTS_ROOT = L"playlists\\{:s}\\";
	static constexpr auto CHANNELS_LOGO_PATH = L"dune_plugin\\icons\\channels\\";
	static constexpr auto CATEGORIES_LOGO_PATH = L"dune_plugin\\icons\\";
	static constexpr auto PACK_PATH = L"{:s}_plugin\\";

#ifdef _DEBUG
	static constexpr auto PACK_DLL = L"dll\\7za.dll";
#else
	static constexpr auto PACK_DLL = L"7za.dll";
#endif // _DEBUG

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
inline std::basic_string<T>& string_ltrim(std::basic_string<T>& str, const T* chars)
{
	return str.erase(0, str.find_first_not_of(chars));
}

template<typename T>
inline std::basic_string<T>& string_rtrim(std::basic_string<T>& str, const T* chars)
{
	return str.erase(str.find_last_not_of(chars) + 1);
}

template<typename T>
inline std::basic_string<T>& string_trim(std::basic_string<T>& str, const T* chars)
{
	return string_ltrim(string_rtrim(str, chars), chars);
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
/// Converts char string to int.
/// </summary>
/// <param name="str">string</param>
/// <returns>int value if success, 0 otherwise</returns>
int char_to_int(const std::string& str);

/// <summary>
/// Converts char string to int.
/// </summary>
/// <param name="str">string</param>
/// <returns>int value if success, 0 otherwise</returns>
int wchar_to_int(const std::wstring& str);

/// <summary>
/// Converts int to char string.
/// </summary>
/// <param name="value">int value</param>
/// <returns>string representaion of value, empty string otherwise</returns>
std::string int_to_char(int value);

/// <summary>
/// Converts int to char string.
/// </summary>
/// <param name="value">int value</param>
/// <returns>string representaion of value, empty string otherwise</returns>
std::wstring int_to_wchar(int value);

/// <summary>
/// Allocate node using memory_pool allocator
/// </summary>
/// <param name="alloc">Allocator</param>
/// <param name="name">Node name</param>
/// <param name="value">Node value</param>
/// <returns>Allocated node</returns>
rapidxml::xml_node<>* alloc_node(rapidxml::memory_pool<>& alloc, const char* name, const char* value = nullptr);

/// <summary>
/// return int value from xml node
/// </summary>
/// <param name="node"></param>
/// <returns>int value</returns>
int get_value_int(rapidxml::xml_node<>* node);

/// <summary>
/// return string value from xml node
/// </summary>
/// <param name="node"></param>
/// <returns>string</returns>
std::string get_value_string(rapidxml::xml_node<>* node);

/// <summary>
/// return wstring value from xml node
/// </summary>
/// <param name="node"></param>
/// <returns>wstring</returns>
std::wstring get_value_wstring(rapidxml::xml_node<>* node);

/// <summary>
/// Converts a UTF-16 string to a UTF-8 string.
/// </summary>
/// <param name="w">A two byte character UTF-16 string.</param>
/// <returns>A single byte character UTF-8 string.</returns>
std::string utf16_to_utf8(const std::wstring& w);

/// <summary>
/// Converts a UTF-8 string to a UTF-16
/// </summary>
/// <param name="s">A single byte character UTF-8 string.</param>
/// <returns>A two byte character UTF-16 string.</returns>
std::wstring utf8_to_utf16(const std::string& s);

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
	vector_to_streambuf(std::vector<BYTE>& vec)
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

bool CrackUrl(const std::wstring& url, std::wstring& host = std::wstring(), std::wstring& path = std::wstring());

bool DownloadFile(const std::wstring& url, std::vector<BYTE>& image);

std::string entityDecrypt(const std::string& text);

BOOL LoadImage(const std::wstring& fullPath, CImage& image);
void SetImage(const CImage& image, CStatic& wnd);

}
