/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2023): sharky72 (https://github.com/KocourKuba)

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

constexpr auto APP_SETTINGS = L"Application";
constexpr auto CHANNELS_LIST_VERSION = 7;
constexpr auto PACK_DLL = L"7za.dll";

constexpr auto CMP_FLAG_TITLE          = 0x01;
constexpr auto CMP_FLAG_ICON           = 0x02;
constexpr auto CMP_FLAG_ARCHIVE        = 0x04;
constexpr auto CMP_FLAG_EPG1           = 0x08;
constexpr auto CMP_FLAG_EPG2           = 0x10;
constexpr auto CMP_FLAG_ALL            = CMP_FLAG_TITLE | CMP_FLAG_ICON | CMP_FLAG_ARCHIVE | CMP_FLAG_EPG1 | CMP_FLAG_EPG2;

constexpr auto REG_WINDOW_POS		     = _T("WindowPos");
constexpr auto REG_ICON_WINDOW_POS	     = _T("IconsWindowPos");
constexpr auto REG_ICON_COLUMNS_WIDTH    = _T("IconColumnsWidth");
constexpr auto REG_EPG_WINDOW_POS	     = _T("EpgWindowPos");
constexpr auto REG_EPG_COLUMNS_WIDTH	 = _T("EpgColumnsWidth");
constexpr auto REG_VOD_WINDOW_POS	     = _T("VodWindowPos");
constexpr auto REG_ACC_WINDOW_POS	     = _T("AccountWindowPos");
constexpr auto REG_PLUGIN_CFG_WINDOW_POS = _T("PluginConfigWindowPos");
constexpr auto REG_CONFIG_WINDOW_POS     = _T("ConfigWindowPos");
constexpr auto REG_FILL_INFO_WINDOW_POS	 = _T("FillInfoWindowPos");

// app
constexpr auto REG_NEXT_UPDATE         = _T("NextUpdate");
constexpr auto REG_AVAIL_UPDATE        = _T("AvailableUpdate");
constexpr auto REG_DUNE_IP             = _T("DuneIP");

// settings dialog
constexpr auto REG_PLAYER              = _T("Player");
constexpr auto REG_FFPROBE             = _T("FFProbe");
constexpr auto REG_LISTS_PATH          = _T("ListsPath");
constexpr auto REG_OUTPUT_PATH         = _T("PluginsPath");
constexpr auto REG_WEB_UPDATE_PATH     = _T("PluginsWebUpdatePath");
constexpr auto REG_SAVE_SETTINGS_PATH  = _T("PluginsSettingsPath");
constexpr auto REG_AUTO_SYNC           = _T("AutoSyncChannel");
constexpr auto REG_AUTO_HIDE           = _T("AutoHideToTray");
constexpr auto REG_CONVERT_DUPES       = _T("ConvertDupes");
constexpr auto REG_MAX_THREADS         = _T("MaxStreamThreads");
constexpr auto REG_MAX_CACHE_TTL       = _T("MaxCacheTTL");
constexpr auto REG_LANGUAGE            = _T("Language");
constexpr auto REG_CMP_FLAGS           = _T("CompareFlags");
constexpr auto REG_UPDATE_FREQ         = _T("UpdateFrequencies");
constexpr auto REG_UPDATE_PL           = _T("UpdatePlaylists");
constexpr auto REG_COLOR_ADDED         = _T("ColorAdded");
constexpr auto REG_COLOR_NOT_ADDED     = _T("ColorNotAdded");
constexpr auto REG_COLOR_CHANGED       = _T("ColorChanged");
constexpr auto REG_COLOR_UNKNOWN       = _T("ColorUnknown");
constexpr auto REG_COLOR_HEVC          = _T("ColorHEVC");
constexpr auto REG_COLOR_DUPLICATED    = _T("ColorDuplicated");

// main dialog
constexpr auto REG_PLUGIN              = _T("PluginType");
constexpr auto REG_ICON_SOURCE         = _T("IconSource");
constexpr auto REG_DAYS_BACK           = _T("DaysBack");
constexpr auto REG_HOURS_BACK          = _T("HoursBack");
constexpr auto REG_SHOW_URL            = _T("ShowStreamUrl");
constexpr auto REG_SHOW_EPG            = _T("ShowEPG");

// Plugin dependent

// accounts data
constexpr auto REG_LOGIN               = _T("Login");
constexpr auto REG_PASSWORD            = _T("Password");
constexpr auto REG_TOKEN               = _T("AccessKey");
constexpr auto REG_DOMAIN              = _T("Domain");
constexpr auto REG_VPORTAL             = _T("VPortal");
constexpr auto REG_EMBED_INFO          = _T("EmbedInfo");
constexpr auto REG_DEVICE_ID           = _T("DeviceID");
constexpr auto REG_PROFILE_ID          = _T("ProfileID");
constexpr auto REG_QUALITY_ID          = _T("QualityID");
constexpr auto REG_CUSTOM_XMLTV_SOURCE = _T("CustomXmltSource");

constexpr auto REG_FILTER_STRING_S     = _T("FilterString");
constexpr auto REG_FILTER_STRING_H     = _T("FilterStringHide");
constexpr auto REG_FILTER_STRING_LST_S = _T("FilterStringList");
constexpr auto REG_FILTER_STRING_LST_H = _T("FilterStringListHide");
constexpr auto REG_FILTER_REGEX_S      = _T("FilterUseRegex");
constexpr auto REG_FILTER_REGEX_H      = _T("FilterUseRegexHide");
constexpr auto REG_FILTER_CASE_S       = _T("FilterUseCase");
constexpr auto REG_FILTER_CASE_H       = _T("FilterUseCaseHide");
constexpr auto REG_FILTER_STATE_S      = _T("FilterState");
constexpr auto REG_FILTER_STATE_H      = _T("FilterStateHide");
constexpr auto REG_CHANNELS_TYPE       = _T("ChannelsType");
constexpr auto REG_PLAYLIST_TYPE       = _T("PlaylistType");
constexpr auto REG_STREAM_TYPE         = _T("StreamType");
constexpr auto REG_CUSTOM_PLAYLIST     = _T("CustomPlaylist");
constexpr auto REG_CUSTOM_PL_FILE      = _T("CustomPlaylistFile");
constexpr auto REG_CREDENTIALS         = _T("Credentials");
constexpr auto REG_PLUGIN_SUFFIX       = _T("PluginSuffix");
constexpr auto REG_ACCOUNT_DATA        = _T("AccountData");
constexpr auto REG_ACTIVE_ACCOUNT      = _T("ActiveAccount");
constexpr auto REG_ACTIVE_CH_LIST      = _T("ActiveChannelsList");

constexpr auto REPL_API_URL            = L"{API_URL}";       // special url used to get information from provider
constexpr auto REPL_LIVE_URL           = L"{LIVE_URL}";      // live url, used in archive template substitution
constexpr auto REPL_CGI_BIN            = L"{CGI_BIN}";       // Url points to plugin cgi_bin folder
constexpr auto REPL_DOMAIN             = L"{DOMAIN}";        // stream url domain (set from playlist)
constexpr auto REPL_PORT               = L"{PORT}";          // stream url port (set from playlist)
constexpr auto REPL_ID                 = L"{ID}";            // id (set from playlist)
constexpr auto REPL_PL_DOMAIN          = L"{PL_DOMAIN}";     // playlist domain (set from settings or set by provider)
constexpr auto REPL_VOD_DOMAIN         = L"{VOD_DOMAIN}";     // playlist domain (set from settings or set by provider)
constexpr auto REPL_SUBDOMAIN          = L"{SUBDOMAIN}";     // domain (set from settings or set by provider)
constexpr auto REPL_TOKEN              = L"{TOKEN}";         // token (set from playlist or set by provider)
constexpr auto REPL_LOGIN              = L"{LOGIN}";         // login (set from settings)
constexpr auto REPL_PASSWORD           = L"{PASSWORD}";      // password (set from settings)
constexpr auto REPL_INT_ID             = L"{INT_ID}";        // internal id (reads from playlist)
constexpr auto REPL_HOST               = L"{HOST}";          // host (reads from playlist)
constexpr auto REPL_SERVER             = L"{SERVER}";        // server name (read from settings)
constexpr auto REPL_SERVER_ID          = L"{SERVER_ID}";     // server id (read from settings)
constexpr auto REPL_DEVICE_ID          = L"{DEVICE_ID}";     // device id (read from settings)
constexpr auto REPL_QUALITY_ID         = L"{QUALITY_ID}";    // quality id (set from settings)
constexpr auto REPL_PROFILE_ID         = L"{PROFILE_ID}";    // profile id (read from settings)
constexpr auto REPL_VAR1               = L"{VAR1}";          // Custom capture group variable
constexpr auto REPL_VAR2               = L"{VAR2}";          // Custom capture group variable
constexpr auto REPL_VAR3               = L"{VAR3}";          // Custom capture group variable

constexpr auto REPL_EPG_DOMAIN         = L"{EPG_DOMAIN}";    // epg domain. can be obtain from provider
constexpr auto REPL_EPG_ID             = L"{EPG_ID}";        // epg id (set from playlist)
constexpr auto REPL_START              = L"{START}";         // EPG archive start time (unix timestamp)
constexpr auto REPL_NOW                = L"{NOW}";           // EPG archive current time (unix timestamp)
constexpr auto REPL_DATE               = L"{DATE}";          // EPG date (set by format)
constexpr auto REPL_TIMESTAMP          = L"{TIMESTAMP}";     // EPG time, unix timestamp (set by format)
constexpr auto REPL_OFFSET             = L"{OFFSET}";        // EPG archive current time (unix timestamp)
constexpr auto REPL_DUNE_IP            = L"{DUNE_IP}";       // dune IP address. Useful for using My EPG Server plugin

constexpr auto REPL_DURATION           = L"{DURATION}";      // archive duration (in second) in flussonic archive
constexpr auto REPL_STOP               = L"{STOP}";          // archive end time (unix timestamp)

constexpr auto REPL_YEAR               = L"{YEAR}";          // Year subst template, used in epg_date_format, epg_time_format
constexpr auto REPL_MONTH              = L"{MONTH}";         // Month subst template, used in epg_date_format, epg_time_format
constexpr auto REPL_DAY                = L"{DAY}";           // Day subst template, used in epg_date_format, epg_time_format
constexpr auto REPL_HOUR               = L"{HOUR}";          // Hour subst template, used in epg_time_format
constexpr auto REPL_MIN                = L"{MIN}";           // Minute subst template, used in epg_time_format

constexpr auto REPL_TYPE               = L"{TYPE}";          // Plugin type template, used to compile plugin file/package
constexpr auto REPL_NAME               = L"{NAME}";          // Plugin name template, used to compile plugin file/package
constexpr auto REPL_COMMENT            = L"{COMMENT}";       // Account comment, used to compile plugin file/package
constexpr auto REPL_VERSION            = L"{VERSION}";       // Plugin version template, used to compile plugin file/package
constexpr auto REPL_VERSION_INDEX      = L"{VERSION_INDEX}"; // Plugin version index template, used to compile plugin file/package
