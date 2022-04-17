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
#include "json.hpp"

#define JSON_ALL_TRY try {
#define JSON_ALL_CATCH } \
		catch (const nlohmann::json::parse_error& ex) \
		{ \
			/* parse errors are ok, because input may be random bytes*/ \
			TRACE("\nparse error: %s\n", ex.what()); \
		} \
		catch (const nlohmann::json::out_of_range& ex) \
		{ \
			/* out of range errors may happen if provided sizes are excessive */ \
			TRACE("\nout of range error: %s\n", ex.what()); \
		} \
		catch (const nlohmann::detail::type_error& ex) \
		{ \
			/* type error */ \
			TRACE("\ntype error: %s\n", ex.what()); \
		} \
		catch (...) \
		{ \
			TRACE("\nunknown exception\n"); \
		}
