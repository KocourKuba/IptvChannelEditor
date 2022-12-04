#pragma once

constexpr auto APP_SETTINGS = L"Application";

constexpr auto CMP_FLAG_TITLE          = 0x01;
constexpr auto CMP_FLAG_ICON           = 0x02;
constexpr auto CMP_FLAG_ARCHIVE        = 0x04;
constexpr auto CMP_FLAG_EPG1           = 0x08;
constexpr auto CMP_FLAG_EPG2           = 0x10;
constexpr auto CMP_FLAG_ALL            = CMP_FLAG_TITLE | CMP_FLAG_ICON | CMP_FLAG_ARCHIVE | CMP_FLAG_EPG1 | CMP_FLAG_EPG2;

constexpr auto REG_WINDOW_POS		     = _T("WindowPos");
constexpr auto REG_ICON_WINDOW_POS	     = _T("IconsWindowPos");
constexpr auto REG_EPG_WINDOW_POS	     = _T("EpgWindowPos");
constexpr auto REG_VOD_WINDOW_POS	     = _T("VodWindowPos");
constexpr auto REG_ACC_WINDOW_POS	     = _T("AccountWindowPos");
constexpr auto REG_PLUGIN_CFG_WINDOW_POS = _T("PluginConfigWindowPos");
constexpr auto REG_CONFIG_WINDOW_POS     = _T("ConfigWindowPos");

// app
constexpr auto REG_NEXT_UPDATE         = _T("NextUpdate");
constexpr auto REG_AVAIL_UPDATE        = _T("AvailableUpdate");

// settings dialog
constexpr auto REG_PLAYER              = _T("Player");
constexpr auto REG_FFPROBE             = _T("FFProbe");
constexpr auto REG_LISTS_PATH          = _T("ListsPath");
constexpr auto REG_OUTPUT_PATH         = _T("PluginsPath");
constexpr auto REG_WEB_UPDATE_PATH     = _T("PluginsWebUpdatePath");
constexpr auto REG_SAVE_SETTINGS_PATH  = _T("PluginsSettingsPath");
constexpr auto REG_AUTO_SYNC           = _T("AutoSyncChannel");
constexpr auto REG_AUTO_HIDE           = _T("AutoHideToTray");
constexpr auto REG_MAX_THREADS         = _T("MaxStreamThreads");
constexpr auto REG_LANGUAGE            = _T("Language");
constexpr auto REG_CMP_FLAGS           = _T("CompareFlags");
constexpr auto REG_UPDATE_FREQ         = _T("UpdateFrequencies");
constexpr auto REG_UPDATE_PL           = _T("UpdatePlaylists");
constexpr auto REG_COLOR_ADDED         = _T("ColorAdded");
constexpr auto REG_COLOR_NOT_ADDED     = _T("ColorNotAdded");
constexpr auto REG_COLOR_CHANGED       = _T("ColorChanged");
constexpr auto REG_COLOR_UNKNOWN       = _T("ColorUnknown");
constexpr auto REG_COLOR_HEVC          = _T("ColorHEVC");

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
constexpr auto REG_LIST_DOMAIN         = _T("ListDomain");
constexpr auto REG_EPG_DOMAIN          = _T("EpgDomain");
constexpr auto REG_PLUGIN_SUFFIX       = _T("PluginSuffix");
constexpr auto REG_ACCOUNT_DATA        = _T("AccountData");
constexpr auto REG_ACTIVE_ACCOUNT      = _T("ActiveAccount");
constexpr auto REG_ACTIVE_CH_LIST      = _T("ActiveChannelsList");
