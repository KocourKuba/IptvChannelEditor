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
#include <variant>
#include <map>
#include "UtilsLib\json_wrapper.h"

enum class StreamType
{
	enBase = -2,
	enChannels = -1,
	enAntifriz,
	enEdem,
	enFox,
	enGlanz,
	enItv,
	enOneCent,
	enOneUsd,
	enSharaclub,
	enSharavoz,
	enVipLime,
	enSharaTV,
	enTvTeam,
	enOneOtt,
	enLightIptv,
	enCbilling,
	enOttclub,
	enIptvOnline,
	enVidok,
	enShuraTV,
	enTVClub,
};

enum class AccountAccessType
{
	enUnknown = -1,
	enOtt,
	enPin,
	enLoginPass
};

constexpr auto APP_SETTINGS = L"Application";

constexpr auto CMP_FLAG_TITLE   = 0x01;
constexpr auto CMP_FLAG_ICON    = 0x02;
constexpr auto CMP_FLAG_ARCHIVE = 0x04;
constexpr auto CMP_FLAG_EPG1    = 0x08;
constexpr auto CMP_FLAG_EPG2    = 0x10;
constexpr auto CMP_FLAG_ALL     = CMP_FLAG_TITLE | CMP_FLAG_ICON | CMP_FLAG_ARCHIVE | CMP_FLAG_EPG1 | CMP_FLAG_EPG2;

constexpr auto REG_WINDOW_POS      = _T("WindowPos");
constexpr auto REG_ICON_WINDOW_POS = _T("IconsWindowPos");
constexpr auto REG_EPG_WINDOW_POS  = _T("EpgWindowPos");

// app
constexpr auto REG_NEXT_UPDATE     = _T("NextUpdate");
constexpr auto REG_AVAIL_UPDATE    = _T("AvailableUpdate");

// settings dialog
constexpr auto REG_PLAYER          = _T("Player");
constexpr auto REG_FFPROBE         = _T("FFProbe");
constexpr auto REG_LISTS_PATH      = _T("ListsPath");
constexpr auto REG_OUTPUT_PATH     = _T("PluginsPath");
constexpr auto REG_AUTO_SYNC       = _T("AutoSyncChannel");
constexpr auto REG_AUTO_HIDE       = _T("AutoHideToTray");
constexpr auto REG_MAX_THREADS     = _T("MaxStreamThreads");
constexpr auto REG_LANGUAGE        = _T("Language");
constexpr auto REG_CMP_FLAGS       = _T("CompareFlags");
constexpr auto REG_UPDATE_FREQ     = _T("UpdateFrequencies");
constexpr auto REG_UPDATE_PL       = _T("UpdatePlaylists");

// main dialog
constexpr auto REG_PLUGIN          = _T("PluginType");
constexpr auto REG_ICON_SOURCE     = _T("IconSource");
constexpr auto REG_DAYS_BACK       = _T("DaysBack");
constexpr auto REG_HOURS_BACK      = _T("HoursBack");
constexpr auto REG_SHOW_URL        = _T("ShowStreamUrl");

// Plugin dependent
constexpr auto REG_LOGIN               = _T("Login");
constexpr auto REG_PASSWORD            = _T("Password");
constexpr auto REG_TOKEN               = _T("AccessKey");
constexpr auto REG_DOMAIN              = _T("Domain");
constexpr auto REG_ACCESS_URL          = _T("AccessUrl");
constexpr auto REG_HOST                = _T("Host");
constexpr auto REG_PORTAL              = _T("VPortal");
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
constexpr auto REG_DEVICE_ID           = _T("DeviceID");
constexpr auto REG_CREDENTIALS         = _T("Credentials");

typedef struct
{
	StreamType type;
	std::wstring title;
	std::string short_name;
	std::string int_name;
	std::string version;
} PluginDesc;

//////////////////////////////////////////////////////////////////////////

class ThreadConfig
{
public:
	void NotifyParent(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);

	std::vector<BYTE>* m_data = nullptr;
	void* m_parent = nullptr;
	HANDLE m_hStop = nullptr;
	StreamType m_pluginType = StreamType::enEdem;
	std::wstring m_rootPath;
};

class PluginsConfig
{
public:
	static PluginsConfig& Instance()
	{
		static PluginsConfig _instance;
		return _instance;
	}
	using map_variant = std::map<std::wstring, std::variant<int, __int64, std::wstring, std::vector<BYTE>>>;

private:
	PluginsConfig() = default;
	~PluginsConfig() = default;

	PluginsConfig(const PluginsConfig& source) = delete;

public:
	void SaveSettings();
	void LoadSettings();

	void SaveSettingsToJson();
	void SaveSettingsToRegistry();

	void UpdatePluginSettings();

	void RemovePortableSettings();

	const std::vector<PluginDesc>& get_plugins_info() const;
	PluginDesc get_plugin_info(const StreamType plugin) const;

	int get_plugin_idx() const;
	void set_plugin_idx(int val);

	StreamType get_plugin_type() const;
	void set_plugin_type(StreamType val);

	std::wstring GetCurrentPluginName(bool bCamel = false) const;

	BOOL IsPortable() const { return m_bPortable; }
	void SetPortable(BOOL val) { m_bPortable = val; }

	AccountAccessType get_plugin_account_access_type() const;

public:
	std::wstring get_string(bool isApp, const std::wstring& key, const wchar_t* def = L"") const;
	void set_string(bool isApp, const std::wstring& key, const std::wstring& value);

	int get_int(bool isApp, const std::wstring& key, const int def = 0) const;
	void set_int(bool isApp, const std::wstring& key, const int value);

	__int64 get_int64(bool isApp, const std::wstring& key, const __int64 def = 0) const;
	void set_int64(bool isApp, const std::wstring& key, const __int64 value);

	std::vector<BYTE> get_binary(bool isApp, const std::wstring& key) const;
	bool get_binary(bool isApp, const std::wstring& key, LPBYTE* pbData, size_t& dwSize) const;

	void set_binary(bool isApp, const std::wstring& key, const std::vector<BYTE>& value);
	void set_binary(bool isApp, const std::wstring& key, const BYTE* value, const size_t value_size);

public:
	static std::wstring DEV_PATH;
	static std::wstring PACK_DLL_PATH;

protected:
	void ReadSettingsRegistry(StreamType plugin_type);
	void SaveSectionRegistry(StreamType plugin_type);

	bool ReadSettingsJson(StreamType plugin_type);
	void UpdateSettingsJson(StreamType plugin_type);

private:
	std::map<StreamType, map_variant> m_settings;
	StreamType m_pluginType = StreamType::enEdem;
	nlohmann::json m_config;
	BOOL m_bPortable = FALSE;
};

inline PluginsConfig& GetConfig() { return PluginsConfig::Instance(); }

template<typename T>
static std::basic_string<T> GetPluginName(const StreamType plugin_type, bool bCamel = false)
{
	for (const auto& item : GetConfig().get_plugins_info())
	{
		if (item.type != plugin_type) continue;

		std::basic_string<T> plugin_name(item.short_name.begin(), item.short_name.end());
		if (bCamel)
			plugin_name[0] = std::toupper(plugin_name[0]);

		return plugin_name;
	}

	return std::basic_string<T>();
}
