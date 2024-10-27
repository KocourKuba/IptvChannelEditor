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

#pragma once
#include "nlohmann\detail\macro_scope.hpp"

#define ENUM_TO_STRING(ENUM_TYPE, STRING_TYPE, ...)                                                                    \
inline STRING_TYPE enum_to_string(const ENUM_TYPE e)                                                                   \
{                                                                                                                      \
	static_assert(std::is_enum<ENUM_TYPE>::value, #ENUM_TYPE " must be an enum!");                                     \
	static const std::pair<ENUM_TYPE, STRING_TYPE> m[] = __VA_ARGS__;                                                  \
	auto it = std::find_if(std::begin(m), std::end(m), [&e](const std::pair<ENUM_TYPE, STRING_TYPE>& ej_pair)          \
	{                                                                                                                  \
		return ej_pair.first == e;                                                                                     \
	});                                                                                                                \
	return (it != std::end(m)) ? it->second : STRING_TYPE();                                                           \
}

enum class StreamType
{
	enHLS = 0,
	enMPEGTS,
	enLast,
};

NLOHMANN_JSON_SERIALIZE_ENUM(StreamType,
{
	{ StreamType::enHLS,    "hls"  },
	{ StreamType::enMPEGTS, "mpeg" },
	{ StreamType::enLast,   "" }
})

enum class CatchupType {
	cu_shift,
	cu_append,
	cu_flussonic,
	cu_default,
	cu_not_set,
};

NLOHMANN_JSON_SERIALIZE_ENUM(CatchupType,
{
	{ CatchupType::cu_shift,     "shift"     },
	{ CatchupType::cu_append,    "append"    },
	{ CatchupType::cu_flussonic, "flussonic" }
})

enum class AccountAccessType
{
	enUnknown = -1,
	enOtt,
	enPin,
	enLoginPass,
	enNone,
};

NLOHMANN_JSON_SERIALIZE_ENUM(AccountAccessType,
{
	{ AccountAccessType::enUnknown,   "unknown" },
	{ AccountAccessType::enOtt,       "ottkey"  },
	{ AccountAccessType::enPin,       "pin"     },
	{ AccountAccessType::enLoginPass, "login"   },
	{ AccountAccessType::enNone,      "none"    }
})

enum class ImageLibType
{
	enNone = 0,
	enFile,
	enLink,
	enM3U,
	enHTML,
};

NLOHMANN_JSON_SERIALIZE_ENUM(ImageLibType,
{
	{ ImageLibType::enNone, "none" },
	{ ImageLibType::enFile, "file" },
	{ ImageLibType::enLink, "link" },
	{ ImageLibType::enM3U,  "m3u"  },
	{ ImageLibType::enHTML, "html" },
})

enum class VodEngine
{
	enNone = 0,
	enM3U,
	enJson,
	enXC,
	enLast,
};

NLOHMANN_JSON_SERIALIZE_ENUM(VodEngine,
{
	{ VodEngine::enNone,   "None"        },
	{ VodEngine::enM3U,    "M3U"         },
	{ VodEngine::enJson,   "Json"        },
	{ VodEngine::enXC,     "XtreamCodes" },
})

ENUM_TO_STRING(VodEngine, std::wstring,
{
	{ VodEngine::enNone,   L"None"        },
	{ VodEngine::enM3U,    L"M3U"         },
	{ VodEngine::enJson,   L"Json"        },
	{ VodEngine::enXC,     L"XtreamCodes" },
	{ VodEngine::enLast,   L"Last"        },
})

enum class DynamicParamsType
{
	enUnknown = -1,
	enServers,
	enDevices,
	enQuality,
	enProfiles,
	enFiles,
	enLinks,
	enManifest,
	enPlaylistTV,
	enPlaylistVOD,
	enDomains,
};

ENUM_TO_STRING(DynamicParamsType, std::wstring,
{
	{ DynamicParamsType::enUnknown,       L"Unknown"      },
	{ DynamicParamsType::enServers,       L"Servers"      },
	{ DynamicParamsType::enDevices,       L"Devices"      },
	{ DynamicParamsType::enQuality,       L"Quality"      },
	{ DynamicParamsType::enProfiles,      L"Profiles"     },
	{ DynamicParamsType::enFiles,         L"Files"        },
	{ DynamicParamsType::enLinks,         L"Links"        },
	{ DynamicParamsType::enManifest,      L"Manifest"     },
	{ DynamicParamsType::enPlaylistTV,    L"Playlist TV"  },
	{ DynamicParamsType::enPlaylistVOD,   L"Playlist VOD" },
	{ DynamicParamsType::enDomains,       L"Domains"      },
})

