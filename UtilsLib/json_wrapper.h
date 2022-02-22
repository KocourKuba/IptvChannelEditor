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
