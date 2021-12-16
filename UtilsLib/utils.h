#pragma once
#include <algorithm>
#include <locale>

namespace utils
{
	static constexpr auto DUNE_PLUGIN_NAME = L"dune_plugin_{:s}_mod.zip";
	static constexpr auto ICON_TEMPLATE = L"plugin_file://icons/channel_unset.png";
	static constexpr auto PLUGIN_PATH = R"(plugin_file://)";

	static constexpr auto VERSION_INFO = "version_info";
	static constexpr auto LIST_VERSION = "list_version";
	static constexpr auto CHANNELS_SETUP = "channels_setup";
	static constexpr auto ACCESS_TOKEN = "access_key";
	static constexpr auto ACCESS_DOMAIN = "access_domain";
	static constexpr auto ACCESS_LOGIN = "access_login";
	static constexpr auto ACCESS_PASSWORD = "access_password";

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
/// Converts a UTF-16 string to a UTF-8 string.
/// </summary>
/// <param name="w">A two byte character UTF-16 string.</param>
/// <returns>A single byte character UTF-8 string.</returns>
std::string utf16_to_utf8(const std::wstring& w);
std::string utf16_to_utf8(const wchar_t* srcData, size_t srcSize);

/// <summary>
/// Converts a UTF-8 string to a UTF-16
/// </summary>
/// <param name="s">A single byte character UTF-8 string.</param>
/// <returns>A two byte character UTF-16 string.</returns>
std::wstring utf8_to_utf16(const std::string& s);
std::wstring utf8_to_utf16(const char* srcData, size_t srcSize);

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
}
