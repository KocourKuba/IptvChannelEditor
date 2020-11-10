#pragma once
#include "rapidxml.hpp"

namespace utils
{
	static constexpr auto URI_TEMPLATE = "http://ts://{SUBDOMAIN}/iptv/{UID}/{ID}/index.m3u8";
	static constexpr auto ICON_TEMPLATE = "plugin_file://icons/channel_unset.png";

	static constexpr auto TV_INFO = "tv_info";
	static constexpr auto TV_CATEGORIES = "tv_categories";
	static constexpr auto TV_CHANNELS = "tv_channels";
	static constexpr auto CHANNELS_LOGO_URL = "icons/channels/";
	static constexpr auto CATEGORIES_LOGO_URL = "icons/";

#ifdef _DEBUG
	static constexpr auto PLUGIN_ROOT = L"..\\edem_plugin\\";
	static constexpr auto CHANNELS_CONFIG = L"..\\edem_plugin\\edem_channel_list.xml";
	static constexpr auto CHANNELS_LOGO_PATH = L"..\\edem_plugin\\icons\\channels\\";
	static constexpr auto CATEGORIES_LOGO_PATH = L"..\\edem_plugin\\icons\\";
#else
	static constexpr auto PLUGIN_ROOT = L".\\edem_plugin\\";
	static constexpr auto CHANNELS_CONFIG = L".\\edem_plugin\\edem_channel_list.xml";
	static constexpr auto CHANNELS_LOGO_PATH = L".\\edem_plugin\\icons\\channels\\";
	static constexpr auto CATEGORIES_LOGO_PATH = L".\\edem_plugin\\icons\\";
#endif // _DEBUG


/// <summary>
/// Our own implementation of alpha numeric instead of std::isalnum to avoid
/// taking global lock for performance reasons.
/// </summary>
inline bool is_alnum(const unsigned char uch) noexcept
{ // test if uch is an alnum character
	// special casing char to avoid branches
	// clang-format off
	static bool is_alnum_table[UCHAR_MAX + 1] = {
		/*         X0     X1     X2     X3     X4     X5     X6     X7     X8     X9     XA     XB     XC     XD     XE     XF */
		/* 0X */   false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		/* 1X */   false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		/* 2X */   false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		/* 3X */   true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, false, false, false, false, false, /* 0-9 */
		/* 4X */   false, true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, /* A-Z */
		/* 5X */   true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, false, false, false, false,
		/* 6X */   false, true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, /* a-z */
		/* 7X */   true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, false, false, false, false
		/* non-ASCII values initialized to false */
	};
	// clang-format on
	return (is_alnum_table[uch]);
}

template<typename Target>
Target scan_string(const std::wstring& str)
{
	Target t;
	std::wstringstream iss(str);
	iss.imbue(std::locale::classic());
	iss >> t;
	if (iss.bad())
	{
		throw std::bad_cast();
	}
	return t;
}

struct to_lower_ch_impl
{
	char operator()(char c) const noexcept
	{
		if (c >= 'A' && c <= 'Z') return static_cast<char>(c - 'A' + 'a');
		return c;
	}

	wchar_t operator()(wchar_t c) const noexcept
	{
		if (c >= L'A' && c <= L'Z') return static_cast<wchar_t>(c - L'A' + L'a');
		return c;
	}
};

constexpr to_lower_ch_impl to_lower_ch{};

struct eq_lower_ch_impl
{
	template<class CharT>
	inline CharT operator()(const CharT left, const CharT right) const noexcept
	{
		return to_lower_ch(left) == to_lower_ch(right);
	}
};

constexpr eq_lower_ch_impl eq_lower_ch{};

struct lt_lower_ch_impl
{
	template<class CharT>
	inline CharT operator()(const CharT left, const CharT right) const noexcept
	{
		return to_lower_ch(left) < to_lower_ch(right);
	}
};

constexpr lt_lower_ch_impl lt_lower_ch{};

inline void inplace_tolower(std::string& target) noexcept
{
	for (auto& ch : target)
	{
		ch = to_lower_ch(ch);
	}
}

inline void inplace_tolower(std::wstring& target) noexcept
{
	for (auto& ch : target)
	{
		ch = to_lower_ch(ch);
	}
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

/// <summary>
/// Converts char string to int.
/// </summary>
/// <param name="str">string</param>
/// <returns>int value if success, 0 otherwise</returns>
int char_to_int(const std::string& str);;

/// <summary>
/// Converts int to char string.
/// </summary>
/// <param name="value">int value</param>
/// <returns>string representaion of value, empty string otherwise</returns>
std::string int_to_char(int value);

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

bool CrackUrl(const std::wstring& url, std::wstring& host = std::wstring(), std::wstring& path = std::wstring());

bool DownloadFile(const std::wstring& url, std::vector<BYTE>& image);

}
