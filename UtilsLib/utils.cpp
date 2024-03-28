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
#include <chrono>
#include <random>

#include "utils.h"

#include <boost/regex.hpp>
#include <fmt/chrono.h>

namespace utils
{
constexpr auto LOW_3BITS = 0x7;
constexpr auto LOW_4BITS = 0xF;
constexpr auto LOW_5BITS = 0x1F;
constexpr auto LOW_6BITS = 0x3F;

constexpr auto BIT4 = 0x8;
constexpr auto BIT5 = 0x10;
constexpr auto BIT6 = 0x20;
constexpr auto BIT7 = 0x40;
constexpr auto BIT8 = 0x80;

constexpr auto L_SURROGATE_START = 0xDC00;
constexpr auto L_SURROGATE_END = 0xDFFF;
constexpr auto H_SURROGATE_START = 0xD800;
constexpr auto H_SURROGATE_END = 0xDBFF;
constexpr auto SURROGATE_PAIR_START = 0x10000;

constexpr auto DATE_OFF_YEAR = 0;
constexpr auto DATE_OFF_MON  = 4;
constexpr auto DATE_OFF_DAY  = 6;
constexpr auto DATE_OFF_HOUR = 8;
constexpr auto DATE_OFF_MIN  = 10;
constexpr auto DATE_OFF_SEC  = 12;

inline int atoi_2(const char* s)
{
	return (int)(s[0] - '0') * 10 + (s[1] - '0');
}

inline int atoi_4(const char* s)
{
	return atoi_2(s) * 100 + atoi_2(s + 2);
}

// simulation of Windows GetTickCount()
uint64_t ChronoGetTickCount()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

uint64_t GetTimeDiff(uint64_t dwStartTime)
{
	uint64_t dwCurrent = ChronoGetTickCount();

	if (dwStartTime > dwCurrent)
		return (unsigned long long) - 1 - dwCurrent - dwStartTime;

	return (dwCurrent - dwStartTime);
}

bool CheckForTimeOut(uint64_t dwStartTime, uint32_t dwTimeOut)
{
	return (GetTimeDiff(dwStartTime) > dwTimeOut);
}

inline size_t count_utf8_to_utf16(const char* sData, size_t sSize)
{
	size_t destSize(sSize);

	try
	{
		for (size_t index = 0; index < sSize;)
		{
			if (sData[index] >= 0)
			{
				// use fast inner loop to skip single byte code points (which are
				// expected to be the most frequent)
				while ((++index < sSize) && (sData[index] >= 0))
					;

				if (index >= sSize) break;
			}

			// start special handling for multi-byte code points
			const char c{ sData[index++] };

			if ((c & BIT7) == 0)
			{
				throw std::range_error("UTF-8 string character can never start with 10xxxxxx");
			}
			if ((c & BIT6) == 0) // 2 byte character, 0x80 to 0x7FF
			{
				if (index == sSize)
				{
					throw std::range_error("UTF-8 string is missing bytes in character");
				}

				const char c2{ sData[index++] };
				if ((c2 & 0xC0) != BIT8)
				{
					throw std::range_error("UTF-8 continuation byte is missing leading bit mask");
				}

				// can't require surrogates for 7FF
				--destSize;
			}
			else if ((c & BIT5) == 0) // 3 byte character, 0x800 to 0xFFFF
			{
				if (sSize - index < 2)
				{
					throw std::range_error("UTF-8 string is missing bytes in character");
				}

				const char c2{ sData[index++] };
				const char c3{ sData[index++] };
				if (((c2 | c3) & 0xC0) != BIT8)
				{
					throw std::range_error("UTF-8 continuation byte is missing leading bit mask");
				}

				destSize -= 2;
			}
			else if ((c & BIT4) == 0) // 4 byte character, 0x10000 to 0x10FFFF
			{
				if (sSize - index < 3)
				{
					throw std::range_error("UTF-8 string is missing bytes in character");
				}

				const char c2{ sData[index++] };
				const char c3{ sData[index++] };
				const char c4{ sData[index++] };
				if (((c2 | c3 | c4) & 0xC0) != BIT8)
				{
					throw std::range_error("UTF-8 continuation byte is missing leading bit mask");
				}

				const uint32_t codePoint =
					((c & LOW_3BITS) << 18) | ((c2 & LOW_6BITS) << 12) | ((c3 & LOW_6BITS) << 6) | (c4 & LOW_6BITS);
				destSize -= (3 - (codePoint >= SURROGATE_PAIR_START));
			}
			else
			{
				throw std::range_error("UTF-8 string has invalid Unicode code point");
			}
		}
	}
	catch (std::range_error& ex)
	{
		ex;
		destSize = 0;
	}

	return destSize;
}

inline size_t count_utf16_to_utf8(const wchar_t* srcData, size_t srcSize)
{
	size_t destSize(srcSize);
	try
	{
		for (size_t index = 0; index < srcSize; ++index)
		{
			const std::wstring::value_type ch(srcData[index]);
			if (ch <= 0x7FF)
			{
				if (ch > 0x7F) // 2 bytes needed (11 bits used)
				{
					++destSize;
				}
			}
			// Check for high surrogate.
			else if (ch >= H_SURROGATE_START && ch <= H_SURROGATE_END) // 4 bytes needed (21 bits used)
			{
				++index;
				if (index == srcSize)
				{
					throw std::range_error("UTF-16 string is missing low surrogate");
				}

				const auto lowSurrogate = srcData[index];
				if (lowSurrogate < L_SURROGATE_START || lowSurrogate > L_SURROGATE_END)
				{
					throw std::range_error("UTF-16 string has invalid low surrogate");
				}

				destSize += 2;
			}
			else // 3 bytes needed (16 bits used)
			{
				destSize += 2;
			}
		}
	}
	catch (std::range_error& ex)
	{
		ex;
		destSize = 0;
	}

	return destSize;
}

std::string utf16_to_utf8(const wchar_t* srcData, size_t srcSize)
{
	std::string dest(count_utf16_to_utf8(srcData, srcSize), '\0');
	char* destData = &dest[0];
	size_t destIndex(0);

	for (size_t index = 0; index < srcSize; ++index)
	{
		const std::wstring::value_type src = srcData[index];
		if (src <= 0x7FF)
		{
			if (src <= 0x7F) // single byte character
			{
				destData[destIndex++] = static_cast<char>(src);
			}
			else // 2 bytes needed (11 bits used)
			{
				destData[destIndex++] = static_cast<char>(char((src >> 6) | 0xC0));        // leading 5 bits
				destData[destIndex++] = static_cast<char>(char((src & LOW_6BITS) | BIT8)); // trailing 6 bits
			}
		}
		// Check for high surrogate.
		else if (src >= H_SURROGATE_START && src <= H_SURROGATE_END)
		{
			const auto highSurrogate = src;
			const auto lowSurrogate = srcData[++index];

			// To get from surrogate pair to Unicode code point:
			// - subtract 0xD800 from high surrogate, this forms top ten bits
			// - subtract 0xDC00 from low surrogate, this forms low ten bits
			// - add 0x10000
			// Leaves a code point in U+10000 to U+10FFFF range.
			uint32_t codePoint = highSurrogate - H_SURROGATE_START;
			codePoint <<= 10;
			codePoint |= lowSurrogate - L_SURROGATE_START;
			codePoint += SURROGATE_PAIR_START;

			// 4 bytes needed (21 bits used)
			destData[destIndex++] = static_cast<char>((codePoint >> 18) | 0xF0);               // leading 3 bits
			destData[destIndex++] = static_cast<char>(((codePoint >> 12) & LOW_6BITS) | BIT8); // next 6 bits
			destData[destIndex++] = static_cast<char>(((codePoint >> 6) & LOW_6BITS) | BIT8);  // next 6 bits
			destData[destIndex++] = static_cast<char>((codePoint & LOW_6BITS) | BIT8);         // trailing 6 bits
		}
		else // 3 bytes needed (16 bits used)
		{
			destData[destIndex++] = static_cast<char>((src >> 12) | 0xE0);              // leading 4 bits
			destData[destIndex++] = static_cast<char>(((src >> 6) & LOW_6BITS) | BIT8); // middle 6 bits
			destData[destIndex++] = static_cast<char>((src & LOW_6BITS) | BIT8);        // trailing 6 bits
		}
	}

	return dest;
}

std::wstring utf8_to_utf16(const char* srcData, size_t srcSize)
{
	// Save repeated heap allocations, use the length of resulting sequence.
	std::wstring dest(count_utf8_to_utf16(srcData, srcSize), L'\0');
	std::wstring::value_type* const destData = &dest[0];
	size_t destIndex = 0;

	for (size_t index = 0; index < srcSize; ++index)
	{
		char src = srcData[index];
		switch (src & 0xF0)
		{
			case 0xF0: // 4 byte character, 0x10000 to 0x10FFFF
			{
				const char c2{ srcData[++index] };
				const char c3{ srcData[++index] };
				const char c4{ srcData[++index] };
				uint32_t codePoint =
					((src & LOW_3BITS) << 18) | ((c2 & LOW_6BITS) << 12) | ((c3 & LOW_6BITS) << 6) | (c4 & LOW_6BITS);
				if (codePoint >= SURROGATE_PAIR_START)
				{
					// In UTF-16 U+10000 to U+10FFFF are represented as two 16-bit code units, surrogate pairs.
					//  - 0x10000 is subtracted from the code point
					//  - high surrogate is 0xD800 added to the top ten bits
					//  - low surrogate is 0xDC00 added to the low ten bits
					codePoint -= SURROGATE_PAIR_START;
					destData[destIndex++] = static_cast<std::wstring::value_type>((codePoint >> 10) | H_SURROGATE_START);
					destData[destIndex++] =
						static_cast<std::wstring::value_type>((codePoint & 0x3FF) | L_SURROGATE_START);
				}
				else
				{
					// In UTF-16 U+0000 to U+D7FF and U+E000 to U+FFFF are represented exactly as the Unicode code point
					// value. U+D800 to U+DFFF are not valid characters, for simplicity we assume they are not present
					// but will encode them if encountered.
					destData[destIndex++] = static_cast<std::wstring::value_type>(codePoint);
				}
			}
			break;
			case 0xE0: // 3 byte character, 0x800 to 0xFFFF
			{
				const char c2{ srcData[++index] };
				const char c3{ srcData[++index] };
				destData[destIndex++] = static_cast<std::wstring::value_type>(
					((src & LOW_4BITS) << 12) | ((c2 & LOW_6BITS) << 6) | (c3 & LOW_6BITS));
			}
			break;
			case 0xD0: // 2 byte character, 0x80 to 0x7FF
			case 0xC0:
			{
				const char c2{ srcData[++index] };
				destData[destIndex++] =
					static_cast<std::wstring::value_type>(((src & LOW_5BITS) << 6) | (c2 & LOW_6BITS));
			}
			break;
			default: // single byte character, 0x0 to 0x7F
				// try to use a fast inner loop for following single byte characters,
				// since they are quite probable
				do
				{
					destData[destIndex++] = static_cast<std::wstring::value_type>(srcData[index++]);
				} while (index < srcSize && srcData[index] > 0);
				// adjust index since it will be incremented by the for loop
				--index;
		}
	}
	return dest;
}

std::string generateRandomId(size_t length /*= 0*/)
{
	static const std::string allowed_chars{ "123456789BCDFGHJKLMNPQRSTVWXZbcdfghjklmnpqrstvwxz" };

	static thread_local std::default_random_engine randomEngine(std::random_device{}());
	static thread_local std::uniform_int_distribution<int> randomDistribution(0, (int)allowed_chars.size() - 1);

	std::string id(length ? length : 32, '\0');

	for (auto& c : id) {
		c = allowed_chars[randomDistribution(randomEngine)];
	}

	return id;
}

bool is_ascii(const wchar_t* szFilename)
{
	size_t i = 0;
	while (szFilename[i] != 0)
	{
		if (szFilename[i++] > 127)
		{
			return false;
		}
	}

	return true;
}

bool is_number(const wchar_t* szFilename)
{
	size_t i = 0;
	while (szFilename[i] != 0)
	{
		if (!std::isdigit(szFilename[i++]))
		{
			return false;
		}
	}

	return true;
}

/**
 * Timezone parsing code based loosely on the algorithm in
 * filldata.cpp of MythTV.
 */
static time_t parse_xmltv_timezone(const char* tzstr, unsigned int len)
{
	time_t result = 0;

	if ((len == 5) && (tzstr[0] == '+')) {

		result = (3600 * atoi_2(tzstr + 1)) + (60 * atoi_2(tzstr + 3));

	}
	else if ((len == 5) && (tzstr[0] == '-')) {

		result = -(3600 * atoi_2(tzstr + 1)) + (60 * atoi_2(tzstr + 3));

	}

	return result;
}

time_t parse_xmltv_date(const char* sz_date, size_t full_len)
{
	/*
	 * according to the xmltv dtd:
	 *
	 * All dates and times in this the xmltv DTD follow the same format,
	 * loosely based on ISO 8601. They can be 'YYYYMMDDhhmmss' or some initial substring,
	 * for example if you only know the year and month you can have 'YYYYMM'.
	 * Also can be append a timezone to the end;
	 * if no explicit timezone is given, UT is assumed.
	 * Examples:
	 * '200007281733 BST', '200209', '19880523083000 +0300'.  (BST == +0100.)
	 *
	 * thus:
	 * example *date = "20031022220000 +0200"
	 * type:            YYYYMMDDhhmmss ZZzzz"
	 * position:        0         1         2
	 *                  012345678901234567890
	 *
	 * symbolic names of tz is obsolete in my case. Most of providers uses numeric tz offset
	 *
	 * note: since part of the time specification can be omitted, we cannot hard-code the offset to the timezone!
	 */

	 /*
	  * For some reason, mktime() accepts broken-time arguments as localtime,
	  * and there is no corresponding UTC function.
	  * For this reason we have to calculate the offset from GMT to adjust the
	  * argument given to mktime().
	  */

	std::tm pTm = fmt::localtime(time(nullptr));
	long tmz = 0;
	_get_timezone(&tmz);
	long gmtoff = 60 * 60 * pTm.tm_isdst - tmz;

	/* Find where the timezone starts */
	const char* p = sz_date;
	while ((*p >= '0') && (*p <= '9'))
	{
		p++;
	}

	// Calculate the length of the date
	size_t len = p - sz_date;
	time_t tz = 0;
	if (*p == ' ')
	{
		// Parse the timezone, skipping the initial space
		const char* tz_str = p + 1;
		size_t tz_len = full_len - len - 1;
		if (tz_len == 5)
		{
			if (tz_str[0] == '+')
			{
				tz = atoi_2(tz_str + 1) * 3600 + 60 * atoi_2(tz_str + 3);
			}
			else if (tz_str[0] == '-')
			{
				tz = 60 * atoi_2(tz_str + 3) - atoi_2(tz_str + 1) * 3600;
			}
		}
	}
	else if (*p != 0 && *p != '\"')
	{
		// syntax error, bad format
		return 0;
	}

	struct tm tm_obj;

	time_t result = 0;
	if (len >= DATE_OFF_SEC + 2)
	{
		tm_obj.tm_sec = atoi_2(sz_date + DATE_OFF_SEC);
	}
	else
	{
		tm_obj.tm_sec = 0;
	}

	if (len >= DATE_OFF_MIN + 2)
	{
		tm_obj.tm_min = atoi_2(sz_date + DATE_OFF_MIN);
	}
	else
	{
		tm_obj.tm_min = 0;
	}

	if (len >= DATE_OFF_HOUR + 2)
	{
		tm_obj.tm_hour = atoi_2(sz_date + DATE_OFF_HOUR);
	}
	else
	{
		tm_obj.tm_hour = 0;
	}

	if (len >= DATE_OFF_DAY + 2)
	{
		tm_obj.tm_mday = atoi_2(sz_date + DATE_OFF_DAY);
		tm_obj.tm_mon = atoi_2(sz_date + DATE_OFF_MON);
		tm_obj.tm_year = atoi_4(sz_date + DATE_OFF_YEAR);

		tm_obj.tm_sec = (int)(tm_obj.tm_sec - tz + gmtoff);
		tm_obj.tm_mon -= 1;
		tm_obj.tm_year -= 1900;
		tm_obj.tm_isdst = -1;

		result = mktime(&tm_obj);
	}

	return result;
}

}
